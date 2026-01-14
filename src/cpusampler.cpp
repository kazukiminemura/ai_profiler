#include "cpusampler.h"

#include <QtCore/QDateTime>
#include <QtCore/QThread>

#include <windows.h>
#include <tlhelp32.h>

namespace {
quint64 fileTimeToUInt64(const FILETIME &ft) {
    return (static_cast<quint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}
}

CpuSampler::CpuSampler(QObject *parent) : QObject(parent) {}

void CpuSampler::startSampling(quint32 pid) {
    m_stop.store(false);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        emit error(QStringLiteral("OpenProcess failed (pid=%1)").arg(pid));
        return;
    }

    QHash<quint32, quint64> lastThreadTime;

    while (!m_stop.loadAcquire()) {
        DWORD exitCode = 0;
        if (!GetExitCodeProcess(process, &exitCode) || exitCode != STILL_ACTIVE) {
            emit targetExited();
            break;
        }

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            THREADENTRY32 te;
            te.dwSize = sizeof(THREADENTRY32);
            if (Thread32First(snapshot, &te)) {
                do {
                    if (te.th32OwnerProcessID != pid) continue;
                    HANDLE thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, te.th32ThreadID);
                    if (!thread) continue;

                    FILETIME create, exit, kernel, user;
                    if (GetThreadTimes(thread, &create, &exit, &kernel, &user)) {
                        const quint64 total100ns = fileTimeToUInt64(kernel) + fileTimeToUInt64(user);
                        const quint64 prev = lastThreadTime.value(te.th32ThreadID, total100ns);
                        const quint64 delta = total100ns - prev;
                        lastThreadTime.insert(te.th32ThreadID, total100ns);
                        if (delta > 0) {
                            CpuSample sample;
                            sample.epochMs = QDateTime::currentMSecsSinceEpoch();
                            sample.threadId = te.th32ThreadID;
                            sample.cpuDelta100ns = delta;
                            sample.stackId = static_cast<quint64>(te.th32ThreadID); // placeholder stackId
                            emit sampleReady(sample);
                        }
                    }
                    CloseHandle(thread);
                } while (Thread32Next(snapshot, &te));
            }
            CloseHandle(snapshot);
        }

        QThread::msleep(10);
    }

    CloseHandle(process);
}

void CpuSampler::stopSampling() {
    m_stop.store(true);
}
