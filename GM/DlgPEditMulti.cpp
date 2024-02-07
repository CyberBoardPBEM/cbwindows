// DlgPEditMulti.cpp : implementation file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

CPieceEditMultipleDialog::CPieceEditMultipleDialog(size_t s, wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CPieceEditMultipleDialog)
        CB_XRC_CTRL(m_chkTopVisibleOwnersToo)
        CB_XRC_CTRL(m_staticFrontLabel)
        CB_XRC_CTRL(m_chkSetBackText)
        CB_XRC_CTRL(m_chkSetFrontText)
        CB_XRC_CTRL(m_staticBackLabel)
        CB_XRC_CTRL(m_chkTopVisible)
        CB_XRC_CTRL(m_editBack)
        CB_XRC_CTRL(m_editFront)
        CB_XRC_CTRL(m_currSide)
    CB_XRC_END_CTRLS_DEFN(),
    m_sides(s)
{
    m_bSetTopOnlyVisible = FALSE;
    m_bTopOnlyOwnersToo = FALSE;
    m_bSetTexts.resize(m_sides, false);
    m_strs.resize(m_sides);
}


wxBEGIN_EVENT_TABLE(CPieceEditMultipleDialog, wxDialog)
    EVT_CHECKBOX(XRCID("m_chkSetBackText"), OnBtnClickChangeBack)
    EVT_CHECKBOX(XRCID("m_chkSetFrontText"), OnBtnClickChangeFront)
    EVT_CHECKBOX(XRCID("m_chkTopVisible"), OnBtnClickTopVisible)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
    EVT_CHOICE(XRCID("m_currSide"), OnSelchangeCurrSide)
wxEND_EVENT_TABLE()

#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CPieceEditMultipleDialog::UpdateTextControls()
{
    wxASSERT(m_bSetTexts[size_t(0)] == m_editFront->IsEnabled());
    if (m_bSetTexts[size_t(0)])
    {
        CB::string str = m_editFront->GetValue();
        m_strs[size_t(0)] = str;
    }
    if (m_chkSetFrontText->GetValue())
    {
        if (!m_bSetTexts[size_t(0)])
        {
            m_bSetTexts[size_t(0)] = true;
            m_editFront->Enable(TRUE);
            m_editFront->SetValue(m_strs[size_t(0)]);
            m_staticFrontLabel->Enable(TRUE);
        }
    }
    else
    {
        if (m_bSetTexts[size_t(0)])
        {
            m_bSetTexts[size_t(0)] = false;
            m_editFront->Enable(FALSE);
            m_editFront->Clear();
            m_staticFrontLabel->Enable(FALSE);
        }
    }

    // ASSERT(m_prevSide != std::numeric_limits<size_t>::max() --> 1 <= m_prevSide < sides);
    wxASSERT(m_prevSide == std::numeric_limits<size_t>::max() ||
            (size_t(1) <= m_prevSide && m_prevSide < m_sides));

    size_t currSide = std::numeric_limits<size_t>::max();
    // ASSERT(m_prevSide != std::numeric_limits<size_t>::max() --> m_currSide.GetCurSel() != CB_ERR);
    wxASSERT(m_prevSide == std::numeric_limits<size_t>::max() ||
            m_currSide->GetSelection() != wxNOT_FOUND);
    if (m_currSide->GetSelection() != wxNOT_FOUND)
    {
        currSide = value_preserving_cast<size_t>(m_currSide->GetSelection()) + size_t(1);
        wxASSERT(size_t(1) <= currSide && currSide < m_sides);

        if (currSide == m_prevSide)
        {
            // logic here is just like front
            wxASSERT(m_bSetTexts[currSide] == m_editBack->IsEnabled());
            if (m_bSetTexts[currSide])
            {
                CB::string str = m_editBack->GetValue();
                m_strs[currSide] = str;
            }
            if (m_chkSetBackText->GetValue())
            {
                if (!m_bSetTexts[currSide])
                {
                    m_bSetTexts[currSide] = true;
                    m_editBack->Enable(TRUE);
                    m_editBack->SetValue(m_strs[currSide]);
                    m_staticBackLabel->Enable(TRUE);
                }
            }
            else
            {
                if (m_bSetTexts[currSide])
                {
                    m_bSetTexts[currSide] = false;
                    m_editBack->Enable(FALSE);
                    m_editBack->Clear();
                    m_staticBackLabel->Enable(FALSE);
                }
            }
        }
        else
        {
            /* read controls for m_prevSide,
                then set controls for currSide */
            if (m_prevSide < m_sides)
            {
                wxASSERT(m_bSetTexts[m_prevSide] == m_editBack->IsEnabled());
                if (m_bSetTexts[m_prevSide])
                {
                    CB::string str = m_editBack->GetValue();
                    m_strs[m_prevSide] = str;
                }
                m_bSetTexts[m_prevSide] = m_chkSetBackText->GetValue();
            }
            m_chkSetBackText->SetValue(m_bSetTexts[currSide]);
            m_editBack->Enable(m_bSetTexts[currSide]);
            m_staticBackLabel->Enable(m_bSetTexts[currSide]);
            m_editBack->SetValue(m_strs[currSide]);
        }
    }

    m_prevSide = currSide;
}

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog message handlers

void CPieceEditMultipleDialog::OnBtnClickTopVisible(wxCommandEvent& /*event*/)
{
    m_chkTopVisibleOwnersToo->Enable(m_chkTopVisible->GetValue());
}

void CPieceEditMultipleDialog::OnBtnClickChangeBack(wxCommandEvent& /*event*/)
{
    UpdateTextControls();
}

void CPieceEditMultipleDialog::OnBtnClickChangeFront(wxCommandEvent& /*event*/)
{
    UpdateTextControls();
}

bool CPieceEditMultipleDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    m_chkTopVisible->Set3StateValue(wxCHK_UNDETERMINED);                    // Show as indeterminate
    m_chkTopVisibleOwnersToo->SetValue(false);
    m_chkTopVisibleOwnersToo->Enable(FALSE);

    m_chkSetFrontText->SetValue(false);
    m_chkSetBackText->SetValue(false);

    if (m_sides >= size_t(2))
    {
        for (size_t i = size_t(1); i < m_sides; ++i)
        {
            CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", i + size_t(1)));
            if (m_currSide->Append(str) != value_preserving_cast<int>(i - size_t(1)))
            {
                AfxThrowMemoryException();
            }
        }
        m_currSide->SetSelection(0);
    }
    else
    {
        m_chkTopVisible->Enable(false);
        m_currSide->Enable(false);
        m_chkSetBackText->Enable(false);
        m_editBack->Enable(false);
        m_staticBackLabel->Enable(false);
    }

    // satisfy UpdateTextControls expectations
    m_editFront->Enable(false);
    UpdateTextControls();

    return true;
}

bool CPieceEditMultipleDialog::TransferDataFromWindow()
{
    UpdateTextControls();

    m_bSetTopOnlyVisible = m_chkTopVisible->Get3StateValue() != wxCHK_UNDETERMINED;
    m_bTopOnlyVisible = m_chkTopVisible->GetValue();
    m_bTopOnlyOwnersToo = m_chkTopVisibleOwnersToo->GetValue();

    return wxDialog::TransferDataFromWindow();
}

#if 0
BOOL CPieceEditMultipleDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPieceEditMultipleDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

void CPieceEditMultipleDialog::OnSelchangeCurrSide(wxCommandEvent& /*event*/)
{
    UpdateTextControls();
}



