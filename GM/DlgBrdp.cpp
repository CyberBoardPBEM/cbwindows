// DlgBrdp.cpp : implementation file
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

#include    "stdafx.h"
#include    "Gm.h"
#include    "BrdCell.h"
#include    "CellForm.h"
#include    "GdiTools.h"
#include    "DlgBrdp.h"
#include    "DlgBrdsz.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardPropDialog dialog


CBoardPropDialog::CBoardPropDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    /* m_dummy is a way to call LoadDialog()
        before the Refs are initialized */
    m_dummy(wxXmlResource::Get()->LoadDialog(this, parent, "CBoardPropDialog") ? this : nullptr),
    m_staticHalfPixelWidth(XRCCTRL(*this, "m_staticHalfPixelWidth", wxStaticText)),
    m_staticSmallPixelWidth(XRCCTRL(*this, "m_staticSmallPixelWidth", wxStaticText)),
    m_staticSmallPixelHeight(XRCCTRL(*this, "m_staticSmallPixelHeight", wxStaticText)),
    m_staticHalfPixelHeight(XRCCTRL(*this, "m_staticHalfPixelHeight", wxStaticText)),
    m_staticPixelWidth(XRCCTRL(*this, "m_staticPixelWidth", wxStaticText)),
    m_staticPixelHeight(XRCCTRL(*this, "m_staticPixelHeight", wxStaticText)),
    m_editBrdName(XRCCTRL(*this, "m_editBrdName", wxTextCtrl)),
    m_cpCellFrame(XRCCTRL(*this, "m_cpCellFrame", wxColourPickerCtrl)),
    m_staticWidth(XRCCTRL(*this, "m_staticWidth", wxStaticText)),
    m_staticHeight(XRCCTRL(*this, "m_staticHeight", wxStaticText)),
    m_staticRows(XRCCTRL(*this, "m_staticRows", wxStaticText)),
    m_staticCols(XRCCTRL(*this, "m_staticCols", wxStaticText)),
    m_comboStyle(XRCCTRL(*this, "m_comboStyle", wxChoice)),
    m_chkCellLines(XRCCTRL(*this, "m_chkCellLines", wxCheckBox)),
    m_chkGridSnap(XRCCTRL(*this, "m_chkGridSnap", wxCheckBox)),
    m_chkTrackCellNum(XRCCTRL(*this, "m_chkTrackCellNum", wxCheckBox)),
    m_editRowTrkOffset(XRCCTRL(*this, "m_editRowTrkOffset", wxTextCtrl)),
    m_editColTrkOffset(XRCCTRL(*this, "m_editColTrkOffset", wxTextCtrl)),
    m_editXGridSnapOff(XRCCTRL(*this, "m_editXGridSnapOff", wxTextCtrl)),
    m_editYGridSnapOff(XRCCTRL(*this, "m_editYGridSnapOff", wxTextCtrl)),
    m_editXGridSnap(XRCCTRL(*this, "m_editXGridSnap", wxTextCtrl)),
    m_editYGridSnap(XRCCTRL(*this, "m_editYGridSnap", wxTextCtrl)),
    m_chkColTrkInvert(XRCCTRL(*this, "m_chkColTrkInvert", wxCheckBox)),
    m_chkRowTrkInvert(XRCCTRL(*this, "m_chkRowTrkInvert", wxCheckBox)),
    m_chkCellBorderOnTop(XRCCTRL(*this, "m_chkCellBorderOnTop", wxCheckBox)),
    m_chkEnableXParentCells(XRCCTRL(*this, "m_chkEnableXParentCells", wxCheckBox))
{
    m_bCellLines = false;
    m_bGridSnap = false;
    m_bTrackCellNum = false;
    m_nRowTrkOffset = 0;
    m_nColTrkOffset = 0;
    m_fXGridSnapOff = 0.0f;
    m_fYGridSnapOff = 0.0f;
    m_fXGridSnap = 16.0f;
    m_fYGridSnap = 16.0f;
    m_strName = "";
    m_nStyleNum = -1;
    m_bColTrkInvert = false;
    m_bRowTrkInvert = false;
    m_bCellBorderOnTop = false;
    m_bEnableXParentCells = false;

    m_xGridSnap = 16u;
    m_yGridSnap = 16u;
    m_xGridSnapOff = 0u;
    m_yGridSnapOff = 0u;
    m_bShapeChanged = FALSE;
    m_crCellFrame = *wxBLACK;
    m_eCellStyle = cformRect;

    m_chkCellLines->SetValidator(wxGenericValidator(&m_bCellLines));
    m_chkGridSnap->SetValidator(wxGenericValidator(&m_bGridSnap));
    m_chkTrackCellNum->SetValidator(wxGenericValidator(&m_bTrackCellNum));
    m_editRowTrkOffset->SetValidator(CB::MakeValidator(&m_nRowTrkOffset, -250, 250));
    m_editColTrkOffset->SetValidator(CB::MakeValidator(&m_nColTrkOffset, -250, 250));
    m_editXGridSnapOff->SetValidator(CB::MakeValidator(&m_fXGridSnapOff, 0.f, 255.999f));
    m_editYGridSnapOff->SetValidator(CB::MakeValidator(&m_fYGridSnapOff, 0.f, 255.999f));
    m_editXGridSnap->SetValidator(CB::MakeValidator(&m_fXGridSnap, 2.f, 256.f));
    m_editYGridSnap->SetValidator(CB::MakeValidator(&m_fYGridSnap, 2.f, 256.f));
    m_editBrdName->SetValidator(wxTextValidator(wxFILTER_EMPTY, &m_strName));
    m_editBrdName->SetMaxLength(32);
    m_cpCellFrame->SetValidator(wxGenericValidator(&m_crCellFrame));
    m_comboStyle->SetValidator(wxGenericValidator(&m_nStyleNum));
    m_chkColTrkInvert->SetValidator(wxGenericValidator(&m_bColTrkInvert));
    m_chkRowTrkInvert->SetValidator(wxGenericValidator(&m_bRowTrkInvert));
    m_chkCellBorderOnTop->SetValidator(wxGenericValidator(&m_bCellBorderOnTop));
    m_chkEnableXParentCells->SetValidator(wxGenericValidator(&m_bEnableXParentCells));

    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize editSize = m_editXGridSnapOff->GetSizeFromText("999.999");
    m_editRowTrkOffset->SetInitialSize(editSize);
    m_editColTrkOffset->SetInitialSize(editSize);
    m_editXGridSnapOff->SetInitialSize(editSize);
    m_editYGridSnapOff->SetInitialSize(editSize);
    m_editXGridSnap->SetInitialSize(editSize);
    m_editYGridSnap->SetInitialSize(editSize);
    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();
}

wxBEGIN_EVENT_TABLE(CBoardPropDialog, wxDialog)
    EVT_BUTTON(XRCID("OnReshape"), CBoardPropDialog::OnReshape)
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
    IDC_D_BRDPRP_BOARDNAME, IDH_D_BRDPRP_BOARDNAME,
    IDC_D_BRDPRP_BORDERTOP, IDH_D_BRDPRP_BORDERTOP,
    IDC_D_BRDPRP_CELLBORDER, IDH_D_BRDPRP_CELLBORDER,
    IDC_D_BRDPRP_CELLCOLOR, IDH_D_BRDPRP_CELLCOLOR,
    IDC_D_BRDPRP_COLS, IDH_D_BRDPRP_COLS,
    IDC_D_BRDPRP_HEIGHT, IDH_D_BRDPRP_HEIGHT,
    IDC_D_BRDPRP_INVCOLS, IDH_D_BRDPRP_INVCOLS,
    IDC_D_BRDPRP_INVROWS, IDH_D_BRDPRP_INVROWS,
    IDC_D_BRDPRP_PXLHEIGHT, IDH_D_BRDPRP_PXLHEIGHT,
    IDC_D_BRDPRP_PXLWIDTH, IDH_D_BRDPRP_PXLWIDTH,
    IDC_D_BRDPRP_RESHAPE, IDH_D_BRDPRP_RESHAPE,
    IDC_D_BRDPRP_ROWS, IDH_D_BRDPRP_ROWS,
    IDC_D_BRDPRP_SNAPON, IDH_D_BRDPRP_SNAPON,
    IDC_D_BRDPRP_TRACK, IDH_D_BRDPRP_TRACK,
    IDC_D_BRDPRP_TRKCOFF, IDH_D_BRDPRP_TRKCOFF,
    IDC_D_BRDPRP_TRKROFF, IDH_D_BRDPRP_TRKROFF,
    IDC_D_BRDPRP_TRKSTYLE, IDH_D_BRDPRP_TRKSTYLE,
    IDC_D_BRDPRP_WIDTH, IDH_D_BRDPRP_WIDTH,
    IDC_D_BRDPRP_XOFFSET, IDH_D_BRDPRP_XOFFSET,
    IDC_D_BRDPRP_XPARENTCELLS, IDH_D_BRDPRP_XPARENTCELLS,
    IDC_D_BRDPRP_XPIXELS, IDH_D_BRDPRP_XPIXELS,
    IDC_D_BRDPRP_YOFFSET, IDH_D_BRDPRP_YOFFSET,
    IDC_D_BRDPRP_YPIXELS, IDH_D_BRDPRP_YPIXELS,
    0,0
};

BOOL CBoardPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CBoardPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardPropDialog message handlers

bool CBoardPropDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    // Make sure these are within grid size

    m_xGridSnapOff = value_preserving_cast<uint32_t>(std::round(m_fXGridSnapOff * 1000));
    m_yGridSnapOff = value_preserving_cast<uint32_t>(std::round(m_fYGridSnapOff * 1000));
    m_xGridSnap = value_preserving_cast<uint32_t>(std::round(m_fXGridSnap * 1000));
    m_yGridSnap = value_preserving_cast<uint32_t>(std::round(m_fYGridSnap * 1000));

    ASSERT(m_xGridSnap > 0u && m_yGridSnap > 0u);
    if (m_xGridSnap > 0u)
        m_xGridSnapOff = m_xGridSnapOff % m_xGridSnap;
    if (m_yGridSnap > 0u)
        m_yGridSnapOff = m_yGridSnapOff % m_yGridSnap;

    return true;
}

bool CBoardPropDialog::TransferDataToWindow()
{
    m_fXGridSnapOff = value_preserving_cast<float>(m_xGridSnapOff) / 1000.f;
    m_fYGridSnapOff = value_preserving_cast<float>(m_yGridSnapOff) / 1000.f;
    m_fXGridSnap = value_preserving_cast<float>(m_xGridSnap) / 1000.f;
    m_fYGridSnap = value_preserving_cast<float>(m_yGridSnap) / 1000.f;

    UpdateInfoArea();

    return wxDialog::TransferDataToWindow();
}

void CBoardPropDialog::OnReshape(wxCommandEvent& /*event*/)
{
    if (wxMessageBox(CB::string::LoadString(IDS_WARN_RESHAPE),
                        CB::GetAppName(),
                        wxOK | wxCANCEL | wxICON_EXCLAMATION
        ) == wxOK)
    {
        CBoardReshapeDialog dlg;

        dlg.m_eCellStyle = m_eCellStyle;
        dlg.m_nRows = m_nRows;
        dlg.m_nCols = m_nCols;
        dlg.m_nCellHt = m_nCellHt;
        dlg.m_nCellWd = m_nCellWd;
        dlg.m_bStagger = m_bStagger;
        if (m_eCellStyle == cformRect)
        {
            dlg.m_bStagger = CellStagger::Out;
        }

        if (dlg.DoModal() == IDOK)
        {
            m_bShapeChanged = TRUE;

            m_nRows = dlg.m_nRows;
            m_nCols = dlg.m_nCols;
            m_nCellHt = dlg.m_nCellHt;
            m_nCellWd = dlg.m_nCellWd;

            m_bStagger = dlg.m_bStagger;
            if (m_eCellStyle == cformRect)
            {
                m_bStagger = CellStagger::Invalid;
            }

            UpdateInfoArea();
        }
    }
}

//////////////////////////////////////////////////////////////////

void CBoardPropDialog::UpdateInfoArea()
{
    CB::string szNum = std::format("{}", m_nRows);
    m_staticRows->SetLabel(szNum);
    szNum = std::format("{}", m_nCols);
    m_staticCols->SetLabel(szNum);

    if (m_bShapeChanged && m_eCellStyle == cformHexPnt)
        szNum = "??";
    else
        szNum = std::format("{}", m_nCellHt);
    m_staticHeight->SetLabel(szNum);

    if (m_bShapeChanged && m_eCellStyle == cformHexFlat)
        szNum = "??";
    else
        szNum = std::format("{}", m_nCellWd);

    m_staticWidth->SetLabel(szNum);

    CCellForm cfFull;
    CCellForm cfHalf;
    CCellForm cfSmall;

    CBoardArray::GenerateCellDefs(m_eCellStyle,
        value_preserving_cast<int32_t>(m_eCellStyle == cformHexPnt ? m_nCellWd : m_nCellHt), value_preserving_cast<int32_t>(m_nCellWd), m_bStagger,
        cfFull, cfHalf, cfSmall);

    CSize size = cfFull.CalcBoardSize(m_nRows, m_nCols);
    szNum = std::format("{}", size.cx);
    m_staticPixelWidth->SetLabel(szNum);
    szNum = std::format("{}", size.cy);
    m_staticPixelHeight->SetLabel(szNum);

    size = cfHalf.CalcBoardSize(m_nRows, m_nCols);
    szNum = std::format("{}", size.cx);
    m_staticHalfPixelWidth->SetLabel(szNum);
    szNum = std::format("{}", size.cy);
    m_staticHalfPixelHeight->SetLabel(szNum);

    size = cfSmall.CalcBoardSize(m_nRows, m_nCols);
    szNum = std::format("{}", size.cx);
    m_staticSmallPixelWidth->SetLabel(szNum);
    szNum = std::format("{}", size.cy);
    m_staticSmallPixelHeight->SetLabel(szNum);
}

