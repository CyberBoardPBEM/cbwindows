// Board.h
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

#ifndef _BOARD_H
#define _BOARD_H

#ifndef     _BRDCELL_H
#include    "BrdCell.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _DRAWOBJ_H
#include    "DrawObj.h"
#endif

enum LAYER { LAYER_BASE = 1, LAYER_GRID = 2, LAYER_TOP = 3 };

class CBoardBase
{
    friend class CGamDoc;
public:
    CBoardBase();
    virtual ~CBoardBase();
// Attributes
public:
    // (Saved in file...)
    int m_bGridSnap;        // TRUE if edit snap grid is on
    int m_xGridSnap;        // X grid multiple * 1000
    int m_yGridSnap;        // Y grid multiple * 1000
    int m_xGridSnapOff;     // X grid offset (must be < m_xGridSnap) * 1000
    int m_yGridSnapOff;     // Y grid offset (must be < m_yGridSnap) * 1000
    // ------- //
    int GetSerialNumber() { return m_nSerialNum; }
    void SetSerialNumber(int nSerialNum) { m_nSerialNum = nSerialNum; }
    // ------- //
    BOOL GetApplyVisible() { return m_bApplyVisibility; }
    void SetApplyVisible(BOOL bApply) { m_bApplyVisibility = bApply; }
    // ------- //
    void SetBkColor(COLORREF crBkGnd) { m_crBkGnd = crBkGnd; }
    void SetBaseDrawing(CDrawList* pDwg);
    CDrawList* GetBaseDrawing(BOOL bCreate = FALSE);
    void SetMaxDrawLayer(int iMaxLayer = -1) { m_iMaxLayer = iMaxLayer; }
    void SetName(const char *pszName) { m_strBoardName = pszName; }
    virtual void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }

    int GetMaxDrawLayer() { return m_iMaxLayer; }
    CTileManager* GetTileManager() { return m_pTMgr; }
    const char* GetName() { return m_strBoardName; }

// Operations
public:
    void DrawBackground(CDC* pDC, CRect* pDrawRct);
    void DrawDrawingList(CDrawList* pDwg, CDC* pDC, CRect* pDrawRct,
        TileScale eScale, BOOL bApplyVisible, BOOL bDrawPass2Objects = FALSE);
#ifndef GPLAY
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid);
#endif
    // ------- //
    virtual void Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale,
        int nApplyVisible = -1);
    // ------- //
    void Serialize(CArchive& ar);
// Implementation
protected:
    // Saved in file...
    BOOL        m_bApplyVisibility; // Show objects matching scale mask
    int         m_nSerialNum;       // Board serial number
    CString     m_strBoardName;     // Name of board
    COLORREF    m_crBkGnd;          // Default color of the board
    int         m_iMaxLayer;        // Max layer to draw

    // List of base layer drawing primitives (lines, polygons, text...);
    CDrawList*  m_pBaseDwg;         // Various shapes to draw

    // ------- //
    CTileManager* m_pTMgr;          // Tile manager
};

//////////////////////////////////////////////////////////////////////

class CBoard : public CBoardBase
{
    friend class CGamDoc;
public:
    CBoard();
    virtual ~CBoard();
// Attributes
public:
    CSize GetSize(TileScale eScale)
        {
            ASSERT(m_pBrdAry!=NULL);
            return m_pBrdAry->GetSize(eScale);
        }
    int GetWidth(TileScale eScale)
        {
            ASSERT(m_pBrdAry!=NULL);
            return m_pBrdAry->GetWidth(eScale);
        }
    int GetHeight(TileScale eScale)
        {
            ASSERT(m_pBrdAry!=NULL);
            return m_pBrdAry->GetHeight(eScale);
        }
    CSize GetCellSize(TileScale eScale)
        {
            ASSERT(m_pBrdAry!=NULL);
            return m_pBrdAry->GetCellSize(eScale);
        }
    void SetBoardArray(CBoardArray* pDwg);
    CBoardArray* GetBoardArray() { return m_pBrdAry; }
    void SetTopDrawing(CDrawList* pDwg);
    CDrawList* GetTopDrawing(BOOL bCreate = FALSE);
    virtual void SetTileManager(CTileManager* pTMgr)
        {
            ASSERT(m_pBrdAry!=NULL);
            CBoardBase::SetTileManager(pTMgr);
            m_pBrdAry->SetTileManager(pTMgr);
        }
    // -------- //
    BOOL GetCellBorder() { return m_bShowCellBorder; }
    void SetCellBorder(BOOL bShow) { m_bShowCellBorder = bShow; }
    BOOL GetCellBorderOnTop() { return m_bCellBorderOnTop; }
    void SetCellBorderOnTop(BOOL bOnTop) { m_bCellBorderOnTop = bOnTop; }

// Operations
public:
    void DrawCellLines(CDC* pDC, CRect* pCellRct, TileScale eScale);
    void DrawCells(CDC* pDC, CRect* pCellRct, TileScale eScale);
#ifndef GPLAY
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid);
    // ------- //
    void ForceObjectsOntoBoard();
#endif
    // ------- //
    virtual void Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale,
        int nCellBorder = -1, int nApplyVisible = -1);// -1 means use internal
    // ------- //
    void Serialize(CArchive& ar);

// Implementation
protected:
    BOOL    m_bShowCellBorder;      // Show cell frames
    BOOL    m_bCellBorderOnTop;     // Draw cell frames last (was reserved1)
    WORD    m_wReserved1;           // For future need (set to 0)
    WORD    m_wReserved2;           // For future need (set to 0)
    WORD    m_wReserved3;           // For future need (set to 0)
    WORD    m_wReserved4;           // For future need (set to 0)
    // Saved in file...
    CBoardArray* m_pBrdAry;     // Actual board definition
    // List of outer layer drawing primitives (lines, polygons, text...);
    CDrawList*  m_pTopDwg;
    // -------- //
    BOOL IsDrawGridLines(int nOverride)
    {
        return nOverride == -1 ? m_bShowCellBorder : nOverride;
    }

};

//////////////////////////////////////////////////////////////////////

class CBoardManager : public CPtrArray
{
    friend class CGamDoc;
public:
    CBoardManager();
    ~CBoardManager() { DestroyAllElements(); }

// Attributes
public:
    int GetNumBoards() const { return GetSize(); }
    CBoard* GetBoard(int i) { return (CBoard *)GetAt(i); }

    // Access routines for all Tile Editor info...
    void SetForeColor(COLORREF cr) { m_crFore = cr; }
    void SetBackColor(COLORREF cr) { m_crBack = cr; }
    void SetLineWidth(int nWidth) { m_nLineWidth = nWidth; }
    COLORREF GetForeColor() { return m_crFore; }
    COLORREF GetBackColor() { return m_crBack; }
    int GetLineWidth() { return m_nLineWidth; }
    FontID GetFontID() { return m_fontID; }

    BOOL DoBoardFontDialog();

// Operations
public:
    void Add(CBoard* pBoard) { CPtrArray::Add(pBoard); }
    // -------- //
    void DeleteBoard(int nBoard) { DestroyElement(nBoard); }
    int  IssueSerialNumber() { return m_nNextSerialNumber++; }
#ifndef GPLAY
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid);
#endif
    // -------- //
    int FindBoardBySerial(int nSerialNum);
    // -------- //
    void DestroyAllElements(void);
    void DestroyElement(int iElementIdx)
        { delete (CBoard*)GetAt(iElementIdx); RemoveAt(iElementIdx); }
    CBoard* operator[](int nIndex) const
        { return (CBoard*)GetAt(nIndex); }
    CBoard*& operator[](int nIndex)
        { return (CBoard*&)ElementAt(nIndex); }
    // ------- //
    void Serialize(CArchive& ar);
protected:
    // Saved in file...
    int     m_nNextSerialNumber;    // Should be 1 or greater
    WORD    m_wReserved1;           // For future need (set to 0)
    WORD    m_wReserved2;           // For future need (set to 0)
    WORD    m_wReserved3;           // For future need (set to 0)
    WORD    m_wReserved4;           // For future need (set to 0)
    // These are shared for all Board Editors...
    COLORREF    m_crFore;           // Current foreground color
    COLORREF    m_crBack;           // Current background color
    UINT        m_nLineWidth;       // Current line width
    FontID      m_fontID;           // Current font
};

#endif

