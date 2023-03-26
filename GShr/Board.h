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

#if defined(GPLAY)
struct CGeoBoardElement;
#endif

typedef XxxxID16<'B'> BoardID16;
typedef XxxxID32<'B'> BoardID32;
typedef XxxxID<'B'> BoardID;

template<>
struct Invalid<BoardID16>
{
    static constexpr BoardID16 value = BoardID16(0xFFFF);
};

template<>
struct Invalid<BoardID32>
{
    static constexpr BoardID32 value = BoardID32(0xFFFFFFFF);
};

constexpr BoardID nullBid = Invalid_v<BoardID>;
// The starting serial number for geomorpically created boards.
const size_t GEO_BOARD_SERNUM_BASE = 1000;

enum LAYER { LAYER_BASE = 1, LAYER_GRID = 2, LAYER_TOP = 3 };

class CBoardBase
{
    friend class CGamDoc;
public:
    CBoardBase();
    CBoardBase(const CBoardBase&) = delete;
    CBoardBase& operator=(const CBoardBase&) = delete;
    virtual ~CBoardBase() = default;
// Attributes
public:
    // (Saved in file...)
    int m_bGridSnap;        // TRUE if edit snap grid is on
    int m_xGridSnap;        // X grid multiple * 1000
    int m_yGridSnap;        // Y grid multiple * 1000
    int m_xGridSnapOff;     // X grid offset (must be < m_xGridSnap) * 1000
    int m_yGridSnapOff;     // Y grid offset (must be < m_yGridSnap) * 1000
    // ------- //
    BoardID GetSerialNumber() const { return m_nSerialNum; }
    void SetSerialNumber(BoardID nSerialNum) { m_nSerialNum = nSerialNum; }
    // ------- //
    BOOL GetApplyVisible() const { return m_bApplyVisibility; }
    void SetApplyVisible(BOOL bApply) { m_bApplyVisibility = bApply; }
    // ------- //
    void SetBkColor(COLORREF crBkGnd) { m_crBkGnd = crBkGnd; }
    void SetBaseDrawing(OwnerOrNullPtr<CDrawList> pDwg);
    const CDrawList* GetBaseDrawing() const { return m_pBaseDwg ? &*m_pBaseDwg : nullptr; }
    CDrawList* GetBaseDrawing(BOOL bCreate = FALSE);
    void SetMaxDrawLayer(int iMaxLayer = -1) { m_iMaxLayer = iMaxLayer; }
    void SetName(const CB::string& pszName) { m_strBoardName = pszName; }

    int GetMaxDrawLayer() const { return m_iMaxLayer; }
    const CB::string& GetName() const { return m_strBoardName; }

// Operations
public:
    void DrawBackground(CDC& pDC, const CRect& pDrawRct) const;
    static void DrawDrawingList(CDrawList* pDwg, CDC& pDC, const CRect& pDrawRct,
        TileScale eScale, BOOL bApplyVisible, BOOL bDrawPass2Objects = FALSE);
#ifndef GPLAY
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid) const;
#endif
    // ------- //
    virtual void Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale,
        int nApplyVisible = -1);
    // ------- //
    void Serialize(CArchive& ar);
// Implementation
protected:
    // Saved in file...
    BOOL        m_bApplyVisibility; // Show objects matching scale mask
    BoardID     m_nSerialNum;       // Board serial number
    CB::string  m_strBoardName;     // Name of board
    COLORREF    m_crBkGnd;          // Default color of the board
    int         m_iMaxLayer;        // Max layer to draw

    // List of base layer drawing primitives (lines, polygons, text...);
    OwnerOrNullPtr<CDrawList>  m_pBaseDwg;         // Various shapes to draw

    // ------- //
    CB::propagate_const<CTileManager*> m_pTMgr;          // Tile manager
};

//////////////////////////////////////////////////////////////////////

class CBoard : public CBoardBase
{
    friend class CGamDoc;
public:
    CBoard();
    virtual ~CBoard() = default;
// Attributes
public:
    CSize GetSize(TileScale eScale) const
        {
            return GetBoardArray().GetSize(eScale);
        }
    int GetWidth(TileScale eScale) const
        {
            return GetBoardArray().GetWidth(eScale);
        }
    int GetHeight(TileScale eScale) const
        {
            return GetBoardArray().GetHeight(eScale);
        }
    CSize GetCellSize(TileScale eScale) const
        {
            return GetBoardArray().GetCellSize(eScale);
        }
    void SetBoardArray(OwnerOrNullPtr<CBoardArray> pDwg);
    const CBoardArray& GetBoardArray() const { return CheckedDeref(m_pBrdAry); }
    CBoardArray& GetBoardArray() { return const_cast<CBoardArray&>(std::as_const(*this).GetBoardArray()); }
    void SetTopDrawing(OwnerOrNullPtr<CDrawList> pDwg);
    const CDrawList* GetTopDrawing() const { return m_pTopDwg ? &*m_pTopDwg : nullptr; }
    CDrawList* GetTopDrawing(BOOL bCreate = FALSE);
    // -------- //
    BOOL GetCellBorder() const { return m_bShowCellBorder; }
    void SetCellBorder(BOOL bShow) { m_bShowCellBorder = bShow; }
    BOOL GetCellBorderOnTop() const { return m_bCellBorderOnTop; }
    void SetCellBorderOnTop(BOOL bOnTop) { m_bCellBorderOnTop = bOnTop; }

#if defined(GPLAY)
    /* the more general geomorphic board stitching needs to know
        stagger for all corners, not just top-left that
        CCellForm can provide */
    CellStagger GetStagger(Corner c) const;
    /* recognize the Peter Lomax Ogre/GEV board style where "sticking
        out" cells are empty, and rest of edge is filled
        (see https://github.com/CyberBoardPBEM/cbwindows/pull/78) */
    bool IsGEVStyle(Edge e) const;
    bool IsEmpty(const BoardCell& cell) const;
    // WARNING:  only supported on unrotated boards
    OwnerPtr<CBoard> Clone(CGamDoc& doc, Rotation90 r) const;
#endif

// Operations
public:
    void DrawCellLines(CDC& pDC, const CRect& pCellRct, TileScale eScale) const;
    void DrawCells(CDC& pDC, const CRect& pCellRct, TileScale eScale) const;
#ifndef GPLAY
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid) const;
    // ------- //
    void ForceObjectsOntoBoard();
#endif
    // ------- //
    virtual void Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale,
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
    OwnerOrNullPtr<CBoardArray> m_pBrdAry;     // Actual board definition
    // List of outer layer drawing primitives (lines, polygons, text...);
    OwnerOrNullPtr<CDrawList>  m_pTopDwg;
    // -------- //
    BOOL IsDrawGridLines(int nOverride) const
    {
        return nOverride == -1 ? m_bShowCellBorder : nOverride;
    }

private:
    bool IsGEVStyleHelper(Edge e) const;
};

//////////////////////////////////////////////////////////////////////

/* N.B.:  this holds CBoard* instead of CBoard because CBoard
    derives from CBoardBase, which is designed as a polymorphic
    class */
class CBoardManager : private std::vector<OwnerPtr<CBoard>>
{
    friend class CGamDoc;
public:
    CBoardManager(CGamDoc& doc);
    ~CBoardManager();

// Attributes
public:
    size_t GetNumBoards() const { return size(); }
    bool IsEmpty() const { return empty(); }
    const CBoard& GetBoard(size_t i) const
    {
        return *at(i);
    }
    CBoard& GetBoard(size_t i)
    {
        return const_cast<CBoard&>(std::as_const(*this).GetBoard(i));
    }

    // Access routines for all Tile Editor info...
    void SetForeColor(COLORREF cr) { m_crFore = cr; }
    void SetBackColor(COLORREF cr) { m_crBack = cr; }
    void SetLineWidth(int nWidth) { m_nLineWidth = nWidth; }
    COLORREF GetForeColor() const { return m_crFore; }
    COLORREF GetBackColor() const { return m_crBack; }
    int GetLineWidth() const { return m_nLineWidth; }
    FontID GetFontID() const { return m_fontID; }

    BOOL DoBoardFontDialog();

// Operations
public:
    void Add(OwnerPtr<CBoard> pBoard)
    {
        push_back(std::move(pBoard));
    }
    // -------- //
    void DeleteBoard(size_t nBoard) { DestroyElement(nBoard); }
    BoardID IssueSerialNumber();
#ifndef GPLAY
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid) const;
#endif
    // -------- //
    size_t FindBoardBySerial(BoardID nSerialNum) const;
    // -------- //
    void DestroyAllElements(void);
    void DestroyElement(size_t iElementIdx)
        { erase(begin() + value_preserving_cast<ptrdiff_t>(iElementIdx)); }
    const CBoard* operator[](size_t nIndex) const
        { return at(nIndex).get(); }
    OwnerPtr<CBoard>& operator[](size_t nIndex)
        { return at(nIndex); }
    // ------- //
    bool Needs32BitIDs() const
    {
        return static_cast<BoardID::UNDERLYING_TYPE>(m_nNextSerialNumber) >= size_t(0xFFFF);
    }
    void Serialize(CArchive& ar);
protected:
    // Saved in file...
    BoardID m_nNextSerialNumber;    // Should be 1 or greater
    WORD    m_wReserved1;           // For future need (set to 0)
    WORD    m_wReserved2;           // For future need (set to 0)
    WORD    m_wReserved3;           // For future need (set to 0)
    WORD    m_wReserved4;           // For future need (set to 0)
    // These are shared for all Board Editors...
    COLORREF    m_crFore;           // Current foreground color
    COLORREF    m_crBack;           // Current background color
    UINT        m_nLineWidth;       // Current line width
    FontID      m_fontID;           // Current font

#if defined(GPLAY)
    // geomorphic board support
    // share CBoards across CGeomorphicBoards
public:
    const CBoard& Get(const CGeoBoardElement& gelem) const;

private:
    // reduce need for #include "GeoBoard.h"
    class GeoBoardManager;
    OwnerPtr<GeoBoardManager> geoBoardManager;
#endif
};

#endif

