// DlgITray.cpp : implementation file
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

#include "stdafx.h"
#include "Gp.h"
#include "GamDoc.h"
#include "Trays.h"
#include "PPieces.h"
#include "DlgITray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportTraysDlg dialog

CImportTraysDlg::CImportTraysDlg(CGamDoc& doc, CWnd* pParent /*=NULL*/)
    : CDialog(CImportTraysDlg::IDD, pParent),
    m_pDoc(doc)
{
    //{{AFX_DATA_INIT(CImportTraysDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CImportTraysDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CImportTraysDlg)
    DDX_Control(pDX, IDC_D_IMPPCE_GROUPLIST, m_listGroups);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportTraysDlg, CDialog)
    //{{AFX_MSG_MAP(CImportTraysDlg)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_D_IMPPCE_SELECTALL, OnBnClickedSelectAll)
    ON_BN_CLICKED(IDC_D_IMPPCE_CLEARALL, OnBnClickedClearAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_IMPPCE_GROUPLIST, IDH_D_IMPPCE_GROUPLIST,
    0,0
};

BOOL CImportTraysDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CImportTraysDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CImportTraysDlg message handlers

BOOL CImportTraysDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    const CPieceManager& pPMgr = m_pDoc.GetPieceManager();
    CTrayManager& pYMgr = m_pDoc.GetTrayManager();

    // Loop through all the piece groups in the gamebox. If a group
    // already doesn't exist as a tray in the scenario, add it to the
    // listbox.

    for (size_t nPSet = size_t(0); nPSet < pPMgr.GetNumPieceSets(); nPSet++)
    {
        CB::string strName = pPMgr.GetPieceSet(nPSet).GetName();
        size_t nTray;
        for (nTray = size_t(0); nTray < pYMgr.GetNumTraySets(); nTray++)
        {
            if (strName.CompareNoCase(pYMgr.GetTraySet(nTray).GetName()) == 0)
                break;
        }
        if (nTray >= pYMgr.GetNumTraySets())
        {
            // Found one...
            int nIdx = m_listGroups.AddString(strName);
            m_listGroups.SetItemData(nIdx, nPSet);
            m_listGroups.SetCheck(nIdx, 0);
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CImportTraysDlg::OnOK()
{
    const CPieceManager& pPMgr = m_pDoc.GetPieceManager();
    CTrayManager& pYMgr = m_pDoc.GetTrayManager();
    CPieceTable& pPTbl = m_pDoc.GetPieceTable();

    for (int i = 0; i < m_listGroups.GetCount(); i++)
    {
        if (m_listGroups.GetCheck(i) > 0)
        {
            size_t nPSet = value_preserving_cast<size_t>(m_listGroups.GetItemData(i));
            const CPieceSet& pPSet = pPMgr.GetPieceSet(nPSet);

            // Create tray and add pieces...
            size_t nTray = pYMgr.CreateTraySet(pPSet.GetName());
            CTraySet& pYSet = pYMgr.GetTraySet(nTray);

            // Locate only those pieces that aren't currently being used.
            std::vector<PieceID> arrUnusedPieces = pPTbl.LoadUnusedPieceList(nPSet);
            pPTbl.SetPieceListAsFrontUp(arrUnusedPieces);
            pYSet.AddPieceList(arrUnusedPieces);
        }
    }
    CDialog::OnOK();
}

void CImportTraysDlg::OnBnClickedSelectAll()
{
    for (int i = 0; i < m_listGroups.GetCount(); i++)
        m_listGroups.SetCheck(i, 1);
}

void CImportTraysDlg::OnBnClickedClearAll()
{
    for (int i = 0; i < m_listGroups.GetCount(); i++)
        m_listGroups.SetCheck(i, 0);
}
