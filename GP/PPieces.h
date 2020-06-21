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
    BOOL IsUsed()               { return m_nSide != 0xFF; }
    void SetUnused();

    void SetSide(int nSide)     { m_nSide = (BYTE)nSide; }
    void InvertSide()           { m_nSide ^= 1; }
    int  GetSide()              { return (int)m_nSide; }
    BOOL IsFrontUp()            { return m_nSide == 0; }
    BOOL IsBackUp()             { return m_nSide == 1; }

    void SetFacing(int nFacing) { m_nFacing = (WORD)nFacing; }
    int  GetFacing()            { return (int)m_nFacing; }

    DWORD GetOwnerMask()        { return m_dwOwnerMask; }
    void SetOwnerMask(DWORD dwMask) { m_dwOwnerMask = dwMask; }
    BOOL IsOwned()              { return m_dwOwnerMask != 0; }
    BOOL IsOwnedBy(DWORD dwMask)  { return (BOOL)(m_dwOwnerMask & dwMask); }

    BOOL operator != (Piece& pce)
        { return m_nSide != pce.m_nSide || m_nFacing != pce.m_nFacing; }
    BOOL operator == (Piece& pce)
        { return m_nSide == pce.m_nSide && m_nFacing == pce.m_nFacing; }

    void Serialize(CArchive& ar);

protected:
    // If m_nSide is == 0xFF, the piece is not used.
    BYTE        m_nSide;        // Visible side of piece (0 is front, 1 is back)
    WORD        m_nFacing;      // (1 deg incrs) 0 is normal facing
    DWORD       m_dwOwnerMask;  // Who owns the piece
};

//////////////////////////////////////////////////////////////////////
// This table has 1-to-1 entries with the entries in the piece table
// held by the CPieceManager. It holds the playtime state of a piece:
// the side facing up and the rotation of the image. It also has a
// cache of rotated tile images for pieces that are rotated.

class CPieceTable
{
public:
    CPieceTable();
    ~CPieceTable();

// Attributes
public:
    CPieceManager* GetPieceManager() { return m_pPMgr; }
    void SetPieceManager(CPieceManager* pPMgr) { m_pPMgr = pPMgr; }
    void SetDocument(CGamDoc* pDoc) { m_pDoc = pDoc; }

// Operations
public:
    void LoadUnusedPieceList(CWordArray *pPTbl, int nPieceSet,
        BOOL bClear = TRUE);
    void LoadUnusedPieceList(CWordArray *pPTbl, CPieceSet* pPceSet,
        BOOL bClear = TRUE);
    void SetPieceListAsUnused(CWordArray *pPTbl);
    void SetPieceListAsFrontUp(CWordArray *pPTbl);

    void FlipPieceOver(PieceID pid);

    void SetPieceFacing(PieceID pid, int nFacingDegCW);
    int  GetPieceFacing(PieceID pid);

    BOOL IsFrontUp(PieceID pid);
    BOOL Is2Sided(PieceID pid);
    BOOL IsPieceUsed(PieceID pid);

    BOOL IsPieceOwned(PieceID pid);
    BOOL IsPieceOwnedBy(PieceID pid, DWORD dwOwnerMask);
    BOOL IsOwnedButNotByCurrentPlayer(PieceID pid, CGamDoc* pDoc);
    DWORD GetOwnerMask(PieceID pid);
    void SetOwnerMask(PieceID pid, DWORD wwMask);
    void ClearAllOwnership();

    BOOL IsPieceInvisible(PieceID pid);

    TileID GetFrontTileID(PieceID pid, BOOL bWithFacing = FALSE);
    TileID GetBackTileID(PieceID pid, BOOL bWithFacing = FALSE);

    TileID GetActiveTileID(PieceID pid, BOOL bWithFacing = FALSE);
    TileID GetInactiveTileID(PieceID pid, BOOL bWithFacing = FALSE);

    CSize GetPieceSize(PieceID pid, BOOL bWithFacing = FALSE);
    CSize GetStackedSize(CWordArray* pTbl, int xDelta, int yDelta, BOOL bWithFacing = FALSE);

    void CreatePlayingPieceTable();
    void SetPiece(PieceID pid, int nSide = 0, int nFacing = 0);
    void SetPieceUnused(PieceID pid);

    void PurgeUndefinedPieceIDs();

    void Clear();

    CPieceTable* Clone(CGamDoc *pDoc);
    void Restore(CGamDoc *pDoc, CPieceTable* pTbl);
    BOOL Compare(CPieceTable* pTbl);

    void Serialize(CArchive& ar);

#ifdef _DEBUG
    void DumpToTextFile(CFile& file);
#endif

// Implementation
protected:
    Piece*      m_pPieceTbl;        // Global def'ed
    UINT        m_nTblSize;         // Number of alloc'ed ents in Piece table

    WORD        m_wReserved1;       // For future need (set to 0)
    WORD        m_wReserved2;       // For future need (set to 0)
    WORD        m_wReserved3;       // For future need (set to 0)
    WORD        m_wReserved4;       // For future need (set to 0)
    // ------- //
    CPieceManager* m_pPMgr;         // To get access to piece defs.
    CGamDoc*       m_pDoc;          // Used for serialize fixups
    // ------- //
    Piece* GetPiece(PieceID pid);
    PieceDef* GetPieceDef(PieceID pid);
    void GetPieceDefinitionPair(PieceID pid, Piece*& pPce, PieceDef*& pDef);

    TileID GetFacedTileID(PieceID pid, TileID tidBase, int nFacing, int nSide);
};

#endif


