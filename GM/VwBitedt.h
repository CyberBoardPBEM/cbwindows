// VwBitedt.h : implementation file
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

#ifndef _VWBITEDT_H
#define _VWBITEDT_H

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _TOOLIMAG_H
#include    "ToolImag.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitEditView view

class CTileSelView;

class CBitEditView : public CScrollView
{
    friend class CTileSelView;
    DECLARE_DYNCREATE(CBitEditView)
protected:
    CBitEditView();         // Protected constructor used by dynamic creation

// Attributes
public:
    CGamDoc* GetDocument() { return (CGamDoc*)CScrollView::GetDocument(); }
    void SetTileSelectView(CTileSelView* pSelView) { m_pSelView = pSelView; }
    CTileSelView* GetTileSelectView() { return m_pSelView; }
    void SetCurrentBitmap(TileID tid, CBitmap *pBMap, BOOL bFillOnly = FALSE);
    CBitmap* GetCurrentViewBitmap();
    CBitmap* GetCurrentMasterBitmap() { return &m_bmMaster; }
    CBitmap* GetPasteBitmap() { return &m_bmPaste; }
    CSize GetBitmapSize() { return m_size; }
    CSize GetZoomedSize()
        { return CSize(m_size.cx * m_nZoom, m_size.cy * m_nZoom); }
    CRect GetZoomedSelectBorderRect();
    CRect GetZoomedSelectRect();
    void SetSelectRect(CRect& rct) { m_rctPaste = rct; }
    CRect GetSelectRect() { return m_rctPaste; }
    void OffsetSelectRect(CSize size) { m_rctPaste += (CPoint)size; }
    void SetSelectToolControl(BOOL bCapture) { m_bSelectCapture = bCapture; }

    FontID GetBitFont();
    // Current Colors
    UINT GetLineWidth() { return m_pTMgr->GetLineWidth(); }
    COLORREF GetForeColor() { return m_pTMgr->GetForeColor(); }
    COLORREF GetBackColor() { return m_pTMgr->GetBackColor(); }
    void SetForeColor(COLORREF crFore) { m_pTMgr->SetForeColor(crFore); }
    void SetBackColor(COLORREF crBack) { m_pTMgr->SetForeColor(crBack); }

// Operations
public:
    BOOL GetImagePixelLoc(CPoint& point);   // degrids it also
    void GetImagePixelLocClamped(CPoint& point);
    CBitmap* SetViewImageFromMasterImage();
    CBitmap* SetMasterImageFromViewImage();
    void ClearPasteImage();
    void InvalidateViewImage(CRect* pRct, BOOL bUpdate = FALSE);
    void InvalidateFocusBorder(BOOL bUpdate = FALSE);
    BOOL IsPtInImage(CPoint point);
    BOOL IsPtInSelectRect(CPoint point);
    BOOL IsPasteImage() { return m_bmPaste.m_hObject != NULL; }
    void RestoreLastTool() { m_nCurToolID = m_nLastToolID; }

    // Draw tools support
    void DrawImagePixel(CPoint point, UINT nSize);
    void DrawImageToPixel(CPoint prvPt, CPoint curPt, UINT nSize);
    void DrawImageLine(CPoint startPt, CPoint curPt, UINT nSize);
    void DrawImageRect(CPoint startPt, CPoint curPt, UINT nSize);
    void DrawImageEllipse(CPoint startPt, CPoint curPt, UINT nSize);
    void DrawImageFill(CPoint pt);
    void DrawImageChangeColor(CPoint pt);
    void DrawImageSelectRect(CPoint startPt, CPoint curPt);
    void DrawPastedImage();

    // Coordinate space
    void ClientToWorkspace(CPoint& pnt);
    void WorkspaceToClient(CPoint& pnt);
    void WorkspaceToClient(CRect& rct);

    // Text edit support.
    int CalcCaretHeight(int yLoc);
    void SetTextPosition(CPoint ptPos);
    void SetTextCaretPos(CPoint ptPos);
    void CommitCurrentText();
    void FixupTextCaret();
    void UpdateTextView();
    void AddChar(char nChar);
    void DelChar();
    void UpdateFontInfo();
    // Undo support
    void SetUndoFromView();
    void RestoreUndoToView();
    void PurgeUndo();
    BOOL IsUndoAvailable() { return m_listUndo.GetCount() > 0; }

// Implementation
protected:
    void ClearAllImages();
    void RecalcScrollLimits();
    CRect GetImageRect();
    IToolType MapToolType(UINT nToolResID);

    // ------ //
    FontID      m_fontID;       // Current fontID Shadow variable
    int         m_tmHeight;     // Font's height
    // ------ //
    UINT        m_nCurToolID;   // Current tool ID
    UINT        m_nLastToolID;  // Previous tool ID
    BOOL        m_bFillOnly;    // Only allow fills
    // ------ //
    UINT        m_nZoom;        // 1, 2, 6, 8
    BOOL        m_bGridVisible; // TRUE if grid visible in zoom 6 and 8
    TileID      m_tid;          // Used for image update broadcasts
    CSize       m_size;         // Size of bitmap
    CPoint      m_ptCaret;      // Caret position within bitmap (x=-1, disable)
    CPoint      m_ptText;       // Text position within bitmap
    int         m_nTxtExtent;   // Length of text in pixels
    // ------ //
    CString     m_strText;      // Text editing.
    // ------ //
    CBitmap     m_bmMaster;     // master (current) bitmap image
    CBitmap     m_bmView;       // bitmap shown in edit window
    // ------ //
    CBitmap     m_bmPaste;      // bitmap being pasted or moved
    CRect       m_rctPaste;     // rect surrounding the paste bitmap
    BOOL        m_bSelectCapture;// Select tool has mouse captured
    // ------ //
    // Undo FIFO
    CObList     m_listUndo;     // List of undo bitmaps
    // ------ //
    CTileSelView* m_pSelView;
    CTileManager* m_pTMgr;
protected:
    virtual ~CBitEditView();
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual void OnInitialUpdate();     // first time after construct
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnActivateView(BOOL bActivate, CView *pActivateView,
        CView* pDeactivateView);

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // Generated message map functions
    //{{AFX_MSG(CBitEditView)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnImageGridLines();
    afx_msg BOOL OnToolPalette(UINT id);
    afx_msg LRESULT OnSetColor(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCustomColors(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetLineWidth(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUpdateImageGridLines(CCmdUI* pCmdUI);
    afx_msg void OnUpdateToolPalette(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorForeground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorBackground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorTransparent(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorCustom(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLineWidth(CCmdUI* pCmdUI);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnImageBoardMask();
    afx_msg void OnViewZoomIn();
    afx_msg void OnUpdateViewZoomIn(CCmdUI* pCmdUI);
    afx_msg void OnViewZoomOut();
    afx_msg void OnUpdateViewZoomOut(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImageBoardMask(CCmdUI* pCmdUI);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnDwgFont();
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorCellNum(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnViewToggleScale();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif

