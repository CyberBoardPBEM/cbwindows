// PBoard.h
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

#ifndef _PBOARD_H
#define _PBOARD_H

#ifndef     _GAMDOC_H
#include    "GamDoc.h"
#endif

#ifndef     _BOARD_H
#include    "Board.h"
#endif

//////////////////////////////////////////////////////////////////

class CPieceTable;
class CDrawList;
class CBoard;
class CBoardManager;
class CGeomorphicBoard;

//////////////////////////////////////////////////////////////////

class CPlayBoard
{
public:
    CPlayBoard(CGamDoc& doc);
    CPlayBoard(const CPlayBoard&) = delete;
    CPlayBoard& operator=(const CPlayBoard&) = delete;
    CPlayBoard(CPlayBoard&& other) noexcept = default;
    CPlayBoard& operator=(CPlayBoard&& other) noexcept = default;
    ~CPlayBoard() = default;

// Attributes
public:
    const CDrawList* GetPieceList() const { return m_pPceList.get(); }
    CDrawList* GetPieceList() { return const_cast<CDrawList*>(std::as_const(*this).GetPieceList()); }
    const CDrawList* GetIndicatorList() const { return m_pIndList.get(); }
    CDrawList* GetIndicatorList() { return const_cast<CDrawList*>(std::as_const(*this).GetIndicatorList()); }
    BOOL GetPlotMoveMode() const { return m_bPlotMode; }
    void SetPlotMoveMode(BOOL bPlotMode) { m_bPlotMode = bPlotMode; }
    void InitPlotStartPoint() { m_ptPrevPlot = CPoint(-1, -1); }
    CPoint GetPrevPlotPoint() const { return m_ptPrevPlot; }
    void SetPrevPlotPoint(CPoint pnt) { m_ptPrevPlot = pnt; }

    void SetPiecesVisible(BOOL bVisible = TRUE) { m_bPVisible = bVisible; }
    BOOL GetPiecesVisible() const { return m_bPVisible; }

    void SetDrawLockedBeneath(BOOL bDrawBeneath = TRUE) { m_bLockedDrawnBeneath = bDrawBeneath; }
    BOOL GetDrawLockedBeneath() const { return m_bLockedDrawnBeneath; }

    void SetRotateBoard180(BOOL bRotate) { m_bRotate180 = bRotate; }
    BOOL IsBoardRotated180() const { return m_bRotate180; }

    void SetIndicatorsOnTop(BOOL bDrawOnTop = TRUE) { m_bIndOnTop = bDrawOnTop; }
    BOOL GetIndicatorsOnTop() const { return m_bIndOnTop; }

    void SetLocksEnforced(BOOL bEnforce = TRUE) { m_bEnforceLocks = bEnforce; }
    BOOL GetLocksEnforced() const { return m_bEnforceLocks; }

    void SetBoard(OwnerPtr<CGeomorphicBoard> pGeoBoard, BOOL bInheritSettings = FALSE);
    void SetBoard(CBoard& pBoard, BOOL bInheritSettings = FALSE);
    const CBoard* GetBoard() const { return m_pBoard.get(); }
    CBoard* GetBoard() { return const_cast<CBoard*>(std::as_const(*this).GetBoard()); }

    COLORREF GetLineColor() const { return m_crLineColor; }
    UINT     GetLineWidth() const { return m_nLineWidth; }

    BoardID GetSerialNumber() const { return m_nSerialNum; }
    void SetSerialNumber(BoardID nSerialNum) { m_nSerialNum = nSerialNum; }

    DWORD GetOwnerMask() const { return m_dwOwnerMask; }
    void SetOwnerMask(DWORD dwMask) { m_dwOwnerMask = dwMask; }
    BOOL IsOwned() const { return m_dwOwnerMask != 0; }
    BOOL IsOwnedBy(DWORD dwMask) const { return (BOOL)(m_dwOwnerMask & dwMask); }
    BOOL IsOwnedButNotByCurrentPlayer(const CGamDoc& pDoc) const;

    void PropagateOwnerMaskToAllPieces();

    BOOL IsNonOwnerAccessAllowed() const { return m_bNonOwnerAccess; }
    void SetNonOwnerAccess(BOOL bAllowAccess) { m_bNonOwnerAccess = bAllowAccess; }
    bool IsPrivate() const { return m_bPrivate; }
    void SetPrivate(bool b) { m_bPrivate = b; }

// Operations
public:
    void Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale);
    // ------- //
    CPieceObj& AddPiece(CPoint pnt, PieceID pid);
    const CPieceObj* FindPieceID(PieceID pid) const;
    CPieceObj* FindPieceID(PieceID pid)
    {
        return const_cast<CPieceObj*>(std::as_const(*this).FindPieceID(pid));
    }
    const CDrawObj* FindObjectID(ObjectID oid) const;
    CDrawObj* FindObjectID(ObjectID oid)
    {
        return const_cast<CDrawObj*>(std::as_const(*this).FindObjectID(oid));
    }
    //  void RemovePiece(CPieceObj* pObj);
    // ------- //
    void AddIndicatorObject(CDrawObj::OwnerPtr pObj);
    void FlushAllIndicators();
    // ------- //
    BOOL IsObjectOnBoard(const CDrawObj& pObj) const;
    void RemoveObject(const CDrawObj& pObj);
    void LimitRectToBoard(CRect& rct) const;
    // ------- //
    void ExpungePieceList();
    // ------- //
    OwnerPtr<CPlayBoard> Clone(CGamDoc& pDoc) const;
    void Restore(CGamDoc& pDoc, const CPlayBoard& pBrd);
    bool Compare(const CPlayBoard& pBrd) const;
    // ------- //
    bool NeedsGeoRotate() const;
    void Serialize(CArchive& ar);

    CB::string GetCellNumberStr(CPoint pnt, TileScale eScale) const;

// Vars...
public:
    BOOL    m_bGridSnap;        // TRUE if snap grid is on
    BOOL    m_bGridRectCenters; // TRUE if should snap to centers of rects
    uint32_t m_xGridSnap;        // X grid multiple * 1000
    uint32_t m_yGridSnap;        // Y grid multiple * 1000
    uint32_t m_xGridSnapOff;     // X grid offset (must be < m_xGridSnap) * 1000
    uint32_t m_yGridSnapOff;     // Y grid offset (must be < m_yGridSnap) * 1000

    BOOL    m_bOpenBoardOnLoad; // If TRUE automatically open board on project load
    BOOL    m_bShowSelListAndTinyMap;   // If TRUE, shows sel list and tiny map views in window

    BOOL    m_bSnapMovePlot;    // TRUE if plotted move point should lie on grid

    int     m_xStackStagger;    // Stacking stagger in x direction
    int     m_yStackStagger;    // Stacking stagger in y direction

    BoardID m_nSerialNum;       // GameBox board serial number

    BOOL    m_bLockedDrawnBeneath; // Locked objects are to rendered under other objects
    BOOL    m_bPVisible;        // Pieces are visible
    BOOL    m_bRotate180;       // Show board rotated by 180 degrees
    BOOL    m_bIVisible;        // Indicators are visible (move plot...)
    BOOL    m_bIndOnTop;        // Indicators are drawn on top of pieces.
    BOOL    m_bEnforceLocks;    // Enforce locked objects
    int     m_bCellBorders;     // Cells have borders.
    int     m_bSmallCellBorders;// Small map cells have borders.

    COLORREF m_crPlotLineColor; // Move plot line color.
    UINT     m_nPlotLineWidth;  // Move plot line width

    COLORREF m_crLineColor;     // Line color
    UINT     m_nLineWidth;      // Line width

    COLORREF m_crTextColor;     // Text color
    COLORREF m_crTextBoxColor;  // Text box background color
    // allow default move operations to work right
public:
    FontID   m_fontID;          // Text font

    DWORD    m_dwOwnerMask;     // Who can change it
    BOOL     m_bNonOwnerAccess; // Allow non-owner access to owned stuff.
                                // ..Visiblity is still enforced.

    // allow default move operations to work right
private:
    // extra cleanup needed
    class DeleteGeoBoard
    {
    public:
        void operator()(CGeomorphicBoard* p) const;
    };
    using OwnerOrNullGeomorphicBoard = CB::propagate_const<std::unique_ptr<CGeomorphicBoard, DeleteGeoBoard>>;
    OwnerOrNullGeomorphicBoard m_pGeoBoard; // If not NULL. Board is generated on each load

// Implementation
protected:
    void Clear();
    CBoard& CreateGeoBoard();

// Implementation
protected:
    CPoint  m_ptPrevPlot;       // Previous selected move point
    BOOL    m_bPlotMode;        // Plot move mode

    OwnerOrNullPtr<CDrawList> m_pPceList;      // Piece draw list
    OwnerOrNullPtr<CDrawList> m_pIndList;      // Indicator draw list.

    // don't show board to non-owner
    bool    m_bPrivate;

    // For reference only...
    CB::propagate_const<CBoard*> m_pBoard;       // Loaded from Game Box
    RefPtr<CGamDoc> m_pDoc;         // Document pointer
};

//////////////////////////////////////////////////////////////////

class CPBoardManager : private std::vector<OwnerPtr<CPlayBoard>>
{
public:
    CPBoardManager(CGamDoc& doc);
    CPBoardManager(const CPBoardManager&) = delete;
    CPBoardManager& operator=(const CPBoardManager&) = delete;
    ~CPBoardManager() { DestroyAllElements(); }

// Attributes
public:
    const CBoardManager* GetBoardManager() const { return &m_pDoc->GetBoardManager(); }
    size_t GetNumPBoards() const { return size(); }
    bool IsEmpty() const { return empty(); }
    const CPlayBoard& GetPBoard(size_t nBrd) const { return *at(nBrd); }
    CPlayBoard& GetPBoard(size_t nBrd) { return const_cast<CPlayBoard&>(std::as_const(*this).GetPBoard(nBrd)); }
    BoardID IssueGeoSerialNumber();

// Operations
public:
    bool GetPBoardList(std::vector<BoardID>& tblBrds) const;
    void SetPBoardList(const std::vector<BoardID>& tblBrds);
    void AddBoard(BoardID nSerialNum, BOOL bInheritSettings = TRUE);
    void AddBoard(CBoard& pBoard, BOOL bInheritSettings = TRUE);
    void AddBoard(OwnerPtr<CGeomorphicBoard> pGeoBoard, BOOL bInheritSettings = TRUE);
    void DeletePBoard(size_t nBrd);
    void FindPBoardsNotInList(const std::vector<BoardID>& tblBrdSerNum, std::vector<CB::not_null<CPlayBoard*>>& tblNotInList);

    void ClearAllOwnership();
    void PropagateOwnerMaskToAllPieces();

    size_t FindPBoardByRef(const CPlayBoard& pPBrd) const;
    size_t FindPBoardBySerial(BoardID nSerialNum) const;
    const CPlayBoard* GetPBoardBySerial(BoardID nSerialNum) const;
    CPlayBoard* GetPBoardBySerial(BoardID nSerialNum)
    {
        return const_cast<CPlayBoard*>(std::as_const(*this).GetPBoardBySerial(nSerialNum));
    }
    const CPlayBoard* FindObjectOnBoard(const CDrawObj& pObj) const;
    CPlayBoard* FindObjectOnBoard(const CDrawObj& pObj)
    {
        return const_cast<CPlayBoard*>(std::as_const(*this).FindObjectOnBoard(pObj));
    }
    const CPlayBoard* FindObjectOnBoard(ObjectID oid, const CDrawObj*& ppObj) const;
    CPlayBoard* FindObjectOnBoard(ObjectID oid, CDrawObj*& ppObj)
    {
        return const_cast<CPlayBoard*>(std::as_const(*this).FindObjectOnBoard(oid, const_cast<const CDrawObj*&>(ppObj)));
    }
    const CPlayBoard* FindPieceOnBoard(PieceID pid, const CPieceObj*& ppObj) const;
    CPlayBoard* FindPieceOnBoard(PieceID pid, CPieceObj*& ppObj)
    {
        return const_cast<CPlayBoard*>(std::as_const(*this).FindPieceOnBoard(pid, const_cast<const CPieceObj*&>(ppObj)));
    }
    const CPlayBoard* FindPieceOnBoard(PieceID pid) const
    {
        const CPieceObj* dummy;
        return FindPieceOnBoard(pid, dummy);
    }

    OwnerPtr<CDrawObj> RemoveObjectID(ObjectID oid);
    // ------- //
    OwnerPtr<CPBoardManager> Clone(CGamDoc& pDoc) const;
    void Restore(CGamDoc& pDoc, const CPBoardManager& pMgr);
    bool Compare(const CPBoardManager& pMgr) const;
    // ------- //
    void Serialize(CArchive& ar);

// Implementation - methods
protected:
    void DestroyAllElements();

// Implementation - vars
protected:
    RefPtr<CGamDoc> m_pDoc;

    BoardID m_nNextGeoSerialNum;        // Next geomorphic board serial number to issue (WAS m_wReserved1)
    // WORD m_wReserved1;           // For future need (set to 0) // Now is m_nNextGeoSerialNum
    WORD    m_wReserved2;           // For future need (set to 0)
    WORD    m_wReserved3;           // For future need (set to 0)
    WORD    m_wReserved4;           // For future need (set to 0)
};

#endif

