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

const int maxMarks = 32000;
typedef unsigned short MarkID;

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
    CMarkSet() { m_eMarkViz = mtrayVizNormal; }

    CWordArray* GetMarkIDTable() { return &m_midTbl; }
    BOOL HasMarkID(MarkID mid);

    const char* GetName() const { return m_strName; }
    void SetName(const char *pszName) { m_strName = pszName; }

    BOOL IsRandomMarkerPull() { return m_eMarkViz != mtrayVizNormal; }
    void SetMarkerTrayContentVisibility(MarkerTrayViz eMarkViz) { m_eMarkViz = eMarkViz; }
    MarkerTrayViz GetMarkerTrayContentVisibility() { return m_eMarkViz; }

// Operations
public:
    void AddMarkID(MarkID mid, int nPos = -1);
    void RemoveMarkID(MarkID mid);

#ifdef GPLAY
    // Entries are appended to the table.
    void GetRandomSelection(int nCount, CWordArray& tblIDs, CGamDoc* pDoc);
#endif

    void Serialize(CArchive& ar);

// Implementation
protected:
    CString     m_strName;
    CWordArray  m_midTbl;       // MarkIDs in this set.
    MarkerTrayViz m_eMarkViz;   // Content visibility flags // V2.0
};

//////////////////////////////////////////////////////////////////////

class CGameElementStringMap;

class CMarkManager
{
    friend class CGamDoc;
public:
    CMarkManager();
    ~CMarkManager();

// Attributes
public:

    // Mark Set attributes
    int GetNumMarkSets() const { return m_MSetTbl.GetSize(); }
    CMarkSet* GetMarkSet(UINT nMSet)
        { return (CMarkSet*)m_MSetTbl.GetAt(nMSet); }
    void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }

// Operations
public:
    MarkDef* GetMark(MarkID mid);
    BOOL IsMarkIDValid(MarkID mid);
    int FindMarkerSetFromPieceID(MarkID mid);
    MarkID CreateMark(int nMSet, TileID tid, WORD wFlags = 0);
    void DeleteMark(MarkID mid, CGameElementStringMap* pMapString = NULL,
        BOOL bFromSetAlso = TRUE);
    // ------- //
    BOOL IsMarkerInGroup(int nGroup, MarkID mid);
    int  FindMarkInMarkSet(MarkID mid);
    // ------- //
    BOOL PurgeMissingTileIDs(CGameElementStringMap* pMapString = NULL);
    BOOL IsTileInUse(TileID tid);
    // ------- //
    CSize GetMarkSize(MarkID mid);
    // ------- //
    int CreateMarkSet(const char* pszName);
    void DeleteMarkSet(int nMSet, CGameElementStringMap* pMapString = NULL);
    void Clear();
    // ---------- //
    void Serialize(CArchive& ar);
    void SerializeMarkSets(CArchive& ar);

// Implementation
protected:
    MarkDef*    m_pMarkTbl; // Global def'ed
    UINT        m_nTblSize;     // Number of alloc'ed ents in Mark table
    CPtrArray   m_MSetTbl;      // Table of piece set pointers
    WORD        m_wReserved1;   // For future need (set to 0)
    WORD        m_wReserved2;   // For future need (set to 0)
    WORD        m_wReserved3;   // For future need (set to 0)
    WORD        m_wReserved4;   // For future need (set to 0)
    // ------- //
    CTileManager* m_pTMgr;          // Supporting tile manager
    // ------- //
    MarkID CreateMarkIDEntry();
    void ResizeMarkTable(UINT nEntsNeeded);
    void RemoveMarkIDFromMarkSets(MarkID mid);
};

#endif

