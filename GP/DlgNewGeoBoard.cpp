// DlgNewGeoBoard.cpp : implementation file
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

CCreateGeomorphicBoardDialog::CCreateGeomorphicBoardDialog(CGamDoc& doc, CWnd* pParent /*=NULL*/)
    : CDialog(CCreateGeomorphicBoardDialog::IDD, pParent),
    m_pDoc(&doc)
{
    //{{AFX_DATA_INIT(CCreateGeomorphicBoardDialog)
    //}}AFX_DATA_INIT
    m_pGeoBoard = NULL;
    m_pRootBoard = NULL;
    m_nCurrentRowHeight = size_t(0);
    m_nCurrentColumn = size_t(0);
    m_nMaxColumns = size_t(0);
    m_nRowNumber = size_t(0);
}

/////////////////////////////////////////////////////////////////////////////

void CCreateGeomorphicBoardDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCreateGeomorphicBoardDialog)
    DDX_Control(pDX, IDOK, m_btnOK);
    DDX_Control(pDX, IDC_D_NEWGEO_CLEAR, m_btnClearList);
    DDX_Control(pDX, IDC_D_NEWGEO_ADD_BREAK, m_btnAddBreak);
    DDX_Control(pDX, IDC_D_NEWGEO_ADD_BOARD, m_btnAddBoard);
    DDX_Control(pDX, IDC_D_NEWGEO_GEOMORPHIC_LIST, m_listGeo);
    DDX_Control(pDX, IDC_D_NEWGEO_BOARD_NAME, m_editBoardName);
    DDX_Control(pDX, IDC_D_NEWGEO_BOARD_LIST, m_listBoard);
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CCreateGeomorphicBoardDialog, CDialog)
    //{{AFX_MSG_MAP(CCreateGeomorphicBoardDialog)
    ON_LBN_SELCHANGE(IDC_D_NEWGEO_BOARD_LIST, OnSelChangeBoardList)
    ON_BN_CLICKED(IDC_D_NEWGEO_ADD_BOARD, OnBtnPressedAddBoard)
    ON_BN_CLICKED(IDC_D_NEWGEO_ADD_BREAK, OnBtnPressedAddBreak)
    ON_BN_CLICKED(IDC_D_NEWGEO_CLEAR, OnBtnPressClear)
    ON_EN_CHANGE(IDC_D_NEWGEO_BOARD_NAME, OnChangeBoardName)
    ON_LBN_DBLCLK(IDC_D_NEWGEO_BOARD_LIST, OnDblClickBoardList)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_D_NEWGEO_HELP, OnBtnPressedHelp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

/////////////////////////////////////////////////////////////////////////////

void CCreateGeomorphicBoardDialog::LoadBoardListWithCompliantBoards()
{
    m_listBoard.ResetContent();
    if (m_nMaxColumns != size_t(0) && m_nCurrentColumn == m_nMaxColumns)
        return;               // No boards allowed since row break is only option

    // All boards must use hexes and have an odd number of columns
    // or rows along the flat side of the hex.
    CBoardManager* pBMgr = m_pDoc->GetBoardManager();

    for (size_t i = 0; i < pBMgr->GetNumBoards(); i++)
    {
        CBoard& pBrd = pBMgr->GetBoard(i);
        CBoardArray& pBArray = pBrd.GetBoardArray();
        const CCellForm& pCellForm = pBArray.GetCellForm(fullScale);

        if (!(pCellForm.GetCellType() == cformHexFlat ||
              pCellForm.GetCellType() == cformHexPnt))
            continue;                           // These maps aren't compliant at all
        if (static_cast<BoardID::UNDERLYING_TYPE>(pBrd.GetSerialNumber()) >= GEO_BOARD_SERNUM_BASE)
            continue;                           // Can't build geo maps from geo maps

        if (m_pRootBoard != NULL)
        {
            // Already have at least one map selected. The rest must
            // comply with the geometry of the root one.
            const CBoardArray& rootBArray = m_pRootBoard->GetBoardArray();
            const CCellForm& rootCellForm = rootBArray.GetCellForm(fullScale);
            if (!rootCellForm.CompareEqual(pCellForm))
                continue;
            /* probably could handle GEV mismatch by making the
                row/col check smarter, but don't bother until
                someone provides a use case worth analyzing */
            bool gevMismatch = false;
            for (Edge e : { Edge::Top, Edge::Bottom, Edge::Left, Edge::Right })
            {
                if (pBrd.IsGEVStyle(e) != m_pRootBoard->IsGEVStyle(e))
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
        int nItem = m_listBoard.AddString(pBrd.GetName());
        m_listBoard.SetItemData(nItem, value_preserving_cast<DWORD_PTR>(static_cast<BoardID::UNDERLYING_TYPE>(pBrd.GetSerialNumber())));
    }
    if (m_listBoard.GetCount() > 0)
        m_listBoard.SetCurSel(0);
}

void CCreateGeomorphicBoardDialog::UpdateButtons()
{
    BOOL bEnableOK = m_editBoardName.GetWindowTextLength() > size_t(0) &&
         m_listGeo.GetCount() > size_t(0) &&
        (m_nCurrentColumn == m_nMaxColumns || m_nMaxColumns == size_t(0));
    m_btnOK.EnableWindow(bEnableOK);

    if (m_pRootBoard == NULL)
    {
        // Nothing added yet. Don't allow row break.
        m_btnAddBreak.EnableWindow(FALSE);
        m_btnAddBoard.EnableWindow(m_listBoard.GetCurSel() >= 0);
    }
    else if (m_nMaxColumns == size_t(0) && !m_tblColWidth.empty())
    {
        // At least one column but no row break yet. Allow anything.
        m_btnAddBreak.EnableWindow(TRUE);
        m_btnAddBoard.EnableWindow(m_listBoard.GetCurSel() >= 0);
    }
    else if (m_nMaxColumns > size_t(0) && m_nCurrentColumn == m_nMaxColumns)
    {
        m_btnAddBreak.EnableWindow(TRUE);
        m_btnAddBoard.EnableWindow(FALSE);
    }
    else
    {
        // Somewhere other than the end of a follow on row
        // of boards. Only allow adding a board.
        m_btnAddBreak.EnableWindow(FALSE);
        m_btnAddBoard.EnableWindow(m_listBoard.GetCurSel() >= 0);
    }

    m_btnClearList.EnableWindow(m_listGeo.GetCount() > 0);
}

CGeomorphicBoard* CCreateGeomorphicBoardDialog::DetachGeomorphicBoard()
{
    CGeomorphicBoard* pGeoBoard = m_pGeoBoard;
    m_pGeoBoard = NULL;
    return pGeoBoard;
}

/////////////////////////////////////////////////////////////////////////////
// CCreateGeomorphicBoardDialog message handlers

void CCreateGeomorphicBoardDialog::OnSelChangeBoardList()
{
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnChangeBoardName()
{
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnDblClickBoardList()
{
    if (m_btnAddBoard.IsWindowEnabled())
        OnBtnPressedAddBoard();
}

void CCreateGeomorphicBoardDialog::OnBtnPressedAddBoard()
{
    ASSERT(m_listBoard.GetCurSel() >= 0);
    BoardID dwItemData = static_cast<BoardID>(m_listBoard.GetItemData(m_listBoard.GetCurSel()));

    CBoardManager* pBMgr = m_pDoc->GetBoardManager();
    size_t nBrdNum = pBMgr->FindBoardBySerial(dwItemData);
    ASSERT(nBrdNum != Invalid_v<size_t>);
    CBoard& pBrd = pBMgr->GetBoard(nBrdNum);
    CBoardArray& pBArray = pBrd.GetBoardArray();
    m_pRootBoard = &pBrd;

    if (m_nMaxColumns == size_t(0))
        m_tblColWidth.push_back(pBArray.GetCols());

    if (m_nCurrentRowHeight == size_t(0))
        m_nCurrentRowHeight = pBArray.GetRows();

    CString strLabel = pBrd.GetName();

    int nItem = m_listGeo.AddString(strLabel);
    m_listGeo.SetItemData(nItem, value_preserving_cast<DWORD_PTR>(static_cast<BoardID::UNDERLYING_TYPE>(dwItemData)));

    m_nCurrentColumn++;
    if (m_nCurrentColumn == m_nMaxColumns)
    {
        m_btnAddBoard.EnableWindow(FALSE);
        m_btnAddBreak.EnableWindow(TRUE);
    }
    else if (m_nMaxColumns == size_t(0) && !m_tblColWidth.empty())
        m_btnAddBreak.EnableWindow(TRUE);

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnBtnPressedAddBreak()
{
    CString str;
    str.LoadString(IDS_ROW_BREAK);

    int nItem = m_listGeo.AddString(str);
    m_listGeo.SetItemData(nItem, value_preserving_cast<DWORD_PTR>(static_cast<BoardID::UNDERLYING_TYPE>(nullBid)));

    m_nMaxColumns = m_tblColWidth.size();
    m_nRowNumber++;
    m_nCurrentColumn = size_t(0);
    m_nCurrentRowHeight = size_t(0);

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
}

void CCreateGeomorphicBoardDialog::OnBtnPressClear()
{
    m_listGeo.ResetContent();
    m_nMaxColumns = size_t(0);
    m_nCurrentColumn = size_t(0);
    m_nCurrentRowHeight = size_t(0);
    m_nRowNumber = size_t(0);
    m_pRootBoard = NULL;
    m_tblColWidth.clear();

    LoadBoardListWithCompliantBoards();
    UpdateButtons();
}

BOOL CCreateGeomorphicBoardDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    LoadBoardListWithCompliantBoards();
    UpdateButtons();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateGeomorphicBoardDialog::OnOK()
{
    m_pGeoBoard = new CGeomorphicBoard(*m_pDoc);

    CString strName;
    m_editBoardName.GetWindowText(strName);
    m_pGeoBoard->SetName(strName);
    m_pGeoBoard->SetSerialNumber(m_pDoc->GetPBoardManager()->IssueGeoSerialNumber());

    m_pGeoBoard->SetBoardRowCount(m_nRowNumber + size_t(1));
    m_pGeoBoard->SetBoardColCount(m_tblColWidth.size());

    for (int i = 0; i < m_listGeo.GetCount(); i++)
    {
        BoardID dwItemData = static_cast<BoardID>(m_listGeo.GetItemData(i));
        if (nullBid == dwItemData)
            continue;                       // Skip the row break
        m_pGeoBoard->AddElement(dwItemData);
    }

    CDialog::OnOK();
}

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

