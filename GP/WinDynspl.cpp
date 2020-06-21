// WinDynspl.cpp
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

#include "StdAfx.h"
#include "WinDynSpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CCbSplitterWnd, CSplitterWndEx)

/////////////////////////////////////////////////////////////////////////////
// CCbSplitterWnd

CCbSplitterWnd::CCbSplitterWnd()
{
    m_nHiddenCol = -1;
    m_nHiddenRow = -1;
}

CCbSplitterWnd::~CCbSplitterWnd()
{
    // TODO: add destruction code here.
}

BEGIN_MESSAGE_MAP(CCbSplitterWnd, CSplitterWndEx)
	// Message handlers are added here.
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCbSplitterWnd  message handlers

void CCbSplitterWnd::HideRow(int nRowHide)
{
    ASSERT_VALID( this );
    ASSERT( m_nRows > 1 );
    ASSERT( nRowHide < m_nRows );
    ASSERT( m_nHiddenRow == -1 );
    m_nHiddenRow = nRowHide;

    int nActiveRow, nActiveCol;

    // if the nRow has an active window -- change it
    if (GetActivePane( &nActiveRow, &nActiveCol ) != NULL)
    {
        if (nActiveRow == nRowHide)
        {
            if (++nActiveRow >= m_nRows)
                nActiveRow = 0;
            SetActivePane( nActiveRow, nActiveCol );
        }
    }

    // hide all nRow panes.
    for (int nCol = 0; nCol < m_nCols; ++nCol)
    {
        CWnd* pPaneHide = GetPane( nRowHide, nCol );
        ASSERT( pPaneHide != NULL );

        pPaneHide->ShowWindow( SW_HIDE );
        pPaneHide->SetDlgCtrlID( AFX_IDW_PANE_FIRST+nCol * 16+m_nRows );

        for (int nRow = nRowHide+1; nRow < m_nRows; ++nRow)
        {
            CWnd* pPane = GetPane( nRow, nCol );
            ASSERT( pPane != NULL );

            pPane->SetDlgCtrlID( IdFromRowCol( nRow-1, nCol ));
        }
    }

    m_nRows--;
    m_pRowInfo[m_nRows].nCurSize = m_pRowInfo[nRowHide].nCurSize;
    RecalcLayout();
}

void CCbSplitterWnd::ShowRow()
{
    ASSERT_VALID(this);
    ASSERT(m_nRows < m_nMaxRows);
    ASSERT(m_nHiddenRow != -1);

    int nShowRow = m_nHiddenRow;
    m_nHiddenRow = -1;

    int cyNew = m_pRowInfo[m_nRows].nCurSize;
    m_nRows++;

    ASSERT(m_nRows == m_nMaxRows);

    int nRow;

    // Show the hidden nRow
    for (int nCol = 0; nCol < m_nCols; ++nCol)
    {
        CWnd* pPaneShow = GetDlgItem( AFX_IDW_PANE_FIRST+nCol * 16+m_nRows );
        ASSERT( pPaneShow != NULL );
        pPaneShow->ShowWindow( SW_SHOWNA );

        for (nRow = m_nRows - 2; nRow >= nShowRow; --nRow)
        {
            CWnd* pPane = GetPane( nCol, nRow );
            ASSERT( pPane != NULL );
            pPane->SetDlgCtrlID( IdFromRowCol( nRow + 1, nCol ));
        }

        pPaneShow->SetDlgCtrlID( IdFromRowCol( nShowRow, nCol ));
    }

    // New panes have been created -- recalculate layout
    for (nRow = nShowRow+1; nRow < m_nRows; nRow++)
        m_pRowInfo[nRow].nIdealSize = m_pRowInfo[nRow - 1].nCurSize;

    m_pRowInfo[nShowRow].nIdealSize = cyNew;
    RecalcLayout();
}

void CCbSplitterWnd::HideColumn(int nColHide)
{
    ASSERT_VALID( this );
    ASSERT( m_nCols > 1 );
    ASSERT( nColHide < m_nCols );
    ASSERT( m_nHiddenCol == -1 );
    m_nHiddenCol = nColHide;

    // if the column has an active window -- change it
    int nActiveRow, nActiveCol;
    if (GetActivePane( &nActiveRow, &nActiveCol ) != NULL)
    {
        if (nActiveCol == nColHide)
        {
            if (++nActiveCol >= m_nCols)
                nActiveCol = 0;
            SetActivePane( nActiveRow, nActiveCol );
        }
    }

    // hide all column panes
    for (int nRow = 0; nRow < m_nRows; nRow++)
    {
        CWnd* pPaneHide = GetPane(nRow, nColHide);
        ASSERT( pPaneHide != NULL );

        pPaneHide->ShowWindow(SW_HIDE);
        pPaneHide->SetDlgCtrlID( AFX_IDW_PANE_FIRST+nRow * 16+m_nCols );

        for (int nCol = nColHide + 1; nCol < m_nCols; nCol++)
        {
            CWnd* pPane = GetPane( nRow, nCol );
            ASSERT( pPane != NULL );

            pPane->SetDlgCtrlID( IdFromRowCol( nRow, nCol - 1 ));
        }
    }

    m_nCols--;
    m_pColInfo[m_nCols].nCurSize = m_pColInfo[nColHide].nCurSize;
    RecalcLayout();
}

void CCbSplitterWnd::ShowColumn()
{
    ASSERT_VALID( this );
    ASSERT( m_nCols < m_nMaxCols );
    ASSERT( m_nHiddenCol != -1 );

    int nShowCol = m_nHiddenCol;
    m_nHiddenCol = -1;

    int cxNew = m_pColInfo[m_nCols].nCurSize;
    m_nCols++;  // add a column

    ASSERT( m_nCols == m_nMaxCols );

    int nCol;

    // Show the hidden column
    for (int nRow = 0; nRow < m_nRows; ++nRow)
    {
        CWnd* pPaneShow = GetDlgItem( AFX_IDW_PANE_FIRST+nRow * 16+m_nCols );
        ASSERT( pPaneShow != NULL );
        pPaneShow->ShowWindow( SW_SHOWNA );

        for (nCol = m_nCols - 2; nCol >= nShowCol; --nCol)
        {
            CWnd* pPane = GetPane( nRow, nCol );
            ASSERT( pPane != NULL );
            pPane->SetDlgCtrlID( IdFromRowCol( nRow, nCol + 1 ));
        }

        pPaneShow->SetDlgCtrlID( IdFromRowCol( nRow, nShowCol ));
    }

    // new panes have been created -- recalculate layout
    for (nCol = nShowCol+1; nCol < m_nCols; nCol++)
        m_pColInfo[nCol].nIdealSize = m_pColInfo[nCol - 1].nCurSize;

    m_pColInfo[nShowCol].nIdealSize = cxNew;
    RecalcLayout();
}

BOOL CCbSplitterWnd::ReplaceView(int nRow, int nCol, CRuntimeClass *pViewClass)
{
    CView *pView = DYNAMIC_DOWNCAST( CView, GetPane( nRow, nCol ));
    ASSERT_KINDOF( CView, pView );

    if (pView->IsKindOf( pViewClass ))
        return FALSE;

    // Get pointer to CDocument object so that it can be used in the creation
    // process of the new view
    CDocument *pDoc = pView->GetDocument();

    int nID = pView->GetDlgCtrlID();
    int nWidth, nHeight, nMinWidth, nMinHeight;

    GetRowInfo( nRow, nWidth, nMinWidth );
    GetColumnInfo( nCol, nHeight, nMinHeight );

    // Delete existing view
    pView->DestroyWindow();

    // Create new view
    CCreateContext context;
    context.m_pCurrentDoc = pDoc;
    CreateView( nRow, nCol, pViewClass, CSize( nWidth, nHeight ), &context );
    pView = DYNAMIC_DOWNCAST( CView, GetPane( nRow, nCol ));
    ASSERT_KINDOF( CView, pView );

    pView->SetDlgCtrlID( nID );

    SetRowInfo( nRow, nWidth, nMinWidth );
    SetColumnInfo( nCol, nHeight, nMinHeight );
    RecalcLayout();

    return TRUE;
}

BOOL CCbSplitterWnd::SwitchView(int nRow, int nCol, CView *pNewView)
{
    CView *pOldView = DYNAMIC_DOWNCAST(CView, GetPane(nRow, nCol));
    ASSERT_KINDOF( CView, pOldView );

    if (pOldView == pNewView)
        return FALSE;

    int nOldID, nNewID;

    nOldID = pOldView->GetDlgCtrlID();
    nNewID = pNewView->GetDlgCtrlID();

    pOldView->ShowWindow(SW_HIDE);
    pNewView->ShowWindow(SW_SHOW);

    pOldView->SetDlgCtrlID(nNewID);
    pNewView->SetDlgCtrlID(nOldID);

    RecalcLayout();

    return TRUE;
}
