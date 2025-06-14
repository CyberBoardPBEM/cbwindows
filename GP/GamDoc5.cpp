// GamDoc5.cpp : just plain miscellaneous stuff
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
#include    "FrmMain.h"
#include    "GamDoc.h"
#include    "GameBox.h"
#include    "GMisc.h"
#include    "StrLib.h"
#include    "MoveHist.h"
#include    "PPieces.h"
#include    "DrawObj.h"
#include    "PBoard.h"
#include    "VwPbrd.h"
#include    "Trays.h"
#include    "Player.h"

#include    "DlgSmsg.h"
#include    "DlgDice.h"
#include    "DlgEdtel.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

void CModelessDialogCleaner::operator()(CDialog* p) const
{
    if (p)
    {
        if (p->m_hWnd)
        {
            p->DestroyWindow();
        }
        delete p;
    }
}

////////////////////////////////////////////////////////////////////////////

DWORD  CGamDoc::GetCurrentPlayerMask() const
{
    return m_bSimulateSpectator ? 0 :  m_dwCurrentPlayer;
}

////////////////////////////////////////////////////////////////////////////
// Clears ownsership masks for everything.

void CGamDoc::ClearAllOwnership()
{
    GetPieceTable().ClearAllOwnership();
    GetTrayManager().ClearAllOwnership();
    GetPBoardManager().ClearAllOwnership();
}

////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::IsCurrentPlayerReferee() const
{
    return HasPlayers() && CPlayerManager::IsReferee(GetCurrentPlayerMask());
}

////////////////////////////////////////////////////////////////////////////
// Get "Owned by" string for piece

CB::string CGamDoc::GetPieceOwnerName(PieceID pid) const
{
    CB::string strName;
    if (GetPieceTable().IsPieceOwned(pid))
    {
        strName = GetPlayerManager()->GetPlayerUsingMask(
            GetPieceTable().GetOwnerMask(pid)).m_strName;
    }
    return strName;
}

////////////////////////////////////////////////////////////////////////////
// Support for strings associated with game elements (pieces, markers)
// A two level approach is used. First, the game's string map is checked
// for the element's string. If it isn't found, the gamebox's string
// map is checked. This way changes to the gamebox's strings will show
// up in all scenarios and games that haven't redefined them.

CB::string CGamDoc::GetGameElementString(GameElement gelem) const
{
    CB::string str;
    if (!m_mapStrings.Lookup(gelem, str))
        m_pGbx->GetGameBoxStringMap().Lookup(gelem, str);
    return str;
}

BOOL CGamDoc::HasGameElementString(GameElement gelem) const
{
    CB::string str;
    if (!m_mapStrings.Lookup(gelem, str))
        return m_pGbx->GetGameBoxStringMap().Lookup(gelem, str);
    return TRUE;
}

void CGamDoc::SetGameElementString(GameElement gelem, const CB::string* pszString)
{
    if (pszString != NULL && !pszString->empty())
    {
        // Add the new string value
        m_mapStrings.SetAt(gelem, *pszString);
    }
    else
    {
        // Remove the string
        m_mapStrings.RemoveKey(gelem);
    }
}

GameElement CGamDoc::GetGameElementCodeForObject(const CDrawObj& pDObj,
    size_t nSide /*= Invalid_v<size_t>*/) const
{
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        PieceID pid = pObj.m_pid;
        if (nSide == Invalid_v<size_t>)
        {
            nSide = GetPieceTable().GetSide(pid);
        }
        return MakePieceElement(pid, value_preserving_cast<unsigned>(nSide));
    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        const CMarkObj& pObj = static_cast<const CMarkObj&>(pDObj);
        return MakeObjectIDElement(pObj.GetObjectID());
    }
    ASSERT(!"invalid CDrawObj subtype");
    return Invalid_v<GameElement>;
}

GameElement CGamDoc::GetVerifiedGameElementCodeForObject(const CDrawObj& pDObj,
    size_t nSide /*= Invalid_v<size_t>*/) const
{
    GameElement elem = Invalid_v<GameElement>;
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        PieceID pid = pObj.m_pid;
        if (nSide == Invalid_v<size_t>)
        {
            nSide = GetPieceTable().GetSide(pid);
        }

        elem = MakePieceElement(pid, value_preserving_cast<unsigned>(nSide));
        if (!HasGameElementString(elem) || pObj.IsOwnedButNotByCurrentPlayer())
            elem = Invalid_v<GameElement>;
    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        const CMarkObj& pObj = static_cast<const CMarkObj&>(pDObj);
        elem = MakeObjectIDElement(pObj.GetObjectID());
        if (!HasGameElementString(elem))
        {
            // Try the actual marker ID for string existance.
            elem = MakeMarkerElement(pObj.m_mid);
            if (!HasGameElementString(elem))
                elem = Invalid_v<GameElement>;
        }
    }
    else
    {
        ASSERT(!"invalid CDrawObj subtype");
    }
    return elem;
}

void CGamDoc::GetTipTextForObject(const CDrawObj& pDObj, CB::string &strTip,
    CB::string* pStrTitle /* = NULL */) const
{
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        PieceID pid = pObj.m_pid;
        uint8_t nSide = GetPieceTable().GetSide(pid);
        strTip = GetGameElementString(MakePieceElement(pid, nSide));
    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        const CMarkObj& pObj = static_cast<const CMarkObj&>(pDObj);
        strTip = GetGameElementString(MakeObjectIDElement(pObj.GetObjectID()));
        if (strTip.empty())
            strTip = GetGameElementString(MakeMarkerElement(pObj.m_mid));
    }
}

////////////////////////////////////////////////////////////////////

void CGamDoc::DoEditPieceText(PieceID pid)
{
    CEditElementTextDialog dlg;

    uint8_t nSide = GetPieceTable().GetSide(pid);
    GameElement elem = MakePieceElement(pid, nSide);
    dlg.m_strText = GetGameElementString(elem);
    dlg.m_nSides = GetPieceTable().GetSides(pid);

    /* dlg.m_bSetAllSides = sides >= 2 &&
                            all side texts are currently same */
    dlg.m_bSetAllSides = dlg.m_nSides >= size_t(2);
    for (size_t i = size_t(0) ; i < dlg.m_nSides ; ++i)
    {
        GameElement elemDown = MakePieceElement(pid, value_preserving_cast<unsigned>(i));
        CB::string strDown = GetGameElementString(elemDown);
        if (strDown != dlg.m_strText)
        {
            dlg.m_bSetAllSides = FALSE;
            break;
        }
    }

    if (dlg.ShowModal() != wxID_OK)
        return;

    AssignNewMoveGroup();

    if (!dlg.m_bSetAllSides)
    {
        SetObjectText(elem, dlg.m_strText.empty() ? NULL : &dlg.m_strText);
    }
    else
    {
        for (size_t i = size_t(0) ; i < dlg.m_nSides ; ++i)
        {
            GameElement elemDown = MakePieceElement(pid, value_preserving_cast<unsigned>(i));
            SetObjectText(elemDown, dlg.m_strText.empty() ? NULL : &dlg.m_strText);
        }
    }
}

void CGamDoc::DoEditObjectText(const CDrawObj& pDObj)
{
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;
        DoEditPieceText(pid);
        return;
    }

    CEditElementTextDialog dlg;

    CB::string strTip;
    GetTipTextForObject(pDObj, strTip, NULL);

    dlg.m_strText = strTip;

    if (dlg.ShowModal() != wxID_OK)
        return;

    GameElement elem = GetGameElementCodeForObject(pDObj);
    AssignNewMoveGroup();

    SetObjectText(elem, dlg.m_strText.empty() ? NULL : &dlg.m_strText);
}

////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::IsPlayingLastHistory() const
{
    return IsPlayingHistory() &&
        m_nCurHist == m_pHistTbl->GetNumHistRecords() - 1;
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::SetDieRollState(OwnerOrNullPtr<CRollState> pRState)
{
    m_pRollState = std::move(pRState);
}

const CRollState* CGamDoc::GetDieRollState() const
{
    return m_pRollState.get();
}

////////////////////////////////////////////////////////////////////////////
// Show the message text as a notification tip at the given workspace
// coordinates. If either of the coordinates are -1, the center of
// the view receives the tip.

void CGamDoc::EventShowBoardNotification(BoardID nBrdSerNum, CPoint pntTipLoc, const CB::string& strMsg)
{
    if (IsQuietPlayback()) return;

    CPlayBoard* pPBoard = GetPBoardManager().GetPBoardBySerial(nBrdSerNum);
    ASSERT(pPBoard);
    CPlayBoardView* pView;

    if (pntTipLoc.x == -1 || pntTipLoc.y == -1)
    {
        EnsureBoardVisible(*pPBoard);
        pView = (CPlayBoardView*)FindPBoardView(*pPBoard);
        CRect rct;
        pView->GetClientRect(rct);
        pntTipLoc = rct.CenterPoint();
    }
    else
    {
        EnsureBoardLocationVisible(*pPBoard, pntTipLoc);
        pView = (CPlayBoardView*)FindPBoardView(*pPBoard);
        pView->WorkspaceToClient(pntTipLoc);
    }
    pView->SetNotificationTip(pntTipLoc, &strMsg);
}

////////////////////////////////////////////////////////////////////////////
// Show the message text as a notification tip at the given piece index
// If the piece id is null or not found in the tray, the center of the
// listbox receives the tip.

void CGamDoc::EventShowTrayNotification(size_t nTrayNum, PieceID pid, const CB::string& strMsg)
{
    if (IsQuietPlayback()) return;

    CTraySet& pYGrp = GetTrayManager().GetTraySet(nTrayNum);
    SelectTrayItem(pYGrp, pid, &strMsg);
}

////////////////////////////////////////////////////////////////////////////
// Called by record playback.

void CGamDoc::MsgSetMessageText(const CB::string& str)
{
    m_astrMsgHist.push_back(str);

    if (!m_strCurMsg.empty())
    {
        if (!m_strCurMsg.empty() && m_strCurMsg[m_strCurMsg.a_size() - size_t(1)] != '\n')
            m_strCurMsg += "\r\n";
        CB::string strDivider = CB::string::LoadString(IDS_MSG_DIVIDER);
        m_strCurMsg += strDivider + str;
    }
    else
        m_strCurMsg = str;

    GetMainFrame()->GetMessageWindow()->SetText(this);
    if (!IsQuietPlayback() && !m_bMsgWinVisible)
        m_bMsgWinVisible = TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Returns the current unaccepted message text.

const CB::string& CGamDoc::MsgGetMessageText() const
{
    return m_strCurMsg;
}

// Return current message history.
const std::vector<CB::string>& CGamDoc::MsgGetMessageHistory() const
{
    return m_astrMsgHist;
}

////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::MsgSendDialogOpen(BOOL bShowDieRoller /* = FALSE */)
{
    if (m_pMsgDialog != NULL)
    {
        return TRUE;
    }
    CSendMsgDlgPtr pDlg(new CSendMsgDialog(*this));
    pDlg->m_bShowDieRoller = bShowDieRoller;
    if (!pDlg->Create(IDD_SENDMESSAGE))
    {
        TRACE0("Failed to create Send Message Dialog\n");
        return FALSE;
    }
    // Note that dialog requests its data from the document.
    m_pMsgDialog = std::move(pDlg);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Called by dialog code to close send dialog in various ways.

void CGamDoc::MsgDialogSend(const CB::string& str, BOOL bCloseDialog /* = TRUE */)
{
    AssignNewMoveGroup();

    CB::string strReadOnly;
    CB::string strEditable;
    MsgSeperateIntoPieces(str, strReadOnly, strEditable);
    CB::string strMsg = strReadOnly + strEditable;

    RecordMessage(strMsg);

    // Add to message history
    m_astrMsgHist.push_back(std::move(strMsg));

    // Clear out current message buffer
    m_strCurMsg.clear();

    if (bCloseDialog)
    {
        m_pMsgDialog = NULL;
    }
    SetModifiedFlag();
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::MsgDialogCancel(BOOL bDiscardHistory /* = FALSE */)
{
    m_pMsgDialog = NULL;
    m_strCurMsg.clear();
    if (bDiscardHistory)
        m_astrMsgHist.clear();
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::MsgDialogForceDefer()
{
    if (m_pMsgDialog != NULL)
        m_pMsgDialog->SendMessage(WM_COMMAND, MAKEWPARAM(uint16_t(IDC_D_SMSG_CLOSE), uint16_t(BN_CLICKED)));
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::MsgDialogClose(const CB::string& str)
{
    m_strCurMsg = str;              // Just save the current encoded message string
    m_pMsgDialog = NULL;
}

///////////////////////////////////////////////////////////////////////////
// All legacy historical messages that have been recorded are stored
// with the current message text. Each historical message ends with the
// character sequence ">>\r\n". All text between the last ">>\r\n"
// and a paragraph marker character (reverse P) is read-only text for the
// message currently being composed. All text after the paragraph
// marker is the editable portion of the message being composed.

// This routine breaks an encoded send message string into its parts.
/* static */
void CGamDoc::MsgSeperateLegacyMsgIntoPieces(CB::string& strMsg, CB::string& strHistory,
    CB::string& strReadOnly, CB::string& strEditable)
{
    strHistory.clear();
    strReadOnly.clear();
    strEditable.clear();

    // Search the message string backwards for a ">>\r\n" or start of buffer.
    size_t i = strMsg.rfind("\xBB\r");
    if (i != CB::string::npos)
    {
        ASSERT(strMsg[i + size_t(2)] == '\n');
        i += size_t(3);                 // Position to start of next line
    }
    else
    {
        i = size_t(0);
    }
    strHistory = strMsg.substr(size_t(0), i);    // Extract historical messages
    strMsg = strMsg.substr(i);         // Remove historical msgs from main string
    if (strMsg.empty())
        return;
    // Search the message string backwards for a "reverse P" or start of buffer.
    i = strMsg.rfind('\xB6');   // 0xB6 is paragraph marker
    if (i == CB::string::npos)
        strReadOnly = strMsg;
    else
    {
        strReadOnly = strMsg.substr(size_t(0), i);   // Extract read-only part of current msg
        strEditable = strMsg.substr(i + size_t(1));  // Extract editable part of current msg
    }
}

////////////////////////////////////////////////////////////////////////////
// All text between the start of the string and a paragraph marker
// character (reverse P) is read-only text for the message currently
// being composed. All text after the paragraph marker is the editable
// portion of the message being composed.
//
// This routine breaks the encoded send message buffer into its parts.

/* static */
void CGamDoc::MsgSeperateIntoPieces(const CB::string& strMsg, CB::string& strReadOnly,
    CB::string& strEditable)
{
    strReadOnly.clear();
    strEditable.clear();

    if (strMsg.empty())
        return;

    // Search the message string backwards for a "reverse P" or start of buffer.
    size_t i = strMsg.rfind('\xB6'); // 0xB6 is paragraph marker
    if (i == CB::string::npos)
        strReadOnly = strMsg;
    else
    {
        strReadOnly = strMsg.substr(size_t(0), i);   // Extract read-only part of current msg
        strEditable = strMsg.substr(i + size_t(1));  // Extract editable part of current msg
    }
}

////////////////////////////////////////////////////////////////////////////
/* static */
CB::string CGamDoc::MsgEncodeFromPieces(const CB::string& strReadOnly,  const CB::string& strEditable)
{
    CB::string str;
    str = strReadOnly;
    if (!strEditable.empty())
        str += '\xB6' + strEditable;
    return str;
}

////////////////////////////////////////////////////////////////////////////
/* static */
void CGamDoc::MsgParseLegacyHistory(CB::string& strLegacyMsg,
    std::vector<CB::string>& astrMsgHist, CB::string& strCurMsg)
{
    astrMsgHist.clear();

    CB::string strHistory;
    CB::string strReadOnly;
    CB::string strEditable;

    MsgSeperateLegacyMsgIntoPieces(strLegacyMsg, strHistory, strReadOnly, strEditable);

    // New encoding only holds read-only and editable strings
    strCurMsg = MsgEncodeFromPieces(strReadOnly, strEditable);

    size_t nPos;
    // The loop searches for a chevron followed by <cr><lf>
    while ((nPos = strHistory.find("\xBB\r\n")) != CB::string::npos)
    {
        CB::string strBfr;
        strBfr = strHistory.substr(size_t(0), nPos + size_t(3));         // Buffer the message
        strHistory = strHistory.substr(nPos + size_t(3));      // Remove the message
        // Finally we need to remove the divider string
        VERIFY((nPos = strBfr.find("\r\n\xAB")) != CB::string::npos);
        strBfr = strBfr.substr(size_t(0), nPos);
        astrMsgHist.push_back(std::move(strBfr));
    }
    if (!strHistory.empty())
        astrMsgHist.push_back(std::move(strHistory));
}


