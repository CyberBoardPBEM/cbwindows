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

typedef XxxxID16<'P'> PieceID16;
typedef XxxxID32<'P'> PieceID32;
typedef XxxxID<'P'> PieceID;

template<>
struct Invalid<PieceID16>
{
    static constexpr PieceID16 value = PieceID16(0xFFFF);
};

template<>
struct Invalid<PieceID32>
{
    static constexpr PieceID32 value = PieceID32(0xFFFFFFFF);
};

constexpr PieceID nullPid = Invalid_v<PieceID>;

//////////////////////////////////////////////////////////////////////

const size_t pieceTblBaseSize = 32;            // PieceDef table allocation strategy
const size_t pieceTblIncrSize = 8;

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
    BOOL IsEmpty() const { return m_tidFront == nullTid && m_tidBack == nullTid; }
    // ---------- //
    void Serialize(CArchive& ar);
    TileID GetFrontTID() const { return m_tidFront; }
    TileID GetBackTID() const { return m_tidBack; }
    BOOL Is2Sided() const { return m_tidBack != nullTid; }
};

//////////////////////////////////////////////////////////////////////

class CPieceSet
{
    friend class CGamDoc;
// Attributes
public:
    CPieceSet() noexcept = default;
    CPieceSet(const CPieceSet&) = delete;
    CPieceSet& operator=(const CPieceSet&) = delete;
    CPieceSet(CPieceSet&&) noexcept = default;
    CPieceSet& operator=(CPieceSet&&) noexcept = default;
    ~CPieceSet() = default;

    const std::vector<PieceID>& GetPieceIDTable() const { return m_pidTbl; }
    BOOL HasPieceID(PieceID pid) const;

    const char* GetName() const { return m_strName.c_str(); }
    void SetName(const char *pszName) { m_strName = pszName; }

// Operations
public:
    void AddPieceID(PieceID pid, size_t nPos = Invalid_v<size_t>);
    void RemovePieceID(PieceID pid);
    // ---------- //
    void Serialize(CArchive& ar);
// Implementation
protected:
    std::string m_strName;
    std::vector<PieceID> m_pidTbl;
};

//////////////////////////////////////////////////////////////////////

class GameElement32;
class GameElement64;
using GameElement = std::conditional_t<std::is_same_v<TileID, TileID16>, GameElement32, GameElement64>;
template<typename KEY>
class CGameElementStringMapT;
using CGameElementStringMap = CGameElementStringMapT<GameElement>;

class CPieceManager
{
    friend class CPieceTable;
    friend class CGamDoc;
public:
    CPieceManager();
    ~CPieceManager() = default;

// Attributes
public:

    // Piece Set attributes
    size_t GetNumPieceSets() const { return m_PSetTbl.size(); }
    const CPieceSet& GetPieceSet(size_t nPSet) const
        { return m_PSetTbl.at(nPSet); }
    CPieceSet& GetPieceSet(size_t nPSet)
    {
        return const_cast<CPieceSet&>(std::as_const(*this).GetPieceSet(nPSet));
    }
    void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }

// Operations
public:
    const PieceDef& GetPiece(PieceID pid) const;
    PieceDef& GetPiece(PieceID pid)
    {
        return const_cast<PieceDef&>(std::as_const(*this).GetPiece(pid));
    }
    BOOL IsPieceIDValid(PieceID pid) const;
    size_t FindPieceSetFromPieceID(PieceID pid) const;
    PieceID CreatePiece(size_t nPSet, TileID tidFront, TileID tidBack);
    void DeletePiece(PieceID pid, CGameElementStringMap* mapStrings = NULL,
        BOOL bFromSetAlso = TRUE);
    // ------- //
    BOOL PurgeMissingTileIDs(CGameElementStringMap* pMapStrings = NULL);
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid) const;
    // ------- //
    size_t CreatePieceSet(const char* pszName);
    void DeletePieceSet(size_t nPSet, CGameElementStringMap* mapStrings = NULL);
    void Clear();
    // ---------- //
    void Serialize(CArchive& ar);
    void SerializePieceSets(CArchive& ar);
// Implementation
protected:
    XxxxIDTable<PieceID, PieceDef,
                pieceTblBaseSize, pieceTblIncrSize,
                true> m_pPieceTbl;
    std::vector<CPieceSet> m_PSetTbl;
    WORD        m_wReserved1;       // For future need (set to 0)
    WORD        m_wReserved2;       // For future need (set to 0)
    WORD        m_wReserved3;       // For future need (set to 0)
    WORD        m_wReserved4;       // For future need (set to 0)
    // ------- //
    CTileManager* m_pTMgr;          // Supporting tile manager
    // ------- //
    size_t GetPieceTableSize() const { return m_pPieceTbl.GetSize(); }
    void RemovePieceIDFromPieceSets(PieceID pid);
};

#endif

