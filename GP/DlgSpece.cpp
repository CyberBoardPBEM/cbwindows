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

CSetPiecesDialog::CSetPiecesDialog(CGamDoc& doc, CWnd* pParent /*=NULL*/)
    : CDialog(CSetPiecesDialog::IDD, pParent),
    m_listTray(doc),
    m_pDoc(&doc),
    m_pYMgr(&m_pDoc->GetTrayManager()),
    m_pPTbl(&m_pDoc->GetPieceTable())
{
    //{{AFX_DATA_INIT(CSetPiecesDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_nYSel = -1;
}

void CSetPiecesDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSetPiecesDialog)
    DDX_Control(pDX, IDC_D_SETPCE_TRAYNAME, m_comboYGrp);
    DDX_Control(pDX, IDC_D_SETPCE_TRAYLIST, m_listTray);
    DDX_Control(pDX, IDC_D_SETPCE_PCELIST, m_listPiece);
    DDX_Control(pDX, IDC_D_SETPCE_PCEGRP, m_comboPGrp);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetPiecesDialog, CDialog)
    //{{AFX_MSG_MAP(CSetPiecesDialog)
    ON_CBN_SELCHANGE(IDC_D_SETPCE_PCEGRP, OnSelChangePieceGroup)
    ON_BN_CLICKED(IDC_D_SETPCE_REMALL, OnRemoveAll)
    ON_BN_CLICKED(IDC_D_SETPCE_COPYALL, OnCopyAll)
    ON_BN_CLICKED(IDC_D_SETPCE_COPYSEL, OnCopySelections)
    ON_BN_CLICKED(IDC_D_SETPCE_REMSEL, OnRemoveSelections)
    ON_CBN_SELCHANGE(IDC_D_SETPCE_TRAYNAME, OnSelChangeTrayName)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

/////////////////////////////////////////////////////////////////////////////

void CSetPiecesDialog::LoadPieceNameList()
{
    m_comboPGrp.ResetContent();
    const CPieceManager& pPMgr = m_pPTbl->GetPieceManager();
    for (size_t i = size_t(0); i < pPMgr.GetNumPieceSets(); i++)
        m_comboPGrp.AddString(pPMgr.GetPieceSet(i).GetName());
    m_comboPGrp.SetCurSel(0);
    UpdatePieceList();
}

void CSetPiecesDialog::LoadTrayNameList()
{
    m_comboYGrp.ResetContent();
    for (size_t i = 0; i < m_pYMgr->GetNumTraySets(); i++)
        m_comboYGrp.AddString(m_pYMgr->GetTraySet(i).GetName());
    m_comboYGrp.SetCurSel(m_nYSel == -1 ? 0 : m_nYSel);
    UpdateTrayList();
}

/////////////////////////////////////////////////////////////////////////////

void CSetPiecesDialog::UpdatePieceList()
{
    int nSel = m_comboPGrp.GetCurSel();
    if (nSel < 0)
    {
        m_listPiece.SetItemMap(NULL);
        return;
    }
    m_tblPiece = m_pPTbl->LoadUnusedPieceList(value_preserving_cast<size_t>(nSel));
    m_listPiece.SetItemMap(&m_tblPiece, FALSE);
}

void CSetPiecesDialog::UpdateTrayList()
{
    int nSel = m_comboYGrp.GetCurSel();
    if (nSel < 0)
    {
        m_listTray.SetItemMap(NULL);
        return;
    }
    const std::vector<PieceID>& pPieceTbl = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nSel)).GetPieceIDTable();

    m_tblTray = pPieceTbl;              // Clone the table

    m_listTray.SetItemMap(&pPieceTbl, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CSetPiecesDialog message handlers

void CSetPiecesDialog::OnSelChangePieceGroup()
{
    UpdatePieceList();
}

void CSetPiecesDialog::OnSelChangeTrayName()
{
    UpdateTrayList();
}

void CSetPiecesDialog::OnCopyAll()
{
    int nTraySet = m_comboYGrp.GetCurSel();
    if (nTraySet < 0)
        return;
    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));

    const std::vector<PieceID>* pPList = m_listPiece.GetItemMap();
    if (pPList == NULL)
        return;

    m_pPTbl->SetPieceListAsFrontUp(*pPList);
    pYSet.AddPieceList(*pPList);

    UpdatePieceList();
    UpdateTrayList();
}

void CSetPiecesDialog::OnCopySelections()
{
    int nTraySet = m_comboYGrp.GetCurSel();
    if (nTraySet < 0)
        return;
    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));

    std::vector<PieceID> selList = m_listPiece.GetCurMappedItemList();
    if (selList.empty())
        return;                     // Nothing to copy.

    m_pPTbl->SetPieceListAsFrontUp(selList);
    pYSet.AddPieceList(selList);

    UpdatePieceList();
    UpdateTrayList();
}

void CSetPiecesDialog::OnRemoveSelections()
{
    int nTraySet = m_comboYGrp.GetCurSel();
    if (nTraySet < 0)
        return;
    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));


    std::vector<PieceID> selList = m_listTray.GetCurMappedItemList();
    if (selList.empty())
        return;                     // Nothing to remove

    m_pPTbl->SetPieceListAsUnused(selList);
    pYSet.RemovePieceList(selList);

    UpdatePieceList();
    UpdateTrayList();
}

void CSetPiecesDialog::OnRemoveAll()
{
    int nTraySet = m_comboYGrp.GetCurSel();
    if (nTraySet < 0)
        return;

    CTraySet& pYSet = m_pYMgr->GetTraySet(value_preserving_cast<size_t>(nTraySet));

    // Use cloned list as basis for delete or nasty things happen
    m_pPTbl->SetPieceListAsUnused(m_tblTray);
    pYSet.RemovePieceList(m_tblTray);

    UpdatePieceList();
    UpdateTrayList();
}

BOOL CSetPiecesDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_listPiece.SetDocument(*m_pDoc);
    m_listTray.SetTrayContentVisibility(trayVizAllSides);

    LoadPieceNameList();
    LoadTrayNameList();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

