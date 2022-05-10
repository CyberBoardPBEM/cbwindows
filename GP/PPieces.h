// PPieces.h
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

#ifndef _PPIECES_H
#define _PPIECES_H

#ifndef     __AFXTEMPL_H__
#include    <afxtempl.h>
#endif

#ifndef     _PIECES_H
#include    "Pieces.h"
#endif

//////////////////////////////////////////////////////////////////////

class CGamDoc;

//////////////////////////////////////////////////////////////////////
// Piece structures are in a table that parallels the
// the Game Box PieceDef table.

enum PieceViz           // Piece visiblity options
{
    pceVizFully = 0,
    pceVizTopOnly = 1,
    pceVizGeneric = 2,
    pceVizInvisible = 3
};

class Piece
{
    friend CPieceTable;     // For debug access

public:
    BOOL IsUsed() const         { return m_nSide != 0xFF; }
    void SetUnused();

private:
    // only for CPieceTable's use
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] void SetSide(uint8_t nSide) { m_nSide = nSide; }
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] void InvertSide()           { m_nSide ^= uint8_t(1); }
public:
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] uint8_t GetSide() const     { return m_nSide; }
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] BOOL IsFrontUp() const      { return m_nSide == 0; }
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] BOOL IsBackUp() const       { return m_nSide == 1; }

    void SetFacing(uint16_t nFacing) { m_nFacing = nFacing; }
    uint16_t GetFacing() const  { return m_nFacing; }

    uint32_t GetOwnerMask() const  { return m_dwOwnerMask; }
    void SetOwnerMask(uint32_t dwMask) { m_dwOwnerMask = dwMask; }
    BOOL IsOwned() const        { return m_dwOwnerMask != uint32_t(0); }
    BOOL IsOwnedBy(uint32_t dwMask) const { return (BOOL)(m_dwOwnerMask & dwMask); }

    BOOL operator != (const Piece& pce) const
        { return m_nSide != pce.m_nSide || m_nFacing != pce.m_nFacing; }
    BOOL operator == (const Piece& pce) const
        { return m_nSide == pce.m_nSide && m_nFacing == pce.m_nFacing; }

    void Serialize(CArchive& ar);

protected:
    // If m_nSide is == 0xFF, the piece is not used.
    uint8_t     m_nSide;        // Visible side of piece (0 is front, 1 is back)
    uint16_t    m_nFacing;      // (1 deg incrs) 0 is normal facing
    uint32_t    m_dwOwnerMask;  // Who owns the piece
};

//////////////////////////////////////////////////////////////////////
// This table has 1-to-1 entries with the entries in the piece table
// held by the CPieceManager. It holds the playtime state of a piece:
// the side facing up and the rotation of the image. It also has a
// cache of rotated tile images for pieces that are rotated.

class CPieceTable
{
public:
    CPieceTable(const CPieceManager& pPMgr, CGamDoc& pDoc);
    ~CPieceTable() = default;
    bool Empty() const { return m_pPieceTbl.Empty(); }

// Attributes
public:
    const CPieceManager& GetPieceManager() { return m_pPMgr; }

// Operations
public:
    std::vector<PieceID> LoadUnusedPieceList(size_t nPieceSet) const;
    std::vector<PieceID> LoadUnusedPieceList(const CPieceSet& pPceSet) const;
    void SetPieceListAsUnused(const std::vector<PieceID>& pPTbl);
    void SetPieceListAsFrontUp(const std::vector<PieceID>& pPTbl);

    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] void FlipPieceOver(PieceID pid);
    enum Flip { fInvalid, fPrev, fNext, fSelect, fRandom };
    void FlipPieceOver(PieceID pid, Flip flip, size_t side);

    void SetPieceFacing(PieceID pid, uint16_t nFacingDegCW);
    uint16_t GetPieceFacing(PieceID pid) const;

    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] BOOL IsFrontUp(PieceID pid) const;
    uint8_t GetSide(PieceID pid) const;
#if 0
    /* when showing all sides in row, the top side is the
        leftmost, and other sides are shifted,
        i.e., n 0 1 2 ... n-1 n+1 n+2 ... sides-1.
        This converts between display index and side */
#else
    /* when showing all sides in row, the sides are rotated so
        the top side is leftmost,
        i.e., n n+1 n+2 ... sides-1 0 1 2 ... n-1.
        This converts between display index and side */
#endif
    uint8_t GetSide(PieceID pid, size_t displayIndex) const;
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] BOOL Is2Sided(PieceID pid) const;
    size_t GetSides(PieceID pid) const;
    BOOL IsPieceUsed(PieceID pid) const;

    BOOL IsPieceOwned(PieceID pid) const;
    BOOL IsPieceOwnedBy(PieceID pid, uint32_t dwOwnerMask) const;
    BOOL IsOwnedButNotByCurrentPlayer(PieceID pid, const CGamDoc& pDoc) const;
    uint32_t GetOwnerMask(PieceID pid) const;
    void SetOwnerMask(PieceID pid, uint32_t wwMask);
    void ClearAllOwnership();

    BOOL IsPieceInvisible(PieceID pid) const;

    TileID GetFrontTileID(PieceID pid) const;
    TileID GetFrontTileID(PieceID pid, BOOL bWithFacing);

    TileID GetActiveTileID(PieceID pid) const;
    TileID GetActiveTileID(PieceID pid, BOOL bWithFacing);
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] TileID GetInactiveTileID(PieceID pid) const;
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] TileID GetInactiveTileID(PieceID pid, BOOL bWithFacing);
    std::vector<TileID> GetInactiveTileIDs(PieceID pid) const;
    std::vector<TileID> GetInactiveTileIDs(PieceID pid, BOOL bWithFacing);

    CSize GetPieceSize(PieceID pid) const;
    CSize GetPieceSize(PieceID pid, BOOL bWithFacing);
    CSize GetStackedSize(const std::vector<PieceID>& pTbl, int xDelta, int yDelta) const;
    CSize GetStackedSize(const std::vector<PieceID>& pTbl, int xDelta, int yDelta, BOOL bWithFacing);

    void CreatePlayingPieceTable();
    [[deprecated("need to get ready for pieces with multiple \"back\" sides")]] void SetPiece(PieceID pid, uint8_t nSide = uint8_t(0), uint16_t nFacing = uint16_t(0));
    void SetPieceUnused(PieceID pid);

    void PurgeUndefinedPieceIDs();

    void Clear();

    OwnerPtr<CPieceTable> Clone() const;
    void Restore(const CPieceTable& pTbl);
    BOOL Compare(const CPieceTable& pTbl) const;

    void Serialize(CArchive& ar);

#ifdef _DEBUG
    void DumpToTextFile(CFile& file) const;
#endif

// Implementation
protected:
    XxxxIDTable<PieceID, Piece,
                pieceTblBaseSize, pieceTblIncrSize,
                true> m_pPieceTbl;

    WORD        m_wReserved1;       // For future need (set to 0)
    WORD        m_wReserved2;       // For future need (set to 0)
    WORD        m_wReserved3;       // For future need (set to 0)
    WORD        m_wReserved4;       // For future need (set to 0)
    // ------- //
    const CPieceManager& m_pPMgr;         // To get access to piece defs.
    RefPtr<CGamDoc> m_pDoc;          // Used for serialize fixups
    // ------- //
    const Piece& GetPiece(PieceID pid) const;
    Piece& GetPiece(PieceID pid) { return const_cast<Piece&>(std::as_const(*this).GetPiece(pid)); }
    const PieceDef& GetPieceDef(PieceID pid) const;
    void GetPieceDefinitionPair(PieceID pid, const Piece*& pPce, const PieceDef*& pDef) const;
    void GetPieceDefinitionPair(PieceID pid, Piece*& pPce, const PieceDef*& pDef)
    {
        const Piece* temp;
        GetPieceDefinitionPair(pid, temp, pDef);
        pPce = const_cast<Piece*>(temp);
    }

    // non-const due to creating new tile for rotated tidBase
    TileID GetFacedTileID(PieceID pid, TileID tidBase, uint16_t nFacing, uint8_t nSide);
};

#endif


