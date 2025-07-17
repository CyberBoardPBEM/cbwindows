// DlgSpece.cpp : implementation file
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "GamDoc.h"
#include    "PPieces.h"
#include    "Trays.h"
#include    "DlgSpece.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetPiecesDialog dialog

CSetPiecesDialog::CSetPiecesDialog(CGamDoc& doc, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CSetPiecesDialog)
        CB_XRC_CTRL(m_comboPGrp)
        CB_XRC_CTRL(m_listPiece)
        CB_XRC_CTRL(m_comboYGrp)
        CB_XRC_CTRL(m_listTray)
    CB_XRC_END_CTRLS_DEFN(),
    m_pDoc(&doc),
    m_pYMgr(&m_pDoc->GetTrayManager()),
    m_pPTbl(&m_pDoc->GetPieceTable())
{
    m_listTray->Init(doc);
    m_nYSel = -1;
}

wxBEGIN_EVENT_TABLE(CSetPiecesDialog, wxDialog)
    EVT_CHOICE(XRCID("m_comboPGrp"), OnSelChangePieceGroup)
    EVT_BUTTON(XRCID("OnRemoveAll"), OnRemoveAll)
    EVT_BUTTON(XRCID("OnCopyAll"), OnCopyAll)
    EVT_BUTTON(XRCID("OnCopySelections"), OnCopySelections)
    EVT_BUTTON(XRCID("OnRemoveSelections"), OnRemoveSelections)
    EVT_CHOICE(XRCID("m_comboYGrp"), OnSelChangeTrayName)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
wxEND_EVENT_TABLE()

#if 0
/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SETPCE_TRAYNAME, IDH_D_SETPCE_TRAYNAME,
    IDC_D_SETPCE_TRAYLIST, IDH_D_SETPCE_TRAYLIST,
    IDC_D_SETPCE_PCELIST, IDH_D_SETPCE_PCELIST,
    IDC_D_SETPCE_PCEGRP, IDH_D_SETPCE_PCEGRP,
    IDC_D_SETPCE_REMALL, IDH_D_SETPCE_REMALL,
    IDC_D_SETPCE_COPYALL, IDH_D_SETPCE_COPYALL,
    IDC_D_SETPCE_COPYSEL, IDH_D_SETPCE_COPYSEL,
    IDC_D_SETPCE_REMSEL, IDH_D_SETPCE_REMSEL,
    0,0
};

BOOL CSetPiecesDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSetPiecesDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CSetPiecesDialog::LoadPieceNameList()
{
    m_comboPGrp->Clear();
    const CPieceManager& pPMgr = m_pPTbl->GetPieceManager();
    for (size_t i = size_t(0); i < pPMgr.GetNumPieceSets(); i++)
    {
        m_comboPGrp->Append(pPMgr.GetPieceSet(i).GetName());
    }
    m_comboPGrp->SetSelection(0);
    UpdatePieceList();
}

void CSetPiecesDialog::LoadTrayNameList()
{
    m_comboYGrp->Clear();
    for (size_t i = size_t(0); i < m_pYMgr->GetNumTraySets(); i++)
    {
        m_comboYGrp->Append(m_pYMgr->GetTraySet(i).GetName());
    }
    m_comboYGrp->SetSelection(m_nYSel == -1 ? 0 : m_nYSel);
    UpdateTrayList();
}

/////////////////////////////////////////////////////////////////////////////

void CSetPiecesDialog::UpdatePieceList()
{
    int nSel = m_comboPGrp->GetSelection();
    if (nSel == wxNOT_FOUND)
    {
        m_listPiece->SetItemMap(NULL);
        return;
    }
    m_tblPiece = m_pPTbl->LoadUnusedPieceList(value_preserving_cast<size_t>(nSel));
    m_listPiece->SetItemMap(&m_tblPiece, FALSE);
}

void CSetPiecesDialog::UpdateTrayList()
{
    int nSel = m_comboYGrp->GetSelection();
    if (nSel == wxNOT_FOUND)
    {
        m_listTray->SetItemMap(NULL);
        return;
    }
    const std::vector<PieceID>& pPieceTbl = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nSel)).GetPieceIDTable();

    m_tblTray = pPieceTbl;              // Clone the table

    m_listTray->SetItemMap(&pPieceTbl, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CSetPiecesDialog message handlers

void CSetPiecesDialog::OnSelChangePieceGroup(wxCommandEvent& /*event*/)
{
    UpdatePieceList();
}

void CSetPiecesDialog::OnSelChangeTrayName(wxCommandEvent& /*event*/)
{
    UpdateTrayList();
}

void CSetPiecesDialog::OnCopyAll(wxCommandEvent& /*event*/)
{
    int nTraySet = m_comboYGrp->GetSelection();
    if (nTraySet == wxNOT_FOUND)
        return;
    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));

    const std::vector<PieceID>* pPList = m_listPiece->GetItemMap();
    if (pPList == NULL)
        return;

    m_pPTbl->SetPieceListAsFrontUp(*pPList);
    pYSet.AddPieceList(*pPList);

    UpdatePieceList();
    UpdateTrayList();
}

void CSetPiecesDialog::OnCopySelections(wxCommandEvent& /*event*/)
{
    int nTraySet = m_comboYGrp->GetSelection();
    if (nTraySet == wxNOT_FOUND)
        return;
    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));

    std::vector<PieceID> selList = m_listPiece->GetCurMappedItemList();
    if (selList.empty())
        return;                     // Nothing to copy.

    m_pPTbl->SetPieceListAsFrontUp(selList);
    pYSet.AddPieceList(selList);

    UpdatePieceList();
    UpdateTrayList();
}

void CSetPiecesDialog::OnRemoveSelections(wxCommandEvent& /*event*/)
{
    int nTraySet = m_comboYGrp->GetSelection();
    if (nTraySet == wxNOT_FOUND)
        return;
    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));


    std::vector<PieceID> selList = m_listTray->GetCurMappedItemList();
    if (selList.empty())
        return;                     // Nothing to remove

    m_pPTbl->SetPieceListAsUnused(selList);
    pYSet.RemovePieceList(selList);

    UpdatePieceList();
    UpdateTrayList();
}

void CSetPiecesDialog::OnRemoveAll(wxCommandEvent& /*event*/)
{
    int nTraySet = m_comboYGrp->GetSelection();
    if (nTraySet == wxNOT_FOUND)
        return;

    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));

    // Use cloned list as basis for delete or nasty things happen
    m_pPTbl->SetPieceListAsUnused(m_tblTray);
    pYSet.RemovePieceList(m_tblTray);

    UpdatePieceList();
    UpdateTrayList();
}

bool CSetPiecesDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    m_listPiece->SetDocument(*m_pDoc);
    m_listTray->SetTrayContentVisibility(trayVizAllSides);

    LoadPieceNameList();
    LoadTrayNameList();

    return TRUE;
}

