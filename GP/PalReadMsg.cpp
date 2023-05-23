// PalReadMsg.cpp - Dockable message output window
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
#include "Gp.h"
#include "GamDoc.h"
#include "FrmMain.h"
#include "PalReadMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

const COLORREF MSG_DIVIDER_COLOR = RGB(255, 0, 0);
const DWORD    MSG_DIVIDER_EFFECT = CFE_BOLD;

const COLORREF MSG_DICE_ROLL_COLOR = RGB(128, 0, 128);
const DWORD    MSG_DICE_ROLL_EFFECT = 0;

const COLORREF MSG_TEXT_COLOR = RGB(0, 0, 0);
const DWORD    MSG_TEXT_EFFECT = 0;

const COLORREF MSG_SENDER_NAME_COLOR = RGB(0, 128, 128);
const DWORD    MSG_SENDER_EFFECT = CFE_BOLD;

/////////////////////////////////////////////////////////////////////////////

namespace {
const CB::string CHAR_CHEVRON(size_t(1), '\xBB');               // '>>'
const CB::string CHAR_CHEVRON_REV(size_t(1), '\xAB');           // '<<'
const CB::string STR_MESSAGE_DIVIDER = "\xAB--------------------\xBB\r\n";
}

/////////////////////////////////////////////////////////////////////////////

const UINT IDC_MSG_RICHEDIT = 1;                // Control ID of rich edit control

/////////////////////////////////////////////////////////////////////////////

struct CharFormat : public CHARFORMAT
{
    CharFormat()
    {
        memset(this, 0, sizeof(CHARFORMAT));
        this->cbSize = sizeof(CHARFORMAT);
    }
    CharFormat(const CHARFORMAT &newf)
    {
        *(CHARFORMAT *)this = newf;
    }
    inline operator CHARFORMAT &()
    {
        return *(CHARFORMAT *)this;
    }
};

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CReadMsgWnd, CDockablePane)

BEGIN_MESSAGE_MAP(CReadMsgWnd, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_MESSAGE(WM_PALETTE_HIDE, OnPaletteHide)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
//    ON_WM_CHAR()
//    ON_WM_KEYUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadMsgWnd

CReadMsgWnd::CReadMsgWnd()
{
    m_pDoc = NULL;
    m_nMsgCount = size_t(0);
}

CReadMsgWnd::~CReadMsgWnd()
{
}

/////////////////////////////////////////////////////////////////////////////

int CReadMsgWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create the edit control
    if (!m_editCtrl.Create(WS_CHILD | WS_VISIBLE | ES_READONLY |
            ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        CRect(0,0,0,0), this, IDC_MSG_RICHEDIT))
    {
        TRACE0("Failed to create rich edit control.\n");
        return -1;
    }
    m_editCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    m_editCtrl.SetEventMask(ENM_MOUSEEVENTS | ENM_KEYEVENTS);

    SetDefaults();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);
    CRect rct;
    GetClientRect(rct);
    if (m_editCtrl.m_hWnd != NULL)
        m_editCtrl.MoveWindow(&rct);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CReadMsgWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    MSGFILTER * pMsgFilter = (MSGFILTER *)lParam;
    if ((wParam == IDC_MSG_RICHEDIT) && (pMsgFilter->nmhdr.code == EN_MSGFILTER) &&
        (pMsgFilter->msg == WM_RBUTTONDOWN))
    {
        CPoint point;
        ::GetCursorPos(&point); // Get current mouse position

        CMenu menu;
        VERIFY(menu.LoadMenuW(IDR_MENU_PLAYER_POPUPS));

        CMenu *pPopupMenu = menu.GetSubMenu(MENU_MV_RICHEDIT);
        ASSERT(pPopupMenu != NULL);
        pPopupMenu->TrackPopupMenu((TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON),
            point.x, point.y, this);

        pPopupMenu->DestroyMenu();
    }
    return CDockablePane::OnNotify(wParam, lParam, pResult);
}

void CReadMsgWnd::OnPressCloseButton()
{
    CDockablePane::OnPressCloseButton();
    SendMessage(WM_PALETTE_HIDE);
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::SetText(CGamDoc* pDoc)
{
    if (pDoc != m_pDoc)
    {
        m_editCtrl.SetWindowText(""_cbstring);
        m_pDoc = pDoc;
        m_nMsgCount = size_t(0);
    }
    if (m_pDoc != NULL)
        ProcessMessages();
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::ProcessMessages()
{
    const std::vector<CB::string>& astrHist = m_pDoc->MsgGetMessageHistory();
    if (astrHist.empty() || astrHist.size() < m_nMsgCount)
    {
        m_nMsgCount = size_t(0);
        m_editCtrl.SetWindowText(""_cbstring);
    }

    size_t nOldCount = m_nMsgCount;            // Save previous length for a moment
    m_nMsgCount = astrHist.size();       // Set new high water mark
    for (size_t i = nOldCount ; i < m_nMsgCount ; ++i)
    {
        CB::string strBfr = astrHist[i];
        if (strBfr.empty() || strBfr[strBfr.a_size()-size_t(1)] != '\n')
        {
            strBfr += "\r\n";
        }
        if (i > size_t(0))
        {
            SetTextStyle(MSG_DIVIDER_COLOR, MSG_DIVIDER_EFFECT);
            InsertText(STR_MESSAGE_DIVIDER);
        }

        CB::string strLine;

        while (GetLine(strBfr, strLine))
        {
            if (strLine.empty() || strLine[size_t(0)] == '\r')
                InsertText("\r\n");
            else if (strLine.front() == CHAR_CHEVRON.front())      // Dice Roll
            {
                SetTextStyle(MSG_DICE_ROLL_COLOR, MSG_DICE_ROLL_EFFECT);
                InsertText(strLine.substr(size_t(2)));           // Don't show chevron
            }
            else                                            // Normal Message
            {
                SetTextStyle(MSG_TEXT_COLOR, MSG_TEXT_EFFECT);
                InsertText(strLine);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool CReadMsgWnd::GetLine(CB::string& strBfr, CB::string& strLine)
{
    if (strBfr.empty())
    {
        strLine.clear();
        return false;
    }
    size_t nCrLf = strBfr.find('\n');
    if (nCrLf == CB::string::npos)
    {
        // Last string in buffer
        strLine = strBfr;
        strBfr.clear();
        return true;
    }
    strLine = strBfr.substr(size_t(0), nCrLf + size_t(1));
    strBfr = strBfr.substr(nCrLf + size_t(1));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::InsertText(const CB::string& pszText, BOOL bAtEnd /* = TRUE */)
{
    if (bAtEnd)
    {
        int nLen = m_editCtrl.GetTextLength();
        m_editCtrl.SetSel(nLen, nLen);
    }
    m_editCtrl.ReplaceSel(pszText);
    if (bAtEnd)
        m_editCtrl.SendMessage(WM_VSCROLL, SB_BOTTOM);
}

void CReadMsgWnd::SetDefaults()
{
    // Set the font for the edit control.
    CharFormat cf;

    m_editCtrl.GetDefaultCharFormat(cf);
    cf.dwMask = CFM_COLOR | CFM_EFFECTS;
    cf.dwEffects = 0;
    cf.crTextColor = RGB(0, 0, 0);

    m_editCtrl.SetDefaultCharFormat(cf);
}

void CReadMsgWnd::SetTextStyle(COLORREF cr, DWORD dwEffect /* = 0 */)
{
    CharFormat cf;
    GetCurCharFormat(cf);

    cf.dwMask = CFM_COLOR | CFM_EFFECTS;
    cf.crTextColor = cr;
    cf.dwEffects = dwEffect;

    SetCharFormat(cf);
}

void CReadMsgWnd::SetTextColor(COLORREF cr)
{
    CharFormat cf;
    GetCurCharFormat(cf);

    cf.dwMask = CFM_COLOR;
    cf.crTextColor = cr;

    SetCharFormat(cf);
}

void CReadMsgWnd::SetTextEffect(DWORD dwEffect)
{
    CharFormat cf;
    GetCurCharFormat(cf);

    cf.dwMask = CFM_EFFECTS;
    cf.dwEffects = dwEffect;

    SetCharFormat(cf);
}

void CReadMsgWnd::SetInsertAtEnd()
{
    m_editCtrl.SetSel(-1, -1);
}

void CReadMsgWnd::GetCurCharFormat(CHARFORMAT& cf)
{
    m_editCtrl.GetSelectionCharFormat(cf);
}

void CReadMsgWnd::SetCharFormat(CHARFORMAT& cf)
{
    m_editCtrl.SetSelectionCharFormat(cf);
}

/////////////////////////////////////////////////////////////////////////////
// CReadMsgWnd message handlers

// Called when window is undocked. Message sent by CCBMiniFrameWnd class.
LRESULT CReadMsgWnd::OnPaletteHide(WPARAM, LPARAM)
{
    GetMainFrame()->SendMessage(WM_COMMAND, ID_PBCK_READMESSAGE);
    return (LRESULT)0;
}

void CReadMsgWnd::OnEditCopy()
{
    m_editCtrl.Copy();
}

void CReadMsgWnd::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
    CHARRANGE cr;
    m_editCtrl.GetSel(cr);
    pCmdUI->Enable(cr.cpMin != cr.cpMax);
}
