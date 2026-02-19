// PalTray.h : header file
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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

#ifndef _PALTRAY_H
#define _PALTRAY_H

#include    "Gp.h"
#ifndef     _LBOXTRAY_H
#include    "LBoxTray.h"
#endif

#ifndef     _WSTATEGP_H
#include    "WStateGp.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CDockTrayPalette;
class CGamDoc;
class CTrayPaletteContainer;
class CTraySet;

/////////////////////////////////////////////////////////////////////////////
// CTrayPalette window - Tray palette's are owned and reside in
// the document object.

class CTrayPalette : public wxPanel
{
    wxDECLARE_CLASS(CTrayPalette);

// Construction
public:
    CTrayPalette(CTrayPaletteContainer& container, CGamDoc& pDoc, UINT palID);

    BOOL Create(/*wxWindow& pOwnerWnd, DWORD dwStyle = 0, UINT nID = 0*/);

// Attributes
private:
    void SetPaletteID(UINT nID) { m_nID = nID; }
public:

// Operations
public:
    void DeselectAll();
    void SelectTrayPiece(size_t nGroup, PieceID pid, const CB::string* pszNotificationTip);
    void ShowTrayIndex(size_t nGroup, size_t nPos);

    void UpdatePaletteContents(const CTraySet* pTray = NULL);
    void Serialize(CArchive &ar);

// Implementation - vars
protected:
    RefPtr<CTrayPaletteContainer> m_pContainer;
    RefPtr<CGamDoc> m_pDoc;
    UINT        m_nID;
    CB::propagate_const<CDockTrayPalette*> m_pDockingFrame;

    wxBitmap     m_bmpMenuBtn;
    wxSize       m_sizeMenuBtn;
    CB::ToolTip  m_toolTipMenu;
    CB::ToolTip  m_toolTipCombo;                    // For combobox overlay

    // This dummy area only contains a single entry. It is used
    // when only single entry should be shown in the Tray listbox.
    // This is pretty much a hack but is was easier than reworking
    // CGrafixListBox to support this oddball situation.
    std::vector<PieceID> m_dummyArray;

    // Enclosed controls....
    CB::propagate_const<wxBitmapButton*> m_bpMenuBtn;
    CB::propagate_const<wxChoice*> m_comboYGrp;
    CB::propagate_const<CTrayListBoxWx*> m_listTray;
#if 0
    CRect        m_rctMenuBtn;                      // phony menu button dims
#endif
    int          m_nComboHeight;

    void LoadTrayNameList();
    void UpdateTrayList();
    size_t GetSelectedTray() const;
    int  FindTrayIndex(size_t nTrayNum) const;

    // Some temporary vars used during windows position restoration.
    // They are loaded during the de-serialization process.
    BOOL m_bStateVarsArmed;                         // Set so state restore is one-shot process
    uint32_t  m_nComboIndex;
    uint32_t  m_nListTopindex;

    std::vector<int>  m_tblListBoxSel;

// Overrides
public:
#if 0
    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
#endif

// Implementation - methods
protected:
    void LoadMenuButtonBitmap();
    void DoEditSelectedPieceText();
    BOOL EnsureTooltipExistance();

    void DoMenu(wxPoint point, bool rightButton);
private:
    GameElement menuGameElement = Invalid_v<GameElement>;

protected:

#if 0
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
#endif
    void OnTrayNameCbnSelchange(wxCommandEvent& event);
    void OnTrayListDoubleClick(wxCommandEvent& event);
    void OnDragItem(DragDropEvent& event);
#if 0
    afx_msg LRESULT OnOverrideSelectedItemList(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetDragSize(WPARAM wParam, LPARAM lParam);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPieceTrayShuffle();
    afx_msg void OnUpdatePieceTrayShuffle(CCmdUI* pCmdUI);
    afx_msg void OnPieceTrayShuffleSelected();
    afx_msg void OnUpdatePieceTrayShuffleSelected(CCmdUI* pCmdUI);
    afx_msg void OnEditElementText();
    afx_msg void OnUpdateEditElementText(CCmdUI* pCmdUI);
    afx_msg BOOL OnActTurnOver(UINT id);
    afx_msg void OnUpdateActTurnOver(CCmdUI* pCmdUI);
    afx_msg void OnPieceTrayAbout();
    afx_msg void OnUpdatePieceTrayAbout(CCmdUI* pCmdUI);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnToolTipShow(UINT id, NMHDR *pNMH, LRESULT *pResult);
#endif
    void OnMessageRestoreWinState(WinStateRestoreEvent& event);
    void OnPaletteHide(wxCommandEvent& event);
#if 0
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
#endif

    wxDECLARE_EVENT_TABLE();
};

class CTrayPaletteContainer : public CWnd,
                                public CB::wxNativeContainerWindowMixin
{
public:
    CTrayPaletteContainer(CGamDoc& pDoc, UINT palID);
    BOOL Create(CWnd& pOwnerWnd/*, DWORD dwStyle = 0, UINT nID = 0*/);

#if 0
    void PostNcDestroy() override;
#endif

    operator const CTrayPalette& () const { return *child; }
    operator CTrayPalette& ()
    {
        return const_cast<CTrayPalette&>(static_cast<const CTrayPalette&>(std::as_const(*this)));
    }
    const CTrayPalette* operator->() const { return &static_cast<const CTrayPalette&>(*this); }
    CTrayPalette* operator->()
    {
        return const_cast<CTrayPalette*>(std::as_const(*this).operator->());
    }

    const CDockTrayPalette* GetDockingFrame() const { return m_pDockingFrame.get(); }
    CDockTrayPalette* GetDockingFrame()
    {
        return const_cast<CDockTrayPalette*>(std::as_const(*this).GetDockingFrame());
    }
    void SetDockingFrame(CDockTrayPalette* pDockingFrame);

    void Serialize(CArchive& ar) override { wxASSERT(false); AfxThrowNotSupportedException(); }

private:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

    CB::propagate_const<CDockTrayPalette*> m_pDockingFrame = nullptr;
    // owned by wx
    CB::propagate_const<CTrayPalette*> child = nullptr;
};

/////////////////////////////////////////////////////////////////////////////

#endif

