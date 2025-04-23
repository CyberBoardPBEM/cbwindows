// DlgDice.cpp : implementation file
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
#include "GMisc.h"
#include "DlgDice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CArchive& operator<<(CArchive& ar, const CRollState& rs)
{
    switch (SCHEMA_ROLLSTATE)
    {
        case 1:
            ar << value_preserving_cast<uint8_t>(SCHEMA_ROLLSTATE)
                << rs.m_bFirstRoll
                << rs.m_nSetsToRoll
                << rs.m_strUserSeed
                << rs.m_nSeedCarryOver
                << rs.m_nBias
                << value_preserving_cast<uint8_t>(rs.m_tbl.size());
            for (size_t i = size_t(0) ; i < rs.m_tbl.size() ; ++i)
            {
                ar << rs.m_tbl[i].m_bRoll
                    << rs.m_tbl[i].m_nDice
                    << rs.m_tbl[i].m_nFaces;
            }
            break;
        default:
            AfxThrowArchiveException(CArchiveException::badSchema);
    }
    return ar;
}

CArchive& operator>>(CArchive& ar, CRollState& rs)
{
    uint8_t schema, size;
    ar >> schema;
    switch (schema)
    {
        case 1:
            ar >> rs.m_bFirstRoll
                >> rs.m_nSetsToRoll
                >> rs.m_strUserSeed
                >> rs.m_nSeedCarryOver
                >> rs.m_nBias
                >> size;
            if (size != rs.m_tbl.size())
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
            for (size_t i = size_t(0) ; i < rs.m_tbl.size() ; ++i)
            {
                ar >> rs.m_tbl[i].m_bRoll
                    >> rs.m_tbl[i].m_nDice
                    >> rs.m_tbl[i].m_nFaces;
            }
            break;
        default:
            AfxThrowArchiveException(CArchiveException::badSchema);
    }
    return ar;
}

/////////////////////////////////////////////////////////////////////////////
// CDieRollerDlg dialog

CDieRollerDlg::CDieRollerDlg(wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CDieRollerDlg)
        CB_XRC_CTRL(m_btnResetSeed)
        CB_XRC_CTRL_VAL(m_editSeed, m_strSeed, wxFILTER_ASCII, 40)
        , m_btnOK(XRCCTRL(*this, "wxID_OK", std::remove_reference_t<decltype(*m_btnOK)>))
        CB_XRC_CTRL_VAL(m_chkRoll3, m_bRoll3)
        CB_XRC_CTRL_VAL(m_chkRoll2, m_bRoll2)
        CB_XRC_CTRL_VAL(m_chkRoll1, m_bRoll1)
        CB_XRC_CTRL_VAL(m_editSets, m_nSets, uint32_t(1), uint32_t(75))
        CB_XRC_CTRL_VAL(m_editDice1, m_nDice1, uint32_t(0), uint32_t(100))
        CB_XRC_CTRL_VAL(m_editFaces1, m_nFaces1, uint32_t(2), uint32_t(32767))
        CB_XRC_CTRL_VAL(m_editDice2, m_nDice2, uint32_t(0), uint32_t(100))
        CB_XRC_CTRL_VAL(m_editFaces2, m_nFaces2, uint32_t(2), uint32_t(32767))
        CB_XRC_CTRL_VAL(m_editDice3, m_nDice3, uint32_t(0), uint32_t(100))
        CB_XRC_CTRL_VAL(m_editFaces3, m_nFaces3, uint32_t(2), uint32_t(32767))
        CB_XRC_CTRL_VAL(m_editBias, m_nBias, -5000, 5000)
    CB_XRC_END_CTRLS_DEFN()
{
    m_strSeed = "";
    m_nFaces1 = uint32_t(6);
    m_nFaces2 = uint32_t(6);
    m_nFaces3 = uint32_t(6);
    m_nDice1 = uint32_t(1);
    m_nDice2 = uint32_t(0);
    m_nDice3 = uint32_t(0);
    m_bRoll1 = TRUE;
    m_bRoll2 = FALSE;
    m_bRoll3 = FALSE;
    m_nBias = int32_t(0);
    m_nSets = uint32_t(1);
    m_bFirstRoll = TRUE;
    m_strInitialSeed = "";

    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize editSize = m_editSets->GetSizeFromText("999");
    m_editSets->SetInitialSize(editSize);
    m_editDice1->SetInitialSize(editSize);
    m_editDice2->SetInitialSize(editSize);
    m_editDice3->SetInitialSize(editSize);
    editSize = m_editFaces1->GetSizeFromText("99999");
    m_editFaces1->SetInitialSize(editSize);
    m_editFaces2->SetInitialSize(editSize);
    m_editFaces3->SetInitialSize(editSize);
    m_editBias->SetInitialSize(editSize);
    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();
}

wxBEGIN_EVENT_TABLE(CDieRollerDlg, wxDialog)
    EVT_CHECKBOX(XRCID("m_chkRoll1"), OnDieRoll1)
    EVT_CHECKBOX(XRCID("m_chkRoll2"), OnDieRoll2)
    EVT_CHECKBOX(XRCID("m_chkRoll3"), OnDieRoll3)
    EVT_TEXT(XRCID("m_editDice1"), OnChangeDieNumdies1)
    EVT_TEXT(XRCID("m_editDice2"), OnChangeDieNumdies2)
    EVT_TEXT(XRCID("m_editDice3"), OnChangeDieNumdies3)
    EVT_BUTTON(XRCID("m_btnResetSeed"), OnDieResetSeed)
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
    IDC_DIE_RESET_SEED, IDH_DIE_RESET_SEED,
    IDC_DIE_SEED, IDH_DIE_SEED,
    IDC_DIE_ROLL3, IDH_DIE_ROLL,
    IDC_DIE_ROLL2, IDH_DIE_ROLL,
    IDC_DIE_ROLL1, IDH_DIE_ROLL,
    IDC_DIE_SEED, IDH_DIE_SEED,
    IDC_DIE_FACES1, IDH_DIE_FACES,
    IDC_DIE_FACES2, IDH_DIE_FACES,
    IDC_DIE_FACES3, IDH_DIE_FACES,
    IDC_DIE_NUMDIES1, IDH_DIE_NUMDIES,
    IDC_DIE_NUMDIES2, IDH_DIE_NUMDIES,
    IDC_DIE_NUMDIES3, IDH_DIE_NUMDIES,
    IDC_DIE_ROLL1, IDH_DIE_ROLL,
    IDC_DIE_ROLL2, IDH_DIE_ROLL,
    IDC_DIE_ROLL3, IDH_DIE_ROLL,
    IDC_DIE_BIAS, IDH_DIE_BIAS,
    IDC_DIE_SETS, IDH_DIE_SETS,
    0, 0
};
#endif

/////////////////////////////////////////////////////////////////////////////

void CDieRollerDlg::MakeFormattedRollResult()
{
    if (!IsAnyDieSelected())
    {
        CB::string temp = CB::string::LoadString(IDS_MSG_NOROLLS);
        m_strRoll = temp;
        return;
    }

    const OwnerPtr<CRollState> temp = GetRollState();
    const CRollState& rs = *temp;
    uint32_t nSets = rs.m_nSetsToRoll;
    if (nSets == uint32_t(0)) nSets = uint32_t(1);

    m_strRoll.clear();

    for (uint32_t nSet = uint32_t(0) ; nSet < nSets ; ++nSet)
    {
        // Start with special roll prefix character
        m_strRoll += "\xBB ";              // 0xBB = Chevron
        if (nSets > uint32_t(1))
        {
            CB::string str = CB::string::LoadString(IDS_MSG_ROLLSET);
            m_strRoll += str;
            str = std::format(" {}, ", nSet + uint32_t(1));
            m_strRoll += str;
        }
        // Show seed string if one was specified...
        CB::string strRollType;
        if (!m_strSeed.empty())
            strRollType = AfxFormatString1(IDS_MSG_ROLLWITHSEED, m_strSeed);
        else
            strRollType = CB::string::LoadString(IDS_MSG_ROLLWITHOUTSEED);
        m_strRoll += strRollType;

        // Build roll spec next...
        BOOL bPlusNeeded = FALSE;
        for (size_t i = size_t(0) ; i < rs.m_tbl.size() ; ++i)
        {
            if (rs.m_tbl[i].m_bRoll && rs.m_tbl[i].m_nDice > uint32_t(0))
            {
                if (bPlusNeeded)
                    m_strRoll += " + ";
                else
                    bPlusNeeded = TRUE;
                CB::string szBfr = std::format(L"{}D{}", rs.m_tbl[i].m_nDice,  rs.m_tbl[i].m_nFaces);
                m_strRoll += szBfr;
            }
        }
        if (rs.m_nBias != 0)
        {
            CB::string szBfr = std::format(L" {} {}", rs.m_nBias < 0 ? '-' : '+', abs(rs.m_nBias));
            m_strRoll += szBfr;
        }
        m_strRoll += "\r\n";
        CB::string str = CB::string::LoadString(IDS_MSG_ROLLRESULT);
        m_strRoll += str;

        // Now do the actual roll...
        bPlusNeeded = FALSE;
        uint32_t nDies = uint32_t(0);
        int32_t nRandomTotal = 0;
        for (size_t i = size_t(0) ; i < rs.m_tbl.size() ; ++i)
        {
            if (rs.m_tbl[i].m_bRoll && rs.m_tbl[i].m_nDice > uint32_t(0))
            {
                for (uint32_t j = uint32_t(0) ; j < rs.m_tbl[i].m_nDice ; ++j)
                {
                    int32_t nRandomNum;
                    if (m_bFirstRoll)       // Is this the first roll?
                    {
                        if (rs.m_strUserSeed.empty())
                        {
                            // No seed given
                            nRandomNum = CalcRandomNumber(1, rs.m_tbl[i].m_nFaces,
                                &m_nSeedCarryOver);
                        }
                        else
                        {
                            // Seed string given. Use MD5 to hash the string
                            // for a starting seed value.
                            MD5_CTX mdContext;
                            MD5Calc(&mdContext, rs.m_strUserSeed.a_str(),
                                rs.m_strUserSeed.a_size());

                            nRandomNum = CalcRandomNumberUsingSeed(1,
                                rs.m_tbl[i].m_nFaces,
                                *reinterpret_cast<uint32_t*>(mdContext.digest),
                                &m_nSeedCarryOver);
                        }
                        m_bFirstRoll = FALSE;
                    }
                    else
                        nRandomNum = CalcRandomNumberUsingSeed(1, rs.m_tbl[i].m_nFaces,
                            m_nSeedCarryOver, &m_nSeedCarryOver);

                    nRandomTotal += nRandomNum;
                    nDies++;
                    if (bPlusNeeded)
                        m_strRoll += " + ";
                    else
                        bPlusNeeded = TRUE;
                    CB::string szBfr = std::format(L"{}", nRandomNum);
                    m_strRoll += szBfr;
                }
            }
        }
        if (rs.m_nBias != 0)
        {
            nRandomTotal += rs.m_nBias;
            CB::string szBfr = std::format(L" [{} {}]", rs.m_nBias < 0 ? '-' : '+', abs(rs.m_nBias));
            m_strRoll += szBfr;
        }
        if (rs.m_nBias != 0 || nDies > uint32_t(1))
        {
            CB::string szBfr = std::format(L" = {}", nRandomTotal);
            m_strRoll += szBfr;
        }
        // Finish up...
        m_strRoll += "\r\n";
    }
}

/////////////////////////////////////////////////////////////////////////////

OwnerPtr<CRollState> CDieRollerDlg::GetRollState() const
{
    CRollState rstate;
    rstate.m_bFirstRoll = m_bFirstRoll;
    rstate.m_nSetsToRoll = m_nSets;
    rstate.m_strUserSeed = m_strSeed;
    rstate.m_nSeedCarryOver = m_nSeedCarryOver;

    rstate.m_nBias = m_nBias;

    rstate.m_tbl[0].m_bRoll = m_bRoll1;
    rstate.m_tbl[0].m_nDice = m_nDice1;
    rstate.m_tbl[0].m_nFaces = m_nFaces1;

    rstate.m_tbl[1].m_bRoll = m_bRoll2;
    rstate.m_tbl[1].m_nDice = m_nDice2;
    rstate.m_tbl[1].m_nFaces = m_nFaces2;

    rstate.m_tbl[2].m_bRoll = m_bRoll3;
    rstate.m_tbl[2].m_nDice = m_nDice3;
    rstate.m_tbl[2].m_nFaces = m_nFaces3;
    return MakeOwner<CRollState>(rstate);
}

void CDieRollerDlg::SetRollState(const CRollState& rstate)
{
    m_bFirstRoll = rstate.m_bFirstRoll;
    m_nSets = rstate.m_nSetsToRoll;
    m_strSeed = rstate.m_strUserSeed.wx_str();
    m_strInitialSeed = rstate.m_strUserSeed;    // Saved to detect changes

    m_nSeedCarryOver = rstate.m_nSeedCarryOver;

    m_nBias = rstate.m_nBias;

    m_bRoll1 = rstate.m_tbl[0].m_bRoll;
    m_nDice1 = rstate.m_tbl[0].m_nDice;
    m_nFaces1 = rstate.m_tbl[0].m_nFaces;

    m_bRoll2 = rstate.m_tbl[1].m_bRoll;
    m_nDice2 = rstate.m_tbl[1].m_nDice;
    m_nFaces2 = rstate.m_tbl[1].m_nFaces;

    m_bRoll3 = rstate.m_tbl[2].m_bRoll;
    m_nDice3 = rstate.m_tbl[2].m_nDice;
    m_nFaces3 = rstate.m_tbl[2].m_nFaces;
}

/////////////////////////////////////////////////////////////////////////////

bool CDieRollerDlg::IsAnyDieSelected() const
{
    static const auto GetDlgItemInt = [](const wxTextCtrl& tc) -> int
    {
        CB::string s = tc.GetValue();
        return std::stoi(s.w_str());
    };
    return (m_chkRoll1->GetValue() && GetDlgItemInt(*m_editDice1) > 0) ||
        (m_chkRoll2->GetValue() && GetDlgItemInt(*m_editDice2) > 0) ||
        (m_chkRoll3->GetValue() && GetDlgItemInt(*m_editDice3) > 0);
}

/////////////////////////////////////////////////////////////////////////////

void CDieRollerDlg::UpdateControls()
{
    m_btnOK->Enable(IsAnyDieSelected());
}

/////////////////////////////////////////////////////////////////////////////
// CDieRollerDlg message handlers

bool CDieRollerDlg::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }
    UpdateControls();

    if (!m_bFirstRoll && !m_strSeed.empty())
        m_editSeed->Enable(false);
    else
        m_btnResetSeed->Enable(false);

    return TRUE;
}

bool CDieRollerDlg::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    if (m_strInitialSeed.wx_str() != m_strSeed)
        m_bFirstRoll = TRUE;
    MakeFormattedRollResult();

    return true;
}

void CDieRollerDlg::OnDieRoll1(wxCommandEvent& /*event*/)
{
    UpdateControls();
}

void CDieRollerDlg::OnDieRoll2(wxCommandEvent& /*event*/)
{
    UpdateControls();
}

void CDieRollerDlg::OnDieRoll3(wxCommandEvent& /*event*/)
{
    UpdateControls();
}

void CDieRollerDlg::OnChangeDieNumdies1(wxCommandEvent& /*event*/)
{
    UpdateControls();
}

void CDieRollerDlg::OnChangeDieNumdies2(wxCommandEvent& /*event*/)
{
    UpdateControls();
}

void CDieRollerDlg::OnChangeDieNumdies3(wxCommandEvent& /*event*/)
{
    UpdateControls();
}

void CDieRollerDlg::OnDieResetSeed(wxCommandEvent& /*event*/)
{
    m_editSeed->Enable(true);
    m_btnResetSeed->Enable(false);
    m_bFirstRoll = TRUE;
}

#if 0
BOOL CDieRollerDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CDieRollerDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

