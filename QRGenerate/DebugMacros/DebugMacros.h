// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Define Any Combination At The *Project Level* (compiler flags / project settings) Before Building. Or Manually Define:
//
//   _DBG_USE_DEBUGSTR          -> DbgView
//   _DBG_USE_FILE              -> File
//   _DBG_USE_CONSOLE           -> Console (Default)
//
// *In Release Mode*, None Of These Will Work Unless This Is Also Defined At The Project Level:
//
//   _DBG_FORCE
//
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
#pragma once
#ifndef DEBUG_MACROS_H
#define DEBUG_MACROS_H

#include <Windows.h>
#include <Strsafe.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//                                                  HELPERS
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#ifndef BUFFER_SIZE_2048
#define BUFFER_SIZE_2048        2048
#endif

#ifndef GET_FILENAMEA
#define GET_FILENAMEA(PATHA)   PathFindFileNameA(PATHA)
#endif

#ifndef GET_FILENAMEW
#define GET_FILENAMEW(PATHW)   PathFindFileNameW(PATHW)
#endif

#if !defined(_DBG_USE_DEBUGSTR) && !defined(_DBG_USE_FILE) && !defined(_DBG_USE_CONSOLE)
#define _DBG_USE_CONSOLE
#endif

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//                                          INTERNAL FUNCTION DECLARATIONS
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#ifdef __cplusplus
extern "C" {
#endif

    VOID DbgWrite(LPCSTR pszFile, INT nLine, LPCSTR pszFmt, ...);
    VOID DbgClose(VOID);

#ifdef __cplusplus
}
#endif

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//                                                DBG & DBG_CLOSE
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define DBG_CLOSE()         DbgClose()

#if defined(_DEBUG) || defined(_DBG_FORCE)
#define DBG(fmt, ...)       DbgWrite(GET_FILENAMEA(__FILE__), __LINE__, fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)       ((void)0)
#endif

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//                                          DBG_LAST_ERROR & DBG_HEX_ERROR
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#if defined(_DEBUG) || defined(_DBG_FORCE)

#define DBG_LAST_ERROR(APINAME)                                         \
        do {                                                            \
            DWORD _dwLastErr = GetLastError();                          \
            DBG("[!] %s Failed With Error: %lu", APINAME, _dwLastErr);  \
        } while (0)
#define DBG_HEX_ERROR(APINAME, HEXCODE)    DBG("[!] %s Failed With Error: 0x%0.8X", APINAME, HEXCODE)

#else

#define DBG_LAST_ERROR(APINAME)          ((void)0)
#define DBG_HEX_ERROR(APINAME, ERROR)    ((void)0)

#endif

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//                                                SOME AUTOMATION
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#if defined(_DEBUG) || defined(_DBG_FORCE)

#define DBG_HEAPALLOC_ERROR()            DBG_LAST_ERROR("HeapAlloc")
#define DBG_HEAPREALLOC_ERROR()          DBG_LAST_ERROR("HeapReAlloc")
#define DBG_LOCALALLOC_ERROR()           DBG_LAST_ERROR("LocalAlloc")
#define DBG_LOCALREALLOC_ERROR()         DBG_LAST_ERROR("LocalReAlloc")

#define DBG_STRINGCCHCOPY_ERROR(RSLT)    DBG_HEX_ERROR("StringCchCopy",     RSLT)
#define DBG_STRINGCCHCAT_ERROR(RSLT)     DBG_HEX_ERROR("StringCchCat",      RSLT)
#define DBG_STRINGCCHPRINTF_ERROR(RSLT)  DBG_HEX_ERROR("StringCchPrintf",   RSLT)

#define DBG_STRINGCBCOPY_ERROR(RSLT)     DBG_HEX_ERROR("StringCbCopy",      RSLT)
#define DBG_STRINGCBCAT_ERROR(RSLT)      DBG_HEX_ERROR("StringCbCat",       RSLT)
#define DBG_STRINGCBPRINTF_ERROR(RSLT)   DBG_HEX_ERROR("StringCbPrintf",    RSLT)

#else

#define DBG_HEAPALLOC_ERROR()            ((void)0)
#define DBG_HEAPREALLOC_ERROR()          ((void)0)
#define DBG_LOCALALLOC_ERROR()           ((void)0)
#define DBG_LOCALREALLOC_ERROR()         ((void)0)

#define DBG_STRINGCCHCOPY_ERROR(ERROR)   ((void)0)
#define DBG_STRINGCCHCAT_ERROR(ERROR)    ((void)0)
#define DBG_STRINGCCHPRINTF_ERROR(ERROR) ((void)0)

#define DBG_STRINGCBCOPY_ERROR(ERROR)    ((void)0)
#define DBG_STRINGCBCAT_ERROR(ERROR)     ((void)0)
#define DBG_STRINGCBPRINTF_ERROR(ERROR)  ((void)0)

#endif

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// OTHER MACROS
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#ifndef NT_SUCCESS
#define NT_SUCCESS(ntStatus)    (((NTSTATUS)(ntStatus)) >= 0x00)
#endif

#define HEAP_ALLOC(ptr, size)                                                           \
    do {                                                                                \
        (ptr) = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (size));                  \
        if (!(ptr)) DBG_HEAPALLOC_ERROR();                                              \
    } while (0)

#define HEAP_REALLOC(ptr, size)                                                         \
    do {                                                                                \
        LPVOID _pTmp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (ptr), (size));  \
        if (!_pTmp) { DBG_HEAPREALLOC_ERROR(); }                                        \
        else        { (ptr) = _pTmp;           }                                        \
    } while (0)

#define HEAP_FREE(ptr)                                                                  \
    do {                                                                                \
        if (ptr) {                                                                      \
            HeapFree(GetProcessHeap(), 0, (ptr));                                       \
            (ptr) = NULL;                                                               \
        }                                                                               \
    } while (0)

#define HEAP_SECURE_FREE(ptr, size)                                                     \
    do {                                                                                \
        if (ptr) {                                                                      \
            SecureZeroMemory((ptr), (size));                                            \
            HeapFree(GetProcessHeap(), 0, (ptr));                                       \
            (ptr) = NULL;                                                               \
        }                                                                               \
    } while (0)


#define CLOSE_HANDLE(handle)                                                            \
    do {                                                                                \
        if ((handle) && (handle) != INVALID_HANDLE_VALUE) {                             \
            CloseHandle((handle));                                                      \
            (handle) = NULL;                                                            \
        }                                                                               \
    } while (0)

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==


#endif // !DEBUG_MACROS_H