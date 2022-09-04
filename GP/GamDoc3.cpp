// GamDoc3.cpp -- serialization support for the document.
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
#include    <io.h>
#include    "Gp.h"
#include    "FrmMain.h"
#include    "GamDoc.h"
#include    "GameBox.h"
#include    "GMisc.h"
#include    "StrLib.h"

#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Marks.h"
#include    "Trays.h"
#include    "MoveMgr.h"
#include    "GamState.h"
#include    "MoveHist.h"
#include    "WStateGp.h"
#include    "Player.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
/*
Game file format for versions before 2.90:
    +-----------------------------------+
    | "GAM" File Header                 |
+---|   - Offset to History Index Table |
|   +-----------------------------------+ (GAME_HDR_SIZE)
|   | Move History 0 (Move List Stream) |<--+
|   +-----------------------------------+   |
|   | Move History 1 (Move List Stream) |<--+
|   +-----------------------------------+   |
|   |       :                 :         |   :
|   +-----------------------------------+   |
|   | Move History k (Move List Stream) |<--+
|   +-----------------------------------+   |
+-->| Current Game State:               |   |
    |   - Tile Manager Stream           |   |
    |   - Board Manager Stream          |   |
    |   - Piece Table Stream            |   |
    |   - Current Move List             |   |
    |   - Bookmark Stream               |   |
    +-----------------------------------+   |
    | Move History Index Table Stream   |---+
    +-----------------------------------+
*/

/*
Game file format for versions 2.90 and greater:
    +-----------------------------------+
    | "GAM" File Header                 |
    +-----------------------------------+
    | Scenario/Game Common Data         |
    +-----------------------------------+
    | Current Game State                |
    +-----------------------------------+
    | Move History Table (incl/streams) }
    +-----------------------------------+
*/

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::Serialize(CArchive& ar)
{
    ar.m_pDocument = this;
    if (IsScenario())
        SerializeScenario(ar);
    else
        SerializeGame(ar);
    SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SerializeMoveSet(CArchive& ar, CHistRecord*& pHist)
{
    if (ar.IsStoring())
    {
        // Scenario file header
        ar.Write(FILEGMVSIGNATURE, 4);
        ar << (BYTE)fileGmvVerMajor;
        ar << (BYTE)fileGmvVerMinor;
        ar << (BYTE)progVerMajor;
        ar << (BYTE)progVerMinor;

        ar << m_dwScenarioID;
        pHist->Serialize(ar);
    }
    else
    {
        // Process the file header
        char szID[5];
        ar.Read(&szID, 4);
        szID[4] = 0;
        if (strcmp(szID, FILEGMVSIGNATURE) != 0)
        {
            AfxMessageBox(IDS_ERR_NOTAMOVEFILE, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        BYTE verMajor, verMinor;
        ar >> verMajor;
        ar >> verMinor;
        if (NumVersion(verMajor, verMinor) >
            NumVersion(fileGmvVerMajor, fileGmvVerMinor) &&
            // file 3.90 is the same as 3.10
            NumVersion(verMajor, verMinor) != NumVersion(3, 90))
        {
            AfxMessageBox(IDS_ERR_GAMENEWER, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(verMajor, verMinor),
                                                        NumVersion(fileGamVerMajor, fileGamVerMinor));

        BYTE byteBucket;        // Place to dump unused bytes
        ar >> byteBucket;       // Eat the program version
        ar >> byteBucket;

        DWORD dwScnID;
        ar >> dwScnID;          // load and check scenario ID
        if (dwScnID != m_dwScenarioID)
        {
            AfxMessageBox(IDS_ERR_WRONGGAME, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        ASSERT(pHist == NULL);
        pHist = new CHistRecord;

        pHist->Serialize(ar);
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))
        {
            pHist->m_pMList = MakeOwner<CMoveList>();
            pHist->m_pMList->Serialize(ar, FALSE);             // before Ver2.90
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SerializeGame(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // Scenario file header
        ar.Write(FILEGAMSIGNATURE, 4);

        SerializeScenarioOrGame(ar);

        ar << m_dwCurrentPlayer;
        ar << m_dwPlayerHash;
        ar << m_strPlayerFileDescr;
        ASSERT(m_eState != stateNotRecording);  // Shouldn't save this state
        ar << (WORD)m_eState;
        ar << m_strCurMsg;
        ar << m_astrMsgHist;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ASSERT(m_nCurMove == Invalid_v<size_t> ||
                    m_nCurMove < size_t(0xFFFF));
            ar << (m_nCurMove == Invalid_v<size_t> ? WORD(0xFFFF) : value_preserving_cast<WORD>(m_nCurMove));
            ASSERT(m_nFirstMove == Invalid_v<size_t> ||
                    m_nFirstMove < size_t(0xFFFF));
            ar << (m_nFirstMove == Invalid_v<size_t> ? WORD(0xFFFF) : value_preserving_cast<WORD>(m_nFirstMove));
            ASSERT(m_nCurHist == Invalid_v<size_t> ||
                    m_nCurHist < size_t(0xFFFF));
            ar << (m_nCurHist == Invalid_v<size_t> ? WORD(0xFFFF) : value_preserving_cast<WORD>(m_nCurHist));
            ASSERT(m_nMoveIdxAtBookMark == Invalid_v<size_t> ||
                    m_nMoveIdxAtBookMark < size_t(0xFFFF));
            ar << (m_nMoveIdxAtBookMark == Invalid_v<size_t> ? WORD(0xFFFF) : value_preserving_cast<WORD>(m_nMoveIdxAtBookMark));
        }
        else
        {
            CB::WriteCount(ar, m_nCurMove);
            CB::WriteCount(ar, m_nFirstMove);
            CB::WriteCount(ar, m_nCurHist);
            CB::WriteCount(ar, m_nMoveIdxAtBookMark);
        }

        ar << (WORD)m_bStepToNextHist;
        ar << (WORD)m_bKeepSkipInd;
        ar << (WORD)m_bAutoStep;
        ar << (WORD)m_bMsgWinVisible;

        ar << (DWORD)m_nSeedCarryOver;

        ar << (BYTE)(m_pRcdMoves != NULL ? 1 : 0);
        if (m_pRcdMoves)
            m_pRcdMoves->Serialize(ar, TRUE);   // Save undo stuff too

        ar << (BYTE)(m_pHistMoves != NULL ? 1 : 0);
        if (m_pHistMoves)
            m_pHistMoves->Serialize(ar, TRUE);  // Save undo stuff too

        ar << (BYTE)(m_pPlayHist != NULL ? 1 : 0);
        if (m_pPlayHist)
            m_pPlayHist->Serialize(ar);

        ar << (BYTE)(m_pBookMark != NULL ? 1 : 0);
        if (m_pBookMark)
            m_pBookMark->Serialize(ar);

        ar << (BYTE)(m_pHistTbl != NULL ? 1 : 0);
        if (m_pHistTbl)
            m_pHistTbl->Serialize(ar);
    }
    else
    {
        // Process the file header
        char szID[5];
        ar.Read(&szID, 4);
        szID[4] = 0;
        if (strcmp(szID, FILEGAMSIGNATURE) != 0)
        {
            AfxMessageBox(IDS_ERR_NOTAGAME, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }

        SerializeScenarioOrGame(ar);

        WORD wTmp;
        BYTE cTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
            {
                ar >> wTmp;
                m_dwCurrentPlayer = UPGRADE_OWNER_MASK(wTmp);
            }
            else
                ar >> m_dwCurrentPlayer;
            ar >> m_dwPlayerHash;
            ar >> m_strPlayerFileDescr;

            if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10) && m_dwCurrentPlayer != 0)
                m_dwPlayerHash = CalculateHashForCurrentPlayerMask(); // Recompute player hash
        }
        else
        {
            m_dwCurrentPlayer = 0;
            m_dwPlayerHash = 0;
            m_strPlayerFileDescr.Empty();
        }

        ar >> wTmp; m_eState = (GameState)wTmp;
        ASSERT(m_eState != stateNotRecording);  // Shouldn't save this state
        ar >> m_strCurMsg;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))
            ar >> m_astrMsgHist;
        else
            MsgParseLegacyHistory(m_strCurMsg, m_astrMsgHist, m_strCurMsg);
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar >> wTmp; m_nCurMove = (wTmp == 0xFFFF ? Invalid_v<size_t> : value_preserving_cast<size_t>(wTmp));
            ar >> wTmp; m_nFirstMove = (wTmp == 0xFFFF ? Invalid_v<size_t> : value_preserving_cast<size_t>(wTmp));
            ar >> wTmp; m_nCurHist = (wTmp == 0xFFFF ? Invalid_v<size_t> : value_preserving_cast<size_t>(wTmp));
            ar >> wTmp; m_nMoveIdxAtBookMark = (wTmp == 0xFFFF ? Invalid_v<size_t> : value_preserving_cast<size_t>(wTmp));
        }
        else
        {
            m_nCurMove = CB::ReadCount(ar);
            m_nFirstMove = CB::ReadCount(ar);
            m_nCurHist = CB::ReadCount(ar);
            m_nMoveIdxAtBookMark = CB::ReadCount(ar);
        }

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))
        {
            ar >> wTmp; m_bStepToNextHist = (BOOL)wTmp; // Ver2.90
            ar >> wTmp; m_bKeepSkipInd = (BOOL)wTmp;    // Ver2.90
            ar >> wTmp; m_bAutoStep = (BOOL)wTmp;       // Ver2.90
            ar >> wTmp; m_bMsgWinVisible = (BOOL)wTmp;  // Ver2.90
        }

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            DWORD dwTmp;
            ar >> dwTmp;
            m_nSeedCarryOver = (UINT)dwTmp;
        }

        // Process Move List
        ar >> cTmp;
        if (cTmp != 0)
        {
            m_pRcdMoves = MakeOwner<CMoveList>();
            m_pRcdMoves->Serialize(ar, TRUE);
        }
        else
        {
            // If no recording is under way make sure the random number seed
            // is new in case the first recorded move record is a random number
            // operation.
            m_nSeedCarryOver = (UINT)GetTickCount();
        }

        // Process History Playback Move List
        ar >> cTmp;
        if (cTmp != 0)
        {
            m_pHistMoves = MakeOwner<CMoveList>();
            m_pHistMoves->Serialize(ar, TRUE);
        }

        // Process file playback history record.
        ar >> cTmp;
        if (cTmp)
        {
            m_pPlayHist = new CHistRecord;
            m_pPlayHist->Serialize(ar);
        }

        // Process bookmark
        ar >> cTmp;
        if (cTmp)
        {
            m_pBookMark = new CGameState(this);
            m_pBookMark->Serialize(ar);
        }

        // Process History Table
        ar >> cTmp;
        if (cTmp)
        {
            m_pHistTbl = new CHistoryTable;
            m_pHistTbl->Serialize(ar);
        }

        // Reconstitute various objects based on m_eState.
        if (m_eState == stateRecording || m_eState == stateMovePlay)
            m_pMoves = m_pRcdMoves.get();
        else
            m_pMoves = m_pHistMoves.get();

        // If the file being loaded is pre V2.90 vintage then
        // we want to load all the history records into memory.
        // In the old days the records were kept on disk and loaded
        // when required. Sounds like a good idea but it caused
        // many nasty side effects as the file format evolved.
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90) && m_pHistTbl != NULL)
        {
            if (!LoadVintageHistoryMoveLists(*ar.GetFile()))
            {
                 AfxThrowArchiveException(CArchiveException::genericException);
            }
        }

        // This code will repair a game containing extraneous ID 0 Pieces
        // in trays. It will only fix the gamebox if the actual piece is
        // on a board somewhere.

        if (GetPBoardManager()->FindPieceOnBoard(PieceID(0)) != NULL)
        {
            CTrayManager *pTMgr = GetTrayManager();
            while (pTMgr->FindPieceIDInTraySet(PieceID(0)) != NULL)
            {
                TRACE0("Removed bogus PieceID 0 from tray\n");
                pTMgr->RemovePieceIDFromTraySets(PieceID(0));
            }
        }
        GetTrayManager()->PropagateOwnerMaskToAllPieces(this);
        GetPBoardManager()->PropagateOwnerMaskToAllPieces();
        GetMainFrame()->GetMessageWindow()->SetText(this);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SerializeScenario(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // Scenario file header
        ar.Write(FILEGSNSIGNATURE, 4);
    }
    else
    {
        // Process the file header
        char szID[5];
        ar.Read(&szID, 4);
        szID[4] = 0;
        if (strcmp(szID, FILEGSNSIGNATURE) != 0)
        {
            AfxMessageBox(IDS_ERR_NOTASCENARIO, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
    }
    SerializeScenarioOrGame(ar);

    // This code will repair a game containing extraneous ID 0 Pieces
    // in trays. It will only fix the gamebox if the actual piece is
    // on a board somewhere.
    if (GetPBoardManager()->FindPieceOnBoard(PieceID(0)) != NULL)
    {
        CTrayManager *pTMgr = GetTrayManager();
        while (pTMgr->FindPieceIDInTraySet(PieceID(0)) != NULL)
        {
            TRACE0("Removed bogus PieceID 0 from tray\n");
            pTMgr->RemovePieceIDFromTraySets(PieceID(0));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// File signature already processed
void CGamDoc::SerializeScenarioOrGame(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (BYTE)fileGsnVerMajor;
        ar << (BYTE)fileGsnVerMinor;
        ar << (BYTE)progVerMajor;
        ar << (BYTE)progVerMinor;

        ar << (WORD)m_bDisableOwnerTips;
        // ar << m_wReserved;  (now m_bDisableOwnerTips);
        ar << m_wReserved2;             // Spares
        ar << m_wReserved3;
        ar << m_wReserved4;

        // Rest of header
        ar << m_dwGBoxID;               // Gamebox ID
        ar << m_dwMajorRevs;            // Gamebox major rev #
        ar << m_dwMinorRevs;            // Gamebox minor rev #
        ar << m_strGBoxFile;            // Filename of last Gamebox used

        ar << m_dwScenarioID;           // Rest of scenario header info
        ar << m_strScnTitle;
        ar << m_strScnAuthor;
        ar << m_strScnDescr;

        ar << (WORD)m_bKeepGamBackup;
        ar << (WORD)m_bKeepMoveHist;
        ar << (WORD)m_bVrfyGameState;
        ar << (WORD)m_bVrfySaveState;
        ar << (WORD)m_bShowObjTipText;          // V2.0
        m_mapStrings.Serialize(ar);             // V2.0

        if (m_pPlayerMgr != NULL)               // V2.0
        {
            ar << (BYTE)1;                      // 1 -> player accounts follow
            m_pPlayerMgr->Serialize(ar);
        }
        else
            ar << (BYTE)0;                      // 0 -> no player accounts

        ar << (WORD)m_bSaveWindowPositions;     // V2.0

        if (m_pWinState != NULL)
        {
            ar << (BYTE)1;                      // 0 -> win state serialized   // V2.0
            m_pWinState->Serialize(ar);         // V2.0
        }
        else
            ar << (BYTE)0;                      // 0 -> no win state serialize // V2.0

        ar << (WORD)m_bTrayAVisible;
        m_palTrayA.Serialize(ar);               // Save tray position on screen
        ar << (WORD)m_bTrayBVisible;
        m_palTrayB.Serialize(ar);               // Save tray position on screen
        ar << (WORD)m_bMarkPalVisible;
        m_palMark.Serialize(ar);                // Save tray position on screen

        // Main content serialization....

        m_pPBMgr->Serialize(ar);        // Board contents
        m_pYMgr->Serialize(ar);         // Tray contents
        m_pPTbl->Serialize(ar);         // Playing Piece state table
    }
    else
    {
        BYTE verMajor, verMinor;
        WORD wTmp;
        DWORD dwCurFileSlot = DWORD(0xffffffff);

        ar >> verMajor;
        ar >> verMinor;
        if (NumVersion(verMajor, verMinor) >
            NumVersion(fileGsnVerMajor, fileGsnVerMinor) &&
            // file 3.90 is the same as 3.10
            NumVersion(verMajor, verMinor) != NumVersion(3, 90))
        {
            AfxMessageBox(IDS_ERR_SCENARIONEWER, MB_OK | MB_ICONEXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        if (NumVersion(verMajor, verMinor) < NumVersion(fileGbxVerMajor, fileGbxVerMinor))
        {
            if (AfxMessageBox(IDS_WARN_FILE_UPGRADE,
                MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING) != IDOK)
            {
                AfxThrowArchiveException(CArchiveException::genericException);
            }
        }
        m_nLoadedFileVersion = NumVersion(verMajor, verMinor);
        SetLoadingVersion(m_nLoadedFileVersion);

        BYTE byteBucket;        // Place to dump unused bytes
        ar >> byteBucket;       // Eat the program version
        ar >> byteBucket;

        ar >> wTmp; m_bDisableOwnerTips = (BOOL)wTmp;
        // ar >> m_wReserved1;
        ar >> m_wReserved2;             // Spares
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        // Process the Gamebox info.

        ar >> m_dwGBoxID;               // Gamebox ID
        ar >> m_dwMajorRevs;            // Gamebox major rev #
        ar >> m_dwMinorRevs;            // Gamebox minor rev #
        ar >> m_strGBoxFile;            // Filename of last Gamebox used

        // Load gamebox using archival procedures.

        LoadGameBoxFileForSerialize();  // Throws exception if failed

        // Restore our loading version.
        SetLoadingVersion(m_nLoadedFileVersion);

        // Update game box version info...

        BOOL bMajorChg = m_dwMajorRevs != m_pGbx->m_dwMajorRevs;
        m_dwMajorRevs = m_pGbx->m_dwMajorRevs;
        m_dwMinorRevs = m_pGbx->m_dwMinorRevs;

        // Past the tough stuff...Process the rest of the scenario

        ar >> m_dwScenarioID;           // Rest of scenario header info
        ar >> m_strScnTitle;
        ar >> m_strScnAuthor;
        ar >> m_strScnDescr;
        TRACE1("Scenario Description length is %d\n", m_strScnDescr.GetLength());

        ar >> wTmp; m_bKeepGamBackup = (BOOL)wTmp;
        ar >> wTmp; m_bKeepMoveHist = (BOOL)wTmp;
        ar >> wTmp; m_bVrfyGameState = (BOOL)wTmp;
        ar >> wTmp; m_bVrfySaveState = (BOOL)wTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            BYTE bTmp;

            ar >> wTmp;
            m_bShowObjTipText = (BOOL)wTmp;         // V2.0
            m_mapStrings.Serialize(ar);             // V2.0

            ar >> bTmp;                             // 1 -> player accounts follow // V2.0

            if (bTmp)
            {
                if (m_pPlayerMgr != NULL)
                    delete m_pPlayerMgr;
                m_pPlayerMgr = new CPlayerManager;
                m_pPlayerMgr->Serialize(ar);        // V2.0
            }


            ar >> wTmp;
            m_bSaveWindowPositions = (BOOL)wTmp;    // V2.0

            if (m_pWinState != NULL)
                delete m_pWinState;                 // Delete old version
            m_pWinState = NULL;

            ar >> bTmp;                             // State data follows flag // V2.0
            if (bTmp)
            {
                m_pWinState = new CGpWinStateMgr;
                m_pWinState->SetDocument(this);
                m_pWinState->Serialize(ar);
            }
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   // Ver 2.90
            {
                // Discard old save data because we can't use it.
                delete m_pWinState;
                m_pWinState = NULL;
            }
        }

        ar >> wTmp; m_bTrayAVisible = (BOOL)wTmp;
        m_palTrayA.Serialize(ar);                   // Restore tray position on screen
        ar >> wTmp; m_bTrayBVisible = (BOOL)wTmp;
        m_palTrayB.Serialize(ar);                   // Restore tray position on screen
        ar >> wTmp; m_bMarkPalVisible = (BOOL)wTmp;
        m_palMark.Serialize(ar);                    // Restore tray position on screen

        // OK....retrieve the file offset of the game data...
        if (NumVersion(verMajor, verMinor) < NumVersion(2, 90))
            ar >> dwCurFileSlot;                    // Send don't need to keep m_dwCurFileSlot anymore

        if (!IsScenario() && NumVersion(verMajor, verMinor) < NumVersion(2, 90))
        {
            // Move the stream forward.
            ar.Flush();
            ar.GetFile()->Seek(dwCurFileSlot, CFile::begin);
        }

        // Create the scenario objects prior to restoration

        m_pPBMgr = new CPBoardManager(*this);

        // Create the playing piece table...
        m_pPTbl = new CPieceTable(*m_pGbx->GetPieceManager(), *this);

        // Create the tray manager.
        m_pYMgr = new CTrayManager;
        m_pYMgr->SetTileManager(m_pGbx->GetTileManager());

        // Finally set up the tray palettes
        m_palTrayA.Create(GetMainFrame()->GetDockingTrayAWindow());
        m_palTrayB.Create(GetMainFrame()->GetDockingTrayBWindow());
        m_palMark.Create(GetMainFrame()->GetDockingMarkerWindow());

        // Main content serialization....
        m_pPBMgr->Serialize(ar);    // Board contents
        m_pYMgr->Serialize(ar);     // Tray contents

        // Note: the playing piece table MUST be deserialized AFTER
        // the board and trays since the piece table code may need to
        // fix trays and boards due to piece table truncation.

        m_pPTbl->Serialize(ar);     // Playing Piece state table

        m_pPTbl->PurgeUndefinedPieceIDs(); // Final fixups
    }
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SerializeCurrentGameData(CFile* pFile, long lOffset, BOOL bSaving)
{
    if (lOffset != -1)          // If false use current position
        pFile->Seek(lOffset, CFile::begin);
    CArchive ar(pFile, (bSaving ? CArchive::store : CArchive::load) |
        CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;
    m_pHistTbl->Serialize(ar);

}

/////////////////////////////////////////////////////////////////////////////

CMoveList* CGamDoc::DeserializeMovesFromFile(CFile* pFile, long lOffset)
{
    CMoveList* pLst = new CMoveList;
    if (lOffset != -1)          // If false use current position
        pFile->Seek(lOffset, CFile::begin);
    CArchive ar(pFile, CArchive::load | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;
    pLst->Serialize(ar);
    ar.Close();                 // Flushes and Detaches CFile.
    return pLst;
}

/////////////////////////////////////////////////////////////////////////////

long CGamDoc::SerializeMovesToFile(CFile* pFile, long lOffset, CMoveList* pLst)
{
    if (lOffset != -1)              // If false use current position
        pFile->Seek(lOffset, CFile::begin);
    CArchive ar(pFile, CArchive::store | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;
    pLst->Serialize(ar);
    ar.Close();                 // Flushes and Detaches CFile.
    return (long)pFile->GetPosition();
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::LoadGameBoxFileForSerialize()
{
    char szPath[MAX_PATH];
    char szFPath[MAX_PATH];
    char szGbxRootName[MAX_PATH];

    m_pGbx = new CGameBox;      // Create game box object

    // First see if the unmodifed gamebox file name contained in the
    // Game/Scenario file exists. If it does, just use it. Otherwise,
    // Check if the gamebox is stored where the game or scenario is.
    // If we can't find it there, prompt for the location.
    if (_access(m_strGBoxFile, 0) == -1)
    {
        strcpy(szPath, m_strTmpPathName);
        StrTruncatePath(szPath);
        StrExtractFilename(szGbxRootName, m_strGBoxFile);

        StrBuildFullFilename(szFPath, szPath, szGbxRootName);

        if (_access(szFPath, 0) == -1)
        {
            // File doesn't exist where the game file is.
            // Use open file dialog to locate file.
            CString str;
            AfxFormatString1(str, IDP_ERR_NOGAMEBOX, szFPath);
            if (AfxMessageBox(str, MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
                AfxThrowArchiveException(CArchiveException::genericException);

            CString strFilter;
            strFilter.LoadString(IDS_GBOX_FILTER);
            CString strTitle;
            strTitle.LoadString(IDS_GBOX_SELECT);

            CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
                strFilter, NULL, 0);
            dlg.m_ofn.lpstrTitle = strTitle;

            if (dlg.DoModal() != IDOK)
                AfxThrowArchiveException(CArchiveException::genericException);

            m_strGBoxFile = dlg.GetPathName();      // Set new name
            strcpy(szFPath, dlg.GetPathName());
        }
    }
    else
        strcpy(szFPath, m_strGBoxFile);
    CString strErr;
    if (!m_pGbx->Load(this, szFPath, strErr, m_dwGBoxID))
    {
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        AfxThrowArchiveException(CArchiveException::genericException);
    }
    // Finally check the major revision number and warn if different.
    if (m_pGbx->m_dwMajorRevs != m_dwMajorRevs)
    {
        if (AfxMessageBox(IDS_ERR_MAJORREV, MB_OKCANCEL |
            MB_ICONEXCLAMATION) != IDOK)
        {
            AfxThrowArchiveException(CArchiveException::genericException);
        }
    }
}

