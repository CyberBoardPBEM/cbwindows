// PalMark.cpp : implementation file
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

#include    "stdafx.h"
#include    "Gp.h"
#include    "GMisc.h"
#include    "FrmMain.h"
#include    "ResTbl.h"
#include    "Marks.h"
#include    "PalMark.h"
#include    "GamDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if 0
wxIMPLEMENT_CLASS(CMarkerPalette, wxPanel)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CMarkerPalette, wxPanel)
#if 0
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_WINDOWPOSCHANGING()
#endif
    EVT_CHOICE(XRCID("m_comboMGrp"), OnMarkerNameCbnSelchange)
    EVT_OVERRIDE_SELECTED_ITEM(OnOverrideSelectedItem)
    EVT_GET_DRAG_SIZE(OnGetDragSize)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CREATE()
#endif
    EVT_COMMAND(wxID_ANY, WM_PALETTE_HIDE_WX, OnPaletteHide)
    EVT_WINSTATE_RESTORE(OnMessageRestoreWinState)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CMarkerPaletteContainer, CWnd)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarkerPalette

CMarkerPalette::CMarkerPalette(CMarkerPaletteContainer& container, CGamDoc& pDoc) :
    m_pContainer(&container),
    m_pDoc(&pDoc),
    m_comboMGrp(nullptr),
    m_listMark(nullptr)
{
    m_dummyArray.push_back(MarkID(0));
    m_bStateVarsArmed = FALSE;
    m_nComboHeight = 0;
}

BOOL CMarkerPalette::Create(/*CWnd& pOwnerWnd, DWORD dwStyle, UINT nID*/)
{
    if (!CB::XrcLoad(*this, *m_pContainer, "CMarkerPalette"))
    {
        return false;
    }
    m_comboMGrp = XRCCTRL(*this, "m_comboMGrp", wxChoice);
    m_listMark = XRCCTRL(*this, "m_listMark", CMarkListBoxWx);
    (*m_pContainer)->Layout();

    m_listMark->EnableDrag(TRUE);
    m_listMark->SetDocument(&*m_pDoc);

    UpdatePaletteContents();

    // Queue up a message to finish up state restore.
    QueueEvent(WinStateRestoreEvent().Clone());
    return TRUE;
}

#if 0
int CMarkerPalette::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    CRect rctCombo;
    GetClientRect(&rctCombo);
    rctCombo.bottom = 10 * g_res.tm8ssb.tmHeight;

    if (!m_comboMGrp.Create(
        WS_CHILD | WS_VSCROLL | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT,
        rctCombo, this, IDC_W_MARKLIST))
    {
        TRACE("Failed to create Marker name combo-box\n");
        return -1;
    }
    m_comboMGrp.SetFont(CFont::FromHandle(g_res.h8ssb));
    m_comboMGrp.SetDroppedWidth(8 * g_res.tm8ssb.tmMaxCharWidth);

    m_comboMGrp.GetWindowRect(&rctCombo);   // Fetch result of create
    m_nComboHeight = rctCombo.Height();     // Save for later use

    m_comboMGrp.MoveWindow(0, 0, rctCombo.Width(), 10 * m_nComboHeight, TRUE);

    CRect rctLBox;
    GetClientRect(&rctLBox);
    rctLBox.top = m_nComboHeight;

    if (!m_listMark.Create(
        WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
            LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT,
        rctLBox, this, 0))
    {
        TRACE("Failed to create Marker list box.\n");
        return -1;
    }
    m_listMark.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    return 0;
}
#endif

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_WINSTATE_RESTORE. The
// message is posted during view initial update if the state of
// the windows should be restored.

void CMarkerPalette::OnMessageRestoreWinState(WinStateRestoreEvent& /*event*/)
{
    UpdatePaletteContents();

    if (!m_bStateVarsArmed)
        return;

    m_comboMGrp->SetSelection(m_nComboIndex == uint32_t(wxNOT_FOUND) ?
                                    wxNOT_FOUND
                                :
                                    value_preserving_cast<int>(m_nComboIndex));

    UpdateMarkerList();

    m_listMark->SetSelection(m_nListCurSel == uint32_t(wxNOT_FOUND) ?
                                    wxNOT_FOUND
                                :
                                    value_preserving_cast<int>(m_nListCurSel));
    m_listMark->ScrollToRow(value_preserving_cast<size_t>(m_nListTopindex));

    m_bStateVarsArmed = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::OnPaletteHide(wxCommandEvent& /*event*/)
{
    GetMainFrame()->SendMessage(WM_COMMAND, ID_VIEW_MARKERPAL);
}

/////////////////////////////////////////////////////////////////////////////

size_t CMarkerPalette::GetSelectedMarkerGroup() const
{
    int nSel = m_comboMGrp->GetSelection();
    if (nSel == wxNOT_FOUND)
        return Invalid_v<size_t>;
    return value_preserving_cast<size_t>(reinterpret_cast<uintptr_t>(m_comboMGrp->GetClientData(value_preserving_cast<unsigned>(nSel))));
}

int CMarkerPalette::FindMarkerGroupIndex(size_t nGroupNum) const
{
    if (m_comboMGrp->IsEmpty())
        return wxNOT_FOUND;
    for (size_t nIdx = size_t(0) ; nIdx < m_comboMGrp->GetCount() ; ++nIdx)
    {
        if (reinterpret_cast<uintptr_t>(m_comboMGrp->GetClientData(value_preserving_cast<unsigned>(nIdx))) == nGroupNum)
            return value_preserving_cast<int>(nIdx);
    }
    return wxNOT_FOUND;
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::OnOverrideSelectedItem(OverrideSelectedItemEvent& event)
{
    size_t nSel = GetSelectedMarkerGroup();
    if (nSel == Invalid_v<size_t>)
    {
        event.Skip();
        return;
    }

    CMarkManager& pMMgr = m_pDoc->GetMarkManager();
    CMarkSet& pMSet = pMMgr.GetMarkSet(nSel);
    if (pMSet.IsRandomMarkerPull())
    {
        OverrideSelectedItemEvent& oi = event;

        uint32_t nRandSeed = m_pDoc->GetRandomNumberSeed();

        int32_t nRandNum = CalcRandomNumberUsingSeed(0, value_preserving_cast<uint32_t>(pMSet.GetMarkIDTable().size()),
            nRandSeed, &nRandSeed);

        oi.ID<MarkID>() = pMSet.GetMarkIDTable().at(value_preserving_cast<size_t>(nRandNum));

        m_pDoc->SetRandomNumberSeed(nRandSeed);
    }
}

void CMarkerPalette::OnGetDragSize(GetDragSizeEvent& event)
{
    size_t nSel = GetSelectedMarkerGroup();
    if (nSel == Invalid_v<size_t>)
    {
        wxASSERT(!"bad tray");
        event.Skip();
        return;
    }
    CMarkManager& pMMgr = m_pDoc->GetMarkManager();
    CMarkSet& pMSet = pMMgr.GetMarkSet(nSel);

    std::vector<int> items;
    if (pMSet.IsRandomMarkerPull())
    {
        items.reserve(m_listMark->GetItemCount());
        for (size_t i = size_t(0) ; i < m_listMark->GetItemCount() ; ++i)
        {
            items.push_back(value_preserving_cast<int>(i));
        }
    }
    else
    {
        items.push_back(m_listMark->GetSelection());
    }

    CTileManager& tileMgr = m_pDoc->GetTileManager();
    wxSize retval(0, 0);
    for (int item : items)
    {
        MarkID mid = m_listMark->MapIndexToItem(value_preserving_cast<size_t>(item));
        MarkDef& pMark = pMMgr.GetMark(mid);
        wxASSERT(pMark.m_tid != nullTid);
        wxSize size = CB::Convert(tileMgr.GetTile(pMark.m_tid).GetSize());
        retval.x = std::max(retval.x, size.x);
        retval.y = std::max(retval.y, size.y);
    }

    event.SetSize(retval);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::SelectMarker(MarkID mid)
{
    size_t nGrp = m_pDoc->GetMarkManager().FindMarkInMarkSet(mid);
    wxASSERT(nGrp != Invalid_v<size_t>);
    size_t nSel = GetSelectedMarkerGroup();
    if (nSel != nGrp)
    {
        m_comboMGrp->SetSelection(FindMarkerGroupIndex(nGrp));
        UpdateMarkerList();
    }
    m_listMark->SelectMarker(mid);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(m_comboMGrp->GetSelection()));
        ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(m_listMark->GetVisibleRowsBegin()));
        ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(m_listMark->GetSelection()));
    }
    else
    {
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))          // V2.90
        {
            uint32_t dwTmp;
            ar >> dwTmp; m_nComboIndex = dwTmp;
            ar >> dwTmp; m_nListTopindex = dwTmp;
            ar >> dwTmp; m_nListCurSel = dwTmp;
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))       // V2.0
        {
            uint32_t dwTmp;
            ar >> dwTmp; m_nComboIndex = dwTmp;
            ar >> dwTmp; m_nListTopindex = dwTmp;
            ar >> dwTmp; m_nListCurSel = dwTmp;
            CWinPlacement wndSink;
            ar >> wndSink;                      // Eat this puppy
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else                                    // Pre V2.0 data
        {
            // Just eat the old data and go with the defaults
            uint16_t sTmp;
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::LoadMarkerNameList()
{
    CMarkManager& pMMgr = m_pDoc->GetMarkManager();

    m_comboMGrp->Clear();
    for (size_t i = size_t(0); i < pMMgr.GetNumMarkSets(); i++)
    {
        int nIdx = m_comboMGrp->Append(pMMgr.GetMarkSet(i).GetName());
        m_comboMGrp->SetClientData(value_preserving_cast<unsigned>(nIdx), reinterpret_cast<void*>(value_preserving_cast<uintptr_t>(i)));    // Store the marker index in the data item
    }
    m_comboMGrp->SetSelection(0);
    UpdateMarkerList();
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::UpdatePaletteContents()
{
    size_t nSel = GetSelectedMarkerGroup();
    if (nSel == Invalid_v<size_t>)
        nSel = 0;               // Force first entry (if any)
    LoadMarkerNameList();
    if (nSel < m_comboMGrp->GetCount())
        m_comboMGrp->SetSelection(FindMarkerGroupIndex(nSel));
    UpdateMarkerList();
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::UpdateMarkerList()
{
    CMarkManager& pMMgr = m_pDoc->GetMarkManager();

    size_t nSel = GetSelectedMarkerGroup();
    if (nSel == Invalid_v<size_t>)
    {
        m_listMark->SetItemMap(NULL);
        return;
    }

    CMarkSet& pMSet = pMMgr.GetMarkSet(nSel);
    const std::vector<MarkID>* pMarkTbl = &pMSet.GetMarkIDTable();

    CB::string str = "";
    if (pMSet.GetMarkerTrayContentVisibility() == mtrayVizEachGenericRandPull)
        str = CB::string::LoadString(IDS_TRAY_RANDHIDDEN);
    else if (pMSet.GetMarkerTrayContentVisibility() == mtrayVizNoneRandPull)
    {
        str = CB::string::LoadString(IDS_TRAY_ALLRANDHIDDEN);
        // Set the first (and only) element in the dummy array
        // if there are any markers in the group
        m_dummyArray.clear();
        if (!pMarkTbl->empty())
            m_dummyArray.push_back(pMarkTbl->front());
        pMarkTbl = &m_dummyArray;
    }
    m_listMark->SetTrayContentVisibility(pMSet.GetMarkerTrayContentVisibility(), str);
    m_listMark->SetItemMap(pMarkTbl);
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerPalette message handlers

#if 0
BOOL CMarkerPalette::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;        // controls take care of erase
}

void CMarkerPalette::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    CRect rctCombo;

    if (m_comboMGrp.m_hWnd != NULL)
    {
        m_comboMGrp.GetWindowRect(&rctCombo);
        m_comboMGrp.MoveWindow(0, 0, cx, 10 * m_nComboHeight, TRUE);
    }
    if (m_listMark.m_hWnd != NULL)
    {
        m_listMark.MoveWindow(0, m_nComboHeight, cx, cy - m_nComboHeight, TRUE);
    }
}

void CMarkerPaletteContainer::PostNcDestroy()
{
    /* DO NOTHING - FRAME CLASS WOULD DELETE SELF! */
}

void CMarkerPalette::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
    CWnd::OnWindowPosChanging(lpwndpos);

    // Need this code to avoid a detached combo list box.
    if (m_comboMGrp.m_hWnd != NULL)
        m_comboMGrp.ShowDropDown(FALSE);
}
#endif

void CMarkerPalette::OnMarkerNameCbnSelchange(wxCommandEvent& /*event*/)
{
    UpdateMarkerList();
}

#if 0
BOOL CMarkerPalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gp-ref-pal-markers.htm");
    return TRUE;
}
#endif

CMarkerPaletteContainer::CMarkerPaletteContainer(CGamDoc& pDoc) :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this)),
    child(new CMarkerPalette(*this, pDoc))
{
}

void CMarkerPaletteContainer::SetDockingFrame(CDockMarkPalette* pDockingFrame)
{
    m_pDockingFrame = pDockingFrame;
    SetParent(pDockingFrame);
}

BOOL CMarkerPaletteContainer::Create(CWnd& pOwnerWnd/*, DWORD dwStyle = 0, UINT nID = 0*/)
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE;
    if (!CWnd::Create(AfxRegisterWndClass(0), NULL, dwStyle,
        CRect(0, 0, 200, 100), &pOwnerWnd, 0))
    {
        TRACE("Failed to create Tray palette container window.\n");
        return FALSE;
    }

    return TRUE;
}

int CMarkerPaletteContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    if (!child->Create())
    {
        TRACE("Failed to create Tray palette window.\n");
        return -1;
    }

    return 0;
}

// MFC puts the focus here, so move it to the useful window
void CMarkerPaletteContainer::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);
    child->SetFocus();
}

void CMarkerPaletteContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return CWnd::OnSize(nType, cx, cy);
}
