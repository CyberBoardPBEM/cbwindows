// PalMark.h : header file
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

#ifndef _PALMARK_H
#define _PALMARK_H

#ifndef     _LBOXMARK_H
#include    "LBoxMark.h"
#endif

#ifndef     _WSTATEGP_H
#include    "WStateGp.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////
// CMarkerPalette window - Marker palette's are part of the document object.

class CMarkerPalette : public CWnd
{
    DECLARE_DYNCREATE(CMarkerPalette)

// Construction
public:
    CMarkerPalette();
    BOOL Create(CWnd* pOwnerWnd, DWORD dwStyle = 0, UINT nID = 0);

// Attributes
public:
    void SetDocument(CGamDoc *pDoc);

// Operations
public:
    int  GetSelectedMarkerGroup();

    void UpdatePaletteContents();

    void SelectMarker(MarkID mid);

    void Serialize(CArchive &ar);

    CDockablePane* GetDockingFrame() { return m_pDockingFrame; }
    void SetDockingFrame(CDockablePane* pDockingFrame)
    {
        m_pDockingFrame = pDockingFrame;
        SetParent(pDockingFrame);
    }

// Implementation
protected:
    CGamDoc*    m_pDoc;
    CRect       m_rctPos;
    CDockablePane* m_pDockingFrame;

    // This dummy area only contains a single entry. It is used
    // when only single entry should be shown in the Tray listbox.
    // This is pretty much a hack but is was easier than reworking
    // CGrafixListBox to support this oddball situation.
    CWordArray  m_dummyArray;

    // Enclosed controls....
    CComboBox   m_comboMGrp;
    CMarkListBox m_listMark;

    void LoadMarkerNameList();
    void UpdateMarkerList();

    int  FindMarkerGroupIndex(int nGroupNum);

    // Some temporary vars used during windows position restoration.
    // They are loaded during the de-serialization process.
    BOOL m_bStateVarsArmed;                         // Set so state restore is one-shot process
    int  m_nComboIndex;
    int  m_nListTopindex;
    int  m_nListCurSel;
    int  m_nComboHeight;

    CWinPlacement m_wndPlace;

// Implementation
public:
    virtual void PostNcDestroy();

    // Generated message map functions
protected:
    //{{AFX_MSG(CMarkerPalette)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnMarkerNameCbnSelchange();
    afx_msg LRESULT OnOverrideSelectedItem(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    afx_msg LRESULT OnPaletteHide(WPARAM, LPARAM);
    afx_msg LRESULT OnMessageRestoreWinState(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif

