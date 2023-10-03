//  dibapi.cpp
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include    "stdafx.h"
#include    <io.h>
#include    <errno.h>
#include    "GdiTools.h"
#include    "DibApi.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

/*
 * Dib Header Marker - used in writing DIBs to files
 */

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

///////////////////////////////////////////////////////////////////////
