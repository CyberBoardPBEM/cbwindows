// FrmDockTile.cpp - container window for the tile palette.
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "stdafx.h"
#include "Gm.h"
#include "FrmMain.h"
#include "FrmDockTile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

CDockTilePalette::CDockTilePalette() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
    m_pChildWnd = NULL;
}

CDockTilePalette::~CDockTilePalette()
{
}

/////////////////////////////////////////////////////////////////////////////

void CDockTilePalette::SetChild(CTilePalette* pChildWnd)
{
    if (m_pChildWnd == pChildWnd)
        return;

    if (m_pChildWnd != NULL)
    {
        m_pChildWnd->Hide();
        m_pChildWnd->SetDockingFrame(NULL);
        Invalidate(TRUE);
    }
    // We need to set this field explicitly rather than
    // using CDockablePane::SetChild() since this function
    // insists that the window be non-NULL even though it's
    // perfectly fine to be NULL!
    m_pChildWnd = pChildWnd;
    if (pChildWnd != NULL)
    {
        pChildWnd->SetDockingFrame(this);
        m_pChildWnd->Show();
    }
    else
        GetMainFrame()->ShowPane(this, FALSE, TRUE, FALSE);
}
