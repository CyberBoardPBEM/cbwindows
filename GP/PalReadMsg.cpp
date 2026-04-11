// PalReadMsg.cpp - Dockable message output window
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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

const wxColour     MSG_DIVIDER_COLOR = *wxRED;
const wxFontWeight MSG_DIVIDER_EFFECT = wxFONTWEIGHT_BOLD;

const wxColour     MSG_DICE_ROLL_COLOR = wxColour(128, 0, 128);
const wxFontWeight MSG_DICE_ROLL_EFFECT = wxFONTWEIGHT_NORMAL;

const wxColour     MSG_TEXT_COLOR = *wxBLACK;
const wxFontWeight MSG_TEXT_EFFECT = wxFONTWEIGHT_NORMAL;

/////////////////////////////////////////////////////////////////////////////

namespace {
const CB::string CHAR_CHEVRON(size_t(1), '\xBB');               // '>>'
const CB::string CHAR_CHEVRON_REV(size_t(1), '\xAB');           // '<<'
const CB::string STR_MESSAGE_DIVIDER = "\xAB--------------------\xBB\r\n";
}

/////////////////////////////////////////////////////////////////////////////

const UINT IDC_MSG_RICHEDIT = 1;                // Control ID of rich edit control

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CReadMsgWnd, CReadMsgWnd::BASE)
#if 0
    ON_WM_CREATE()
#endif
    EVT_SIZE(OnSize)
    EVT_CONTEXT_MENU(OnContextMenu)
    EVT_MENU(wxID_COPY, OnEditCopy)
    EVT_UPDATE_UI(wxID_COPY, OnUpdateEditCopy)
//    ON_WM_CHAR()
//    ON_WM_KEYUP()
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CReadMsgWndContainer, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_MESSAGE(WM_PALETTE_HIDE, OnPaletteHide)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadMsgWnd

CReadMsgWnd::CReadMsgWnd(CReadMsgWndContainer& container) :
    m_pContainer(&container),
    m_editCtrl(new wxTextCtrl)
{
    m_pDoc = NULL;
    m_nMsgCount = size_t(0);
}

CReadMsgWnd::~CReadMsgWnd()
{
}

BOOL CReadMsgWnd::Create()
{
    if (!wxPanel::Create(*m_pContainer))
    {
        TRACE("Failed to create Tray palette window.\n");
        return FALSE;
    }

    m_editCtrl->Create(this, XRCID("m_editCtrl"),
                                wxEmptyString,
                                wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);

    SetDefaults();

    return TRUE;
}

#if 0
/////////////////////////////////////////////////////////////////////////////

int CReadMsgWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (BASE::OnCreate(lpCreateStruct) == -1)
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::OnSize(wxSizeEvent& event)
{
    wxRect rct = GetClientRect();
    m_editCtrl->SetSize(rct);
    event.Skip();
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::OnContextMenu(wxContextMenuEvent& /*event*/)
{
    std::unique_ptr<wxMenuBar> menu(wxXmlResource::Get()->LoadMenuBar("IDR_MENU_PLAYER_POPUPS"));
    wxASSERT(menu);

    int index = menu->FindMenu("6=MV_RICHEDIT");
    wxASSERT(index != wxNOT_FOUND);
    // wx doesn't allow popup menu to be part of menubar
    std::unique_ptr<wxMenu> pPopupMenu(menu->Remove(value_preserving_cast<size_t>(index)));
    wxASSERT(pPopupMenu);
    PopupMenu(&*pPopupMenu);
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgWnd::SetText(CGamDoc* pDoc)
{
    if (pDoc != m_pDoc)
    {
        m_editCtrl->Clear();
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
        m_editCtrl->Clear();
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

void CReadMsgWnd::InsertText(const CB::string& pszText)
{
    m_editCtrl->AppendText(pszText);
}

void CReadMsgWnd::SetDefaults()
{
    wxTextAttr attr = m_editCtrl->GetDefaultStyle();
    attr.SetFontWeight(wxFONTWEIGHT_NORMAL);
    attr.SetTextColour(*wxBLACK);
    m_editCtrl->SetDefaultStyle(attr);
}

void CReadMsgWnd::SetTextStyle(wxColour cr, wxFontWeight dwEffect)
{
    wxTextAttr attr = m_editCtrl->GetDefaultStyle();
    attr.SetTextColour(cr);
    attr.SetFontWeight(dwEffect);
    m_editCtrl->SetDefaultStyle(attr);
}

/////////////////////////////////////////////////////////////////////////////
// CReadMsgWnd message handlers

// Called when window is undocked. Message sent by CCBMiniFrameWnd class.
LRESULT CReadMsgWndContainer::OnPaletteHide(WPARAM, LPARAM)
{
#if 0
    GetMainFrame()->SendMessage(WM_COMMAND, ID_PBCK_READMESSAGE);
#else
    AfxThrowNotSupportedException();
#endif
    return (LRESULT)0;
}

void CReadMsgWnd::OnEditCopy(wxCommandEvent& /*event*/)
{
    m_editCtrl->Copy();
}

void CReadMsgWnd::OnUpdateEditCopy(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_editCtrl->CanCopy());
}

CReadMsgWndContainer::CReadMsgWndContainer() :
    CB::NativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CReadMsgWndContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (BASE::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    if (!child->Create())
    {
        TRACE("Failed to create ReadMessage palette window.\n");
        return -1;
    }

    return 0;
}

void CReadMsgWndContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return BASE::OnSize(nType, cx, cy);
}
