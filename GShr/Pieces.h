// Pieces.h
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

#ifndef _PIECES_H
#define _PIECES_H

#ifndef     _TILE_H
#include    "Tile.h"
#endif

//////////////////////////////////////////////////////////////////////

const UINT maxPieces = 32000;
typedef unsigned short PieceID;

//////////////////////////////////////////////////////////////////////

struct PieceDef
{
    TileID  m_tidFront;
    TileID  m_tidBack;
    WORD    m_flags;

    enum
    {
        flagShowOnlyOwnersToo   = 0x4000,   // The show only flag applies to owners too
        flagShowOnlyVisibleSide = 0x8000    // The show only the top side of piece
    };

    // -------- //
    void SetEmpty() { m_tidFront = m_tidBack = nullTid; }
    BOOL IsEmpty() { return m_tidFront == nullTid && m_tidBack == nullTid; }
    // ---------- //
    void Serialize(CArchive& ar);
    TileID GetFrontTID() { return m_tidFront; }
    TileID GetBackTID() { return m_tidBack; }
    BOOL Is2Sided() { return m_tidBack != nullTid; }
};

//////////////////////////////////////////////////////////////////////

class CPieceSet
{
    friend class CGamDoc;
// Attributes
public:
    CWordArray* GetPieceIDTable() { return &m_pidTbl; }
    BOOL HasPieceID(PieceID pid);

    const char* GetName() const { return m_strName; }
    void SetName(const char *pszName) { m_strName = pszName; }

// Operations
public:
    void AddPieceID(PieceID pid, int nPos = -1);
    void RemovePieceID(PieceID pid);
    // ---------- //
    void Serialize(CArchive& ar);
// Implementation
protected:
    CString     m_strName;
    CWordArray  m_pidTbl;       // PieceIDs in this set.
};

//////////////////////////////////////////////////////////////////////

class CGameElementStringMap;

class CPieceManager
{
    friend class CPieceTable;
    friend class CGamDoc;
public:
    CPieceManager();
    ~CPieceManager();

// Attributes
public:

    // Piece Set attributes
    int GetNumPieceSets() const { return m_PSetTbl.GetSize(); }
    CPieceSet* GetPieceSet(UINT nPSet)
        { return (CPieceSet*)m_PSetTbl.GetAt(nPSet); }
    void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }

// Operations
public:
    PieceDef* GetPiece(PieceID pid);
    BOOL IsPieceIDValid(PieceID pid);
    int FindPieceSetFromPieceID(PieceID pid);
    PieceID CreatePiece(int nPSet, TileID tidFront, TileID tidBack);
    void CPieceManager::DeletePiece(PieceID pid, CGameElementStringMap* mapStrings = NULL,
        BOOL bFromSetAlso = TRUE);
    // ------- //
    BOOL PurgeMissingTileIDs(CGameElementStringMap* pMapStrings = NULL);
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid);
    // ------- //
    int CreatePieceSet(const char* pszName);
    void DeletePieceSet(int nPSet, CGameElementStringMap* mapStrings = NULL);
    void Clear();
    // ---------- //
    void Serialize(CArchive& ar);
    void SerializePieceSets(CArchive& ar);
// Implementation
protected:
    PieceDef * m_pPieceTbl; // Global def'ed
    UINT        m_nTblSize;         // Number of alloc'ed ents in Piece table
    CPtrArray   m_PSetTbl;          // Table of piece set pointers
    WORD        m_wReserved1;       // For future need (set to 0)
    WORD        m_wReserved2;       // For future need (set to 0)
    WORD        m_wReserved3;       // For future need (set to 0)
    WORD        m_wReserved4;       // For future need (set to 0)
    // ------- //
    CTileManager* m_pTMgr;          // Supporting tile manager
    // ------- //
    UINT GetPieceTableSize() { return m_nTblSize; }
    PieceID CreatePieceIDEntry();
    void ResizePieceTable(UINT nEntsNeeded);
    void RemovePieceIDFromPieceSets(PieceID pid);
};

#endif

