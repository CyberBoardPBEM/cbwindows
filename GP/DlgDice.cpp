// DlgDice.cpp : implementation file
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

#include "stdafx.h"
#include "Gp.h"
#include "GMisc.h"
#include "DlgDice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDieRollerDlg dialog

CDieRollerDlg::CDieRollerDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDieRollerDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDieRollerDlg)
    m_strSeed = "";
    m_nFaces1 = 6;
    m_nFaces2 = 6;
    m_nFaces3 = 6;
    m_nDice1 = 1;
    m_nDice2 = 0;
    m_nDice3 = 0;
    m_bRoll1 = TRUE;
    m_bRoll2 = FALSE;
    m_bRoll3 = FALSE;
    m_nBias = 0;
    m_nSets = 1;
    //}}AFX_DATA_INIT
    m_bFirstRoll = TRUE;
    m_strInitialSeed = "";
}

void CDieRollerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDieRollerDlg)
    DDX_Control(pDX, IDC_DIE_RESET_SEED, m_btnResetSeed);
    DDX_Control(pDX, IDC_DIE_SEED, m_editSeed);
    DDX_Control(pDX, IDOK, m_btnOK);
    DDX_Control(pDX, IDC_DIE_ROLL3, m_chkRoll3);
    DDX_Control(pDX, IDC_DIE_ROLL2, m_chkRoll2);
    DDX_Control(pDX, IDC_DIE_ROLL1, m_chkRoll1);
    DDX_Text(pDX, IDC_DIE_SEED, m_strSeed);
    DDV_MaxChars(pDX, m_strSeed, 40);
    DDX_Text(pDX, IDC_DIE_FACES1, m_nFaces1);
    DDV_MinMaxUInt(pDX, m_nFaces1, 2, 32767);
    DDX_Text(pDX, IDC_DIE_FACES2, m_nFaces2);
    DDV_MinMaxUInt(pDX, m_nFaces2, 2, 32767);
    DDX_Text(pDX, IDC_DIE_FACES3, m_nFaces3);
    DDV_MinMaxUInt(pDX, m_nFaces3, 2, 32767);
    DDX_Text(pDX, IDC_DIE_NUMDIES1, m_nDice1);
    DDV_MinMaxUInt(pDX, m_nDice1, 0, 100);
    DDX_Text(pDX, IDC_DIE_NUMDIES2, m_nDice2);
    DDV_MinMaxUInt(pDX, m_nDice2, 0, 100);
    DDX_Text(pDX, IDC_DIE_NUMDIES3, m_nDice3);
    DDV_MinMaxUInt(pDX, m_nDice3, 0, 100);
    DDX_Check(pDX, IDC_DIE_ROLL1, m_bRoll1);
    DDX_Check(pDX, IDC_DIE_ROLL2, m_bRoll2);
    DDX_Check(pDX, IDC_DIE_ROLL3, m_bRoll3);
    DDX_Text(pDX, IDC_DIE_BIAS, m_nBias);
    DDV_MinMaxInt(pDX, m_nBias, -5000, 5000);
    DDX_Text(pDX, IDC_DIE_SETS, m_nSets);
    DDV_MinMaxUInt(pDX, m_nSets, 1, 75);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDieRollerDlg, CDialog)
    //{{AFX_MSG_MAP(CDieRollerDlg)
    ON_BN_CLICKED(IDC_DIE_ROLL1, OnDieRoll1)
    ON_BN_CLICKED(IDC_DIE_ROLL2, OnDieRoll2)
    ON_BN_CLICKED(IDC_DIE_ROLL3, OnDieRoll3)
    ON_EN_CHANGE(IDC_DIE_NUMDIES1, OnChangeDieNumdies1)
    ON_EN_CHANGE(IDC_DIE_NUMDIES2, OnChangeDieNumdies2)
    ON_EN_CHANGE(IDC_DIE_NUMDIES3, OnChangeDieNumdies3)
    ON_BN_CLICKED(IDC_DIE_RESET_SEED, OnDieResetSeed)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

/////////////////////////////////////////////////////////////////////////////

void CDieRollerDlg::MakeFormattedRollResult()
{
    TCHAR szBfr[256];           // EGAD I HATE HARD CODED NUMBERS!
    CString str;

    if (!IsAnyDieSelected())
    {
        m_strRoll.LoadString(IDS_MSG_NOROLLS);
        return;
    }

    CRollState rs;
    GetRollState(rs);
    int nSets = rs.m_nSetsToRoll;
    if (nSets <= 0) nSets = 1;

    m_strRoll.Empty();

    for (int nSet = 0; nSet < nSets; nSet++)
    {
        // Start with special roll prefix character
        m_strRoll += "\xBB ";              // 0xBB = Chevron
        if (nSets > 1)
        {
            CString str;
            str.LoadString(IDS_MSG_ROLLSET);
            m_strRoll += str;
            str.Format(" %d, ", nSet + 1);
            m_strRoll += str;
        }
        // Show seed string if one was specified...
        CString strRollType;
        if (!m_strSeed.IsEmpty())
            AfxFormatString1(strRollType, IDS_MSG_ROLLWITHSEED, m_strSeed);
        else
            strRollType.LoadString(IDS_MSG_ROLLWITHOUTSEED);
        m_strRoll += strRollType;

        // Build roll spec next...
        BOOL bPlusNeeded = FALSE;
        for (int i = 0; i < DICETYPES; i++)
        {
            if (rs.m_tbl[i].m_bRoll && rs.m_tbl[i].m_nDice > 0)
            {
                if (bPlusNeeded)
                    m_strRoll += " + ";
                else
                    bPlusNeeded = TRUE;
                sprintf(szBfr, "%dD%d", rs.m_tbl[i].m_nDice,  rs.m_tbl[i].m_nFaces);
                m_strRoll += szBfr;
            }
        }
        if (rs.m_nBias != 0)
        {
            sprintf(szBfr, " %c %d", rs.m_nBias < 0 ? '-' : '+', abs(rs.m_nBias));
            m_strRoll += szBfr;
        }
        m_strRoll += "\r\n";
        str.LoadString(IDS_MSG_ROLLRESULT);
        m_strRoll += str;

        // Now do the actual roll...
        bPlusNeeded = FALSE;
        int nDies = 0;
        int nRandomTotal = 0;
        for (int i = 0; i < DICETYPES; i++)
        {
            if (rs.m_tbl[i].m_bRoll && rs.m_tbl[i].m_nDice > 0)
            {
                for (UINT j = 0; j < rs.m_tbl[i].m_nDice; j++)
                {
                    int nRandomNum;
                    if (m_bFirstRoll)       // Is this the first roll?
                    {
                        if (rs.m_strUserSeed.IsEmpty())
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
                            MD5Calc(&mdContext, (BYTE*)(LPCTSTR)rs.m_strUserSeed,
                                rs.m_strUserSeed.GetLength());

                            nRandomNum = CalcRandomNumberUsingSeed(1,
                                rs.m_tbl[i].m_nFaces,
                                *((UINT*)mdContext.digest),
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
                    sprintf(szBfr, "%d", nRandomNum);
                    m_strRoll += szBfr;
                }
            }
        }
        if (rs.m_nBias != 0)
        {
            nRandomTotal += rs.m_nBias;
            sprintf(szBfr, " [%c %d]", rs.m_nBias < 0 ? '-' : '+', abs(rs.m_nBias));
            m_strRoll += szBfr;
        }
        if (rs.m_nBias != 0 || nDies > 1)
        {
            sprintf(szBfr, " = %d", nRandomTotal);
            m_strRoll += szBfr;
        }
        // Finish up...
        m_strRoll += "\r\n";
    }
}

/////////////////////////////////////////////////////////////////////////////

CRollState* CDieRollerDlg::GetRollState()
{
    CRollState* pRState = new CRollState;
    GetRollState(*pRState);
    return pRState;
}

void CDieRollerDlg::GetRollState(CRollState& rstate)
{
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
}

void CDieRollerDlg::SetRollState(CRollState* pRState, BOOL bTakeOwnership /* = TRUE */)
{
    SetRollState(*pRState);
    if (bTakeOwnership)
        delete pRState;
}

void CDieRollerDlg::SetRollState(CRollState& rstate)
{
    m_bFirstRoll = rstate.m_bFirstRoll;
    m_nSets = rstate.m_nSetsToRoll;
    m_strSeed = rstate.m_strUserSeed;
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

BOOL CDieRollerDlg::IsAnyDieSelected()
{
    return (m_chkRoll1.GetCheck() && GetDlgItemInt(IDC_DIE_NUMDIES1) > 0) ||
        (m_chkRoll2.GetCheck() && GetDlgItemInt(IDC_DIE_NUMDIES2) > 0) ||
        (m_chkRoll3.GetCheck() && GetDlgItemInt(IDC_DIE_NUMDIES3) > 0);
}

/////////////////////////////////////////////////////////////////////////////

void CDieRollerDlg::UpdateControls()
{
    m_btnOK.EnableWindow(IsAnyDieSelected());
}

/////////////////////////////////////////////////////////////////////////////
// CDieRollerDlg message handlers

BOOL CDieRollerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    UpdateControls();

    if (!m_bFirstRoll && !m_strSeed.IsEmpty())
        m_editSeed.EnableWindow(FALSE);
    else
        m_btnResetSeed.EnableWindow(FALSE);

    return TRUE;
}

void CDieRollerDlg::OnOK()
{
    CDialog::OnOK();
    if (m_strInitialSeed != m_strSeed)
        m_bFirstRoll = TRUE;
    MakeFormattedRollResult();
}

void CDieRollerDlg::OnDieRoll1()
{
    UpdateControls();
}

void CDieRollerDlg::OnDieRoll2()
{
    UpdateControls();
}

void CDieRollerDlg::OnDieRoll3()
{
    UpdateControls();
}

void CDieRollerDlg::OnChangeDieNumdies1()
{
    UpdateControls();
}

void CDieRollerDlg::OnChangeDieNumdies2()
{
    UpdateControls();
}

void CDieRollerDlg::OnChangeDieNumdies3()
{
    UpdateControls();
}

void CDieRollerDlg::OnDieResetSeed()
{
    m_editSeed.EnableWindow(TRUE);
    m_btnResetSeed.EnableWindow(FALSE);
    m_bFirstRoll = TRUE;
}

BOOL CDieRollerDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CDieRollerDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

