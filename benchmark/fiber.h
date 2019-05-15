#pragma once

//#define USERSPACE

#ifdef USERSPACE

#include "src/ult.h"

#define ConvertThreadToFiber() ult_convert()
#define CreateFiber(dwStackSize, lpStartAddress, lpParameter) ult_creat(dwStackSize, lpStartAddress, lpParameter)
#define SwitchToFiber(lpFiber) ult_switch_to(lpFiber)
#define FlsAlloc(lpCallback) fls_alloc()
#define FlsFree(dwFlsIndex)	fls_free(dwFlsIndex)
#define FlsGetValue(dwFlsIndex) fls_get(dwFlsIndex)
#define FlsSetValue(dwFlsIndex, lpFlsData) fls_set((dwFlsIndex), (long long)(lpFlsData))

#else


#include "../libs/userfibers.h"

#define ConvertThreadToFiber() convert_thread_to_fiber()
#define CreateFiber(dwStackSize, lpStartAddress, lpParameter) create_fiber(dwStackSize, (void*) lpStartAddress, (void*) lpParameter)
#define SwitchToFiber(lpFiber) switch_to_fiber((long) lpFiber)
#define FlsAlloc(lpCallback) _fls_alloc()
#define FlsFree(dwFlsIndex) _fls_free((long) dwFlsIndex)
#define FlsGetValue(dwFlsIndex) _fls_get_value((long) dwFlsIndex)
#define FlsSetValue(dwFlsIndex, lpFlsData) _fls_set_value((long) dwFlsIndex, (void*) lpFlsData)

#endif /* USERSPACE */
