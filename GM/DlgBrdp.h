// dlgbrdp.h : header file
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

/////////////////////////////////////////////////////////////////////////////
// CBoardPropDialog dialog

enum CellFormType;

class CBoardPropDialog : public CDialog
{
// Construction
public:
    CBoardPropDialog(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    //{{AFX_DATA(CBoardPropDialog)
    enum { IDD = IDD_BRDPRP };
    CStatic m_staticHalfPixelWidth;
    CStatic m_staticSmallPixelWidth;
    CStatic m_staticSmallPixelHeight;
    CStatic m_staticHalfPixelHeight;
    CStatic m_staticPixelWidth;
    CStatic m_staticPixelHeight;
    CEdit   m_editBrdName;
    CMFCColorButton m_cpCellFrame;
    CStatic m_staticWidth;
    CStatic m_staticHeight;
    CStatic m_staticRows;
    CStatic m_staticCols;
    CComboBox m_comboStyle;
    BOOL    m_bCellLines;
    BOOL    m_bGridSnap;
    BOOL    m_bTrackCellNum;
    int     m_nRowTrkOffset;
    int     m_nColTrkOffset;
    float   m_fXGridSnapOff;
    float   m_fYGridSnapOff;
    float   m_fXGridSnap;
    float   m_fYGridSnap;
    CB::string m_strName;
    int     m_nStyleNum;
    BOOL    m_bColTrkInvert;
    BOOL    m_bRowTrkInvert;
    BOOL    m_bCellBorderOnTop;
    BOOL    m_bEnableXParentCells;
    //}}AFX_DATA

    COLORREF        m_crCellFrame;

    BOOL            m_bShapeChanged;// Set to true if reshape desired

    UINT            m_xGridSnap;
    UINT            m_yGridSnap;
    UINT            m_xGridSnapOff;
    UINT            m_yGridSnapOff;

    CellFormType    m_eCellStyle;   // Used to disable cell a cell dimension
    size_t          m_nRows;
    size_t          m_nCols;
    int             m_nCellHt;
    int             m_nCellWd;
    CellStagger     m_bStagger;

// Implementation
protected:
    void UpdateInfoArea();

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CBoardPropDialog)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnReshape();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

