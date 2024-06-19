// VwBitedt.h : implementation file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

#include    <deque>
#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _TOOLIMAG_H
#include    "ToolImag.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitEditView view

class CTileSelView;

class CBitEditView : public wxScrolledCanvas
{
    friend class CBitEditViewContainer;
    friend class CTileSelViewContainer;
protected:
    CBitEditView(CBitEditViewContainer& parent);

// Attributes
public:
    const CGamDoc& GetDocument() const { return *document; }
    CGamDoc& GetDocument()
    {
        return const_cast<CGamDoc&>(std::as_const(*this).GetDocument());
    }
    void SetTileSelectView(CTileSelView& pSelView) { m_pSelView = &pSelView; }
    const CTileSelView& GetTileSelectView() const { return CheckedDeref(m_pSelView); }
    CTileSelView& GetTileSelectView()
    {
        return const_cast<CTileSelView&>(std::as_const(*this).GetTileSelectView());
    }
    void SetCurrentBitmap(TileID tid, const wxBitmap& pBMap, BOOL bFillOnly = FALSE);
    const wxBitmap& GetCurrentViewBitmap() const;
    const wxBitmap& GetCurrentMasterBitmap() const { return m_bmMaster; }
    wxBitmap& GetCurrentMasterBitmap()
    {
        return const_cast<wxBitmap&>(std::as_const(*this).GetCurrentMasterBitmap());
    }
    const wxBitmap& GetPasteBitmap() const { return m_bmPaste; }
    wxBitmap& GetPasteBitmap()
    {
        return const_cast<wxBitmap&>(std::as_const(*this).GetPasteBitmap());
    }
    wxSize GetBitmapSize() const { return m_size; }
    wxSize GetZoomedSize() const
        { return wxSize(m_size.x * m_nZoom, m_size.y * m_nZoom); }
    wxRect GetZoomedSelectBorderRect() const;
    wxRect GetZoomedSelectRect() const;
    void SetSelectRect(const wxRect& rct) { m_rctPaste = rct; }
    wxRect GetSelectRect() const { return m_rctPaste; }
    void OffsetSelectRect(wxSize size) { m_rctPaste.Offset(size.x, size.y); }
    void SetSelectToolControl(BOOL bCapture) { m_bSelectCapture = bCapture; }

    FontID UpdateBitFont();
    // Current Colors
    UINT GetLineWidth() const { return m_pTMgr->GetLineWidth(); }
    wxColour GetForeColor() const { return CB::Convert(m_pTMgr->GetForeColor()); }
    wxColour GetBackColor() const { return CB::Convert(m_pTMgr->GetBackColor()); }
    void SetForeColor(wxColour crFore) { m_pTMgr->SetForeColor(CB::Convert(crFore)); }
    void SetBackColor(wxColour crBack) { m_pTMgr->SetBackColor(CB::Convert(crBack)); }

// Operations
public:
    BOOL GetImagePixelLoc(wxPoint& point) const;   // degrids it also
    void GetImagePixelLocClamped(wxPoint& point) const;
    void SetViewImageFromMasterImage();
    void SetMasterImageFromViewImage();
    void ClearPasteImage();
    void InvalidateViewImage(bool bUpdate);
    void InvalidateFocusBorder();
    BOOL IsPtInImage(wxPoint point) const;
    BOOL IsPtInSelectRect(wxPoint point) const;
    BOOL IsPasteImage() const { return m_bmPaste.IsOk(); }
    void RestoreLastTool() { m_nCurToolID = m_nLastToolID; }

    // Draw tools support
    void DrawImagePixel(wxPoint point, UINT nSize);
    void DrawImageToPixel(wxPoint prvPt, wxPoint curPt, UINT nSize);
    void DrawImageLine(wxPoint startPt, wxPoint curPt, UINT nSize);
    void DrawImageRect(wxPoint startPt, wxPoint curPt, UINT nSize);
    void DrawImageEllipse(wxPoint startPt, wxPoint curPt, UINT nSize);
    void DrawImageFill(wxPoint pt);
    void DrawImageChangeColor(wxPoint pt);
    void DrawImageSelectRect(wxPoint startPt, wxPoint curPt);
    void DrawPastedImage();

    // Coordinate space
    void ClientToWorkspace(wxPoint& pnt) const;
    [[nodiscard]] wxPoint ClientToWorkspace(const wxPoint& pnt) const
    {
        wxPoint retval = pnt;
        ClientToWorkspace(retval);
        return retval;
    }
    void WorkspaceToClient(wxPoint& pnt) const;
    [[nodiscard]] wxPoint WorkspaceToClient(const wxPoint& pnt) const
    {
        wxPoint retval = pnt;
        WorkspaceToClient(retval);
        return retval;
    }
    void WorkspaceToClient(wxRect& rct) const;

    // Text edit support.
    int CalcCaretHeight(int yLoc) const;
    void SetTextPosition(wxPoint ptPos);
    void SetTextCaretPos(wxPoint ptPos);
    void CommitCurrentText();
    void FixupTextCaret();
    void UpdateTextView();
    void AddChar(wchar_t nChar);
    void DelChar();
    void UpdateFontInfo();
    // Undo support
    void SetUndoFromView();
    void RestoreUndoToView();
    void PurgeUndo();
    BOOL IsUndoAvailable() const { return !m_listUndo.empty(); }

// Implementation
protected:
    void ClearAllImages();
    void RecalcScrollLimits();
    wxRect GetImageRect() const;
    static IToolType MapToolType(int nToolResID);

    // ------ //
    FontID      m_fontID;       // Current fontID Shadow variable
    int         m_tmHeight;     // Font's height
    // ------ //
    int         m_nCurToolID;   // Current tool ID
    int         m_nLastToolID;  // Previous tool ID
    BOOL        m_bFillOnly;    // Only allow fills
    // ------ //
    UINT        m_nZoom;        // 1, 2, 6, 8
    BOOL        m_bGridVisible; // TRUE if grid visible in zoom 6 and 8
    TileID      m_tid;          // Used for image update broadcasts
    wxSize      m_size;         // Size of bitmap
    wxPoint     m_ptCaret;      // Caret position within bitmap (x=-1, disable)
    wxPoint     m_ptText;       // Text position within bitmap
    int         m_nTxtExtent;   // Length of text in pixels
    // ------ //
    CB::string  m_strText;      // Text editing.
    // ------ //
    wxBitmap    m_bmMaster;     // master (current) bitmap image
    wxBitmap    m_bmView;       // bitmap shown in edit window
    // ------ //
    wxBitmap    m_bmPaste;      // bitmap being pasted or moved
    wxRect      m_rctPaste;     // rect surrounding the paste bitmap
    BOOL        m_bSelectCapture;// Select tool has mouse captured
    // ------ //
    // Undo FIFO
    std::deque<wxBitmap> m_listUndo;     // List of undo bitmaps
    // ------ //
    CTileSelView* m_pSelView;
    CTileManager* m_pTMgr;
protected:
    ~CBitEditView() override;
    void OnDraw(wxDC& dc) override;
    void OnInitialUpdate();
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
#if 0
    void OnActivateView(BOOL bActivate, CView *pActivateView,
        CView* pDeactivateView) override;

    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
#endif

#if 0
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
    void OnImageGridLines(wxCommandEvent& event);
    void OnToolPalette(wxCommandEvent& event);
    void OnSetColor(SetColorEvent& event);
    void OnSetCustomColors(SetCustomColorEvent& event);
    void OnSetLineWidth(SetLineWidthEvent& event);
    void OnUpdateImageGridLines(wxUpdateUIEvent& pCmdUI);
    void OnUpdateToolPalette(wxUpdateUIEvent& pCmdUI);
    void OnUpdateColorForeground(wxUpdateUIEvent& pCmdUI);
    void OnUpdateColorBackground(wxUpdateUIEvent& pCmdUI);
    void OnUpdateColorTransparent(wxUpdateUIEvent& pCmdUI);
    void OnUpdateColorCustom(wxUpdateUIEvent& pCmdUI);
    void OnUpdateLineWidth(wxUpdateUIEvent& pCmdUI);
    void OnLButtonDown(wxMouseEvent& event);
    void OnLButtonUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnSetCursor(wxSetCursorEvent& event);
#if 0
    afx_msg void OnImageBoardMask();
#endif
    void OnViewZoomIn(wxCommandEvent& event);
    void OnUpdateViewZoomIn(wxUpdateUIEvent& pCmdUI);
    void OnViewZoomOut(wxCommandEvent& event);
    void OnUpdateViewZoomOut(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnUpdateImageBoardMask(CCmdUI* pCmdUI);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
#endif
    void OnDwgFont(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnEditUndo(wxCommandEvent& event);
    void OnUpdateEditUndo(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
#endif
    void OnUpdateIndicatorCellNum(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    void OnViewToggleScale(wxCommandEvent& event);
    void OnUpdateEnable(wxUpdateUIEvent& pCmdUI);
    wxDECLARE_EVENT_TABLE();

private:
    RefPtr<CBitEditViewContainer> parent;
    RefPtr<CGamDoc> document;
};

class CBitEditViewContainer : public CB::OnCmdMsgOverride<CView>,
                                public CB::wxNativeContainerWindowMixin
{
public:
    const CBitEditView& GetChild() const { return CheckedDeref(child); }
    CBitEditView& GetChild()
    {
        return const_cast<CBitEditView&>(std::as_const(*this).GetChild());
    }
    void OnDraw(CDC* pDC) override;
    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

protected:
    void OnActivateView(BOOL bActivate, CView *pActivateView,
        CView* pDeactivateView) override;

private:
    CBitEditViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CBitEditViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    DECLARE_MESSAGE_MAP()

    // IGetEventHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CBitEditView*> child = nullptr;

    friend CBitEditView;
};

/////////////////////////////////////////////////////////////////////////////
#endif

