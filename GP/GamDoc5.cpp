// GamDoc5.cpp : just plain miscellaneous stuff
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

////////////////////////////////////////////////////////////////////////////

DWORD  CGamDoc::GetCurrentPlayerMask() const
{
    return m_bSimulateSpectator ? 0 :  m_dwCurrentPlayer;
}

////////////////////////////////////////////////////////////////////////////
// Clears ownsership masks for everything.

void CGamDoc::ClearAllOwnership()
{
    GetPieceTable()->ClearAllOwnership();
    GetTrayManager()->ClearAllOwnership();
    GetPBoardManager()->ClearAllOwnership();
}

////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::IsCurrentPlayerReferee()
{
    return HasPlayers() && CPlayerManager::IsReferee(GetCurrentPlayerMask());
}

////////////////////////////////////////////////////////////////////////////
// Get "Owned by" string for piece

void CGamDoc::GetPieceOwnerName(PieceID pid, CString& strName)
{
    strName.Empty();
    if (GetPieceTable()->IsPieceOwned(pid))
    {
        strName = GetPlayerManager()->GetPlayerUsingMask(
            GetPieceTable()->GetOwnerMask(pid)).m_strName;
    }
}

////////////////////////////////////////////////////////////////////////////
// Support for strings associated with game elements (pieces, markers)
// A two level approach is used. First, the game's string map is checked
// for the element's string. If it isn't found, the gamebox's string
// map is checked. This way changes to the gamebox's strings will show
// up in all scenarios and games that haven't redefined them.

CString CGamDoc::GetGameElementString(GameElement gelem)
{
    CString str;
    if (!m_mapStrings.Lookup(gelem, str))
        m_pGbx->GetGameBoxStringMap().Lookup(gelem, str);
    return str;
}

BOOL CGamDoc::HasGameElementString(GameElement gelem)
{
    CString str;
    if (!m_mapStrings.Lookup(gelem, str))
        return m_pGbx->GetGameBoxStringMap().Lookup(gelem, str);
    return TRUE;
}

void CGamDoc::SetGameElementString(GameElement gelem, LPCTSTR pszString)
{
    if (pszString != NULL && *pszString != 0)
    {
        // Add the new string value
        CString str = pszString;
        m_mapStrings.SetAt(gelem, str);
    }
    else
    {
        // Remove the string
        m_mapStrings.RemoveKey(gelem);
    }
}

GameElement CGamDoc::GetGameElementCodeForObject(const CDrawObj& pDObj,
    BOOL bBottomSide /* = FALSE */)
{
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        PieceID pid = pObj.m_pid;
        int nSide = GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
        if (bBottomSide) nSide ^= 1;        // Toggle the side
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
    BOOL bBottomSide /* = FALSE */)
{
    GameElement elem = Invalid_v<GameElement>;
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        PieceID pid = pObj.m_pid;
        int nSide = GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
        if (bBottomSide) nSide ^= 1;        // Toggle the side

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
    return elem;
}

void CGamDoc::GetTipTextForObject(const CDrawObj& pDObj, CString &strTip,
    CString* pStrTitle /* = NULL */)
{
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        PieceID pid = pObj.m_pid;
        int nSide = GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
        strTip = GetGameElementString(MakePieceElement(pid, value_preserving_cast<unsigned>(nSide)));
    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        const CMarkObj& pObj = static_cast<const CMarkObj&>(pDObj);
        strTip = GetGameElementString(MakeObjectIDElement(pObj.GetObjectID()));
        if (strTip.IsEmpty())
            strTip = GetGameElementString(MakeMarkerElement(pObj.m_mid));
    }
}

////////////////////////////////////////////////////////////////////

void CGamDoc::DoEditPieceText(PieceID pid)
{
    CEditElementTextDialog dlg;

    int nSide = GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
    GameElement elem = MakePieceElement(pid, value_preserving_cast<unsigned>(nSide));
    GameElement elemDown = MakePieceElement(pid, value_preserving_cast<unsigned>(nSide ^ 1));

    dlg.m_strText = GetGameElementString(elem);

    if (GetPieceTable()->Is2Sided(pid))
    {
        CString strDown = GetGameElementString(elemDown);
        if (!strDown.IsEmpty() && strDown == dlg.m_strText)
            dlg.m_bSetBothSides = TRUE;
        dlg.m_bDoubleSided = TRUE;
    }

    if (dlg.DoModal() != IDOK)
        return;

    AssignNewMoveGroup();

    SetObjectText(elem, dlg.m_strText.IsEmpty() ? NULL : (LPCTSTR)dlg.m_strText);

    if (dlg.m_bSetBothSides && GetPieceTable()->Is2Sided(pid))
        SetObjectText(elemDown, dlg.m_strText.IsEmpty() ? NULL : (LPCTSTR)dlg.m_strText);
}

void CGamDoc::DoEditObjectText(const CDrawObj& pDObj)
{
    CEditElementTextDialog dlg;

    CString strTip;
    GetTipTextForObject(pDObj, strTip, NULL);

    dlg.m_strText = strTip;

    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;
        if (GetPieceTable()->Is2Sided(pid))
        {
            dlg.m_bDoubleSided = TRUE;
            int nSide = GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
            CString strDown = GetGameElementString(MakePieceElement(pid, value_preserving_cast<unsigned>(nSide ^ 1)));
            if (!strDown.IsEmpty() && strDown == dlg.m_strText)
                dlg.m_bSetBothSides = TRUE;
        }
    }

    if (dlg.DoModal() != IDOK)
        return;

    GameElement elem = GetGameElementCodeForObject(pDObj);
    AssignNewMoveGroup();

    SetObjectText(elem, dlg.m_strText.IsEmpty() ? NULL : (LPCTSTR)dlg.m_strText);
    if (dlg.m_bSetBothSides && pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;
        if (GetPieceTable()->Is2Sided(pid))
        {
            int nSide = GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
            SetObjectText(MakePieceElement(pid, value_preserving_cast<unsigned>(nSide ^ 1)),
                dlg.m_strText.IsEmpty() ? NULL : (LPCTSTR)dlg.m_strText);
        }
    }
}

////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::IsPlayingLastHistory()
{
    return IsPlayingHistory() &&
        m_nCurHist == m_pHistTbl->GetNumHistRecords() - 1;
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::SetDieRollState(CRollState* pRState)
{
    if (m_pRollState != NULL)
        delete m_pRollState;
    m_pRollState = pRState;
}

CRollState* CGamDoc::GetDieRollState()
{
    return m_pRollState;
}

////////////////////////////////////////////////////////////////////////////
// Show the message text as a notification tip at the given workspace
// coordinates. If either of the coordinates are -1, the center of
// the view receives the tip.

void CGamDoc::EventShowBoardNotification(BoardID nBrdSerNum, CPoint pntTipLoc, CString strMsg)
{
    if (IsQuietPlayback()) return;

    CPlayBoard* pPBoard = GetPBoardManager()->GetPBoardBySerial(nBrdSerNum);
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
    pView->SetNotificationTip(pntTipLoc, strMsg);
}

////////////////////////////////////////////////////////////////////////////
// Show the message text as a notification tip at the given piece index
// If the piece id is null or not found in the tray, the center of the
// listbox receives the tip.

void CGamDoc::EventShowTrayNotification(size_t nTrayNum, PieceID pid, CString strMsg)
{
    if (IsQuietPlayback()) return;

    CTraySet& pYGrp = GetTrayManager()->GetTraySet(nTrayNum);
    SelectTrayItem(pYGrp, pid, strMsg);
}

////////////////////////////////////////////////////////////////////////////
// Called by record playback.

void CGamDoc::MsgSetMessageText(CString str)
{
    m_astrMsgHist.Add(str);

    if (!m_strCurMsg.IsEmpty())
    {
        if (!m_strCurMsg.IsEmpty() && m_strCurMsg.GetAt(m_strCurMsg.GetLength() - 1) != '\n')
            m_strCurMsg += "\r\n";
        CString strDivider;
        strDivider.LoadString(IDS_MSG_DIVIDER);
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

CString CGamDoc::MsgGetMessageText()
{
    return m_strCurMsg;
}

// Return current message history.
CStringArray& CGamDoc::MsgGetMessageHistory()
{
    return m_astrMsgHist;
}

////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::MsgSendDialogOpen(BOOL bShowDieRoller /* = FALSE */)
{
    if (m_pMsgDialog != NULL)
    {
        if (m_pMsgDialog->IsKindOf(RUNTIME_CLASS(CSendMsgDialog)))
            return TRUE;
        m_pMsgDialog->DestroyWindow();
        delete m_pMsgDialog;
        m_pMsgDialog = NULL;
    }
    CSendMsgDialog* pDlg = new CSendMsgDialog;
    pDlg->m_pDoc = this;
    pDlg->m_bShowDieRoller = bShowDieRoller;
    if (!pDlg->Create(IDD_SENDMESSAGE))
    {
        TRACE0("Failed to create Send Message Dialog\n");
        delete pDlg;
        return FALSE;
    }
    // Note that dialog requests its data from the document.
    m_pMsgDialog = pDlg;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Called by dialog code to close send dialog in various ways.

void CGamDoc::MsgDialogSend(CString str, BOOL bCloseDialog /* = TRUE */)
{
    AssignNewMoveGroup();

    CString strReadOnly;
    CString strEditable;
    MsgSeperateIntoPieces(str, strReadOnly, strEditable);
    CString strMsg = strReadOnly + strEditable;

    RecordMessage(strMsg);

    // Add to message history
    m_astrMsgHist.Add(strMsg);

    // Clear out current message buffer
    m_strCurMsg.Empty();

    if (bCloseDialog)
    {
        m_pMsgDialog->DestroyWindow();
        delete m_pMsgDialog;
        m_pMsgDialog = NULL;
    }
    SetModifiedFlag();
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::MsgDialogCancel(BOOL bDiscardHistory /* = FALSE */)
{
    if (m_pMsgDialog != NULL)
    {
        m_pMsgDialog->DestroyWindow();
        delete m_pMsgDialog;
        m_pMsgDialog = NULL;
    }
    m_strCurMsg.Empty();
    if (bDiscardHistory)
        m_astrMsgHist.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::MsgDialogForceDefer()
{
    if (m_pMsgDialog != NULL)
        m_pMsgDialog->SendMessage(WM_COMMAND, MAKEWPARAM(uint16_t(IDC_D_SMSG_CLOSE), uint16_t(BN_CLICKED)));
}

////////////////////////////////////////////////////////////////////////////

void CGamDoc::MsgDialogClose(CString str)
{
    m_strCurMsg = str;              // Just save the current encoded message string
    if (m_pMsgDialog != NULL)
    {
        m_pMsgDialog->DestroyWindow();
        delete m_pMsgDialog;
        m_pMsgDialog = NULL;
    }
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
void CGamDoc::MsgSeperateLegacyMsgIntoPieces(CString strMsg, CString& strHistory,
    CString& strReadOnly, CString& strEditable)
{
    strHistory.Empty();
    strReadOnly.Empty();
    strEditable.Empty();

    // Search the message string backwards for a ">>\r\n" or start of buffer.
    int i;
    for (i = strMsg.GetLength()-2; i >= 0; i--)
    {
        if (strMsg.GetAt(i) == '\xBB' && strMsg.GetAt(i+1) == '\r')
        {
            i += 3;                 // Position to start of next line
            break;
        }
    }
    if (i < 0) i = 0;
    strHistory = strMsg.Left(i);    // Extract historical messages
    strMsg = strMsg.Mid(i);         // Remove historical msgs from main string
    if (strMsg.IsEmpty())
        return;
    // Search the message string backwards for a "reverse P" or start of buffer.
    for (i = strMsg.GetLength()-1; i >= 0; i--)
    {
        if (strMsg.GetAt(i) == '\xB6')  // 0xB6 is paragraph marker
            break;
    }
    if (i < 0)
        strReadOnly = strMsg;
    else
    {
        strReadOnly = strMsg.Left(i);   // Extract read-only part of current msg
        strEditable = strMsg.Mid(i+1);  // Extract editable part of current msg
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
void CGamDoc::MsgSeperateIntoPieces(CString strMsg, CString& strReadOnly,
    CString& strEditable)
{
    strReadOnly.Empty();
    strEditable.Empty();

    if (strMsg.IsEmpty())
        return;

    // Search the message string backwards for a "reverse P" or start of buffer.
    int i;
    for (i = strMsg.GetLength()-1; i >= 0; i--)
    {
        if (strMsg.GetAt(i) == '\xB6')  // 0xB6 is paragraph marker
            break;
    }
    if (i < 0)
        strReadOnly = strMsg;
    else
    {
        strReadOnly = strMsg.Left(i);   // Extract read-only part of current msg
        strEditable = strMsg.Mid(i+1);  // Extract editable part of current msg
    }
}

////////////////////////////////////////////////////////////////////////////
/* static */
CString CGamDoc::MsgEncodeFromPieces(CString strReadOnly,  CString strEditable)
{
    CString str;
    str = strReadOnly;
    if (!strEditable.IsEmpty())
        str += '\xB6' + strEditable;
    return str;
}

////////////////////////////////////////////////////////////////////////////
/* static */
void CGamDoc::MsgParseLegacyHistory(CString strLegacyMsg,
    CStringArray& astrMsgHist, CString& strCurMsg)
{
    astrMsgHist.RemoveAll();

    CString strHistory;
    CString strReadOnly;
    CString strEditable;

    MsgSeperateLegacyMsgIntoPieces(strLegacyMsg, strHistory, strReadOnly, strEditable);

    // New encoding only holds read-only and editable strings
    strCurMsg = MsgEncodeFromPieces(strReadOnly, strEditable);

    int nPos;
    // The loop searches for a chevron followed by <cr><lf>
    while ((nPos = strHistory.Find("\xBB\r\n")) != -1)
    {
        CString strBfr;
        strBfr = strHistory.Left(nPos + 3);         // Buffer the message
        strHistory = strHistory.Mid(nPos + 3);      // Remove the message
        // Finally we need to remove the divider string
        VERIFY((nPos = strBfr.Find("\r\n\xAB")) != -1);
        strBfr = strBfr.Left(nPos);
        astrMsgHist.Add(strBfr);
    }
    if (!strHistory.IsEmpty())
        astrMsgHist.Add(strHistory);
}


