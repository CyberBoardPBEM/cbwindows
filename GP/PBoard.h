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
    CPlayBoard();
    ~CPlayBoard();

// Attributes
public:
    void SetDocument(CGamDoc* pDoc) { m_pDoc = pDoc; }

    CDrawList* GetPieceList() { return m_pPceList; }
    CDrawList* GetIndicatorList() { return m_pIndList; }
    BOOL GetPlotMoveMode() { return m_bPlotMode; }
    void SetPlotMoveMode(BOOL bPlotMode) { m_bPlotMode = bPlotMode; }
    void InitPlotStartPoint() { m_ptPrevPlot = CPoint(-1, -1); }
    CPoint GetPrevPlotPoint() { return m_ptPrevPlot; }
    void SetPrevPlotPoint(CPoint pnt) { m_ptPrevPlot = pnt; }

    void SetPiecesVisible(BOOL bVisible = TRUE) { m_bPVisible = bVisible; }
    BOOL GetPiecesVisible() { return m_bPVisible; }

    void SetDrawLockedBeneath(BOOL bDrawBeneath = TRUE) { m_bLockedDrawnBeneath = bDrawBeneath; }
    BOOL GetDrawLockedBeneath() { return m_bLockedDrawnBeneath; }

    void SetRotateBoard180(BOOL bRotate) { m_bRotate180 = bRotate; }
    BOOL IsBoardRotated180() { return m_bRotate180; }

    void SetIndicatorsOnTop(BOOL bDrawOnTop = TRUE) { m_bIndOnTop = bDrawOnTop; }
    BOOL GetIndicatorsOnTop() { return m_bIndOnTop; }

    void SetLocksEnforced(BOOL bEnforce = TRUE) { m_bEnforceLocks = bEnforce; }
    BOOL GetLocksEnforced() { return m_bEnforceLocks; }

    void SetBoard(CGeomorphicBoard* pGeoBoard, BOOL bInheritSettings = FALSE);
    void SetBoard(CBoard* pBoard, BOOL bInheritSettings = FALSE);
    CBoard* GetBoard() { return m_pBoard; }

    COLORREF GetLineColor() { return m_crLineColor; }
    UINT     GetLineWidth() { return m_nLineWidth; }

    int GetSerialNumber() { return m_nSerialNum; }
    void SetSerialNumber(int nSerialNum) { m_nSerialNum = nSerialNum; }

    DWORD GetOwnerMask() { return m_dwOwnerMask; }
    void SetOwnerMask(DWORD dwMask) { m_dwOwnerMask = dwMask; }
    BOOL IsOwned() { return m_dwOwnerMask != 0; }
    BOOL IsOwnedBy(DWORD dwMask) { return (BOOL)(m_dwOwnerMask & dwMask); }
    BOOL IsOwnedButNotByCurrentPlayer(CGamDoc* pDoc);

    void PropagateOwnerMaskToAllPieces(CGamDoc* pDoc);

    BOOL IsNonOwnerAccessAllowed() { return m_bNonOwnerAccess; }
    void SetNonOwnerAccess(BOOL bAllowAccess) { m_bNonOwnerAccess = bAllowAccess; }

// Operations
public:
    void Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale);
    // ------- //
    CPieceObj* AddPiece(CPoint pnt, PieceID pid);
    CPieceObj* FindPieceID(PieceID pid);
    CDrawObj* FindObjectID(ObjectID oid);
    //  void RemovePiece(CPieceObj* pObj);
    // ------- //
    void AddIndicatorObject(CDrawObj* pObj);
    void FlushAllIndicators();
    // ------- //
    BOOL IsObjectOnBoard(CDrawObj *pObj);
    void RemoveObject(CDrawObj* pObj);
    void LimitRectToBoard(CRect& rct);
    // ------- //
    void ExpungePieceList();
    // ------- //
    CPlayBoard* Clone(CGamDoc *pDoc);
    void Restore(CGamDoc *pDoc, CPlayBoard* pBrd);
    BOOL Compare(CPlayBoard* pBrd);
    // ------- //
    void Serialize(CArchive& ar);

// Vars...
public:
    BOOL    m_bGridSnap;        // TRUE if snap grid is on
    BOOL    m_bGridRectCenters; // TRUE if should snap to centers of rects
    int     m_xGridSnap;        // X grid multiple * 1000
    int     m_yGridSnap;        // Y grid multiple * 1000
    int     m_xGridSnapOff;     // X grid offset (must be < m_xGridSnap) * 1000
    int     m_yGridSnapOff;     // Y grid offset (must be < m_yGridSnap) * 1000

    BOOL    m_bOpenBoardOnLoad; // If TRUE automatically open board on project load
    BOOL    m_bShowSelListAndTinyMap;   // If TRUE, shows sel list and tiny map views in window

    BOOL    m_bSnapMovePlot;    // TRUE if plotted move point should lie on grid

    int     m_xStackStagger;    // Stacking stagger in x direction
    int     m_yStackStagger;    // Stacking stagger in y direction

    int     m_nSerialNum;       // GameBox board serial number

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
    FontID   m_fontID;          // Text font

    DWORD    m_dwOwnerMask;     // Who can change it
    BOOL     m_bNonOwnerAccess; // Allow non-owner access to owned stuff.
                                // ..Visiblity is still enforced.

    CGeomorphicBoard* m_pGeoBoard; // If not NULL. Board is generated on each load

// Implementation
protected:
    void Clear();
    CBoard* CreateGeoBoard();

// Implementation
protected:
    CPoint  m_ptPrevPlot;       // Previous selected move point
    BOOL    m_bPlotMode;        // Plot move mode

    CDrawList* m_pPceList;      // Piece draw list
    CDrawList* m_pIndList;      // Indicator draw list.

    // For reference only...
    CBoard*     m_pBoard;       // Loaded from Game Box
    CGamDoc*    m_pDoc;         // Document pointer
};

//////////////////////////////////////////////////////////////////

class CPBoardManager : public CPtrArray
{
public:
    CPBoardManager();
    ~CPBoardManager() { DestroyAllElements(); }

// Attributes
public:
    void SetDocument(CGamDoc* pDoc) { m_pDoc = pDoc; }
    void SetBoardManager(CBoardManager* pBMgr) { m_pBMgr = pBMgr; }
    CBoardManager* GetBoardManager() { return m_pBMgr; }
    int GetNumPBoards() { return GetSize(); }
    CPlayBoard* GetPBoard(int nBrd) { return (CPlayBoard*)GetAt(nBrd); }
    int  IssueGeoSerialNumber() { return m_nNextGeoSerialNum++; }

// Operations
public:
    BOOL GetPBoardList(CWordArray& tblBrds);
    void SetPBoardList(CWordArray& tblBrds);
    void AddBoard(int nSerialNum, BOOL bInheritSettings = TRUE);
    void AddBoard(CBoard* pBoard, BOOL bInheritSettings = TRUE);
    void AddBoard(CGeomorphicBoard* pGeoBoard, BOOL bInheritSettings = TRUE);
    void DeletePBoard(int nBrd);
    void FindPBoardsNotInList(CWordArray& tblBrdSerNum, CPtrArray& tblNotInList);

    void ClearAllOwnership();
    void PropagateOwnerMaskToAllPieces(CGamDoc* pDoc);

    int FindPBoardByPtr(CPlayBoard *pPBrd);
    int FindPBoardBySerial(int nSerialNum);
    CPlayBoard* GetPBoardBySerial(int nSerialNum);
    CPlayBoard* FindObjectOnBoard(CDrawObj* pObj);
    CPlayBoard* FindObjectOnBoard(ObjectID oid, CDrawObj** ppObj);
    CPlayBoard* FindPieceOnBoard(PieceID pid, CPieceObj** ppObj = NULL);

    CDrawObj* RemoveObjectID(ObjectID oid);    // Doesn't delete it
    // ------- //
    CPBoardManager* Clone(CGamDoc *pDoc);
    void Restore(CGamDoc *pDoc, CPBoardManager* pMgr);
    BOOL Compare(CPBoardManager* pMgr);
    // ------- //
    void Serialize(CArchive& ar);

// Implementation - methods
protected:
    void DestroyAllElements();

// Implementation - vars
protected:
    CGamDoc*        m_pDoc;
    CBoardManager*  m_pBMgr;

    int     m_nNextGeoSerialNum;        // Next geomorphic board serial number to issue (WAS m_wReserved1)
    // WORD m_wReserved1;           // For future need (set to 0) // Now is m_nNextGeoSerialNum
    WORD    m_wReserved2;           // For future need (set to 0)
    WORD    m_wReserved3;           // For future need (set to 0)
    WORD    m_wReserved4;           // For future need (set to 0)
};

#endif

