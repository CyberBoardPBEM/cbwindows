// DlbMkbrd.cpp
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

#include    "stdafx.h"
#include    "Gm.h"
#include    "CellForm.h"
#include    "DlgMkbrd.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridType dialog

CGridType::CGridType(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CGridType)
        CB_XRC_CTRL(m_staticPixelSize)
        CB_XRC_CTRL(m_chkStagger)
        CB_XRC_CTRL_VAL(m_editRows, m_iRows, size_t(1), size_t(1000))
        CB_XRC_CTRL_VAL(m_editCols, m_iCols, size_t(1), size_t(1000))
        CB_XRC_CTRL_VAL(m_editCellWd, m_iCellWd, 4, 32000)
        CB_XRC_CTRL_VAL(m_editCellHt, m_iCellHt, 4, 32000)
        CB_XRC_CTRL_VAL(m_radioRect, m_nBoardTypeHelper)
        CB_XRC_CTRL_VAL(m_editBoardName, m_strBoardName, wxFILTER_EMPTY, 32)
    CB_XRC_END_CTRLS_DEFN()
{
    m_iCellWd = 0;
    m_iCellHt = 0;
    m_iCols = size_t(0);
    m_iRows = size_t(0);
    m_strBoardName = "";
    m_bStagger = CellStagger::Invalid;
    m_nBoardType = cformRect;

    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize editSize = m_editRows->GetSizeFromText("99999");
    m_editRows->SetInitialSize(editSize);
    m_editCols->SetInitialSize(editSize);
    m_editCellWd->SetInitialSize(editSize);
    m_editCellHt->SetInitialSize(editSize);
    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();

    /* KLUDGE:  validator uses ChangeValue() instead of SetValue()
                (see https://groups.google.com/g/wx-dev/c/XQUU4vMjyqI/m/nPQGFABRAAAJ)
    */
    auto OnEditKillFocus = [this](wxFocusEvent& event) {
        UpdateBoardDimensions();
        event.Skip();
    };
    m_editRows->Bind(wxEVT_KILL_FOCUS, OnEditKillFocus);
    m_editCols->Bind(wxEVT_KILL_FOCUS, OnEditKillFocus);
    m_editCellWd->Bind(wxEVT_KILL_FOCUS, OnEditKillFocus);
    m_editCellHt->Bind(wxEVT_KILL_FOCUS, OnEditKillFocus);
}

wxBEGIN_EVENT_TABLE(CGridType, wxDialog)
    EVT_RADIOBUTTON(XRCID("m_radioHBrick"), OnHBrick)
    EVT_RADIOBUTTON(XRCID("m_radioVBrick"), OnVBrick)
    EVT_RADIOBUTTON(XRCID("m_radioRect"), OnRectCell)
    EVT_RADIOBUTTON(XRCID("m_radioHexFlat"), OnHexFlat)
    EVT_RADIOBUTTON(XRCID("m_radioHexPnt"), OnHexPnt)
    EVT_TEXT(XRCID("m_editCellHt"), OnChangeDNewbrdCellheight)
    EVT_TEXT(XRCID("m_editCellWd"), OnChangeDNewbrdCellwidth)
    EVT_TEXT(XRCID("m_editCols"), OnChangeDNewbrdGridcols)
    EVT_TEXT(XRCID("m_editRows"), OnChangeDNewbrdGridrows)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

#if 0
static DWORD adwHelpMap[] =
{
    IDC_D_NEWBRD_BOARDNAME, IDH_D_NEWBRD_BOARDNAME,
    IDC_D_NEWBRD_CELLHEIGHT, IDH_D_NEWBRD_CELLHEIGHT,
    IDC_D_NEWBRD_CELLWIDTH, IDH_D_NEWBRD_CELLWIDTH,
    IDC_D_NEWBRD_GRIDCOLS, IDH_D_NEWBRD_GRIDCOLS,
    IDC_D_NEWBRD_GRIDROWS, IDH_D_NEWBRD_GRIDROWS,
    IDC_D_NEWBRD_HBRICK, IDH_D_NEWBRD_HBRICK,
    IDC_D_NEWBRD_HEXFLAT, IDH_D_NEWBRD_HEXFLAT,
    IDC_D_NEWBRD_HEXPNT, IDH_D_NEWBRD_HEXPNT,
    IDC_D_NEWBRD_PIXSIZE, IDH_D_NEWBRD_PIXSIZE,
    IDC_D_NEWBRD_RECT, IDH_D_NEWBRD_RECT,
    IDC_D_NEWBRD_STAGGERIN, IDH_D_NEWBRD_STAGGERIN,
    IDC_D_NEWBRD_VBRICK, IDH_D_NEWBRD_VBRICK,
    0,0
};

BOOL CGridType::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CGridType::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridType message handlers

namespace {
    template<typename T>
    class DisableRangeCheck
    {
    public:
        DisableRangeCheck(wxIntegerValidator<T>& v) :
            val(v)
        {
            val.GetRange(min, max);
            val.SetRange(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
        }
        ~DisableRangeCheck()
        {
            val.SetRange(min, max);
        }
    private:
        wxIntegerValidator<T>& val;
        T min, max;
    };
}

class CGridType::DisableRangeCheck
{
public:
    DisableRangeCheck(CGridType& d) :
        cellWd(dynamic_cast<wxIntegerValidator<int>&>(*d.m_editCellWd->GetValidator())),
        cellHt(dynamic_cast<wxIntegerValidator<int>&>(*d.m_editCellHt->GetValidator())),
        cols(dynamic_cast<wxIntegerValidator<size_t>&>(*d.m_editCols->GetValidator())),
        rows(dynamic_cast<wxIntegerValidator<size_t>&>(*d.m_editRows->GetValidator()))
    {
    }
private:
    ::DisableRangeCheck<int> cellWd;
    ::DisableRangeCheck<int> cellHt;
    ::DisableRangeCheck<size_t> cols;
    ::DisableRangeCheck<size_t> rows;
};

bool CGridType::TransferDataFromWindowRangeCheck(bool check)
{
    std::unique_ptr<DisableRangeCheck> disableRangeCheck;
    if (!check)
    {
        disableRangeCheck.reset(new DisableRangeCheck(*this));
    }

    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    m_bStagger = static_cast<CellStagger>(m_chkStagger->GetValue());
    wxASSERT(m_bStagger == CellStagger::In || m_bStagger == CellStagger::Out);

    m_nBoardType = static_cast<CellFormType>(m_nBoardTypeHelper);

    // Now check if board is too large....

    CCellForm cf;           // Use a CellForm to do this
    if (m_nBoardType == cformHexPnt)// Only first param is used in this case
        m_iCellHt = m_iCellWd;

    if (!check)
    {
        return true;
    }

    cf.CreateCell(m_nBoardType, m_iCellHt, m_iCellWd);

    if (!cf.CalcTrialBoardSize(m_iRows, m_iCols))
    {
        AfxMessageBox(IDS_ERR_BOARDSIZE, MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    return true;
}

void CGridType::OnHBrick(wxCommandEvent& /*event*/)
{
    m_editCellHt->Enable(TRUE);
    m_editCellWd->Enable(TRUE);
    m_chkStagger->Enable(TRUE);
    UpdateBoardDimensions();
}

void CGridType::OnVBrick(wxCommandEvent& /*event*/)
{
    m_editCellHt->Enable(TRUE);
    m_editCellWd->Enable(TRUE);
    m_chkStagger->Enable(TRUE);
    UpdateBoardDimensions();
}

void CGridType::OnRectCell(wxCommandEvent& /*event*/)
{
    m_editCellHt->Enable(TRUE);
    m_editCellWd->Enable(TRUE);
    m_chkStagger->Enable(FALSE);
    UpdateBoardDimensions();
}

void CGridType::OnHexFlat(wxCommandEvent& /*event*/)
{
    m_editCellHt->Enable(TRUE);
    m_editCellWd->Enable(FALSE);
    m_chkStagger->Enable(TRUE);
    UpdateBoardDimensions();
}

void CGridType::OnHexPnt(wxCommandEvent& /*event*/)
{
    m_editCellHt->Enable(FALSE);
    m_editCellWd->Enable(TRUE);
    m_chkStagger->Enable(TRUE);
    UpdateBoardDimensions();
}

void CGridType::UpdateBoardDimensions()
{
    if (!TransferDataFromWindowRangeCheck(false))
    {
        return;
    }

    if (m_nBoardType == cformRect)
    {
        m_bStagger = CellStagger::Invalid;
    }

    if (m_nBoardType == cformHexPnt)    // Only first param is used
        m_iCellHt = m_iCellWd;

    if (!(m_iRows > size_t(0) && m_iCols > size_t(0) && m_iCellHt >= 4 && m_iCellWd >= 4))
    {
        CB::string str = CB::string::LoadString(IDS_BSIZE_INVALID);
        m_staticPixelSize->SetLabel(str);
        return;
    }

    CCellForm cf;
    // large cell values can cause memory alloc exception
    try
    {
        cf.CreateCell(m_nBoardType, m_iCellHt, m_iCellWd,
            m_bStagger);
    }
    catch (...)
    {
        CPP20_TRACE("ignore exception");
    }
    if (cf.CalcTrialBoardSize(m_iRows, m_iCols))
    {
        CB::string str = CB::string::LoadString(IDS_BSIZE_PATTERN);
        CSize size = cf.CalcBoardSize(m_iRows, m_iCols);
        CB::string szBfr = std::vformat(str, std::make_wformat_args(size.cy, size.cx));
        m_staticPixelSize->SetLabel(szBfr);
    }
    else
    {
        CB::string str = CB::string::LoadString(IDS_BSIZE_INVALID);
        m_staticPixelSize->SetLabel(str);
    }
}

bool CGridType::TransferDataToWindow()
{
    m_nBoardTypeHelper = m_nBoardType;
    if (!wxDialog::TransferDataToWindow())
    {
        wxASSERT(!"TransferDataToWindow failed");
        return false;
    }

    // MFC DDX_Check forces to 0 if out of range
    if (m_bStagger != CellStagger::In &&
        m_bStagger != CellStagger::Out)
    {
        m_bStagger = CellStagger::Out;
    }
    m_chkStagger->SetValue(static_cast<bool>(m_bStagger));

    wxCommandEvent dummy;
    if (m_nBoardType == cformHexFlat)      // Hex flat up
        OnHexFlat(dummy);
    else if (m_nBoardType == cformHexPnt)
        OnHexPnt(dummy);

    UpdateBoardDimensions();

    return true;
}

void CGridType::OnChangeDNewbrdCellheight(wxCommandEvent& /*event*/)
{
    UpdateBoardDimensions();
}

void CGridType::OnChangeDNewbrdCellwidth(wxCommandEvent& /*event*/)
{
    UpdateBoardDimensions();
}

void CGridType::OnChangeDNewbrdGridcols(wxCommandEvent& /*event*/)
{
    UpdateBoardDimensions();
}

void CGridType::OnChangeDNewbrdGridrows(wxCommandEvent& /*event*/)
{
    UpdateBoardDimensions();
}
