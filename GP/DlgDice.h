// DlgDice.h : header file
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

#ifndef _DLGDICE_H_
#define _DLGDICE_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////

#include <array>

#ifndef _GMISC_H
#include "GMisc.h"
#endif

/////////////////////////////////////////////////////////////////////////////

struct RollRecord
{
    bool     m_bRoll;
    uint32_t m_nDice;
    uint32_t m_nFaces;
};

constexpr uint32_t  DICETYPES = uint32_t(3);
const uint16_t SCHEMA_ROLLSTATE = uint16_t(1);

class CRollState
{
public:
    bool        m_bFirstRoll;                   // No roll performed yet
    uint32_t    m_nSetsToRoll;                  // Number of times to roll
    CB::string  m_strUserSeed;                  // Current user seed (if any)
    uint32_t    m_nSeedCarryOver;               // The current seed
    // The actual roll specification
    int32_t     m_nBias;
    std::array<RollRecord, DICETYPES> m_tbl;
};
CArchive& operator<<(CArchive& ar, const CRollState& rs);
CArchive& operator>>(CArchive& ar, CRollState& rs);

/////////////////////////////////////////////////////////////////////////////
// CDieRollerDlg dialog

class CDieRollerDlg : public CDialog
{
// Construction
public:
    CDieRollerDlg(CWnd* pParent = NULL);   // standard constructor

// Methods
public:
    const std::string& GetFormattedRollResult() const { return m_strRoll; }

    OwnerPtr<CRollState> GetRollState() const;
    void SetRollState(const CRollState& rstate);

// Dialog Data
    //{{AFX_DATA(CDieRollerDlg)
    enum { IDD = IDD_DIEROLLER };
    CButton m_btnResetSeed;
    CEdit   m_editSeed;
    CButton m_btnOK;
    CButton m_chkRoll3;
    CButton m_chkRoll2;
    CButton m_chkRoll1;
    CB::string m_strSeed;
    uint32_t m_nFaces1;
    uint32_t m_nFaces2;
    uint32_t m_nFaces3;
    uint32_t m_nDice1;
    uint32_t m_nDice2;
    uint32_t m_nDice3;
    bool    m_bRoll1;
    bool    m_bRoll2;
    bool    m_bRoll3;
    int32_t m_nBias;
    uint32_t m_nSets;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDieRollerDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    void MakeFormattedRollResult();

    bool IsAnyDieSelected() const;

    void UpdateControls();

    std::string m_strRoll;

    bool        m_bFirstRoll;
    CB::string  m_strInitialSeed;               // Used to detect changed user seed
    uint32_t    m_nSeedCarryOver;


    // Generated message map functions
    //{{AFX_MSG(CDieRollerDlg)
    virtual void OnOK();
    afx_msg void OnDieRoll1();
    afx_msg void OnDieRoll2();
    afx_msg void OnDieRoll3();
    afx_msg void OnChangeDieNumdies1();
    afx_msg void OnChangeDieNumdies2();
    afx_msg void OnChangeDieNumdies3();
    virtual BOOL OnInitDialog();
    afx_msg void OnDieResetSeed();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif
