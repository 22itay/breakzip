/**
 * Copyright 2017-2018 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef HELPER_MULTIPROCESS_H
#define HELPER_MULTIPROCESS_H

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <aclapi.h>
#include <sddl.h>
#include <stdio.h>
#include <strsafe.h>
#include <tchar.h>
#include <windows.h>
#include <winternl.h>
#include <iostream>
#else
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#include <vector>

typedef struct sharedMemoryInfo_st {
    void *addr;
    size_t size;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    HANDLE shmHandle;
#else
    int shmFd;
#endif
} sharedMemoryInfo;

int sharedMemoryCreate(const char *name, size_t sz, sharedMemoryInfo *info);

int sharedMemoryOpen(const char *name, size_t sz, sharedMemoryInfo *info);

void sharedMemoryClose(sharedMemoryInfo *info);

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
typedef PROCESS_INFORMATION Process;
#else
typedef pid_t Process;
#endif

int spawnProcess(Process *process, const char *app, char *const *args);

int waitProcess(Process *process);

#define checkIpcErrors(ipcFuncResult)                              \
    if (ipcFuncResult == -1) {                                     \
        fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__); \
        exit(EXIT_FAILURE);                                        \
    }

#if defined(__linux__)
struct ipcHandle_st {
    int socket;
    char *socketName;
};
typedef int ShareableHandle;
#elif defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
struct ipcHandle_st {
    std::vector<HANDLE> hMailslot;  // 1 Handle in case of child and `num
                                    // children` Handles for parent.
};
typedef HANDLE ShareableHandle;
#endif

typedef struct ipcHandle_st ipcHandle;

int ipcCreateSocket(ipcHandle *&handle, const char *name,
                    const std::vector<Process> &processes);

int ipcOpenSocket(ipcHandle *&handle);

int ipcCloseSocket(ipcHandle *handle);

int ipcRecvShareableHandles(ipcHandle *handle,
                            std::vector<ShareableHandle> &shareableHandles);

int ipcSendShareableHandles(
    ipcHandle *handle, const std::vector<ShareableHandle> &shareableHandles,
    const std::vector<Process> &processes);

int ipcCloseShareableHandle(ShareableHandle shHandle);

#endif  // HELPER_MULTIPROCESS_H
