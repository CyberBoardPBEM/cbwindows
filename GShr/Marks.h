// Marks.h
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

#ifndef _MARKS_H
#define _MARKS_H

#ifndef     _TILE_H
#include    "Tile.h"
#endif

//////////////////////////////////////////////////////////////////////

const size_t maxMarks = 32000;
typedef XxxxID16<'M'> MarkID16;
typedef XxxxID32<'M'> MarkID32;
typedef XxxxID<'M'> MarkID;

const       MarkID nullMid = MarkID(0xFFFF);

//////////////////////////////////////////////////////////////////////

const size_t markTblBaseSize = 32;            // MarkDef table allocation strategy
const size_t markTblIncrSize = 8;

//////////////////////////////////////////////////////////////////////

struct MarkDef
{
    TileID  m_tid;
    WORD    m_flags;

    enum { flagPromptText = 0x8000 };

    // -------- //
    void SetEmpty() { m_tid = nullTid; }
    BOOL IsEmpty() { return m_tid == nullTid; }
    // ---------- //
    void Serialize(CArchive& ar);
};

//////////////////////////////////////////////////////////////////////

enum MarkerTrayViz          // Marker Tray content visiblity options
{
    mtrayVizNormal = 0,
    mtrayVizEachGenericRandPull = 1,
    mtrayVizNoneRandPull = 2
};


class CGamDoc;

class CMarkSet
{
    friend class CGamDoc;
// Attributes
public:
    CMarkSet() noexcept { m_eMarkViz = mtrayVizNormal; }
    CMarkSet(const CMarkSet&) = delete;
    CMarkSet& operator=(const CMarkSet&) = delete;
    CMarkSet(CMarkSet&&) noexcept = default;
    CMarkSet& operator=(CMarkSet&&) noexcept = default;
    ~CMarkSet() = default;

    const std::vector<MarkID>& GetMarkIDTable() const { return m_midTbl; }
    BOOL HasMarkID(MarkID mid) const;

    const char* GetName() const { return m_strName.c_str(); }
    void SetName(const char *pszName) { m_strName = pszName; }

    BOOL IsRandomMarkerPull() { return m_eMarkViz != mtrayVizNormal; }
    void SetMarkerTrayContentVisibility(MarkerTrayViz eMarkViz) { m_eMarkViz = eMarkViz; }
    MarkerTrayViz GetMarkerTrayContentVisibility() { return m_eMarkViz; }

// Operations
public:
    void AddMarkID(MarkID mid, size_t nPos = Invalid_v<size_t>);
    void RemoveMarkID(MarkID mid);

#ifdef GPLAY
    // Entries are appended to the table.
    void GetRandomSelection(size_t nCount, std::vector<MarkID>& tblIDs, CGamDoc* pDoc);
#endif

    void Serialize(CArchive& ar);

// Implementation
protected:
    std::string m_strName;
    std::vector<MarkID> m_midTbl;
    MarkerTrayViz m_eMarkViz;   // Content visibility flags // V2.0
};

//////////////////////////////////////////////////////////////////////

class CGameElementStringMap;

class CMarkManager
{
    friend class CGamDoc;
public:
    CMarkManager();
    ~CMarkManager() = default;

// Attributes
public:

    // Mark Set attributes
    size_t GetNumMarkSets() const { return m_MSetTbl.size(); }
    bool IsEmpty() const { return m_MSetTbl.empty(); }
    const CMarkSet& GetMarkSet(size_t nMSet) const
        { return m_MSetTbl.at(nMSet); }
    CMarkSet& GetMarkSet(size_t nMSet)
    {
        return const_cast<CMarkSet&>(std::as_const(*this).GetMarkSet(nMSet));
    }
    void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }

// Operations
public:
    MarkDef& GetMark(MarkID mid);
    BOOL IsMarkIDValid(MarkID mid) const;
    MarkID CreateMark(size_t nMSet, TileID tid, WORD wFlags = 0);
    void DeleteMark(MarkID mid, CGameElementStringMap* pMapString = NULL,
        BOOL bFromSetAlso = TRUE);
    // ------- //
    bool IsMarkerInGroup(size_t nGroup, MarkID mid) const;
    size_t FindMarkInMarkSet(MarkID mid) const;
    // ------- //
    BOOL PurgeMissingTileIDs(CGameElementStringMap* pMapString = NULL);
    BOOL IsTileInUse(TileID tid);
    // ------- //
    CSize GetMarkSize(MarkID mid);
    // ------- //
    size_t CreateMarkSet(const char* pszName);
    void DeleteMarkSet(size_t nMSet, CGameElementStringMap* pMapString = NULL);
    void Clear();
    // ---------- //
    void Serialize(CArchive& ar);
    void SerializeMarkSets(CArchive& ar);

// Implementation
protected:
    XxxxIDTable<MarkID, MarkDef,
                maxMarks, markTblBaseSize, markTblIncrSize,
                true> m_pMarkTbl;
    std::vector<CMarkSet> m_MSetTbl;
    WORD        m_wReserved1;   // For future need (set to 0)
    WORD        m_wReserved2;   // For future need (set to 0)
    WORD        m_wReserved3;   // For future need (set to 0)
    WORD        m_wReserved4;   // For future need (set to 0)
    // ------- //
    CTileManager* m_pTMgr;          // Supporting tile manager
    // ------- //
    void RemoveMarkIDFromMarkSets(MarkID mid);
};

#endif

