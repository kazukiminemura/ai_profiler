#include "countersampler.h"

#include <QtCore/QDateTime>
#include <QtCore/QThread>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

namespace {
quint64 fileTimeToUInt64(const FILETIME &ft) {
    return (static_cast<quint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

int countThreads(quint32 pid) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);
    int count = 0;
    if (Thread32First(snapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) count++;
        } while (Thread32Next(snapshot, &te));
    }
    CloseHandle(snapshot);
    return count;
}
}

CounterSampler::CounterSampler(QObject *parent) : QObject(parent) {}

void CounterSampler::startSampling(quint32 pid) {
    m_stop.store(false);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process) {
        emit error(QStringLiteral("OpenProcess failed (pid=%1)").arg(pid));
        return;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    const int cpuCount = static_cast<int>(sysInfo.dwNumberOfProcessors);

    FILETIME prevCreate, prevExit, prevKernel, prevUser;
    if (!GetProcessTimes(process, &prevCreate, &prevExit, &prevKernel, &prevUser)) {
        CloseHandle(process);
        emit error(QStringLiteral("GetProcessTimes failed"));
        return;
    }

    LARGE_INTEGER freq;
    LARGE_INTEGER prevQpc;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prevQpc);

    while (!m_stop.loadAcquire()) {
        QThread::msleep(1000);

        DWORD exitCode = 0;
        if (!GetExitCodeProcess(process, &exitCode) || exitCode != STILL_ACTIVE) {
            emit targetExited();
            break;
        }

        FILETIME create, exit, kernel, user;
        if (!GetProcessTimes(process, &create, &exit, &kernel, &user)) {
            emit error(QStringLiteral("GetProcessTimes failed"));
            continue;
        }
        LARGE_INTEGER nowQpc;
        QueryPerformanceCounter(&nowQpc);

        const quint64 prevKernel64 = fileTimeToUInt64(prevKernel);
        const quint64 prevUser64 = fileTimeToUInt64(prevUser);
        const quint64 kernel64 = fileTimeToUInt64(kernel);
        const quint64 user64 = fileTimeToUInt64(user);
        const quint64 deltaProc100ns = (kernel64 - prevKernel64) + (user64 - prevUser64);

        const double deltaWallSec = double(nowQpc.QuadPart - prevQpc.QuadPart) / double(freq.QuadPart);
        double cpuPercent = 0.0;
        if (deltaWallSec > 0.0) {
            const double procSec = double(deltaProc100ns) / 1.0e7;
            cpuPercent = (procSec / deltaWallSec) * 100.0 / double(cpuCount);
        }

        PROCESS_MEMORY_COUNTERS_EX pmc;
        ZeroMemory(&pmc, sizeof(pmc));
        double rssMB = 0.0;
        if (GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            rssMB = double(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        }

        CounterFrame frame;
        frame.epochMs = QDateTime::currentMSecsSinceEpoch();
        frame.cpuPercent = cpuPercent;
        frame.rssMB = rssMB;
        frame.threadCount = countThreads(pid);
        emit frameReady(frame);

        prevKernel = kernel;
        prevUser = user;
        prevQpc = nowQpc;
    }

    CloseHandle(process);
}

void CounterSampler::stopSampling() {
    m_stop.store(true);
}
