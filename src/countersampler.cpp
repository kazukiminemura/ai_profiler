#include "countersampler.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QVector>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <cwchar>

struct GpuQuery {
    PDH_HQUERY query = nullptr;
    QVector<PDH_HCOUNTER> counters;
    bool available = false;
    QString error;
};

namespace {

QString pdhStatusToString(PDH_STATUS status) {
    return QStringLiteral("0x%1").arg(QString::number(status, 16));
}

void closeGpuQuery(GpuQuery &gpu) {
    if (gpu.query) {
        PdhCloseQuery(gpu.query);
        gpu.query = nullptr;
    }
    gpu.counters.clear();
    gpu.available = false;
}

bool initGpuQuery(GpuQuery &gpu, quint32 pid) {
    PDH_STATUS st = PdhOpenQueryW(nullptr, 0, &gpu.query);
    if (st != ERROR_SUCCESS) {
        gpu.error = QStringLiteral("PdhOpenQuery failed (%1)").arg(pdhStatusToString(st));
        return false;
    }

    const QString pattern = QStringLiteral("\\\\GPU Engine(pid_%1_*_engtype_*)\\\\Utilization Percentage").arg(pid);
    DWORD needed = 0;
    st = PdhExpandWildCardPathW(nullptr, reinterpret_cast<LPCWSTR>(pattern.utf16()), nullptr, &needed, 0);
    if (st != PDH_MORE_DATA || needed == 0) {
        gpu.error = QStringLiteral("No GPU Engine counters for pid %1").arg(pid);
        closeGpuQuery(gpu);
        return false;
    }

    QVector<wchar_t> buffer;
    buffer.resize(static_cast<int>(needed));
    st = PdhExpandWildCardPathW(nullptr, reinterpret_cast<LPCWSTR>(pattern.utf16()), buffer.data(), &needed, 0);
    if (st != ERROR_SUCCESS) {
        gpu.error = QStringLiteral("PdhExpandWildCardPath failed (%1)").arg(pdhStatusToString(st));
        closeGpuQuery(gpu);
        return false;
    }

    for (wchar_t *p = buffer.data(); *p; p += std::wcslen(p) + 1) {
        PDH_HCOUNTER counter = nullptr;
        st = PdhAddEnglishCounterW(gpu.query, p, 0, &counter);
        if (st == ERROR_SUCCESS && counter) {
            gpu.counters.push_back(counter);
        }
    }

    if (gpu.counters.isEmpty()) {
        gpu.error = QStringLiteral("GPU counters unavailable for pid %1").arg(pid);
        closeGpuQuery(gpu);
        return false;
    }

    PdhCollectQueryData(gpu.query);
    gpu.available = true;
    return true;
}

double readGpuPercent(GpuQuery &gpu) {
    if (!gpu.available || !gpu.query) return 0.0;

    const PDH_STATUS st = PdhCollectQueryData(gpu.query);
    if (st != ERROR_SUCCESS) return 0.0;

    double total = 0.0;
    for (const auto &counter : gpu.counters) {
        PDH_FMT_COUNTERVALUE val;
        DWORD type = 0;
        if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, &type, &val) == ERROR_SUCCESS &&
            val.CStatus == ERROR_SUCCESS) {
            total += val.doubleValue;
        }
    }

    if (total < 0.0) total = 0.0;
    if (total > 100.0) total = 100.0;
    return total;
}

quint64 fileTimeToUInt64(const FILETIME &ft) {
    return (static_cast<quint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

QString win32ErrorMessage(DWORD error) {
    wchar_t *buffer = nullptr;
    const DWORD size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        nullptr);
    QString message;
    if (size && buffer) {
        message = QString::fromWCharArray(buffer).trimmed();
        LocalFree(buffer);
    }
    if (message.isEmpty()) {
        message = QStringLiteral("Unknown error");
    }
    return message;
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

CounterSampler::~CounterSampler() = default;

void CounterSampler::startSampling(quint32 pid) {
    m_stop.storeRelease(false);
    m_gpu = std::make_unique<GpuQuery>();
    bool canReadMemory = true;
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process) {
        process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        canReadMemory = false;
    }
    if (!process) {
        const DWORD err = GetLastError();
        emit error(QStringLiteral("OpenProcess failed (pid=%1, error=%2: %3). Try running as Administrator or target a process you own.")
                       .arg(pid)
                       .arg(err)
                       .arg(win32ErrorMessage(err)));
        return;
    }

    if (!initGpuQuery(*m_gpu, pid)) {
        if (!m_gpu->error.isEmpty()) {
            emit error(QStringLiteral("GPU counters unavailable: %1").arg(m_gpu->error));
        }
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

        double rssMB = 0.0;
        if (canReadMemory) {
            PROCESS_MEMORY_COUNTERS_EX pmc;
            ZeroMemory(&pmc, sizeof(pmc));
            if (GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
                rssMB = double(pmc.WorkingSetSize) / (1024.0 * 1024.0);
            }
        }

        CounterFrame frame;
        frame.epochMs = QDateTime::currentMSecsSinceEpoch();
        frame.cpuPercent = cpuPercent;
        frame.rssMB = rssMB;
        frame.gpuPercent = m_gpu ? readGpuPercent(*m_gpu) : 0.0;
        frame.threadCount = countThreads(pid);
        emit frameReady(frame);

        prevKernel = kernel;
        prevUser = user;
        prevQpc = nowQpc;
    }

    CloseHandle(process);
    if (m_gpu) {
        closeGpuQuery(*m_gpu);
        m_gpu.reset();
    }
}

void CounterSampler::stopSampling() {
    m_stop.storeRelease(true);
}
