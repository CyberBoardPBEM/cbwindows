// DlgPEditMulti.cpp : implementation file
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
#include "Gm.h"
#include "GmHelp.h"
#include "DlgPEditMulti.h"
#include "LibMfc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog dialog

CPieceEditMultipleDialog::CPieceEditMultipleDialog(size_t s, CWnd* pParent /*=NULL*/)
    : CDialog(CPieceEditMultipleDialog::IDD, pParent),
    m_sides(s)
{
    //{{AFX_DATA_INIT(CPieceEditMultipleDialog)
    //}}AFX_DATA_INIT
    m_bSetTopOnlyVisible = FALSE;
    m_bTopOnlyOwnersToo = FALSE;
    m_bSetTexts.resize(m_sides, false);
    m_strs.resize(m_sides);
}

void CPieceEditMultipleDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPieceEditMultipleDialog)
    DDX_Control(pDX, IDC_D_MPEDIT_TOP_VISIBLE_OWNERS_TOO, m_chkTopVisibleOwnersToo);
    DDX_Control(pDX, IDC_D_MPEDIT_FRONT_LABEL, m_staticFrontLabel);
    DDX_Control(pDX, IDC_D_MPEDIT_CHG_BACK_TEXT, m_chkSetBackText);
    DDX_Control(pDX, IDC_D_MPEDIT_CHG_TOP_TEXT, m_chkSetFrontText);
    DDX_Control(pDX, IDC_D_MPEDIT_BACK_LABEL, m_staticBackLabel);
    DDX_Control(pDX, IDC_D_MPEDIT_TOP_VISIBLE, m_chkTopVisible);
    DDX_Control(pDX, IDC_D_MPEDIT_TEXT_BACK, m_editBack);
    DDX_Control(pDX, IDC_D_MPEDIT_TEXT_FRONT, m_editFront);
    DDX_Control(pDX, IDC_CURR_SIDE, m_currSide);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPieceEditMultipleDialog, CDialog)
    //{{AFX_MSG_MAP(CPieceEditMultipleDialog)
    ON_BN_CLICKED(IDC_D_MPEDIT_CHG_BACK_TEXT, OnBtnClickChangeBack)
    ON_BN_CLICKED(IDC_D_MPEDIT_CHG_TOP_TEXT, OnBtnClickChangeFront)
    ON_BN_CLICKED(IDC_D_MPEDIT_TOP_VISIBLE, OnBtnClickTopVisible)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_CBN_SELCHANGE(IDC_CURR_SIDE, &CPieceEditMultipleDialog::OnSelchangeCurrSide)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_MPEDIT_TOP_VISIBLE_OWNERS_TOO, IDH_D_MPEDIT_TOP_VISIBLE_OWNERS_TOO,
    IDC_D_MPEDIT_CHG_BACK_TEXT, IDH_D_MPEDIT_CHG_BACK_TEXT,
    IDC_D_MPEDIT_CHG_TOP_TEXT, IDH_D_MPEDIT_CHG_TOP_TEXT,
    IDC_D_MPEDIT_TOP_VISIBLE, IDH_D_MPEDIT_TOP_VISIBLE,
    IDC_D_MPEDIT_TEXT_BACK, IDH_D_MPEDIT_TEXT_BACK,
    IDC_D_MPEDIT_TEXT_FRONT, IDH_D_MPEDIT_TEXT_FRONT,
    0,0
};

/////////////////////////////////////////////////////////////////////////////

void CPieceEditMultipleDialog::UpdateTextControls()
{
    ASSERT(m_bSetTexts[size_t(0)] == bool(m_editFront.IsWindowEnabled()));
    if (m_bSetTexts[size_t(0)])
    {
        CB::string str = CB::string::GetWindowText(m_editFront);
        m_strs[size_t(0)] = str;
    }
    if (m_chkSetFrontText.GetCheck())
    {
        if (!m_bSetTexts[size_t(0)])
        {
            m_bSetTexts[size_t(0)] = true;
            m_editFront.EnableWindow(TRUE);
            m_editFront.SetWindowText(m_strs[size_t(0)]);
            m_staticFrontLabel.EnableWindow(TRUE);
        }
    }
    else
    {
        if (m_bSetTexts[size_t(0)])
        {
            m_bSetTexts[size_t(0)] = false;
            m_editFront.EnableWindow(FALSE);
            m_editFront.SetWindowText(""_cbstring);
            m_staticFrontLabel.EnableWindow(FALSE);
        }
    }

    // ASSERT(m_prevSide != std::numeric_limits<size_t>::max() --> 1 <= m_prevSide < sides);
    ASSERT(m_prevSide == std::numeric_limits<size_t>::max() ||
            (size_t(1) <= m_prevSide && m_prevSide < m_sides));

    size_t currSide = std::numeric_limits<size_t>::max();
    // ASSERT(m_prevSide != std::numeric_limits<size_t>::max() --> m_currSide.GetCurSel() != CB_ERR);
    ASSERT(m_prevSide == std::numeric_limits<size_t>::max() ||
            m_currSide.GetCurSel() != CB_ERR);
    if (m_currSide.GetCurSel() != CB_ERR)
    {
        currSide = value_preserving_cast<size_t>(m_currSide.GetCurSel()) + size_t(1);
        ASSERT(size_t(1) <= currSide && currSide < m_sides);

        if (currSide == m_prevSide)
        {
            // logic here is just like front
            ASSERT(m_bSetTexts[currSide] == bool(m_editBack.IsWindowEnabled()));
            if (m_bSetTexts[currSide])
            {
                CB::string str = CB::string::GetWindowText(m_editBack);
                m_strs[currSide] = str;
            }
            if (m_chkSetBackText.GetCheck())
            {
                if (!m_bSetTexts[currSide])
                {
                    m_bSetTexts[currSide] = true;
                    m_editBack.EnableWindow(TRUE);
                    m_editBack.SetWindowText(m_strs[currSide]);
                    m_staticBackLabel.EnableWindow(TRUE);
                }
            }
            else
            {
                if (m_bSetTexts[currSide])
                {
                    m_bSetTexts[currSide] = false;
                    m_editBack.EnableWindow(FALSE);
                    m_editBack.SetWindowText(""_cbstring);
                    m_staticBackLabel.EnableWindow(FALSE);
                }
            }
        }
        else
        {
            /* read controls for m_prevSide,
                then set controls for currSide */
            if (m_prevSide < m_sides)
            {
                ASSERT(m_bSetTexts[m_prevSide] == bool(m_editBack.IsWindowEnabled()));
                if (m_bSetTexts[m_prevSide])
                {
                    CB::string str = CB::string::GetWindowText(m_editBack);
                    m_strs[m_prevSide] = str;
                }
                m_bSetTexts[m_prevSide] = m_chkSetBackText.GetCheck();
            }
            m_chkSetBackText.SetCheck(m_bSetTexts[currSide] ? BST_CHECKED : BST_UNCHECKED);
            m_editBack.EnableWindow(m_bSetTexts[currSide]);
            m_staticBackLabel.EnableWindow(m_bSetTexts[currSide]);
            m_editBack.SetWindowText(m_strs[currSide]);
        }
    }

    m_prevSide = currSide;
}

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog message handlers

void CPieceEditMultipleDialog::OnBtnClickTopVisible()
{
    m_chkTopVisibleOwnersToo.EnableWindow(m_chkTopVisible.GetCheck() == 1);
}

void CPieceEditMultipleDialog::OnBtnClickChangeBack()
{
    UpdateTextControls();
}

void CPieceEditMultipleDialog::OnBtnClickChangeFront()
{
    UpdateTextControls();
}

BOOL CPieceEditMultipleDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_chkTopVisible.SetCheck(2);                    // Show as indeterminate
    m_chkTopVisibleOwnersToo.SetCheck(0);
    m_chkTopVisibleOwnersToo.EnableWindow(FALSE);

    m_chkSetFrontText.SetCheck(0);
    m_chkSetBackText.SetCheck(0);

    if (m_sides >= size_t(2))
    {
        for (size_t i = size_t(1); i < m_sides; ++i)
        {
            CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", i + size_t(1)));
            if (m_currSide.AddString(str) != value_preserving_cast<int>(i - size_t(1)))
            {
                AfxThrowMemoryException();
            }
        }
        m_currSide.SetCurSel(0);
    }
    else
    {
        m_chkTopVisible.EnableWindow(false);
        m_currSide.EnableWindow(false);
        m_chkSetBackText.EnableWindow(false);
        m_editBack.EnableWindow(false);
        m_staticBackLabel.EnableWindow(false);
    }

    // satisfy UpdateTextControls expectations
    m_editFront.EnableWindow(false);
    UpdateTextControls();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPieceEditMultipleDialog::OnOK()
{
    UpdateTextControls();

    m_bSetTopOnlyVisible = m_chkTopVisible.GetCheck() != 2;
    m_bTopOnlyVisible = m_chkTopVisible.GetCheck() != 0;
    m_bTopOnlyOwnersToo = m_chkTopVisibleOwnersToo.GetCheck() != 0;

    CDialog::OnOK();
}

BOOL CPieceEditMultipleDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPieceEditMultipleDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

void CPieceEditMultipleDialog::OnSelchangeCurrSide()
{
    UpdateTextControls();
}



