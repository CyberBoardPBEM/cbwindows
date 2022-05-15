// PalTray.h : header file
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

#ifndef _PALTRAY_H
#define _PALTRAY_H

#ifndef     _LBOXTRAY_H
#include    "LBoxTray.h"
#endif

#ifndef     _WSTATEGP_H
#include    "WStateGp.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;
class CTraySet;

/////////////////////////////////////////////////////////////////////////////
// CTrayPalette window - Tray palette's are owned and reside in
// the document object.

class CTrayPalette : public CWnd
{
    DECLARE_DYNAMIC(CTrayPalette)

// Construction
public:
    CTrayPalette(CGamDoc& pDoc);

    BOOL Create(CWnd* pOwnerWnd, DWORD dwStyle = 0, UINT nID = 0);

// Attributes
public:
    void SetPaletteID(UINT nID) { m_nID = nID; }

    CDockablePane* GetDockingFrame() { return m_pDockingFrame; }
    void SetDockingFrame(CDockablePane* pDockingFrame)
    {
        m_pDockingFrame = pDockingFrame;
        SetParent(pDockingFrame);
    }

// Operations
public:
    void DeselectAll();
    void SelectTrayPiece(size_t nGroup, PieceID pid, LPCTSTR pszNotificationTip = NULL);
    void ShowTrayIndex(size_t nGroup, int nPos);

    void UpdatePaletteContents(const CTraySet* pTray = NULL);
    void Serialize(CArchive &ar);

// Implementation - vars
protected:
    RefPtr<CGamDoc> m_pDoc;
    UINT        m_nID;
    CDockablePane* m_pDockingFrame;

    CBitmap     m_bmpMenuBtn;
    CSize       m_sizeMenuBtn;
    CToolTipCtrl m_toolTipMenu;
    CToolTipCtrl m_toolTipCombo;                    // For combobox overlay

    // This dummy area only contains a single entry. It is used
    // when only single entry should be shown in the Tray listbox.
    // This is pretty much a hack but is was easier than reworking
    // CGrafixListBox to support this oddball situation.
    std::vector<PieceID> m_dummyArray;

    // Enclosed controls....
    CComboBox    m_comboYGrp;
    CTrayListBox m_listTray;
    CRect        m_rctMenuBtn;                      // phony menu button dims
    int          m_nComboHeight;

    void LoadTrayNameList();
    void UpdateTrayList();
    size_t GetSelectedTray();
    int  FindTrayIndex(size_t nTrayNum);

    // Some temporary vars used during windows position restoration.
    // They are loaded during the de-serialization process.
    BOOL m_bStateVarsArmed;                         // Set so state restore is one-shot process
    int  m_nComboIndex;
    int  m_nListTopindex;

    CWinPlacement     m_wndPlace;
    CArray<int, int>  m_tblListBoxSel;

// Overrides
public:
    virtual void PostNcDestroy();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Implementation - methods
protected:
    void LoadMenuButtonBitmap();
    void DoEditSelectedPieceText();
    BOOL EnsureTooltipExistance();

// Generated message map functions
protected:

    //{{AFX_MSG(CTrayPalette)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnTrayNameCbnSelchange();
    afx_msg void OnTrayListDoubleClick();
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOverrideSelectedItemList(WPARAM wParam, LPARAM lParam);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnPieceTrayShuffle();
    afx_msg void OnUpdatePieceTrayShuffle(CCmdUI* pCmdUI);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPieceTrayShuffleSelected();
    afx_msg void OnUpdatePieceTrayShuffleSelected(CCmdUI* pCmdUI);
    afx_msg void OnEditElementText();
    afx_msg void OnUpdateEditElementText(CCmdUI* pCmdUI);
    afx_msg void OnActTurnOver();
    afx_msg void OnUpdateActTurnOver(CCmdUI* pCmdUI);
    afx_msg void OnActTurnoverAllPieces();
    afx_msg void OnUpdateActTurnoverAllPieces(CCmdUI* pCmdUI);
    afx_msg void OnPieceTrayAbout();
    afx_msg void OnUpdatePieceTrayAbout(CCmdUI* pCmdUI);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    afx_msg BOOL OnToolTipShow(UINT id, NMHDR *pNMH, LRESULT *pResult);
    afx_msg LRESULT OnMessageRestoreWinState(WPARAM, LPARAM);
    afx_msg LRESULT OnPaletteHide(WPARAM, LPARAM);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif

