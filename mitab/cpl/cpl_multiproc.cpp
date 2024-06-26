/**********************************************************************
 * $Id: cpl_multiproc.cpp 10646 2007-01-18 02:38:10Z warmerdam $
 *
 * Project:  CPL - Common Portability Library
 * Purpose:  CPL Multi-Threading, and process handling portability functions.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 **********************************************************************
 * Copyright (c) 2002, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "cpl_multiproc.h"

#include "cpl_conv.h"

#if !defined(WIN32CE)
#include <ctime>
#else
#include <wce_time.h>
#endif

CPL_CVSID("$Id: cpl_multiproc.cpp 10646 2007-01-18 02:38:10Z warmerdam $");

#if defined(CPL_MULTIPROC_STUB) && !defined(DEBUG)
#define MUTEX_NONE
#endif

/************************************************************************/
/*                           CPLMutexHolder()                           */
/************************************************************************/

CPLMutexHolder::CPLMutexHolder(void **phMutex, double dfWaitInSeconds, const char *pszFileIn,
                               int nLineIn)

{
#ifndef MUTEX_NONE
    pszFile = pszFileIn;
    nLine = nLineIn;

#ifdef DEBUG_MUTEX
    CPLDebug("MH", "Request %p for pid %d at %d/%s", *phMutex, CPLGetPID(), nLine, pszFile);
#endif

    if (!CPLCreateOrAcquireMutex(phMutex, dfWaitInSeconds)) {
        CPLDebug("CPLMutexHolder", "failed to acquire mutex!");
        hMutex = nullptr;
    } else {
#ifdef DEBUG_MUTEX
        CPLDebug("MH", "Acquired %p for pid %d at %d/%s", *phMutex, CPLGetPID(), nLine, pszFile);
#endif

        hMutex = *phMutex;
    }
#endif /* ndef MUTEX_NONE */
}

/************************************************************************/
/*                          ~CPLMutexHolder()                           */
/************************************************************************/

CPLMutexHolder::~CPLMutexHolder()

{
#ifndef MUTEX_NONE
    if (hMutex != nullptr) {
#ifdef DEBUG_MUTEX
        CPLDebug("MH", "Release %p for pid %d at %d/%s", hMutex, CPLGetPID(), nLine, pszFile);
#endif
        CPLReleaseMutex(hMutex);
    }
#endif /* ndef MUTEX_NONE */
}

/************************************************************************/
/*                      CPLCreateOrAcquireMutex()                       */
/************************************************************************/

int CPLCreateOrAcquireMutex(void **phMutex, double dfWaitInSeconds)

{
#ifndef MUTEX_NONE
    static void *hCOAMutex = nullptr;

    /*
    ** ironically, creation of this initial mutex is not threadsafe
    ** even though we use it to ensure that creation of other mutexes
    ** is threadsafe.
    */
    if (hCOAMutex == nullptr) {
        hCOAMutex = CPLCreateMutex();
    } else {
        CPLAcquireMutex(hCOAMutex, dfWaitInSeconds);
    }

    if (*phMutex == nullptr) {
        *phMutex = CPLCreateMutex();
        CPLReleaseMutex(hCOAMutex);
        return TRUE;
    } else {
        CPLReleaseMutex(hCOAMutex);

        int bSuccess = CPLAcquireMutex(*phMutex, dfWaitInSeconds);

        return bSuccess;
    }
#endif /* ndef MUTEX_NONE */

    return TRUE;
}

/************************************************************************/
/*                        CPLCleanupTLSList()                           */
/*                                                                      */
/*      Free resources associated with a TLS vector (implementation     */
/*      independent).                                                   */
/************************************************************************/

static void CPLCleanupTLSList(void **papTLSList)

{
    int i;

    //    printf( "CPLCleanupTLSList(%p)\n", papTLSList );

    if (papTLSList == nullptr)
        return;

    for (i = 0; i < CTLS_MAX; i++) {
        if (papTLSList[i] != nullptr && papTLSList[i + CTLS_MAX] != nullptr) {
            CPLFree(papTLSList[i]);
        }
    }

    CPLFree(papTLSList);
}

#ifdef CPL_MULTIPROC_STUB
/************************************************************************/
/* ==================================================================== */
/*                        CPL_MULTIPROC_STUB                            */
/*                                                                      */
/*      Stub implementation.  Mutexes don't provide exclusion, file     */
/*      locking is achieved with extra "lock files", and thread         */
/*      creation doesn't work.  The PID is always just one.             */
/* ==================================================================== */
/************************************************************************/

/************************************************************************/
/*                        CPLGetThreadingModel()                        */
/************************************************************************/

const char *CPLGetThreadingModel()

{
    return "stub";
}

/************************************************************************/
/*                           CPLCreateMutex()                           */
/************************************************************************/

void *CPLCreateMutex()

{
#ifndef MUTEX_NONE
    unsigned char *pabyMutex = (unsigned char *)CPLMalloc(4);

    pabyMutex[0] = 1;
    pabyMutex[1] = 'r';
    pabyMutex[2] = 'e';
    pabyMutex[3] = 'd';

    return (void *)pabyMutex;
#else
    return (void *)0xdeadbeef;
#endif
}

/************************************************************************/
/*                          CPLAcquireMutex()                           */
/************************************************************************/

int CPLAcquireMutex(void *hMutex, double dfWaitInSeconds)

{
#ifndef MUTEX_NONE
    unsigned char *pabyMutex = (unsigned char *)hMutex;

    CPLAssert(pabyMutex[1] == 'r' && pabyMutex[2] == 'e' && pabyMutex[3] == 'd');

    pabyMutex[0] += 1;

    return TRUE;
#else
    return TRUE;
#endif
}

/************************************************************************/
/*                          CPLReleaseMutex()                           */
/************************************************************************/

void CPLReleaseMutex(void *hMutex)

{
#ifndef MUTEX_NONE
    unsigned char *pabyMutex = (unsigned char *)hMutex;

    CPLAssert(pabyMutex[1] == 'r' && pabyMutex[2] == 'e' && pabyMutex[3] == 'd');

    if (pabyMutex[0] < 1)
        CPLDebug("CPLMultiProc", "CPLReleaseMutex() called on mutex with %d as ref count!",
                 pabyMutex[0]);

    pabyMutex[0] -= 1;
#endif
}

/************************************************************************/
/*                          CPLDestroyMutex()                           */
/************************************************************************/

void CPLDestroyMutex(void *hMutex)

{
#ifndef MUTEX_NONE
    unsigned char *pabyMutex = (unsigned char *)hMutex;

    CPLAssert(pabyMutex[1] == 'r' && pabyMutex[2] == 'e' && pabyMutex[3] == 'd');

    CPLFree(pabyMutex);
#endif
}

/************************************************************************/
/*                            CPLLockFile()                             */
/*                                                                      */
/*      Lock a file.  This implementation has a terrible race           */
/*      condition.  If we don't succeed in opening the lock file, we    */
/*      assume we can create one and own the target file, but other     */
/*      processes might easily try creating the target file at the      */
/*      same time, overlapping us.  Death!  Mayhem!  The traditional    */
/*      solution is to use open() with _O_CREAT|_O_EXCL but this        */
/*      function and these arguments aren't trivially portable.         */
/*      Also, this still leaves a race condition on NFS drivers         */
/*      (apparently).                                                   */
/************************************************************************/

void *CPLLockFile(const char *pszPath, double dfWaitInSeconds)

{
    FILE *fpLock;
    char *pszLockFilename;

    /* -------------------------------------------------------------------- */
    /*      We use a lock file with a name derived from the file we want    */
    /*      to lock to represent the file being locked.  Note that for      */
    /*      the stub implementation the target file does not even need      */
    /*      to exist to be locked.                                          */
    /* -------------------------------------------------------------------- */
    pszLockFilename = (char *)CPLMalloc(strlen(pszPath) + 30);
    sprintf(pszLockFilename, "%s.lock", pszPath);

    fpLock = fopen(pszLockFilename, "r");
    while (fpLock != NULL && dfWaitInSeconds > 0.0) {
        fclose(fpLock);
        CPLSleep(MIN(dfWaitInSeconds, 0.5));
        dfWaitInSeconds -= 0.5;

        fpLock = fopen(pszLockFilename, "r");
    }

    if (fpLock != NULL) {
        fclose(fpLock);
        CPLFree(pszLockFilename);
        return NULL;
    }

    fpLock = fopen(pszLockFilename, "w");

    if (fpLock == NULL) {
        CPLFree(pszLockFilename);
        return NULL;
    }

    fwrite("held\n", 1, 5, fpLock);
    fclose(fpLock);

    return pszLockFilename;
}

/************************************************************************/
/*                           CPLUnlockFile()                            */
/************************************************************************/

void CPLUnlockFile(void *hLock)

{
    char *pszLockFilename = (char *)hLock;

    if (hLock == NULL)
        return;

    VSIUnlink(pszLockFilename);

    CPLFree(pszLockFilename);
}

/************************************************************************/
/*                             CPLGetPID()                              */
/************************************************************************/

int CPLGetPID()

{
    return 1;
}

/************************************************************************/
/*                          CPLCreateThread();                          */
/************************************************************************/

int CPLCreateThread(CPLThreadFunc pfnMain, void *pArg)

{
    return -1;
}

/************************************************************************/
/*                              CPLSleep()                              */
/************************************************************************/

void CPLSleep(double dfWaitInSeconds)

{
    time_t ltime;
    time_t ttime;

    time(&ltime);
    ttime = ltime + (int)(dfWaitInSeconds + 0.5);

    for (; ltime < ttime; time(&ltime)) {
        /* currently we just busy wait.  Perhaps we could at least block on
           io? */
    }
}

/************************************************************************/
/*                           CPLGetTLSList()                            */
/************************************************************************/

static void **papTLSList = NULL;

static void **CPLGetTLSList()

{
    if (papTLSList == NULL)
        papTLSList = (void **)CPLCalloc(sizeof(void *), CTLS_MAX * 2);

    return papTLSList;
}

/************************************************************************/
/*                           CPLCleanupTLS()                            */
/************************************************************************/

void CPLCleanupTLS()

{
    CPLCleanupTLSList(papTLSList);
    papTLSList = NULL;
}

#endif /* def CPL_MULTIPROC_STUB */

#if defined(CPL_MULTIPROC_WIN32)

/************************************************************************/
/* ==================================================================== */
/*                        CPL_MULTIPROC_WIN32                           */
/*                                                                      */
/*    WIN32 Implementation of multiprocessing functions.                */
/* ==================================================================== */
/************************************************************************/

#include <windows.h>

/* windows.h header must be included above following lines. */
#if defined(WIN32CE)
#include "cpl_win32ce_api.h"
#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)
#endif

/************************************************************************/
/*                        CPLGetThreadingModel()                        */
/************************************************************************/

const char *CPLGetThreadingModel()

{
    return "win32";
}

/************************************************************************/
/*                           CPLCreateMutex()                           */
/************************************************************************/

void *CPLCreateMutex()

{
    HANDLE hMutex;

    hMutex = CreateMutex(nullptr, TRUE, nullptr);

    return (void *)hMutex;
}

/************************************************************************/
/*                          CPLAcquireMutex()                           */
/************************************************************************/

int CPLAcquireMutex(void *hMutexIn, double dfWaitInSeconds)

{
    auto hMutex = (HANDLE)hMutexIn;
    DWORD hr;

    hr = WaitForSingleObject(hMutex, (int)(dfWaitInSeconds * 1000));

    return hr != WAIT_TIMEOUT;
}

/************************************************************************/
/*                          CPLReleaseMutex()                           */
/************************************************************************/

void CPLReleaseMutex(void *hMutexIn)

{
    auto hMutex = (HANDLE)hMutexIn;

    ReleaseMutex(hMutex);
}

/************************************************************************/
/*                          CPLDestroyMutex()                           */
/************************************************************************/

void CPLDestroyMutex(void *hMutexIn)

{
    auto hMutex = (HANDLE)hMutexIn;

    CloseHandle(hMutex);
}

/************************************************************************/
/*                            CPLLockFile()                             */
/************************************************************************/

void *CPLLockFile(const char *pszPath, double dfWaitInSeconds)

{
    char *pszLockFilename;
    HANDLE hLockFile;

    pszLockFilename = (char *)CPLMalloc(strlen(pszPath) + 30);
    sprintf(pszLockFilename, "%s.lock", pszPath);

    hLockFile = CreateFile(pszLockFilename, GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, nullptr);

    while (GetLastError() == ERROR_ALREADY_EXISTS && dfWaitInSeconds > 0.0) {
        CloseHandle(hLockFile);
        CPLSleep(MIN(dfWaitInSeconds, 0.125));
        dfWaitInSeconds -= 0.125;

        hLockFile = CreateFile(pszLockFilename, GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    }

    CPLFree(pszLockFilename);

    if (hLockFile == INVALID_HANDLE_VALUE)
        return nullptr;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hLockFile);
        return nullptr;
    }

    return (void *)hLockFile;
}

/************************************************************************/
/*                           CPLUnlockFile()                            */
/************************************************************************/

void CPLUnlockFile(void *hLock)

{
    auto hLockFile = (HANDLE)hLock;

    CloseHandle(hLockFile);
}

/************************************************************************/
/*                             CPLGetPID()                              */
/************************************************************************/

int CPLGetPID()

{
    return GetCurrentThreadId();
}

/************************************************************************/
/*                       CPLStdCallThreadJacket()                       */
/************************************************************************/

using CPLStdCallThreadInfo = struct {
    void *pAppData;
    CPLThreadFunc pfnMain;
};

static DWORD WINAPI CPLStdCallThreadJacket(void *pData)

{
    auto *psInfo = (CPLStdCallThreadInfo *)pData;

    psInfo->pfnMain(psInfo->pAppData);

    CPLFree(psInfo);

    return 0;
}

/************************************************************************/
/*                          CPLCreateThread()                           */
/*                                                                      */
/*      The WIN32 CreateThread() call requires an entry point that      */
/*      has __stdcall conventions, so we provide a jacket function      */
/*      to supply that.                                                 */
/************************************************************************/

int CPLCreateThread(CPLThreadFunc pfnMain, void *pThreadArg)

{
    HANDLE hThread;
    DWORD nThreadId;
    CPLStdCallThreadInfo *psInfo;

    psInfo = (CPLStdCallThreadInfo *)CPLCalloc(sizeof(CPLStdCallThreadInfo), 1);
    psInfo->pAppData = pThreadArg;
    psInfo->pfnMain = pfnMain;

    hThread = CreateThread(nullptr, 0, CPLStdCallThreadJacket, psInfo, 0, &nThreadId);

    if (hThread == nullptr)
        return -1;

    CloseHandle(hThread);

    return nThreadId;
}

/************************************************************************/
/*                              CPLSleep()                              */
/************************************************************************/

void CPLSleep(double dfWaitInSeconds)

{
    Sleep((DWORD)(dfWaitInSeconds * 1000.0));
}

static int bTLSKeySetup = FALSE;
static DWORD nTLSKey;

/************************************************************************/
/*                           CPLGetTLSList()                            */
/************************************************************************/

static void **CPLGetTLSList()

{
    void **papTLSList;

    if (!bTLSKeySetup) {
        nTLSKey = TlsAlloc();
        if (nTLSKey == TLS_OUT_OF_INDEXES) {
            CPLError(CE_Fatal, CPLE_AppDefined, "TlsAlloc() failed!");
        }
        bTLSKeySetup = TRUE;
    }

    papTLSList = (void **)TlsGetValue(nTLSKey);
    if (papTLSList == nullptr) {
        papTLSList = (void **)CPLCalloc(sizeof(void *), CTLS_MAX * 2);
        if (TlsSetValue(nTLSKey, papTLSList) == 0) {
            CPLError(CE_Fatal, CPLE_AppDefined, "TlsSetValue() failed!");
        }
    }

    return papTLSList;
}

/************************************************************************/
/*                           CPLCleanupTLS()                            */
/************************************************************************/

void CPLCleanupTLS()

{
    void **papTLSList;

    if (!bTLSKeySetup)
        return;

    papTLSList = (void **)TlsGetValue(nTLSKey);
    if (papTLSList == nullptr)
        return;

    TlsSetValue(nTLSKey, nullptr);

    CPLCleanupTLSList(papTLSList);
}

#endif /* def CPL_MULTIPROC_WIN32 */

#ifdef CPL_MULTIPROC_PTHREAD
#include <pthread.h>
#include <time.h>

/************************************************************************/
/* ==================================================================== */
/*                        CPL_MULTIPROC_PTHREAD                         */
/*                                                                      */
/*    PTHREAD Implementation of multiprocessing functions.              */
/* ==================================================================== */
/************************************************************************/

/************************************************************************/
/*                        CPLGetThreadingModel()                        */
/************************************************************************/

const char *CPLGetThreadingModel()

{
    return "pthread";
}

/************************************************************************/
/*                           CPLCreateMutex()                           */
/************************************************************************/

void *CPLCreateMutex()

{
    pthread_mutex_t *hMutex;

    hMutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

#if defined(PTHREAD_MUTEX_RECURSIVE)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(hMutex, &attr);
    }
#elif defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
    pthread_mutex_t tmp_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    *hMutex = tmp_mutex;
#else
#error "Recursive mutexes apparently unsupported, configure --without-threads"
#endif

    // mutexes are implicitly acquired when created.
    CPLAcquireMutex(hMutex, 0.0);

    return (void *)hMutex;
}

/************************************************************************/
/*                          CPLAcquireMutex()                           */
/************************************************************************/

int CPLAcquireMutex(void *hMutexIn, double dfWaitInSeconds)

{
    int err;

    /* we need to add timeout support */
    err = pthread_mutex_lock((pthread_mutex_t *)hMutexIn);

    if (err != 0) {
        if (err == EDEADLK)
            CPLDebug("CPLAcquireMutex", "Error = %d/EDEADLK", err);
        else
            CPLDebug("CPLAcquireMutex", "Error = %d", err);

        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/*                          CPLReleaseMutex()                           */
/************************************************************************/

void CPLReleaseMutex(void *hMutexIn)

{
    pthread_mutex_unlock((pthread_mutex_t *)hMutexIn);
}

/************************************************************************/
/*                          CPLDestroyMutex()                           */
/************************************************************************/

void CPLDestroyMutex(void *hMutexIn)

{
    pthread_mutex_destroy((pthread_mutex_t *)hMutexIn);
    CPLFree(hMutexIn);
}

/************************************************************************/
/*                            CPLLockFile()                             */
/************************************************************************/

void *CPLLockFile(const char *pszPath, double dfWaitInSeconds)

{
    CPLError(CE_Failure, CPLE_NotSupported, "PThreads CPLLockFile() not implemented yet.");

    return NULL;
}

/************************************************************************/
/*                           CPLUnlockFile()                            */
/************************************************************************/

void CPLUnlockFile(void *hLock)

{}

/************************************************************************/
/*                             CPLGetPID()                              */
/************************************************************************/

int CPLGetPID()

{
    return (int)pthread_self();
}

/************************************************************************/
/*                       CPLStdCallThreadJacket()                       */
/************************************************************************/

typedef struct {
    void *pAppData;
    CPLThreadFunc pfnMain;
    pthread_t hThread;
} CPLStdCallThreadInfo;

static void *CPLStdCallThreadJacket(void *pData)

{
    CPLStdCallThreadInfo *psInfo = (CPLStdCallThreadInfo *)pData;

    psInfo->pfnMain(psInfo->pAppData);

    CPLFree(psInfo);

    return NULL;
}

/************************************************************************/
/*                          CPLCreateThread()                           */
/*                                                                      */
/*      The WIN32 CreateThread() call requires an entry point that      */
/*      has __stdcall conventions, so we provide a jacket function      */
/*      to supply that.                                                 */
/************************************************************************/

int CPLCreateThread(CPLThreadFunc pfnMain, void *pThreadArg)

{
    CPLStdCallThreadInfo *psInfo;
    pthread_attr_t hThreadAttr;

    psInfo = (CPLStdCallThreadInfo *)CPLCalloc(sizeof(CPLStdCallThreadInfo), 1);
    psInfo->pAppData = pThreadArg;
    psInfo->pfnMain = pfnMain;

    pthread_attr_init(&hThreadAttr);
    pthread_attr_setdetachstate(&hThreadAttr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&(psInfo->hThread), &hThreadAttr, CPLStdCallThreadJacket, (void *)psInfo) !=
        0) {
        CPLFree(psInfo);
        return -1;
    }

    return 1; /* can we return the actual thread pid? */
}

/************************************************************************/
/*                              CPLSleep()                              */
/************************************************************************/

void CPLSleep(double dfWaitInSeconds)

{
    struct timespec sRequest, sRemain;

    sRequest.tv_sec = (int)floor(dfWaitInSeconds);
    sRequest.tv_nsec = (int)((dfWaitInSeconds - sRequest.tv_sec) * 1000000000);
    nanosleep(&sRequest, &sRemain);
}

static int bTLSKeySetup = FALSE;
static pthread_key_t oTLSKey;

/************************************************************************/
/*                           CPLCleanupTLS()                            */
/************************************************************************/

void CPLCleanupTLS()

{
    void **papTLSList;

    if (!bTLSKeySetup)
        return;

    papTLSList = (void **)pthread_getspecific(oTLSKey);
    if (papTLSList == NULL)
        return;

    pthread_setspecific(oTLSKey, NULL);

    CPLCleanupTLSList(papTLSList);
}

/************************************************************************/
/*                           CPLGetTLSList()                            */
/************************************************************************/

static void **CPLGetTLSList()

{
    void **papTLSList;

    if (!bTLSKeySetup) {
        if (pthread_key_create(&oTLSKey, (void (*)(void *))CPLCleanupTLSList) != 0) {
            CPLError(CE_Fatal, CPLE_AppDefined, "pthread_key_create() failed!");
        }
        bTLSKeySetup = TRUE;
    }

    papTLSList = (void **)pthread_getspecific(oTLSKey);
    if (papTLSList == NULL) {
        papTLSList = (void **)CPLCalloc(sizeof(void *), CTLS_MAX * 2);
        if (pthread_setspecific(oTLSKey, papTLSList) != 0) {
            CPLError(CE_Fatal, CPLE_AppDefined, "pthread_setspecific() failed!");
        }
    }

    return papTLSList;
}

#endif /* def CPL_MULTIPROC_PTHREAD */

/************************************************************************/
/*                             CPLGetTLS()                              */
/************************************************************************/

void *CPLGetTLS(int nIndex)

{
    void **papTLSList = CPLGetTLSList();

    CPLAssert(nIndex >= 0 && nIndex < CTLS_MAX);

    return papTLSList[nIndex];
}

/************************************************************************/
/*                             CPLSetTLS()                              */
/************************************************************************/

void CPLSetTLS(int nIndex, void *pData, int bFreeOnExit)

{
    void **papTLSList = CPLGetTLSList();

    CPLAssert(nIndex >= 0 && nIndex < CTLS_MAX);

    papTLSList[nIndex] = pData;
    papTLSList[CTLS_MAX + nIndex] = (void *)bFreeOnExit;
}
