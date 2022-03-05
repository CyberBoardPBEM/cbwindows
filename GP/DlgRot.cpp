// DlgRot.cpp : implementation file
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
#include    "CDib.h"
#include    "DlgRot.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotateDialog dialog

CRotateDialog::CRotateDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CRotateDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CRotateDialog)
    m_nAngle = 0;
    //}}AFX_DATA_INIT
    m_pBMap = NULL;
    m_pDib = NULL;
    m_pRotBMap = NULL;
    m_crTrans = RGB(0, 0, 0);
}

CRotateDialog::~CRotateDialog()
{
    if (m_pRotBMap) delete m_pRotBMap;
    if (m_pDib) delete m_pDib;
}

void CRotateDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CRotateDialog)
    DDX_Control(pDX, IDC_D_ROT_PREVIEW, m_ctlView);
    DDX_Text(pDX, IDC_D_ROT_ANGLE, m_nAngle);
    DDV_MinMaxUInt(pDX, m_nAngle, 0, 355);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRotateDialog, CDialog)
    //{{AFX_MSG_MAP(CRotateDialog)
    ON_BN_CLICKED(IDC_D_ROT_APPLY, OnRotApply)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRotateDialog message handlers

void CRotateDialog::DeleteBMaps()
{
    for (int i = 0; i < 12; i++)
    {
        m_bmapTbl[i] = NULL;
    }
}

void CRotateDialog::OnRotApply()
{
//  if (!UpdateData(TRUE))  //TODO 20200618 WHAT IS THIS CODE DOING? CRotateDialog::OnRotApply()
//      return;
//  ASSERT(m_pBMap);

//  if (m_pRotBMap) delete m_pRotBMap;
//  if (m_pDib) delete m_pDib;
//  m_pRotBMap = NULL;
//  m_pDib = NULL;
//  if (m_nAngle == 0)
//  {
//          CRect rct;
//      m_ctlView.GetWindowRect(&rct);
//      ScreenToClient(&rct);
//      rct.InflateRect(-2, -2);
//      InvalidateRect(&rct);
//      return;
//  }

    DeleteBMaps();
    CDib dibSrc(*m_pBMap, GetAppPalette());

    static int angTbl[12] = { 0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330 };
    for (int i = 0; i < 12; i++)
    {
        CDib pRDib = Rotate16BitDib(dibSrc, angTbl[i], m_crTrans);
        m_bmapTbl[i] = pRDib.DIBToBitmap(GetAppPalette());
    }

    CRect rct;
    m_ctlView.GetWindowRect(&rct);
    ScreenToClient(&rct);
    rct.InflateRect(-2, -2);
    InvalidateRect(&rct);
}

void CRotateDialog::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect rct;
    m_ctlView.GetWindowRect(&rct);
    ScreenToClient(&rct);
    rct.InflateRect(-4, -4);

    SetupPalette(dc);
    for (int i = 0; i < 12; i++)
    {
        int x = (i % 4) * 45;
        int y = (i / 4) * 45;
        if (m_bmapTbl[i])
            BitmapBlt(dc, rct.TopLeft() + CSize(x, y), *m_bmapTbl[i]);
    }
    ResetPalette(dc);

    // Do not call CDialog::OnPaint() for painting messages
}

