// DlgPbprp.cpp - Playing board properties
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
#include    "GdiTools.h"
#include    "Player.h"
#include    "DlgPbprp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPBrdPropDialog dialog


CPBrdPropDialog::CPBrdPropDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CPBrdPropDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPBrdPropDialog)
    m_bGridSnap = FALSE;
    m_bSmallCellBorders = FALSE;
    m_bCellBorders = FALSE;
    m_xStackStagger = 0;
    m_yStackStagger = 0;
    m_strBoardName = "";
    m_bGridRectCenters = FALSE;
    m_bSnapMovePlot = FALSE;
    m_fXGridSnapOff = 0.0f;
    m_fYGridSnapOff = 0.0f;
    m_fXGridSnap = 0.0f;
    m_fYGridSnap = 0.0f;
    m_bOpenBoardOnLoad = FALSE;
    m_bShowSelListAndTinyMap = FALSE;
    m_bDrawLockedBeneath = TRUE;
    //}}AFX_DATA_INIT
    m_crPlotColor = RGB(0,0,0);
    m_nPlotWd = 1;
    m_bNonOwnerAccess = FALSE;
    m_nOwnerSel = -1;
    m_pPlayerMgr = NULL;
    m_bOwnerInfoIsReadOnly = FALSE;
}

void CPBrdPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPBrdPropDialog)
    DDX_Control(pDX, IDC_D_PBPRP_NONOWNER_ACCESS, m_chkAllowAccess);
    DDX_Control(pDX, IDC_D_PBPRP_OWNER_LIST, m_comboOwners);
    DDX_Control(pDX, IDC_D_PBPRP_OWNER_LABEL, m_staticOwnerLabel);
    DDX_Control(pDX, IDC_D_PBPRP_PLOTCOLOR, m_cpPlotColor);
    DDX_Control(pDX, IDC_D_PBPRP_PLOTWIDTH, m_comboPlotWd);
    DDX_Check(pDX, IDC_D_PBPRP_SNAPON, m_bGridSnap);
    DDX_Check(pDX, IDC_D_PBPRP_S_CELLBORDER, m_bSmallCellBorders);
    DDX_Check(pDX, IDC_D_PBPRP_L_CELLBORDER, m_bCellBorders);
    DDX_Text(pDX, IDC_D_PBPRP_XPCE_STGGR, m_xStackStagger);
    DDV_MinMaxInt(pDX, m_xStackStagger, -256, 256);
    DDX_Text(pDX, IDC_D_PBPRP_YPCE_STGGR, m_yStackStagger);
    DDV_MinMaxInt(pDX, m_yStackStagger, -256, 256);
    DDX_Text(pDX, IDC_D_PBPRP_BOARDNAME, m_strBoardName);
    DDX_Check(pDX, IDC_D_PBPRP_S_RECTCENTER, m_bGridRectCenters);
    DDX_Check(pDX, IDC_D_PBPRP_S_SNAPPLOTS, m_bSnapMovePlot);
    DDX_Text(pDX, IDC_D_PBPRP_XOFFSET, m_fXGridSnapOff);
    DDV_MinMaxFloat(pDX, m_fXGridSnapOff, 0.f, 255.999f);
    DDX_Text(pDX, IDC_D_PBPRP_YOFFSET, m_fYGridSnapOff);
    DDV_MinMaxFloat(pDX, m_fYGridSnapOff, 0.f, 255.999f);
    DDX_Text(pDX, IDC_D_PBPRP_XPIXELS, m_fXGridSnap);
    DDV_MinMaxFloat(pDX, m_fXGridSnap, 2.f, 256.f);
    DDX_Text(pDX, IDC_D_PBPRP_YPIXELS, m_fYGridSnap);
    DDV_MinMaxFloat(pDX, m_fYGridSnap, 2.f, 256.f);
    DDX_Check(pDX, IDC_D_PBPRP_AUTOOPEN, m_bOpenBoardOnLoad);
    DDX_Check(pDX, IDC_D_PBPRP_SHOW_SEL_AND_TINY, m_bShowSelListAndTinyMap);
    DDX_Check(pDX, IDC_D_PBPRP_DRAW_LOCKED_BENEATH, m_bDrawLockedBeneath);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPBrdPropDialog, CDialog)
    //{{AFX_MSG_MAP(CPBrdPropDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_PBPRP_NONOWNER_ACCESS, IDH_D_PBPRP_NONOWNER_ACCESS,
    IDC_D_PBPRP_OWNER_LIST, IDH_D_PBPRP_OWNER_LIST,
    IDC_D_PBPRP_OWNER_LABEL, IDH_D_PBPRP_OWNER_LABEL,
    IDC_D_PBPRP_PLOTCOLOR, IDH_D_PBPRP_PLOTCOLOR,
    IDC_D_PBPRP_PLOTWIDTH, IDH_D_PBPRP_PLOTWIDTH,
    IDC_D_PBPRP_SNAPON, IDH_D_PBPRP_SNAPON,
    IDC_D_PBPRP_S_CELLBORDER, IDH_D_PBPRP_S_CELLBORDER,
    IDC_D_PBPRP_L_CELLBORDER, IDH_D_PBPRP_L_CELLBORDER,
    IDC_D_PBPRP_XPCE_STGGR, IDH_D_PBPRP_XPCE_STGGR,
    IDC_D_PBPRP_YPCE_STGGR, IDH_D_PBPRP_YPCE_STGGR,
    IDC_D_PBPRP_BOARDNAME, IDH_D_PBPRP_BOARDNAME,
    IDC_D_PBPRP_S_RECTCENTER, IDH_D_PBPRP_S_RECTCENTER,
    IDC_D_PBPRP_S_SNAPPLOTS, IDH_D_PBPRP_S_SNAPPLOTS,
    IDC_D_PBPRP_XOFFSET, IDH_D_PBPRP_XOFFSET,
    IDC_D_PBPRP_YOFFSET, IDH_D_PBPRP_YOFFSET,
    IDC_D_PBPRP_XPIXELS, IDH_D_PBPRP_XPIXELS,
    IDC_D_PBPRP_YPIXELS, IDH_D_PBPRP_YPIXELS,
    IDC_D_PBPRP_AUTOOPEN, IDH_D_PBPRP_AUTOOPEN,
    IDC_D_PBPRP_SHOW_SEL_AND_TINY, IDH_D_PBPRP_SHOW_SEL_AND_TINY,
    0,0
};

BOOL CPBrdPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPBrdPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CPBrdPropDialog message handlers

void CPBrdPropDialog::OnOK()
{
    m_crPlotColor = m_cpPlotColor.GetColor();

    int nIdx = m_comboPlotWd.GetCurSel();
    if (nIdx >= 0)
        m_nPlotWd = nIdx + 1;

    CDialog::OnOK();

    // Make sure these are within grid size

    m_xGridSnapOff = (UINT)(m_fXGridSnapOff * 1000 + 0.5);
    m_yGridSnapOff = (UINT)(m_fYGridSnapOff * 1000 + 0.5);
    m_xGridSnap = (UINT)(m_fXGridSnap * 1000 + 0.5);
    m_yGridSnap = (UINT)(m_fYGridSnap * 1000 + 0.5);

    ASSERT(m_xGridSnap > 0 && m_yGridSnap > 0);
    if (m_xGridSnap > 0)
        m_xGridSnapOff = m_xGridSnapOff % m_xGridSnap;
    if (m_yGridSnap > 0)
        m_yGridSnapOff = m_yGridSnapOff % m_yGridSnap;

    if (m_pPlayerMgr != NULL && !m_bOwnerInfoIsReadOnly)
    {
        m_nOwnerSel = m_comboOwners.GetCurSel() - 1;
        m_bNonOwnerAccess = m_chkAllowAccess.GetCheck() != 0;
    }
}

BOOL CPBrdPropDialog::OnInitDialog()
{
    m_fXGridSnapOff = (float)m_xGridSnapOff / 1000;
    m_fYGridSnapOff = (float)m_yGridSnapOff / 1000;
    m_fXGridSnap = (float)m_xGridSnap / 1000;
    m_fYGridSnap = (float)m_yGridSnap / 1000;

    CDialog::OnInitDialog();

    m_cpPlotColor.SetColor(m_crPlotColor);

    if (m_nPlotWd <= 0)
        m_nPlotWd = 1;
    ASSERT((int)m_nPlotWd <= m_comboPlotWd.GetCount());
    m_comboPlotWd.SetCurSel(m_nPlotWd - 1);

    if (m_pPlayerMgr == NULL)
    {
        m_comboOwners.EnableWindow(FALSE);
        m_staticOwnerLabel.EnableWindow(FALSE);
        m_chkAllowAccess.SetCheck(0);
        m_chkAllowAccess.EnableWindow(FALSE);
    }
    else
    {
        CString str;
        str.LoadString(IDS_LBL_NO_OWNER);
        m_comboOwners.AddString(str);
        for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
            m_comboOwners.AddString(m_pPlayerMgr->GetAt(i).m_strName);
        m_comboOwners.SetCurSel(m_nOwnerSel + 1);
        m_chkAllowAccess.SetCheck(m_bNonOwnerAccess ? 1 : 0);
    }
    if (m_bOwnerInfoIsReadOnly)
    {
        m_comboOwners.EnableWindow(FALSE);
        m_chkAllowAccess.EnableWindow(FALSE);
    }

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPBrdPropDialog::OnBnClickedDPbprpShowSelAndTiny2()
{
    // TODO: Add your control notification handler code here
}
