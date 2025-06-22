// DlgNewGeoBoard.cpp : implementation file
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

#include    "stdafx.h"
#include    "Gp.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "GamDoc.h"
#include    "GeoBoard.h"
#include    "DlgNewGeoBoard.h"
#include    "gphelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateGeomorphicBoardDialog dialog

CCreateGeomorphicBoardDialog::CCreateGeomorphicBoardDialog(CGamDoc& doc, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CCreateGeomorphicBoardDialog)
        , m_btnOK(XRCCTRL(*this, "wxID_OK", std::remove_reference_t<decltype(*m_btnOK)>))
        CB_XRC_CTRL(m_btnClearList)
        CB_XRC_CTRL(m_btnAddBreak)
        CB_XRC_CTRL(m_btnAddBoard)
        CB_XRC_CTRL(m_listGeo)
        CB_XRC_CTRL(m_editBoardName)
        CB_XRC_CTRL(m_listBoard)
    CB_XRC_END_CTRLS_DEFN(),
    m_pDoc(&doc)
{
    m_nCurrentRowHeight = size_t(0);
    m_nCurrentColumn = size_t(0);
    m_nMaxColumns = size_t(0);
    m_nRowNumber = size_t(0);
}

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CCreateGeomorphicBoardDialog, wxDialog)
    EVT_LISTBOX(XRCID("m_listBoard"), OnSelChangeBoardList)
    EVT_BUTTON(XRCID("m_btnAddBoard"), OnBtnPressedAddBoard)
    EVT_BUTTON(XRCID("m_btnAddBreak"), OnBtnPressedAddBreak)
    EVT_BUTTON(XRCID("m_btnClearList"), OnBtnPressClear)
    EVT_TEXT(XRCID("m_editBoardName"), OnChangeBoardName)
    EVT_LISTBOX_DCLICK(XRCID("m_listBoard"), OnDblClickBoardList)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_D_NEWGEO_HELP, OnBtnPressedHelp)
#endif
wxEND_EVENT_TABLE()

#if 0
/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_NEWGEO_CLEAR, IDH_D_NEWGEO_CLEAR,
    IDC_D_NEWGEO_ADD_BREAK, IDH_D_NEWGEO_ADD_BREAK,
    IDC_D_NEWGEO_ADD_BOARD, IDH_D_NEWGEO_ADD_BOARD,
    IDC_D_NEWGEO_GEOMORPHIC_LIST, IDH_D_NEWGEO_GEOMORPHIC_LIST,
    IDC_D_NEWGEO_BOARD_NAME, IDH_D_NEWGEO_BOARD_NAME,
    IDC_D_NEWGEO_BOARD_LIST, IDH_D_NEWGEO_BOARD_LIST,
    0,0
};
#endif

/////////////////////////////////////////////////////////////////////////////

void CCreateGeomorphicBoardDialog::LoadBoardListWithCompliantBoards()
{
    ResetContent(*m_listBoard);
    if (m_nMaxColumns != size_t(0) && m_nCurrentColumn == m_nMaxColumns)
        return;               // No boards allowed since row break is only option

    // All boards must use hexes and have an odd number of columns
    // or rows along the flat side of the hex.
    // New option:  cformRect
    CBoardManager& pBMgr = m_pDoc->GetBoardManager();

    for (size_t i = size_t(0); i < pBMgr.GetNumBoards(); i++)
    {
        CBoard& pBrd = pBMgr.GetBoard(i);
        CBoardArray& pBArray = pBrd.GetBoardArray();
        const CCellForm& pCellForm = pBArray.GetCellForm(fullScale);

        if (!(pCellForm.GetCellType() == cformHexFlat ||
              pCellForm.GetCellType() == cformHexPnt ||
              pCellForm.GetCellType() == cformRect))
            continue;                           // These maps aren't compliant at all
        if (static_cast<BoardID::UNDERLYING_TYPE>(pBrd.GetSerialNumber()) >= GEO_BOARD_SERNUM_BASE)
            continue;                           // Can't build geo maps from geo maps

        for (auto r : { Rotation90::r0, Rotation90::r90, Rotation90::r180, Rotation90::r270 })
        {
            /* boards with square cells can be rotated 90/270
                because the rotated cells will match the
                original, all others only match when rotated
                180 */
            if (r == Rotation90::r90 || r == Rotation90::r270)
            {
                if (!(pCellForm.GetCellType() == cformRect &&
                    pCellForm.GetCellSize().cx == pCellForm.GetCellSize().cy))
                {
                    continue;
                }
            }

            std::unique_ptr<CGeoBoardElement> ge(new CGeoBoardElement(Invalid_v<size_t>, Invalid_v<size_t>, pBrd.GetSerialNumber(), r));
            const CBoard& pBrd = pBMgr.Get(*ge);
            const CBoardArray& pBArray = pBrd.GetBoardArray();
            const CCellForm& pCellForm = pBArray.GetCellForm(fullScale);

            if (m_pRootBoard != NULL)
            {
                // Already have at least one map selected. The rest must
                // comply with the geometry of the root one.
                const CBoard& rootBoard = pBMgr.Get(*m_pRootBoard);
                const CBoardArray& rootBArray = rootBoard.GetBoardArray();
                const CCellForm& rootCellForm = rootBArray.GetCellForm(fullScale);
                if (!rootCellForm.CompareEqual(pCellForm))
                    continue;
                /* probably could handle GEV mismatch by making the
                    row/col check smarter, but don't bother until
                    someone provides a use case worth analyzing */
                bool gevMismatch = false;
                for (Edge e : { Edge::Top, Edge::Bottom, Edge::Left, Edge::Right })
                {
                    if (pBrd.IsGEVStyle(e) != rootBoard.IsGEVStyle(e))
                    {
                        gevMismatch = true;
                        break;
                    }
                }
                if (gevMismatch)
                {
                    continue;
                }
                if (m_nMaxColumns != size_t(0) && m_tblColWidth[m_nCurrentColumn] != pBArray.GetCols())
                    continue;
                if (m_nCurrentColumn > size_t(0) && pBArray.GetRows() != m_nCurrentRowHeight)
                    continue;
            }
            static const CB::string suffix[] = { "", " -  90°", " - 180°", " - 270°" };
            int nItem = m_listBoard->Append(pBrd.GetName() + suffix[ptrdiff_t(r)]);
            m_listBoard->SetClientData(value_preserving_cast<unsigned int>(nItem), ge.release());
        }
    }
    if (m_listBoard->GetCount() > 0)
        m_listBoard->SetSelection(0);
}

void CCreateGeomorphicBoardDialog::UpdateButtons()
{
    BOOL bEnableOK = !m_editBoardName->GetValue().empty() &&
         m_listGeo->GetCount() > size_t(0) &&
        (m_nCurrentColumn == m_nMaxColumns || m_nMaxColumns == size_t(0));
    m_btnOK->Enable(bEnableOK);

    if (m_pRootBoard == NULL)
    {
        // Nothing added yet. Don't allow row break.
        m_btnAddBreak->Enable(FALSE);
        m_btnAddBoard->Enable(m_listBoard->GetSelection() != wxNOT_FOUND);
    }
    else if (m_nMaxColumns == size_t(0) && !m_tblColWidth.empty())
    {
        // At least one column but no row break yet. Allow anything.
        m_btnAddBreak->Enable(TRUE);
        m_btnAddBoard->Enable(m_listBoard->GetSelection() != wxNOT_FOUND);
    }
    else if (m_nMaxColumns > size_t(0) && m_nCurrentColumn == m_nMaxColumns)
    {
        m_btnAddBreak->Enable(TRUE);
        m_btnAddBoard->Enable(FALSE);
    }
    else
    {
        // Somewhere other than the end of a follow on row
        // of boards. Only allow adding a board.
        m_btnAddBreak->Enable(FALSE);
        m_btnAddBoard->Enable(m_listBoard->GetSelection() != wxNOT_FOUND);
    }

    m_btnClearList->Enable(m_listGeo->GetCount() > 0);
}

OwnerPtr<CGeomorphicBoard> CCreateGeomorphicBoardDialog::DetachGeomorphicBoard()
{
    return std::move(m_pGeoBoard);
}

/////////////////////////////////////////////////////////////////////////////
// CCreateGeomorphicBoardDialog message handlers

void CCreateGeomorphicBoardDialog::OnSelChangeBoardList(wxCommandEvent& /*event*/)
{
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnChangeBoardName(wxCommandEvent& /*event*/)
{
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnDblClickBoardList(wxCommandEvent& event)
{
    if (m_btnAddBoard->IsEnabled())
        OnBtnPressedAddBoard(event);
}

void CCreateGeomorphicBoardDialog::OnBtnPressedAddBoard(wxCommandEvent& /*event*/)
{
    wxASSERT(m_listBoard->GetSelection() != wxNOT_FOUND);
    CGeoBoardElement& ge = *static_cast<CGeoBoardElement*>(m_listBoard->GetClientData(m_listBoard->GetSelection()));
    CBoardManager& pBMgr = m_pDoc->GetBoardManager();
    const CBoard& pBrd = pBMgr.Get(ge);
    const CBoardArray& pBArray = pBrd.GetBoardArray();
    m_pRootBoard.reset(new CGeoBoardElement(ge));

    if (m_nMaxColumns == size_t(0))
        m_tblColWidth.push_back(pBArray.GetCols());

    if (m_nCurrentRowHeight == size_t(0))
        m_nCurrentRowHeight = pBArray.GetRows();

    CB::string strLabel = m_listBoard->GetString(m_listBoard->GetSelection());

    int nItem = m_listGeo->Append(strLabel);
    m_listGeo->SetClientData(value_preserving_cast<unsigned int>(nItem), new CGeoBoardElement(ge));

    m_nCurrentColumn++;
    if (m_nCurrentColumn == m_nMaxColumns)
    {
        m_btnAddBoard->Enable(FALSE);
        m_btnAddBreak->Enable(TRUE);
    }
    else if (m_nMaxColumns == size_t(0) && !m_tblColWidth.empty())
        m_btnAddBreak->Enable(TRUE);

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnBtnPressedAddBreak(wxCommandEvent& /*event*/)
{
    CB::string str = CB::string::LoadString(IDS_ROW_BREAK);

    int nItem = m_listGeo->Append(str);
    m_listGeo->SetClientData(value_preserving_cast<unsigned int>(nItem), nullptr);

    m_nMaxColumns = m_tblColWidth.size();
    m_nRowNumber++;
    m_nCurrentColumn = size_t(0);
    m_nCurrentRowHeight = size_t(0);

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnBtnPressClear(wxCommandEvent& /*event*/)
{
    ResetContent(*m_listGeo);
    m_nMaxColumns = size_t(0);
    m_nCurrentColumn = size_t(0);
    m_nCurrentRowHeight = size_t(0);
    m_nRowNumber = size_t(0);
    m_pRootBoard = NULL;
    m_tblColWidth.clear();

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
}

bool CCreateGeomorphicBoardDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
    // update layout after filling in m_listBoard
    Fit();
    Centre();

    return TRUE;
}

bool CCreateGeomorphicBoardDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    m_pGeoBoard = MakeOwner<CGeomorphicBoard>(*m_pDoc);

    CB::string strName = m_editBoardName->GetValue();
    m_pGeoBoard->SetName(strName);
    m_pGeoBoard->SetSerialNumber(m_pDoc->GetPBoardManager().IssueGeoSerialNumber());

    m_pGeoBoard->SetBoardRowCount(m_nRowNumber + size_t(1));
    m_pGeoBoard->SetBoardColCount(m_tblColWidth.size());

    for (unsigned int i = 0u; i < m_listGeo->GetCount(); i++)
    {
        CGeoBoardElement* ge = static_cast<CGeoBoardElement*>(m_listGeo->GetClientData(i));
        if (!ge)
            continue;                       // Skip the row break
        m_pGeoBoard->AddElement(ge->m_nBoardSerialNum, ge->m_rotation);
    }

    return true;
}

#if 0
BOOL CCreateGeomorphicBoardDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CCreateGeomorphicBoardDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

void CCreateGeomorphicBoardDialog::OnBtnPressedHelp()
{
    GetApp()->DoHelpTopic("gp-adv-geomorphic.htm");
}
#endif

// see https://docs.wxwidgets.org/latest/classwx_window_destroy_event.html
CCreateGeomorphicBoardDialog::~CCreateGeomorphicBoardDialog()
{
    ResetContent(*m_listBoard);
    ResetContent(*m_listGeo);
}

void CCreateGeomorphicBoardDialog::ResetContent(wxListBox& lb)
{
    for (unsigned int i = 0u; i < lb.GetCount(); ++i)
    {
        delete static_cast<CGeoBoardElement*>(lb.GetClientData(i));
    }
    lb.Clear();
}
