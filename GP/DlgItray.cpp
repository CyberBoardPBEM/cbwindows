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

CImportTraysDlg::CImportTraysDlg(CGamDoc& doc, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CImportTraysDlg)
        CB_XRC_CTRL(m_listGroups)
    CB_XRC_END_CTRLS_DEFN(),
    m_pDoc(&doc)
{
    // adjust m_listGroups size
    for (int i = 0; i < 12; ++i)
    {
        m_listGroups->Append(wxEmptyString);
    }
    Layout();
    Fit();
    Centre();
    m_listGroups->Clear();
}

wxBEGIN_EVENT_TABLE(CImportTraysDlg, wxDialog)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
    EVT_BUTTON(XRCID("OnBnClickedSelectAll"), OnBnClickedSelectAll)
    EVT_BUTTON(XRCID("OnBnClickedClearAll"), OnBnClickedClearAll)
wxEND_EVENT_TABLE()

#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportTraysDlg message handlers

bool CImportTraysDlg::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    const CPieceManager& pPMgr = m_pDoc->GetPieceManager();
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();

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
            int nIdx = m_listGroups->Append(strName);
            m_listGroups->SetClientData(value_preserving_cast<unsigned int>(nIdx), reinterpret_cast<void*>(value_preserving_cast<uintptr_t>(nPSet)));
            m_listGroups->Check(value_preserving_cast<unsigned int>(nIdx), false);
        }
    }

    return TRUE;
}

bool CImportTraysDlg::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    const CPieceManager& pPMgr = m_pDoc->GetPieceManager();
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();
    CPieceTable& pPTbl = m_pDoc->GetPieceTable();

    for (unsigned int i = 0u ; i < m_listGroups->GetCount() ; ++i)
    {
        if (m_listGroups->IsChecked(i))
        {
            size_t nPSet = value_preserving_cast<size_t>(reinterpret_cast<uintptr_t>(m_listGroups->GetClientData(i)));
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
    return true;
}

void CImportTraysDlg::OnBnClickedSelectAll(wxCommandEvent& /*event*/)
{
    for (unsigned int i = 0u ; i < m_listGroups->GetCount() ; ++i)
    {
        m_listGroups->Check(i, true);
    }
}

void CImportTraysDlg::OnBnClickedClearAll(wxCommandEvent& /*event*/)
{
    for (unsigned int i = 0u ; i < m_listGroups->GetCount() ; ++i)
    {
        m_listGroups->Check(i, false);
    }
}
