// PalMark.h : header file
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
class CMarkerPaletteContainer;
class CDockMarkPalette;
class WinStateRestoreEvent;

/////////////////////////////////////////////////////////////////////////////
// CMarkerPalette window - Marker palette's are part of the document object.

class CMarkerPalette : public wxPanel
{
// Construction
public:
    CMarkerPalette(CMarkerPaletteContainer& container, CGamDoc& pDoc);
    BOOL Create();

// Attributes
public:

// Operations
public:
    size_t GetSelectedMarkerGroup() const;

    void UpdatePaletteContents();

    void SelectMarker(MarkID mid);

    void Serialize(CArchive &ar);

// Implementation
protected:
    RefPtr<CMarkerPaletteContainer> m_pContainer;
    RefPtr<CGamDoc> m_pDoc;

    // This dummy area only contains a single entry. It is used
    // when only single entry should be shown in the Tray listbox.
    // This is pretty much a hack but is was easier than reworking
    // CGrafixListBox to support this oddball situation.
    std::vector<MarkID> m_dummyArray;

    // Enclosed controls....
    CB::propagate_const<wxChoice*> m_comboMGrp;
    CB::propagate_const<CMarkListBoxWx*> m_listMark;

    void LoadMarkerNameList();
    void UpdateMarkerList();

    int  FindMarkerGroupIndex(size_t nGroupNum) const;

    // Some temporary vars used during windows position restoration.
    // They are loaded during the de-serialization process.
    BOOL m_bStateVarsArmed;                         // Set so state restore is one-shot process
    uint32_t  m_nComboIndex;
    uint32_t  m_nListTopindex;
    uint32_t  m_nListCurSel;
    int  m_nComboHeight;

// Implementation
public:

protected:
#if 0
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
#endif
    void OnMarkerNameCbnSelchange(wxCommandEvent& /*event*/);
    void OnOverrideSelectedItem(OverrideSelectedItemEvent& event);
    void OnGetDragSize(GetDragSizeEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#endif
    void OnPaletteHide(wxCommandEvent& event);
    void OnMessageRestoreWinState(WinStateRestoreEvent& event);

    wxDECLARE_EVENT_TABLE();
};

class CMarkerPaletteContainer : public CWnd,
                                public CB::wxNativeContainerWindowMixin
{
public:
    CMarkerPaletteContainer(CGamDoc& pDoc);
    BOOL Create(CWnd& pOwnerWnd/*, DWORD dwStyle = 0, UINT nID = 0*/);

#if 0
    void PostNcDestroy() override;
#endif

    operator const CMarkerPalette&() const { return *child; }
    operator CMarkerPalette&()
    {
        return const_cast<CMarkerPalette&>(static_cast<const CMarkerPalette&>(std::as_const(*this)));
    }
    const CMarkerPalette* operator->() const { return &static_cast<const CMarkerPalette&>(*this); }
    CMarkerPalette* operator->()
    {
        return const_cast<CMarkerPalette*>(std::as_const(*this).operator->());
    }

    const CDockMarkPalette* GetDockingFrame() const { return m_pDockingFrame.get(); }
    CDockMarkPalette* GetDockingFrame()
    {
        return const_cast<CDockMarkPalette*>(std::as_const(*this).GetDockingFrame());
    }
    void SetDockingFrame(CDockMarkPalette* pDockingFrame);

    void Serialize(CArchive& ar) override { wxASSERT(false); AfxThrowNotSupportedException(); }

private:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

    CB::propagate_const<CDockMarkPalette*> m_pDockingFrame = nullptr;
    // owned by wx
    CB::propagate_const<CMarkerPalette*> child = nullptr;
};

/////////////////////////////////////////////////////////////////////////////

#endif

