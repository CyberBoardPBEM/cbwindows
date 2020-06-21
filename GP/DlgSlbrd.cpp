// DlgSlbrd.cpp : implementation file
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
#include    "GamDoc.h"
#include    "Board.h"
#include    "DlgSlbrd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectBoardsDialog dialog


CSelectBoardsDialog::CSelectBoardsDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSelectBoardsDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSelectBoardsDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CSelectBoardsDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSelectBoardsDialog)
    DDX_Control(pDX, IDOK, m_btnOK);
    DDX_Control(pDX, IDC_D_SETBRD_LIST, m_listBoards);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectBoardsDialog, CDialog)
    //{{AFX_MSG_MAP(CSelectBoardsDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_D_SETBRD_SELECTALL, OnBtnClickedSelectAll)
    ON_BN_CLICKED(IDC_D_SETBRD_CLEARALL, OnBtnClickedClearAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SETBRD_LIST, IDH_D_SETBRD_LIST,
    0,0
};

BOOL CSelectBoardsDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSelectBoardsDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

int CSelectBoardsDialog::FindSerialNumberListIndex(int nSerial)
{
    for (int i = 0; i < m_listBoards.GetCount(); i++)
    {
        if ((int)m_listBoards.GetItemData(i) == nSerial)
            return i;
    }
    return -1;
}

/////////////////////////////////////////////////////////////////////////////
// CSelectBoardsDialog message handlers

BOOL CSelectBoardsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pBMgr != NULL);

    for (int i = 0; i < m_pBMgr->GetNumBoards(); i++)
    {
        CBoard* pBoard = m_pBMgr->GetBoard(i);
        int nIdx = m_listBoards.AddString(pBoard->GetName());
        m_listBoards.SetItemData(nIdx, pBoard->GetSerialNumber());
        m_listBoards.SetCheck(nIdx, 0);
    }

    // If there are actively selected boards. Reselect them.

    for (int i = 0; i < m_tblBrds.GetSize(); i++)
    {
        int nIdx = FindSerialNumberListIndex((int)m_tblBrds[i]);
        if (nIdx == -1)
            AfxMessageBox(IDS_ERR_BOARDNOTEXIST, MB_OK | MB_ICONEXCLAMATION);
        m_listBoards.SetCheck(nIdx, 1);
    }

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSelectBoardsDialog::OnOK()
{
    m_tblBrds.RemoveAll();          // Clear the board table

    for (int i = 0; i < m_listBoards.GetCount(); i++)
    {
        if (m_listBoards.GetCheck(i) > 0)
        {
            // Add serial numbers for selected boards
            m_tblBrds.Add((WORD)m_listBoards.GetItemData(i));
        }
    }
    CDialog::OnOK();
}

void CSelectBoardsDialog::OnBtnClickedSelectAll()
{
    for (int i = 0; i < m_listBoards.GetCount(); i++)
        m_listBoards.SetCheck(i, 1);
}

void CSelectBoardsDialog::OnBtnClickedClearAll()
{
    for (int i = 0; i < m_listBoards.GetCount(); i++)
        m_listBoards.SetCheck(i, 0);
}
