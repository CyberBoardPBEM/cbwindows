//////    Shortened form of WINDOWSX.H ...  //////
/*****************************************************************************\
*                                                                             *
* winext.h  -  Macro APIs, window message crackers, and control APIs         *
*                                                                             *
*               Version 3.10                                                  *
*                                                                             *
*               Copyright (c) 1992, Microsoft Corp.  All rights reserved.     *
*                                                                             *
\*****************************************************************************/

#ifndef _INC_WINDOWSX
#define _INC_WINDOWSX

#ifndef WIN32           //Win16 forms

#ifndef RC_INVOKED
#pragma pack(1)         /* Assume byte packing throughout */
#endif  /* RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/****** KERNEL Macro APIs ****************************************************/

#error needs to be checked
#define     GetInstanceModule(hInstance) \
                GetModuleHandle((LPCSTR)MAKELP(0, hInstance))

#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalLockPtr(lp)       \
                ((BOOL)SELECTOROF(GlobalLock(GlobalPtrHandle(lp))))
#define     GlobalUnlockPtr(lp)     \
                GlobalUnlock(GlobalPtrHandle(lp))

#define     GlobalAllocPtr(flags, cb)   \
                (GlobalLock(GlobalAlloc((flags), (cb))))
#define     GlobalReAllocPtr(lp, cbNew, flags)  \
                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define     GlobalFreePtr(lp)       \
                (GlobalUnlockPtr(lp), (BOOL)GlobalFree(GlobalPtrHandle(lp)))

#ifndef RC_INVOKED
#pragma pack()          /* Revert to default packing */
#endif  /* RC_INVOKED */

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

///////////////////////////////////////////////////////////////////////
#else                   // WIN32 Forms.........

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif /* __cplusplus */

/****** KERNEL Macro APIs ****************************************************/

#define     GetInstanceModule(hInstance) (HMODULE)(hInstance)

#define     GlobalPtrHandle(lp)         \
                ((HGLOBAL)GlobalHandle(lp))

#define     GlobalLockPtr(lp)                \
                ((BOOL)GlobalLock(GlobalPtrHandle(lp)))
#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define     GlobalAllocPtr(flags, cb)        \
                (GlobalLock(GlobalAlloc((flags), (cb))))
#define     GlobalReAllocPtr(lp, cbNew, flags)       \
                (GlobalUnlockPtr(lp), GlobalLock(GlobalReAlloc(GlobalPtrHandle(lp) , (cbNew), (flags))))
#define     GlobalFreePtr(lp)                \
                (GlobalUnlockPtr(lp), (bool)GlobalFree(GlobalPtrHandle(lp)))

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif       /* __cplusplus */

#endif  /* WIN32 */

#endif  /* !_INC_WINDOWSX */

