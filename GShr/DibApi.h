// dibapi.h

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef _INC_DIBAPI
#define _INC_DIBAPI

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

#define WIDTHBYTES(i)     ((unsigned)((i + 31) & (~31)) / 8)  /* ULONG aligned ! */
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

/* Function prototypes */
BOOL    SaveDIB(HDIB hDib, CFile& file);
HDIB    ReadDIBFile(CFile& file);
HDIB    CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBitCount);
BOOL    CreateDIBPalette(HDIB hDIB, CPalette* pPal);
LPSTR   FindDIBBits(LPSTR lpbi);
DWORD   DIBWidth(LPSTR lpDIB);
DWORD   DIBHeight(LPSTR lpDIB);
WORD    PaletteSize(LPSTR lpbi);
WORD    DIBNumColors(LPSTR lpbi);
HANDLE  CopyHandle(HANDLE h);
HBITMAP DIBToBitmap(HANDLE hDIB, HPALETTE hPal);
HANDLE  BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal, uint16_t nBPP = uint16_t(0));
void    InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr,
            DWORD dwWidth, DWORD dwHeight, uint16_t nBPP);
void    InitColorTableMasksIfReqd(LPBITMAPINFO lpBmInfo);
LPBYTE  DibXY(LPSTR lpbi, int x, int y);

HANDLE  ConvertDIBSectionToDIB(HBITMAP hDibSect);

#endif //!_INC_DIBAPI
