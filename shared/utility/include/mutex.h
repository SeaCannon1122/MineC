#pragma once

#ifndef MUTEX_H
#define MUTEX_H

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
typedef CRITICAL_SECTION mutex_t;
#define mutex_create(pMutex) InitializeCriticalSection(pMutex)
#define mutex_lock(pMutex) EnterCriticalSection(pMutex)
#define mutex_unlock(pMutex) LeaveCriticalSection(pMutex)
#define mutex_destroy(pMutex) DeleteCriticalSection(pMutex)
#else
#include <pthread.h>
typedef pthread_mutex_t mutex_t;
#define mutex_create(pMutex) pthread_mutex_init(pMutex, NULL)
#define mutex_lock(pMutex) pthread_mutex_lock(pMutex)
#define mutex_unlock(pMutex) pthread_mutex_unlock(pMutex)
#define mutex_destroy(pMutex) pthread_mutex_destroy(pMutex)
#endif
#endif // !MUTEX_H
