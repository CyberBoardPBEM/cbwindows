// Board.cpp
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

#include    "stdafx.h"

#include <functional>

#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GamDoc.h"
    #include    "GeoBoard.h"
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
    m_iMaxLayer = -1;
    // --------- //
    m_nSerialNum = BoardID(0);           // Needs to be set by creator
    m_bShowCellBorder = TRUE;
    m_bApplyVisibility = TRUE;
    // ------ //
    m_bCellBorderOnTop = FALSE;
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

// ----------------------------------------------------- //

void CBoard::Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale,
    int nCellBorder /* = -1 */, int nApplyVisible /* = -1 */)// -1 means use internal
{
    const CBoardArray& brdAry = GetBoardArray();

    CSize wsize, vsize;
    brdAry.GetBoardScaling(eScale, wsize, vsize);

    DrawBackground(pDC, pDrawRct);  // Moved here so don't need to scale rect
    if (m_pBaseDwg != NULL)
    {
        CRect rct(pDrawRct);
        if (eScale != fullScale)
        {
            pDC.SaveDC();
            pDC.SetMapMode(MM_ANISOTROPIC);
            pDC.SetWindowExt(wsize);
            pDC.SetViewportExt(vsize);
            ScaleRect(rct, wsize, vsize);
        }

        CBoardBase::Draw(pDC, rct, eScale,  nApplyVisible);// Lower Layers

        if (eScale != fullScale)
            pDC.RestoreDC(-1);
    }

    CRect rCellRct;
    if (IsDrawGridLines(nCellBorder))
        brdAry.MapPixelsToCellBounds(pDrawRct, rCellRct, eScale);

    if (m_iMaxLayer < 0 || m_iMaxLayer >= 2)
    {
        if (!IsDrawGridLines(nCellBorder))
        {
            brdAry.MapPixelsToCellBounds(pDrawRct, rCellRct, eScale);
        }
        DrawCells(pDC, rCellRct, eScale);
    }
    if (!m_bCellBorderOnTop && IsDrawGridLines(nCellBorder))
        DrawCellLines(pDC, rCellRct, eScale);

    if ((m_iMaxLayer < 0 || m_iMaxLayer >= 3) && m_pTopDwg != NULL)
    {
        CRect rct(pDrawRct);
        if (eScale != fullScale)
        {
            pDC.SaveDC();
            pDC.SetMapMode(MM_ANISOTROPIC);
            pDC.SetWindowExt(wsize);
            pDC.SetViewportExt(vsize);
            ScaleRect(rct, wsize, vsize);
        }

        DrawDrawingList(&*m_pTopDwg, pDC, rct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible);

        if (eScale != fullScale)
            pDC.RestoreDC(-1);
    }

    if (m_bCellBorderOnTop && IsDrawGridLines(nCellBorder))
        DrawCellLines(pDC, &rCellRct, eScale);

    if ((m_iMaxLayer < 0 || m_iMaxLayer >= 3) && m_pTopDwg != NULL)
    {
        CRect rct(pDrawRct);
        if (eScale != fullScale)
        {
            pDC.SaveDC();
            pDC.SetMapMode(MM_ANISOTROPIC);
            pDC.SetWindowExt(wsize);
            pDC.SetViewportExt(vsize);
            ScaleRect(rct, wsize, vsize);
        }

        DrawDrawingList(&*m_pTopDwg, pDC, &rct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible,
            TRUE);

        if (eScale != fullScale)
            pDC.RestoreDC(-1);
    }
}

void CBoard::Draw(wxDC& pDC, const wxRect& pDrawRct, TileScale eScale,
    int nCellBorder /* = -1 */, int nApplyVisible /* = -1 */)// -1 means use internal
{
    const CBoardArray& brdAry = GetBoardArray();

    wxSize wsize, vsize;
    brdAry.GetBoardScaling(eScale, wsize, vsize);

    DrawBackground(pDC, pDrawRct);  // Moved here so don't need to scale rect
    if (m_pBaseDwg != NULL)
    {
        wxRect rct(pDrawRct);
        CB::DCUserScaleChanger setScale;
        if (eScale != fullScale)
        {
            setScale = CB::DCUserScaleChanger(pDC, double(vsize.x)/wsize.x, double(vsize.y)/wsize.y);
            ScaleRect(rct, wsize, vsize);
        }

        CBoardBase::Draw(pDC, rct, eScale,  nApplyVisible);// Lower Layers
    }

    wxRect rCellRct;
    if (IsDrawGridLines(nCellBorder))
        brdAry.MapPixelsToCellBounds(pDrawRct, rCellRct, eScale);

    if (m_iMaxLayer < 0 || m_iMaxLayer >= 2)
    {
        if (!IsDrawGridLines(nCellBorder))
        {
            brdAry.MapPixelsToCellBounds(pDrawRct, rCellRct, eScale);
        }
        DrawCells(pDC, rCellRct, eScale);
    }
    if (!m_bCellBorderOnTop && IsDrawGridLines(nCellBorder))
        DrawCellLines(pDC, rCellRct, eScale);

    if ((m_iMaxLayer < 0 || m_iMaxLayer >= 3) && m_pTopDwg != NULL)
    {
        wxRect rct(pDrawRct);
        CB::DCUserScaleChanger setScale;
        if (eScale != fullScale)
        {
            setScale = CB::DCUserScaleChanger(pDC, double(vsize.x)/wsize.x, double(vsize.y)/wsize.y);
            ScaleRect(rct, wsize, vsize);
        }

        DrawDrawingList(&*m_pTopDwg, pDC, rct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible);
    }

    if (m_bCellBorderOnTop && IsDrawGridLines(nCellBorder))
        DrawCellLines(pDC, rCellRct, eScale);

    if ((m_iMaxLayer < 0 || m_iMaxLayer >= 3) && m_pTopDwg != NULL)
    {
        wxRect rct(pDrawRct);
        CB::DCUserScaleChanger setScale;
        if (eScale != fullScale)
        {
            setScale = CB::DCUserScaleChanger(pDC, double(vsize.x) / wsize.x, double(vsize.y) / wsize.y);
            ScaleRect(rct, wsize, vsize);
        }

        DrawDrawingList(&*m_pTopDwg, pDC, rct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible,
            TRUE);
    }
}

// ----------------------------------------------------- //

void CBoard::DrawCellLines(CDC& pDC, const CRect& pCellRct, TileScale eScale) const
{
    if (m_pBrdAry)
        m_pBrdAry->DrawCellLines(pDC, pCellRct, eScale);
}

void CBoard::DrawCellLines(wxDC& pDC, const wxRect& pCellRct, TileScale eScale) const
{
    if (m_pBrdAry)
        m_pBrdAry->DrawCellLines(pDC, pCellRct, eScale);
}

// ----------------------------------------------------- //

void CBoard::DrawCells(CDC& pDC, const CRect& pCellRct, TileScale eScale) const
{
    if (m_pBrdAry)
        m_pBrdAry->DrawCells(pDC, pCellRct, eScale);
}

void CBoard::DrawCells(wxDC& pDC, const wxRect& pCellRct, TileScale eScale) const
{
    if (m_pBrdAry)
        m_pBrdAry->DrawCells(pDC, pCellRct, eScale);
}

// ----------------------------------------------------- //

void CBoard::SetBoardArray(OwnerOrNullPtr<CBoardArray> pDwg)
{
    m_pBrdAry = std::move(pDwg);
}

// ----------------------------------------------------- //

CDrawList* CBoard::GetTopDrawing(BOOL bCreate)
{
    if (m_pTopDwg == NULL && bCreate)
        m_pTopDwg = new CDrawList;
    return &*m_pTopDwg;
}

// ----------------------------------------------------- //

void CBoard::SetTopDrawing(OwnerOrNullPtr<CDrawList> pDwg)
{
    m_pTopDwg = std::move(pDwg);
}

#if defined(GPLAY)
CellStagger CBoard::GetStagger(Corner c) const
{
    static const auto Odd = [](size_t x)
    {
        return bool(x & size_t(1));
    };

    /* note that a GEV-style board's "visual" stagger is the
        opposite of its declared stagger */
    const CBoardArray& ba = GetBoardArray();
    const CCellForm& cellform = ba.GetCellForm(fullScale);
    switch (cellform.GetCellType())
    {
        case cformRect:
            ASSERT(!"no stagger for rect boards");
            AfxThrowInvalidArgException();
        case cformHexFlat:
            switch (c)
            {
                case Corner::TL:
                    return !IsGEVStyle(Edge::Top) ?
                                cellform.GetCellStagger()
                            :
                                ~cellform.GetCellStagger();
                case Corner::BL:
                    return !IsGEVStyle(Edge::Bottom) ?
                                ~cellform.GetCellStagger()
                            :
                                cellform.GetCellStagger();
                case Corner::TR:
                case Corner::BR:
                    return Odd(ba.GetCols()) ?
                                GetStagger(c ^ Edge::MaskLR)
                            :
                                ~GetStagger(c ^ Edge::MaskLR);
                default:
                    AfxThrowInvalidArgException();
            }
        case cformHexPnt:
            switch (c)
            {
                case Corner::TL:
                    return !IsGEVStyle(Edge::Left) ?
                                cellform.GetCellStagger()
                            :
                                ~cellform.GetCellStagger();
                case Corner::TR:
                    return !IsGEVStyle(Edge::Right) ?
                                ~cellform.GetCellStagger()
                            :
                                cellform.GetCellStagger();
                case Corner::BL:
                case Corner::BR:
                    return Odd(ba.GetRows()) ?
                                GetStagger(c ^ Edge::MaskTB)
                            :
                                ~GetStagger(c ^ Edge::MaskTB);
                default:
                    AfxThrowInvalidArgException();
            }
        default:
            AfxThrowInvalidArgException();
    }
}

/* recognize the Peter Lomax Ogre/GEV board style where "sticking
    out" cells are empty, and rest of edge is filled
    (see https://github.com/CyberBoardPBEM/cbwindows/pull/78) */
bool CBoard::IsGEVStyle(Edge e) const
{
    /* if the board appears to be GEV style on BOTH sides,
        it's so weird that our code will probably not handle
        it correctly, so reject it until someone presents a
        use case worth analyzing */
    ASSERT(!(IsGEVStyleHelper(e) && IsGEVStyleHelper(~e)));
    return IsGEVStyleHelper(e) && !IsGEVStyleHelper(~e);
}

bool CBoard::IsGEVStyleHelper(Edge e) const
{
    static const auto Odd = [](size_t x)
    {
        return bool(x & size_t(1));
    };

    const CBoardArray& ba = GetBoardArray();
    const CCellForm& cellform = ba.GetCellForm(fullScale);
    switch (cellform.GetCellType())
    {
        case cformRect:
            return false;
        case cformHexFlat:
        case cformHexPnt:
        {
            bool gevIsOddEmpty;
            size_t fixedAxisOuter, fixedAxisInner;
            size_t iterEnd;
            std::function<const BoardCell& (size_t /* fixedAxis */, size_t /* iterAxis */)> GetCell;
            if (cellform.GetCellType() == cformHexFlat)
            {
                if (ba.GetRows() < size_t(2))
                {
                    return false;
                }
                switch (e)
                {
                    case Edge::Top:
                        switch (ba.GetCellForm(fullScale).GetCellStagger())
                        {
                            case CellStagger::Out:
                                gevIsOddEmpty = false;
                                break;
                            case CellStagger::In:
                                gevIsOddEmpty = true;
                                break;
                            default:
                                AfxThrowInvalidArgException();
                        }
                        fixedAxisOuter = size_t(0);
                        fixedAxisInner = size_t(1);
                        break;
                    case Edge::Bottom:
                        switch (ba.GetCellForm(fullScale).GetCellStagger())
                        {
                            case CellStagger::Out:
                                gevIsOddEmpty = true;
                                break;
                            case CellStagger::In:
                                gevIsOddEmpty = false;
                                break;
                            default:
                                AfxThrowInvalidArgException();
                        }
                        fixedAxisOuter = ba.GetRows() - size_t(1);
                        fixedAxisInner = ba.GetRows() - size_t(2);
                        break;
                    case Edge::Left:
                    case Edge::Right:
                        return false;
                    default:
                        AfxThrowInvalidArgException();
                }
                iterEnd = ba.GetCols();
                GetCell = [&ba](size_t fixedAxis, size_t iterAxis)
                {
                    return std::ref(ba.GetCell(fixedAxis, iterAxis));
                };
            }
            else
            {
                ASSERT(cellform.GetCellType() == cformHexPnt);
                if (ba.GetCols() < size_t(2))
                {
                    return false;
                }
                switch (e)
                {
                    case Edge::Left:
                        switch (ba.GetCellForm(fullScale).GetCellStagger())
                        {
                            case CellStagger::Out:
                                gevIsOddEmpty = false;
                                break;
                            case CellStagger::In:
                                gevIsOddEmpty = true;
                                break;
                            default:
                                AfxThrowInvalidArgException();
                        }
                        fixedAxisOuter = size_t(0);
                        fixedAxisInner = size_t(1);
                        break;
                    case Edge::Right:
                        switch (ba.GetCellForm(fullScale).GetCellStagger())
                        {
                            case CellStagger::Out:
                                gevIsOddEmpty = true;
                                break;
                            case CellStagger::In:
                                gevIsOddEmpty = false;
                                break;
                            default:
                                AfxThrowInvalidArgException();
                        }
                        fixedAxisOuter = ba.GetCols() - size_t(1);
                        fixedAxisInner = ba.GetCols() - size_t(2);
                        break;
                    case Edge::Top:
                    case Edge::Bottom:
                        return false;
                    default:
                        AfxThrowInvalidArgException();
                }
                iterEnd = ba.GetRows();
                GetCell = [&ba](size_t fixedAxis, size_t iterAxis)
                {
                    return std::ref(ba.GetCell(iterAxis, fixedAxis));
                };
            }
            for (size_t i = size_t(0) ; i < iterEnd ; ++i)
            {
                if (Odd(i) == gevIsOddEmpty)
                {
                    if (!(IsEmpty(GetCell(fixedAxisOuter, i)) &&
                            !IsEmpty(GetCell(fixedAxisInner, i))))
                    {
                        return false;
                    }
                }
                else
                {
                    if (IsEmpty(GetCell(fixedAxisOuter, i)))
                    {
                        return false;
                    }
                }
            }
            return true;
        }
        default:
            AfxThrowInvalidArgException();
    }
}

bool CBoard::IsEmpty(const BoardCell& cell) const
{
    return cell.IsEmpty() ||
        !cell.IsTileID() && cell.GetColor() == m_crBkGnd;
}
#endif

// ----------------------------------------------------- //
#ifndef GPLAY
BOOL CBoard::PurgeMissingTileIDs()
{
    BOOL bPurge = CBoardBase::PurgeMissingTileIDs();
    if (m_pBrdAry != NULL)
        bPurge |= m_pBrdAry->PurgeMissingTileIDs();
    if (m_pTopDwg != NULL)
        bPurge |= m_pTopDwg->PurgeMissingTileIDs(&*m_pTMgr);
    return bPurge;
}

// ----------------------------------------------------- //

BOOL CBoard::IsTileInUse(TileID tid) const
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
    CSize sizeBrd = GetBoardArray().GetSize(fullScale);
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
        m_pBrdAry = NULL;
        m_pTopDwg = NULL;
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
            CGamDoc& doc = CheckedDeref(CB::ToCGamDoc(ar.m_pDocument));
            m_pBrdAry = new CBoardArray(doc.GetTileManager());
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
    m_xGridSnap = 4000u;
    m_yGridSnap = 4000u;
    m_xGridSnapOff = 0u;
    m_yGridSnapOff = 0u;
    // --------- //
    m_crBkGnd = RGB(128, 128, 128);
}

// ----------------------------------------------------- //

void CBoardBase::Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale,
    int nApplyVisible /* = -1 */)
{
    if (m_iMaxLayer < 0 || m_iMaxLayer >= 1)
        DrawDrawingList(&*m_pBaseDwg, pDC, pDrawRct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible);
}

void CBoardBase::Draw(wxDC& pDC, const wxRect& pDrawRct, TileScale eScale,
    int nApplyVisible /*= -1*/)
{
    if (m_iMaxLayer < 0 || m_iMaxLayer >= 1)
        DrawDrawingList(&*m_pBaseDwg, pDC, pDrawRct, eScale,
            nApplyVisible == -1 ? m_bApplyVisibility : (BOOL)nApplyVisible);
}

// ----------------------------------------------------- //

void CBoardBase::DrawBackground(CDC& pDC, const CRect& pDrawRct) const
{
    COLORREF crPrv = pDC.SetBkColor(m_crBkGnd);
    pDC.ExtTextOut(0, 0, ETO_OPAQUE, pDrawRct, NULL, 0, NULL);
    pDC.SetBkColor(crPrv);
}

void CBoardBase::DrawBackground(wxDC& pDC, const wxRect& pDrawRct) const
{
    wxDCPenChanger setPen(pDC, *wxTRANSPARENT_PEN);
    wxDCBrushChanger setBrush(pDC, wxBrush(CB::Convert(m_crBkGnd)));
    pDC.DrawRectangle(pDrawRct);
}

// ----------------------------------------------------- //

void CBoardBase::DrawDrawingList(CDrawList* pDwg, CDC& pDC, const CRect& pDrawRct,
    TileScale eScale, BOOL bApplyVisible, BOOL bDrawPass2Objects)
{
    if (pDwg == NULL)
        return;
    pDwg->Draw(pDC, pDrawRct, eScale, bApplyVisible, bDrawPass2Objects);
}

void CBoardBase::DrawDrawingList(CDrawList* pDwg, wxDC& pDC, const wxRect& pDrawRct,
    TileScale eScale, BOOL bApplyVisible, BOOL bDrawPass2Objects /*= FALSE*/)
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
    return &*m_pBaseDwg;
}

// ----------------------------------------------------- //

void CBoardBase::SetBaseDrawing(OwnerOrNullPtr<CDrawList> pDwg)
{
    m_pBaseDwg = std::move(pDwg);
}

// ----------------------------------------------------- //
#ifndef GPLAY
BOOL CBoardBase::PurgeMissingTileIDs()
{
    if (m_pBaseDwg != NULL)
        return m_pBaseDwg->PurgeMissingTileIDs(&*m_pTMgr);
    return FALSE;
}

// ----------------------------------------------------- //

BOOL CBoardBase::IsTileInUse(TileID tid) const
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
        ar << m_nSerialNum;
        ar << static_cast<uint16_t>(m_bApplyVisibility);
        ar << static_cast<uint16_t>(m_bGridSnap);
        ar << m_xGridSnap;
        ar << m_yGridSnap;
        ar << m_xGridSnapOff;
        ar << m_yGridSnapOff;

        ar << value_preserving_cast<int16_t>(m_iMaxLayer);
        ar << static_cast<uint32_t>(m_crBkGnd);
        ar << m_strBoardName;
        ar << static_cast<uint16_t>(m_pBaseDwg != NULL ? 1 : 0);
    }
    else
    {
        m_pTMgr = &CheckedDeref(CB::ToCGamDoc(ar.m_pDocument)).GetTileManager();
        m_pBaseDwg = NULL;
        uint16_t wTmp;
        uint32_t dwTmp;
        ar >> m_nSerialNum;
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
            ar >> m_xGridSnap;
            ar >> m_yGridSnap;
            ar >> m_xGridSnapOff;
            ar >> m_yGridSnapOff;
        }
        else
        {
            ar >> wTmp; m_xGridSnap = static_cast<uint32_t>(wTmp) * 1000;
            ar >> wTmp; m_yGridSnap = static_cast<uint32_t>(wTmp) * 1000;
            ar >> wTmp; m_xGridSnapOff = static_cast<uint32_t>(wTmp) * 1000;
            ar >> wTmp; m_yGridSnapOff = static_cast<uint32_t>(wTmp) * 1000;
        }
        int16_t sTmp;
        ar >> sTmp; m_iMaxLayer = value_preserving_cast<decltype(m_iMaxLayer)>(sTmp);
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

#if defined(GPLAY)
class CBoardManager::GeoBoardManager
{
public:
    GeoBoardManager(CBoardManager& o, CGamDoc& doc);
    GeoBoardManager(const GeoBoardManager&) = delete;
    GeoBoardManager& operator=(const GeoBoardManager&) = delete;
    ~GeoBoardManager() = default;

    const CBoard& Get(const CGeoBoardElement& gelem) const;

private:
    RefPtr<CBoardManager> outer;
    CGamDoc& doc;

    class LessCGBE
    {
    public:
        bool operator()(const CGeoBoardElement& lhs, const CGeoBoardElement& rhs) const;
    };
    typedef std::map<CGeoBoardElement, OwnerPtr<CBoard>, LessCGBE> BoardRots;
    // allow lazy eval of ctor
    mutable BoardRots m_boardRots;
};
#endif

CBoardManager::CBoardManager(CGamDoc& doc)
#if defined(GPLAY)
    :
    geoBoardManager(new GeoBoardManager(*this, doc))
#endif
{
    m_nNextSerialNumber = BoardID(1);
    // ------ //
    SetForeColor(RGB(0, 0, 0));
    SetBackColor(RGB(255, 255, 255));
    SetLineWidth(3);
    m_fontID = CGamDoc::GetFontManager().AddFont(TenthPointsToScreenPixels(100),
        taBold, uint8_t(FF_SWISS), "Arial");
    // ------ //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

CBoardManager::~CBoardManager()
{
    DestroyAllElements();
}

void CBoardManager::DestroyAllElements(void)
{
    clear();
    m_fontID = 0;
}

#ifndef GPLAY
BOOL CBoardManager::PurgeMissingTileIDs()
{
    BOOL bPurged = FALSE;
    for (size_t i = 0; i < GetNumBoards(); i++)
        bPurged |= GetBoard(i).PurgeMissingTileIDs();
    return bPurged;
}

BOOL CBoardManager::IsTileInUse(TileID tid) const
{
    for (size_t i = 0; i < GetNumBoards(); i++)
    {
        if (GetBoard(i).IsTileInUse(tid))
            return TRUE;
    }
    return FALSE;
}
#endif // !GPLAY

// Returns Invalid_v<size_t> if board doesn't exist.
size_t CBoardManager::FindBoardBySerial(BoardID nSerialNum) const
{
    for (size_t i = 0; i < GetNumBoards(); i++)
    {
        CPP20_TRACE("Board {} has serial number {}\n", i, GetBoard(i).GetSerialNumber());
        if (GetBoard(i).GetSerialNumber() == nSerialNum)
            return i;
    }
    return Invalid_v<size_t>;
}

BOOL CBoardManager::DoBoardFontDialog()
{
    FontID newFontID = DoFontDialog(m_fontID, GetApp()->m_pMainWnd, TRUE);
    if (newFontID != (FontID)0)
    {
        m_fontID = newFontID;
        return TRUE;
    }
    return FALSE;
}

BoardID CBoardManager::IssueSerialNumber()
{
    if (static_cast<BoardID::UNDERLYING_TYPE>(m_nNextSerialNumber) == GEO_BOARD_SERNUM_BASE)
    {
        AfxThrowMemoryException();
    }
    BoardID retval = m_nNextSerialNumber;
    m_nNextSerialNumber = static_cast<BoardID>(static_cast<BoardID::UNDERLYING_TYPE>(m_nNextSerialNumber) + WORD(1));
    return retval;
}

void CBoardManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_nNextSerialNumber;

        ar << (DWORD)m_crFore;
        ar << (DWORD)m_crBack;
        ar << (WORD)m_nLineWidth;

        CFontTbl& pFontMgr = CGamDoc::GetFontManager();
        pFontMgr.Archive(ar, m_fontID);

        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(GetNumBoards());
        }
        else
        {
            CB::WriteCount(ar, GetNumBoards());
        }
        for (size_t i = size_t(0); i < GetNumBoards(); i++)
            GetBoard(i).Serialize(ar);
    }
    else
    {
        /* delay font deletion in case archive read adds a
            reference */
        FontID tempRef = std::move(m_fontID);
        DestroyAllElements();
        WORD wTmp;
        ar >> m_nNextSerialNumber;

        DWORD dwTmp;
        ar >> dwTmp; m_crFore = (COLORREF)dwTmp;
        ar >> dwTmp; m_crBack = (COLORREF)dwTmp;
        ar >> wTmp; m_nLineWidth = (UINT)wTmp;

        CFontTbl& pFontMgr = CGamDoc::GetFontManager();
        pFontMgr.Archive(ar, m_fontID);

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        size_t count;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar >> wTmp;
            count = wTmp;
        }
        else
        {
            count = CB::ReadCount(ar);
        }
        reserve(count);
        for (size_t i = size_t(0) ; i < count ; i++)
        {
            push_back(MakeOwner<CBoard>());
            back()->Serialize(ar);
        }
    }
}

#if defined(GPLAY)
const CBoard& CBoardManager::Get(const CGeoBoardElement& gelem) const
{
    return geoBoardManager->Get(gelem);
}

CBoardManager::GeoBoardManager::GeoBoardManager(CBoardManager& o, CGamDoc& d) :
    outer(&o),
    doc(d)
{
}

const CBoard& CBoardManager::GeoBoardManager::Get(const CGeoBoardElement& gelem) const
{
    BoardRots::const_iterator it = m_boardRots.find(gelem);
    if (it == m_boardRots.end())
    {
        size_t nBrdIndex = outer->FindBoardBySerial(gelem.m_nBoardSerialNum);
        const CBoard& pBrd = outer->GetBoard(nBrdIndex);
        it = m_boardRots.insert(std::make_pair(gelem, pBrd.Clone(doc, gelem.m_rotation))).first;
    }
    return *it->second;
}

bool CBoardManager::GeoBoardManager::LessCGBE::operator()(const CGeoBoardElement& lhs, const CGeoBoardElement& rhs) const
{
    return static_cast<BoardID::UNDERLYING_TYPE>(lhs.m_nBoardSerialNum) < static_cast<BoardID::UNDERLYING_TYPE>(rhs.m_nBoardSerialNum) ||
            (lhs.m_nBoardSerialNum == rhs.m_nBoardSerialNum && lhs.m_rotation < rhs.m_rotation);
}
#endif
