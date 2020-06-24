// Board.cpp
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
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

#include    "stdafx.h"
#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GamDoc.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "Board.h"
#include    "GMisc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////
// CBoard Class

CBoard::CBoard()
{
    m_pBrdAry = NULL;
    m_pTopDwg = NULL;
    m_iMaxLayer = -1;
    // --------- //
    m_nSerialNum = 0;           // Needs to be set by creator
    m_bShowCellBorder = TRUE;
    m_bApplyVisibility = TRUE;
    // ------ //
    m_bCellBorderOnTop = FALSE;
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

CBoard::~CBoard()
{
    if (m_pBrdAry) delete m_pBrdAry;
    if (m_pTopDwg) delete m_pTopDwg;
}

// ----------------------------------------------------- //

void CBoard::Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale,
    int nCellBorder /* = -1 */, int nApplyVisible /* = -1 */)// -1 means use internal
{
    ASSERT(m_pBrdAry != NULL);

    CSize wsize, vsize;
    m_pBrdAry->GetBoardScaling(eScale, wsize, vsize);

    DrawBackground(pDC, pDrawRct);  // Moved here so don't need to scale rect
    if (m_pBaseDwg != NULL)
    {
        CRect rct(pDrawRct);
        if (eScale != fullScale)
        {
            pDC->SaveDC();
            pDC->SetMapMode(MM_ANISOTROPIC);
            pDC->SetWindowExt(wsize);
            pDC->SetViewportExt(vsize);
            ScaleRect(rct, wsize, vsize);
        }

        CBoardBase::Draw(pDC, &rct, eScale,  nApplyVisible);// Lower Layers

        if (eScale != fullScale)
            pDC->RestoreDC(-1);
    }

    CRect rCellRct;
    if (IsDrawGridLines(nCellBorder))
        m_pBrdAry->MapPixelsToCellBounds(pDrawRct, &rCellRct, eScale);

    if (m_iMaxLayer < 0 || m_iMaxLayer >= 2)
    {
        if (!IsDrawGridLines(nCellBorder))
        {
            ASSERT(m_pBrdAry != NULL);
            m_pBrdAry->MapPixelsToCellBounds(pDrawRct, &rCellRct, eScale);
        }
        DrawCells(pDC, &rCellRct, eScale);
    }
    if (!m_bCellBorderOnTop && IsDrawGridLines(nCellBorder))
        DrawCellLines(pDC, &rCellRct, eScale);

    if ((m_iMaxLayer < 0 || m_iMaxLayer >= 3) && m_pTopDwg != NULL)
    {
        CRect rct(pDrawRct);
        if (eScale != fullScale)
        {
            pDC->SaveDC();
            pDC->SetMapMode(MM_ANISOTROPIC);
            pDC->SetWindowExt(wsize);
            pDC->SetViewportExt(vsize);
            ScaleRect(rct, wsize, vsize);
        }

        DrawDrawingList(m_pTopDwg, pDC, &rct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible);

        if (eScale != fullScale)
            pDC->RestoreDC(-1);
    }

    if (m_bCellBorderOnTop && IsDrawGridLines(nCellBorder))
        DrawCellLines(pDC, &rCellRct, eScale);

    if ((m_iMaxLayer < 0 || m_iMaxLayer >= 3) && m_pTopDwg != NULL)
    {
        CRect rct(pDrawRct);
        if (eScale != fullScale)
        {
            pDC->SaveDC();
            pDC->SetMapMode(MM_ANISOTROPIC);
            pDC->SetWindowExt(wsize);
            pDC->SetViewportExt(vsize);
            ScaleRect(rct, wsize, vsize);
        }

        DrawDrawingList(m_pTopDwg, pDC, &rct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible,
            TRUE);

        if (eScale != fullScale)
            pDC->RestoreDC(-1);
    }
}

// ----------------------------------------------------- //

void CBoard::DrawCellLines(CDC* pDC, CRect* pCellRct, TileScale eScale)
{
    if (m_pBrdAry)
        m_pBrdAry->DrawCellLines(pDC, pCellRct, eScale);
}

// ----------------------------------------------------- //

void CBoard::DrawCells(CDC* pDC, CRect* pCellRct, TileScale eScale)
{
    if (m_pBrdAry)
        m_pBrdAry->DrawCells(pDC, pCellRct, eScale);
}

// ----------------------------------------------------- //

void CBoard::SetBoardArray(CBoardArray* pBrdAry)
{
    if (m_pBrdAry) delete m_pBrdAry;
    m_pBrdAry = pBrdAry;
}

// ----------------------------------------------------- //

CDrawList* CBoard::GetTopDrawing(BOOL bCreate)
{
    if (m_pTopDwg == NULL && bCreate)
        m_pTopDwg = new CDrawList;
    return m_pTopDwg;
}

// ----------------------------------------------------- //

void CBoard::SetTopDrawing(CDrawList* pDwg)
{
    if (m_pTopDwg) delete m_pTopDwg;
    m_pTopDwg = pDwg;
}

// ----------------------------------------------------- //
#ifndef GPLAY
BOOL CBoard::PurgeMissingTileIDs()
{
    BOOL bPurge = CBoardBase::PurgeMissingTileIDs();
    if (m_pBrdAry != NULL)
        bPurge |= m_pBrdAry->PurgeMissingTileIDs();
    if (m_pTopDwg != NULL)
        bPurge |= m_pTopDwg->PurgeMissingTileIDs(m_pTMgr);
    return bPurge;
}

// ----------------------------------------------------- //

BOOL CBoard::IsTileInUse(TileID tid)
{
    if (CBoardBase::IsTileInUse(tid))
        return TRUE;
    if (m_pBrdAry != NULL && m_pBrdAry->IsTileInUse(tid))
        return TRUE;
    if (m_pTopDwg != NULL && m_pTopDwg->IsTileInUse(tid))
        return TRUE;
    return FALSE;
}

// ----------------------------------------------------- //

void CBoard::ForceObjectsOntoBoard()
{
    ASSERT(m_pBrdAry != NULL);
    CSize sizeBrd = m_pBrdAry->GetSize(fullScale);
    CRect rctZone(CPoint(0,0), sizeBrd);
    if (m_pBaseDwg != NULL)
        m_pBaseDwg->ForceIntoZone(&rctZone);
    if (m_pTopDwg != NULL)
        m_pTopDwg->ForceIntoZone(&rctZone);
}
#endif  // !GPLAY
// ----------------------------------------------------- //

void CBoard::Serialize(CArchive& ar)
{
    CBoardBase::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_bShowCellBorder;
        ar << (WORD)m_bCellBorderOnTop;
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
        ar << (WORD)(m_pBrdAry != NULL ? 1 : 0);
        if (m_pBrdAry)
            m_pBrdAry->Serialize(ar);
        ar << (WORD)(m_pTopDwg != NULL ? 1 : 0);
        if (m_pTopDwg)
            m_pTopDwg->Serialize(ar);
    }
    else
    {
        if (m_pBrdAry)
        {
            delete m_pBrdAry;
            m_pBrdAry = NULL;
        }
        if (m_pTopDwg)
        {
            delete m_pTopDwg;
            m_pTopDwg = NULL;
        }
        WORD wTmp;
#ifndef GPLAY
        if (CGamDoc::GetLoadingVersion() > NumVersion(0, 54))
#endif
        {
            ar >> wTmp; m_bShowCellBorder = (BOOL)wTmp;
            ar >> wTmp; m_bCellBorderOnTop = (BOOL)wTmp;

            ar >> m_wReserved1;
        }
#ifndef GPLAY
        else
        {
            ar >> wTmp; m_bShowCellBorder = (BOOL)wTmp;
            // ar >> m_wReserved1;
            ar >> wTmp; m_bCellBorderOnTop = (BOOL)wTmp;
        }
#endif
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        ar >> wTmp;
        if (wTmp != 0)
        {
            m_pBrdAry = new CBoardArray;
            m_pBrdAry->Serialize(ar);
        }
        ar >> wTmp;
        if (wTmp != 0)
        {
            m_pTopDwg = new CDrawList;
            m_pTopDwg->Serialize(ar);
        }
    }
}

///////////////////////////////////////////////////////////////////
// CBaseBoard Class

CBoardBase::CBoardBase()
{
    m_bGridSnap = FALSE;
    m_xGridSnap = 4000;
    m_yGridSnap = 4000;
    m_xGridSnapOff = 0;
    m_yGridSnapOff = 0;
    // --------- //
    m_crBkGnd = RGB(128, 128, 128);
    m_pBaseDwg = NULL;
}

CBoardBase::~CBoardBase()
{
    if (m_pBaseDwg) delete m_pBaseDwg;
}

// ----------------------------------------------------- //

void CBoardBase::Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale,
    int nApplyVisible /* = -1 */)
{
    if (m_iMaxLayer < 0 || m_iMaxLayer >= 1)
        DrawDrawingList(m_pBaseDwg, pDC, pDrawRct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible);
}

// ----------------------------------------------------- //

void CBoardBase::DrawBackground(CDC* pDC, CRect* pDrawRct)
{
    COLORREF crPrv = pDC->SetBkColor(m_crBkGnd);
    pDC->ExtTextOut(0, 0, ETO_OPAQUE, pDrawRct, NULL, 0, NULL);
    pDC->SetBkColor(crPrv);
}

// ----------------------------------------------------- //

void CBoardBase::DrawDrawingList(CDrawList* pDwg, CDC* pDC, CRect* pDrawRct,
    TileScale eScale, BOOL bApplyVisible, BOOL bDrawPass2Objects)
{
    if (pDwg == NULL)
        return;
    pDwg->Draw(pDC, pDrawRct, eScale, bApplyVisible, bDrawPass2Objects);
}

// ----------------------------------------------------- //

CDrawList* CBoardBase::GetBaseDrawing(BOOL bCreate)
{
    if (m_pBaseDwg == NULL && bCreate)
        m_pBaseDwg = new CDrawList;
    return m_pBaseDwg;
}

// ----------------------------------------------------- //

void CBoardBase::SetBaseDrawing(CDrawList* pDwg)
{
    if (m_pBaseDwg) delete m_pBaseDwg;
    m_pBaseDwg = pDwg;
}

// ----------------------------------------------------- //
#ifndef GPLAY
BOOL CBoardBase::PurgeMissingTileIDs()
{
    if (m_pBaseDwg != NULL)
        return m_pBaseDwg->PurgeMissingTileIDs(m_pTMgr);
    return FALSE;
}

// ----------------------------------------------------- //

BOOL CBoardBase::IsTileInUse(TileID tid)
{
    if (m_pBaseDwg != NULL && m_pBaseDwg->IsTileInUse(tid))
        return TRUE;
    return FALSE;
}
#endif // !GPLAY
// ----------------------------------------------------- //
void CBoardBase::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)m_nSerialNum;
        ar << (WORD)m_bApplyVisibility;
        ar << (WORD)m_bGridSnap;
        ar << (DWORD)m_xGridSnap;
        ar << (DWORD)m_yGridSnap;
        ar << (DWORD)m_xGridSnapOff;
        ar << (DWORD)m_yGridSnapOff;

        ar << (WORD)m_iMaxLayer;
        ar << (DWORD)m_crBkGnd;
        ar << m_strBoardName;
        ar << (WORD)(m_pBaseDwg != NULL ? 1 : 0);
    }
    else
    {
        m_pTMgr = ((CGamDoc*)ar.m_pDocument)->GetTileManager();
        if (m_pBaseDwg)
        {
            delete m_pBaseDwg;
            m_pBaseDwg = NULL;
        }
        WORD wTmp;
        DWORD dwTmp;
        ar >> wTmp; m_nSerialNum = (int)wTmp;
#ifndef GPLAY
        if (CGamDoc::GetLoadingVersion() > NumVersion(0, 54))
#endif
        {
            // (all gbox's must be upgraded by designer prog)
            ar >> wTmp; m_bApplyVisibility = (BOOL)wTmp;
        }
        ar >> wTmp; m_bGridSnap = (BOOL)wTmp;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 58))
        {
            ar >> dwTmp; m_xGridSnap = (int)dwTmp;
            ar >> dwTmp; m_yGridSnap = (int)dwTmp;
            ar >> dwTmp; m_xGridSnapOff = (int)dwTmp;
            ar >> dwTmp; m_yGridSnapOff = (int)dwTmp;
        }
        else
        {
            ar >> wTmp; m_xGridSnap = (int)wTmp * 1000;
            ar >> wTmp; m_yGridSnap = (int)wTmp * 1000;
            ar >> wTmp; m_xGridSnapOff = (int)wTmp * 1000;
            ar >> wTmp; m_yGridSnapOff = (int)wTmp * 1000;
        }
        ar >> wTmp; m_iMaxLayer = (int)wTmp;
        ar >> dwTmp; m_crBkGnd = (COLORREF)dwTmp;
        ar >> m_strBoardName;
        ar >> wTmp;
        if (wTmp != 0)
            m_pBaseDwg = new CDrawList;
    }
    if (m_pBaseDwg != NULL)
        m_pBaseDwg->Serialize(ar);
}

///////////////////////////////////////////////////////////////////

CBoardManager::CBoardManager()
{
    m_nNextSerialNumber = 1;
    // ------ //
    SetForeColor(RGB(0, 0, 0));
    SetBackColor(RGB(255, 255, 255));
    SetLineWidth(3);
    m_fontID = CGamDoc::GetFontManager()->AddFont(TenthPointsToScreenPixels(100),
        taBold, FF_SWISS, "Arial");
    // ------ //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

void CBoardManager::DestroyAllElements(void)
{
    for (int i = 0; i < GetSize(); i++)
        delete (CBoard*)GetAt(i);
    RemoveAll();
    CGamDoc::GetFontManager()->DeleteFont(m_fontID);
    m_fontID = 0;
}

#ifndef GPLAY
BOOL CBoardManager::PurgeMissingTileIDs()
{
    BOOL bPurged = FALSE;
    for (int i = 0; i < GetNumBoards(); i++)
        bPurged |= GetBoard(i)->PurgeMissingTileIDs();
    return bPurged;
}

BOOL CBoardManager::IsTileInUse(TileID tid)
{
    for (int i = 0; i < GetNumBoards(); i++)
    {
        if (GetBoard(i)->IsTileInUse(tid))
            return TRUE;
    }
    return FALSE;
}
#endif // !GPLAY

// Returns -1 if board doesn't exist.
int CBoardManager::FindBoardBySerial(int nSerialNum)
{
    for (int i = 0; i < GetNumBoards(); i++)
    {
        TRACE2("Board %d has serial number %d\n", i, GetBoard(i)->GetSerialNumber());
        if (GetBoard(i)->GetSerialNumber() == nSerialNum)
            return i;
    }
    return -1;
}

BOOL CBoardManager::DoBoardFontDialog()
{
    FontID newFontID = DoFontDialog(m_fontID, GetApp()->m_pMainWnd, TRUE);
    if (newFontID != (FontID)0)
    {
        CGamDoc::GetFontManager()->DeleteFont(m_fontID);
        m_fontID = newFontID;
        return TRUE;
    }
    return FALSE;
}

void CBoardManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)m_nNextSerialNumber;

        ar << (DWORD)m_crFore;
        ar << (DWORD)m_crBack;
        ar << (WORD)m_nLineWidth;

        CFontTbl* pFontMgr = CGamDoc::GetFontManager();
        pFontMgr->Archive(ar, m_fontID);

        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        ar << (WORD)GetNumBoards();
        for (int i = 0; i < GetNumBoards(); i++)
            GetBoard(i)->Serialize(ar);
    }
    else
    {
        DestroyAllElements();
        WORD wTmp;
        ar >> wTmp; m_nNextSerialNumber = (int)wTmp;

        DWORD dwTmp;
        ar >> dwTmp; m_crFore = (COLORREF)dwTmp;
        ar >> dwTmp; m_crBack = (COLORREF)dwTmp;
        ar >> wTmp; m_nLineWidth = (UINT)wTmp;
        m_fontID = 0;

        CFontTbl* pFontMgr = CGamDoc::GetFontManager();
        pFontMgr->Archive(ar, m_fontID);

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        ar >> wTmp;
        for (int i = 0; i < (int)wTmp; i++)
        {
            CBoard *pBoard = new CBoard;
            pBoard->Serialize(ar);
            Add(pBoard);
        }
    }
}

