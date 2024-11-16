// PPieces.cpp
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

#include    <stdafx.h>
#include    <algorithm>
#include    "WinExt.h"
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GMisc.h"
#include    "Trays.h"
#include    "DrawObj.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "CDib.h"
#include    "MapFace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////

CPieceTable::CPieceTable(const CPieceManager& pPMgr, CGamDoc& pDoc) :
    m_pPMgr(pPMgr),
    m_pDoc(&pDoc)
{
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

///////////////////////////////////////////////////////////////////////
// Loads array with piece ID's that aren't already marked as
// is in use and are part of a particular piece set.

std::vector<PieceID> CPieceTable::LoadUnusedPieceList(size_t nPieceSet) const
{
    const CPieceSet& pPSet = m_pPMgr.GetPieceSet(nPieceSet);
    return LoadUnusedPieceList(pPSet);
}

std::vector<PieceID> CPieceTable::LoadUnusedPieceList(const CPieceSet& pPceSet) const
{
    std::vector<PieceID> pPTbl;
    const std::vector<PieceID>& pPidTbl = pPceSet.GetPieceIDTable();
    for (size_t i = size_t(0); i < pPidTbl.size(); i++)
    {
        PieceID pid = pPidTbl.at(i);
        if (!GetPiece(pid).IsUsed())
            pPTbl.push_back(pid);
    }
    return pPTbl;
}

///////////////////////////////////////////////////////////////////////
// Marks all piece ID's corresponding to the input array's entries
// as unused.

void CPieceTable::SetPieceListAsUnused(const std::vector<PieceID>& pPTbl)
{
    for (size_t i = 0; i < pPTbl.size(); i++)
    {
        PieceID pid = pPTbl.at(i);
        GetPiece(pid).SetUnused();
    }
}

void CPieceTable::SetPieceListAsFrontUp(const std::vector<PieceID>& pPTbl)
{
    for (size_t i = 0; i < pPTbl.size(); i++)
    {
        PieceID pid = pPTbl.at(i);
        GetPiece(pid).SetSide(uint8_t(0));          // Front is up
    }
}

///////////////////////////////////////////////////////////////////////
// Remove all pieces that are now undefined.  Called from document
// code during deserialize.

void CPieceTable::PurgeUndefinedPieceIDs()
{
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    ASSERT(pYMgr != NULL);
    CPBoardManager* pPBMgr = m_pDoc->GetPBoardManager();
    ASSERT(pPBMgr != NULL);

    size_t nPiecesDeleted = 0;

    for (size_t i = 0; i < m_pPieceTbl.GetSize(); i++)
    {
        Piece* pPce;
        const PieceDef* pDef;
        GetPieceDefinitionPair(static_cast<PieceID>(i), pPce, pDef);
        if (pDef->IsEmpty())
        {
            if (pPce->IsUsed())
            {
                TRACE1("ERROR: Piece %zu was defined but had no Tile Images. "
                       "Piece removed from game!\n", i);
                pYMgr->RemovePieceIDFromTraySets(static_cast<PieceID>(i));
                OwnerPtr<CDrawObj> pObj = pPBMgr->RemoveObjectID(static_cast<ObjectID>(static_cast<PieceID>(i)));
                pPce->SetUnused();      // Render it gone!
                nPiecesDeleted++;
            }
        }
    }

    if (nPiecesDeleted > 0)
    {
        // Post a message that will be displayed after the game has
        // been fully loaded.
        CB::string strMsg = CB::string::LoadString(IDS_WARN_PIECESDELETED);
        std::unique_ptr<CB::string> pStr(new CB::string);
        *pStr = std::vformat(strMsg, std::make_wformat_args(nPiecesDeleted));
        GetApp()->GetMainWnd()->PostMessage(WM_MESSAGEBOX,
            (WPARAM)WMB_PTR_CBSTRING, reinterpret_cast<LPARAM>(pStr.release()));
    }
}

///////////////////////////////////////////////////////////////////////
// Creates an empty playing piece table that is the same size as the
// the GameBoxes' Piece table.

void CPieceTable::CreatePlayingPieceTable()
{
    Clear();
    size_t nPieces = m_pPMgr.GetPieceTableSize();
    ASSERT(nPieces < m_pPieceTbl.maxSize());
    if (nPieces == size_t(0))
        return;
    m_pPieceTbl.ResizeTable(nPieces, &Piece::SetUnused);
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::SetPieceFacing(PieceID pid, uint16_t nFacingDegCW)
{
    GetPiece(pid).SetFacing(nFacingDegCW);
}

uint16_t CPieceTable::GetPieceFacing(PieceID pid) const
{
    return GetPiece(pid).GetFacing();
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::FlipPieceOver(PieceID pid, CPieceTable::Flip flip, size_t side)
{
    Piece& piece = GetPiece(pid);
    uint8_t prevSide = piece.GetSide();
    size_t sides = GetSides(pid);
    size_t offset;
    switch (flip)
    {
        case fNext:
            offset = size_t(1);
            break;
        case fPrev:
            // avoid arithmetic overflow
            offset = sides - size_t(1);
            break;
        case fRandom:
        case fSelect:
            if (side == Invalid_v<size_t>)
            {
                uint32_t nRandSeed = m_pDoc->GetRandomNumberSeed();
                offset = value_preserving_cast<size_t>(CalcRandomNumberUsingSeed(0, value_preserving_cast<uint32_t>(sides),
                    nRandSeed, &nRandSeed));
                m_pDoc->SetRandomNumberSeed(nRandSeed);
            }
            else
            {
                if (side >= sides)
                {
                    AfxThrowInvalidArgException();
                }
                // avoid arithmetic overflow
                offset = side + sides - prevSide;
            }
            break;
        default:
            AfxThrowInvalidArgException();
    }
    side = (prevSide + offset) % sides;
    piece.SetSide(value_preserving_cast<uint8_t>(side));
}

void CPieceTable::SetPieceUnused(PieceID pid)
{
    GetPiece(pid).SetUnused();
}

BOOL CPieceTable::IsPieceUsed(PieceID pid) const
{
    return GetPiece(pid).IsUsed();
}

uint8_t CPieceTable::GetSide(PieceID pid) const
{
    return GetPiece(pid).GetSide();
}

#if 0
/* when showing all sides in row, the top side is the
    leftmost, and other sides are shifted,
    i.e., n 0 1 2 ... n-1 n+1 n+2 ... sides-1.
    This converts between display index and side */
uint8_t CPieceTable::GetSide(PieceID pid, size_t displayIndex) const
{
    uint8_t side = GetSide(pid);
    if (displayIndex == uint8_t(0))
    {
        return side;
    }
    else if (displayIndex > side)
    {
        return value_preserving_cast<uint8_t>(displayIndex);
    }
    else
    {
        return value_preserving_cast<uint8_t>(displayIndex - size_t(1));
    }
}
#else
/* when showing all sides in row, the sides are rotated so
    the top side is leftmost,
    i.e., n n+1 n+2 ... sides-1 0 1 2 ... n-1.
    This converts between display index and side */
uint8_t CPieceTable::GetSide(PieceID pid, size_t displayIndex) const
{
    return value_preserving_cast<uint8_t>((GetSide(pid) + displayIndex) % GetSides(pid));
}
#endif

size_t CPieceTable::GetSides(PieceID pid) const
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    return pDef->GetSides();
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::ClearAllOwnership()
{
    for (size_t i = size_t(0); i < m_pPieceTbl.GetSize(); i++)
        m_pPieceTbl[static_cast<PieceID>(i)].SetOwnerMask(uint32_t(0));
}

BOOL CPieceTable::IsPieceOwned(PieceID pid) const
{
    return GetPiece(pid).IsOwned();
}

BOOL CPieceTable::IsPieceOwnedBy(PieceID pid, uint32_t dwOwnerMask) const
{
    return GetPiece(pid).IsOwnedBy(dwOwnerMask);
}

BOOL CPieceTable::IsOwnedButNotByCurrentPlayer(PieceID pid, const CGamDoc& pDoc) const
{
    return IsPieceOwned(pid) && !IsPieceOwnedBy(pid, pDoc.GetCurrentPlayerMask());
}

uint32_t CPieceTable::GetOwnerMask(PieceID pid) const
{
    return GetPiece(pid).GetOwnerMask();
}

void CPieceTable::SetOwnerMask(PieceID pid, uint32_t dwMask)
{
    GetPiece(pid).SetOwnerMask(dwMask);
}

///////////////////////////////////////////////////////////////////////

CSize CPieceTable::GetPieceSize(PieceID pid) const
{
    CTile tile = m_pDoc->GetTileManager().GetTile(GetActiveTileID(pid));
    return tile.GetSize();
}

CSize CPieceTable::GetPieceSize(PieceID pid, BOOL bWithFacing)
{
    CTile tile = m_pDoc->GetTileManager().GetTile(GetActiveTileID(pid, bWithFacing));
    return tile.GetSize();
}

CSize CPieceTable::GetStackedSize(const std::vector<PieceID>& pTbl, int xDelta, int yDelta) const
{
    CRect rctFull;
    rctFull.SetRectEmpty();

    CPoint pntCtr(0, 0);

    for (size_t i = size_t(0); i < pTbl.size(); i++)
    {
        CSize sz = GetPieceSize(pTbl.at(i));
        // First create rect centered on zero
        CRect rct(CPoint(-sz.cx / 2, -sz.cy / 2), sz);
        // Offset by staggered point
        rct += pntCtr;
        // Combine with master rect.
        if (i == size_t(0))
            rctFull = rct;
        else
            rctFull |= rct;
        // Move stagger point along
        pntCtr += CSize(xDelta, yDelta);
    }
    return rctFull.Size();
}

CSize CPieceTable::GetStackedSize(const std::vector<PieceID>& pTbl, int xDelta, int yDelta,
    BOOL bWithFacing)
{
    CRect rctFull;
    rctFull.SetRectEmpty();

    CPoint pntCtr(0, 0);

    for (size_t i = 0; i < pTbl.size(); i++)
    {
        CSize sz = GetPieceSize(pTbl.at(i), bWithFacing);
        // First create rect centered on zero
        CRect rct(CPoint(-sz.cx/2, -sz.cy/2), sz);
        // Offset by staggered point
        rct += pntCtr;
        // Combine with master rect.
        if (i == size_t(0))
            rctFull = rct;
        else
            rctFull |= rct;
        // Move stagger point along
        pntCtr += CSize(xDelta, yDelta);
    }
    return rctFull.Size();
}

///////////////////////////////////////////////////////////////////////

TileID CPieceTable::GetFrontTileID(PieceID pid) const
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);

    return pDef->GetFrontTID();
}

TileID CPieceTable::GetFrontTileID(PieceID pid, BOOL bWithFacing)
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);

    TileID tidBase = pDef->GetFrontTID();

    if (!bWithFacing || pPce->GetFacing() == 0)
        return tidBase;

    // Handle rotated pieces...
    return GetFacedTileID(pid, tidBase, pPce->GetFacing(), uint8_t(0));
}

///////////////////////////////////////////////////////////////////////

BOOL CPieceTable::IsPieceInvisible(PieceID pid) const
{
    TileID tid = GetActiveTileID(pid);
    CTile tile = m_pDoc->GetTileManager().GetTile(tid, smallScale);
    return tile.GetTransparent() == tile.GetSmallColor();
}

///////////////////////////////////////////////////////////////////////

TileID CPieceTable::GetActiveTileID(PieceID pid) const
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    return pDef->GetTIDs()[pPce->GetSide()];
}

TileID CPieceTable::GetActiveTileID(PieceID pid, BOOL bWithFacing)
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    TileID tidBase = pDef->GetTIDs()[pPce->GetSide()];

    if (!bWithFacing || pPce->GetFacing() == 0)
        return tidBase;

    // Handle rotated pieces...
    return GetFacedTileID(pid, tidBase,  pPce->GetFacing(), pPce->GetSide());
}

std::vector<TileID> CPieceTable::GetInactiveTileIDs(PieceID pid) const
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    std::vector<TileID> retval = pDef->GetTIDs();
#if 0
    retval.erase(retval.begin() + pPce->GetSide());
#else
    std::rotate(retval.begin(), retval.begin() + pPce->GetSide() + size_t(1), retval.end());
    retval.resize(retval.size() - size_t(1));
#endif
    return retval;
}

std::vector<TileID> CPieceTable::GetInactiveTileIDs(PieceID pid, BOOL bWithFacing)
{
    const Piece* pPce;
    const PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    std::vector<TileID> retval = pDef->GetTIDs();

    if (bWithFacing && pPce->GetFacing() != 0)
    {
        for (size_t i = size_t(0) ; i < retval.size() ; ++i)
        {
            retval[i] = GetFacedTileID(pid, retval[i], pPce->GetFacing(), value_preserving_cast<uint8_t>(i));
        }
    }

#if 0
    retval.erase(retval.begin() + pPce->GetSide());
#else
    std::rotate(retval.begin(), retval.begin() + pPce->GetSide() + size_t(1), retval.end());
    retval.resize(retval.size() - size_t(1));
#endif
    return retval;
}

///////////////////////////////////////////////////////////////////////

TileID CPieceTable::GetFacedTileID(PieceID pid, TileID tidBase, uint16_t nFacing, uint8_t nSide)
{
    // Handle rotated pieces...
    ElementState state(pid, nFacing, nSide);
    CTileFacingMap* pMapFacing = m_pDoc->GetFacingMap();
    TileID tidFacing = pMapFacing->GetFacingTileID(state);
    if (tidFacing != nullTid)
        return tidFacing;
    tidFacing = pMapFacing->CreateFacingTileID(state, tidBase);
    return tidFacing;
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::Clear()
{
    m_pPieceTbl.Clear();
}

///////////////////////////////////////////////////////////////////////

const Piece& CPieceTable::GetPiece(PieceID pid) const
{
    ASSERT(m_pPieceTbl != NULL);
    ASSERT(m_pPieceTbl.Valid(pid));
    return m_pPieceTbl[pid];
}

const PieceDef& CPieceTable::GetPieceDef(PieceID pid) const
{
    return m_pPMgr.GetPiece(pid);
}

void CPieceTable::GetPieceDefinitionPair(PieceID pid, const Piece*& pPce,
    const PieceDef*& pDef) const
{
    pPce = &GetPiece(pid);

    pDef = &m_pPMgr.GetPiece(pid);
}

///////////////////////////////////////////////////////////////////////

OwnerPtr<CPieceTable> CPieceTable::Clone() const
{
    // cloning isn't really violation of source's const
    OwnerPtr<CPieceTable> pTbl = new CPieceTable(m_pPMgr, const_cast<CGamDoc&>(*m_pDoc));
    pTbl->m_pPieceTbl = m_pPieceTbl;
    return pTbl;
}

void CPieceTable::Restore(const CPieceTable& pTbl)
{
    Clear();
    m_pPieceTbl = pTbl.m_pPieceTbl;
}

BOOL CPieceTable::Compare(const CPieceTable& pTbl) const
{
    if (m_pPieceTbl.GetSize() != pTbl.m_pPieceTbl.GetSize())
        return FALSE;
    for (size_t i = 0; i < m_pPieceTbl.GetSize(); i++)
    {
        if (m_pPieceTbl[static_cast<PieceID>(i)] != pTbl.m_pPieceTbl[static_cast<PieceID>(i)])
            return FALSE;
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        ar << m_pPieceTbl;
    }
    else
    {
        Clear();

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        ar >> m_pPieceTbl;

        // Check for consistancy with game box piece table.

        size_t nDefSize = m_pPMgr.GetPieceTableSize();
        if (m_pPieceTbl.GetSize() < nDefSize)
        {
            // Need to increase the size of the playing piece table.
            m_pPieceTbl.ResizeTable(nDefSize, &Piece::SetUnused);
        }
        else if (m_pPieceTbl.GetSize() > nDefSize)
        {
            // Piece table in Game box was truncated. Probably
            // bad news.
            if (AfxMessageBox(IDS_ERR_PIECETBLSIZE, MB_OKCANCEL |
                    MB_ICONEXCLAMATION) != IDOK)
                AfxThrowArchiveException(CArchiveException::genericException);
            // Need to decrease the size of the playing piece table.
            size_t nOldTblSize = m_pPieceTbl.GetSize();
            m_pPieceTbl.ResizeTable(nDefSize, nullptr);
            // Purge pieces in use that don't exist anymore
            CTrayManager* pYMgr = m_pDoc->GetTrayManager();
            CPBoardManager* pPBMgr = m_pDoc->GetPBoardManager();
            for (size_t i = m_pPieceTbl.GetSize(); i < nOldTblSize; i++)
            {
                pYMgr->RemovePieceIDFromTraySets(static_cast<PieceID>(i));
                OwnerPtr<CDrawObj> pObj = pPBMgr->RemoveObjectID(static_cast<ObjectID>(static_cast<PieceID>(i)));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////

void Piece::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_nSide;
        ar << m_nFacing;
        ar << m_dwOwnerMask;
    }
    else
    {
        ar >> m_nSide;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   // (support degrees)
        {
            // Convert pre 2.90 single byte (5 degree resolution) facings to degrees
            BYTE chVal;
            ar >> chVal;
            m_nFacing = chVal;
            m_nFacing *= uint16_t(5);             // Convert to new degree format
        }
        else
            ar >> m_nFacing;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 0))
            m_dwOwnerMask = uint32_t(0);
        else if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
        {
            WORD wTmp;
            ar >> wTmp;
            m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
        }
        else
            ar >> m_dwOwnerMask;
    }
}

///////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CPieceTable::DumpToTextFile(CFile& file) const
{
    static const CB::string szHead = "\r\nPiece Table\r\n-----------\r\n";
    file.Write(szHead.a_str(), value_preserving_cast<UINT>(szHead.a_size()));

    for (size_t i = size_t(0); i < m_pPieceTbl.GetSize(); i++)
    {
        const Piece* pPce;
        const PieceDef* pDef;
        GetPieceDefinitionPair(static_cast<PieceID>(i), pPce, pDef);
        CB::string szBfr = std::format(L"PieceID {:.5}: m_nSide={:02X}, m_nFacing={:3}\r\n",
            std::format(L"{:5}", i), pPce->m_nSide, pPce->m_nFacing);
        file.Write(szBfr.a_str(), value_preserving_cast<UINT>(szBfr.a_size()));
        const std::vector<TileID>& tids = pDef->GetTIDs();
        for (size_t j = size_t(0); j < tids.size(); ++j)
        {
            szBfr = std::format(L"\tm_tid[{:5}]={:5}\r\n",
                    j,
                    tids[j]);
            file.Write(szBfr.a_str(), value_preserving_cast<UINT>(szBfr.a_size()));
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////
// NOT INLINED SO WE CAN SET BREAK POINTS

void Piece::SetUnused()
{
    m_nSide = uint8_t(0xFF);
    m_nFacing = uint16_t(0);
    m_dwOwnerMask = uint32_t(0);
}


