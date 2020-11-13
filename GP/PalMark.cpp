// PalMark.cpp : implementation file
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

IMPLEMENT_DYNCREATE(CMarkerPalette, CMiniFrameWnd)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMarkerPalette, CWnd)
    //{{AFX_MSG_MAP(CMarkerPalette)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_WINDOWPOSCHANGING()
    ON_CBN_SELCHANGE(IDC_W_MARKLIST, OnMarkerNameCbnSelchange)
    ON_MESSAGE(WM_OVERRIDE_SELECTED_ITEM, OnOverrideSelectedItem)
    ON_WM_HELPINFO()
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_PALETTE_HIDE, OnPaletteHide)
    ON_MESSAGE(WM_WINSTATE_RESTORE, OnMessageRestoreWinState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarkerPalette

CMarkerPalette::CMarkerPalette()
{
    m_pDoc = NULL;

    m_listMark.EnableDrag(TRUE);
    m_dummyArray.Add(0);
    m_bStateVarsArmed = FALSE;
    m_nComboHeight = 0;
    m_pDockingFrame = NULL;
}

BOOL CMarkerPalette::Create(CWnd* pOwnerWnd, DWORD dwStyle, UINT nID)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;
    if (!CWnd::Create(AfxRegisterWndClass(0), NULL, dwStyle,
        CRect(0, 0, 200, 100), pOwnerWnd, nID))
    {
        TRACE("Failed to create Tray palette window.\n");
        return FALSE;
    }

    UpdatePaletteContents();

    // Queue up a message to finish up state restore.
    PostMessage(WM_WINSTATE_RESTORE);
    return TRUE;
}

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

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_WINSTATE_RESTORE. The
// message is posted during view initial update if the state of
// the windows should be restored.

LRESULT CMarkerPalette::OnMessageRestoreWinState(WPARAM, LPARAM)
{
    UpdatePaletteContents();

    if (!m_bStateVarsArmed)
        return (LRESULT)0;

    m_comboMGrp.SetCurSel(m_nComboIndex);

    UpdateMarkerList();

    m_listMark.SetCurSel(m_nListCurSel);
    m_listMark.SetTopIndex(m_nListTopindex);

    m_bStateVarsArmed = FALSE;

    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CMarkerPalette::OnPaletteHide(WPARAM, LPARAM)
{
    GetMainFrame()->SendMessage(WM_COMMAND, ID_VIEW_MARKERPAL);
    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////

int CMarkerPalette::GetSelectedMarkerGroup()
{
    int nSel = m_comboMGrp.GetCurSel();
    if (nSel < 0)
        return -1;
    return (int)m_comboMGrp.GetItemData(nSel);
}

int CMarkerPalette::FindMarkerGroupIndex(int nGroupNum)
{
    if (m_comboMGrp.GetCount() <= 0)
        return -1;
    for (int nIdx = 0; nIdx < m_comboMGrp.GetCount(); nIdx++)
    {
        if ((int)m_comboMGrp.GetItemData(nIdx) == nGroupNum)
            return nIdx;
    }
    return -1;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CMarkerPalette::OnOverrideSelectedItem(WPARAM wParam, LPARAM lParam)
{
    int nSel = GetSelectedMarkerGroup();
    if (nSel < 0)
        return (LRESULT)0;

    CMarkManager* pMMgr = m_pDoc->GetMarkManager();
    CMarkSet* pMSet = pMMgr->GetMarkSet(nSel);
    ASSERT(pMSet);
    if (pMSet->IsRandomMarkerPull())
    {
        int* pMarkerID = (int*)wParam;

        UINT nRandSeed = m_pDoc->GetRandomNumberSeed();

        int nRandNum = CalcRandomNumberUsingSeed(0, pMSet->GetMarkIDTable()->GetSize(),
            nRandSeed, &nRandSeed);

        *pMarkerID = pMSet->GetMarkIDTable()->GetAt(nRandNum);

        m_pDoc->SetRandomNumberSeed(nRandSeed);
    }

    return (LRESULT)1;
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::SelectMarker(MarkID mid)
{
    int nGrp = m_pDoc->GetMarkManager()->FindMarkInMarkSet(mid);
    ASSERT(nGrp >= 0);
    int nSel = GetSelectedMarkerGroup();
    if (nSel != nGrp)
    {
        m_comboMGrp.SetCurSel(FindMarkerGroupIndex(nGrp));
        UpdateMarkerList();
    }
    m_listMark.SelectMarker(mid);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (DWORD)m_comboMGrp.GetCurSel();
        ar << (DWORD)m_listMark.GetTopIndex();
        ar << (DWORD)m_listMark.GetCurSel();
    }
    else
    {
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))          // V2.90
        {
            DWORD dwTmp;
            ar >> dwTmp; m_nComboIndex = (int)dwTmp;
            ar >> dwTmp; m_nListTopindex = (int)dwTmp;
            ar >> dwTmp; m_nListCurSel = (int)dwTmp;
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))       // V2.0
        {
            DWORD dwTmp;
            ar >> dwTmp; m_nComboIndex = (int)dwTmp;
            ar >> dwTmp; m_nListTopindex = (int)dwTmp;
            ar >> dwTmp; m_nListCurSel = (int)dwTmp;
            CWinPlacement wndSink;
            ar >> wndSink;                      // Eat this puppy
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else                                    // Pre V2.0 data
        {
            // Just eat the old data and go with the defaults
            short sTmp;
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::SetDocument(CGamDoc *pDoc)
{
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    m_pDoc = pDoc;
    m_listMark.SetDocument(pDoc);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::LoadMarkerNameList()
{
    ASSERT(m_pDoc);
    CMarkManager* pMMgr = m_pDoc->GetMarkManager();
    ASSERT(pMMgr != NULL);

    m_comboMGrp.ResetContent();
    for (int i = 0; i < pMMgr->GetNumMarkSets(); i++)
    {
        int nIdx = m_comboMGrp.AddString(pMMgr->GetMarkSet(i)->GetName());
        m_comboMGrp.SetItemData(nIdx, (DWORD)i);    // Store the marker index in the data item
    }
    m_comboMGrp.SetCurSel(0);
    UpdateMarkerList();
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::UpdatePaletteContents()
{
    int nSel = GetSelectedMarkerGroup();
    if (nSel < 0)
        nSel = 0;               // Force first entry (if any)
    LoadMarkerNameList();
    if (nSel < m_comboMGrp.GetCount())
        m_comboMGrp.SetCurSel(FindMarkerGroupIndex(nSel));
    UpdateMarkerList();
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerPalette::UpdateMarkerList()
{
    ASSERT(m_pDoc);
    CMarkManager* pMMgr = m_pDoc->GetMarkManager();
    ASSERT(pMMgr != NULL);

    int nSel = GetSelectedMarkerGroup();
    if (nSel < 0)
    {
        m_listMark.SetItemMap(NULL);
        return;
    }

    CMarkSet* pMSet = pMMgr->GetMarkSet(nSel);
    CWordArray* pPieceTbl = pMSet->GetMarkIDTable();
    ASSERT(pPieceTbl != NULL);

    CString str = "";
    if (pMSet->GetMarkerTrayContentVisibility() == mtrayVizEachGenericRandPull)
        str.LoadString(IDS_TRAY_RANDHIDDEN);
    else if (pMSet->GetMarkerTrayContentVisibility() == mtrayVizNoneRandPull)
    {
        str.LoadString(IDS_TRAY_ALLRANDHIDDEN);
        // Set the first (and only) element in the dummy array
        // if there are any markers in the group
        m_dummyArray.RemoveAll();
        if (pPieceTbl->GetSize() > 0)
            m_dummyArray.Add(pPieceTbl->GetAt(0));
        pPieceTbl = &m_dummyArray;
    }
    m_listMark.SetTrayContentVisibility(pMSet->GetMarkerTrayContentVisibility(), str);
    m_listMark.SetItemMap(pPieceTbl);
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerPalette message handlers

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

void CMarkerPalette::PostNcDestroy()
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

void CMarkerPalette::OnMarkerNameCbnSelchange()
{
    if (!m_comboMGrp.GetDroppedState())
        UpdateMarkerList();
}

BOOL CMarkerPalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gp-ref-pal-markers.htm");
    return TRUE;
}
