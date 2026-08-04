// GamDoc3.cpp -- serialization support for the document.
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

#include    "stdafx.h"
#include    <io.h>
#include    "Gp.h"
#include    "DlgDice.h"
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
#include    "FrmDock.h"
#include    "PalMark.h"
#include    "PalTray.h"
#include    "PalReadMsg.h"

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
    ar.m_pDocument = *this;
    if (IsScenario())
        SerializeScenario(ar);
    else
        SerializeGame(ar);
    SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    SetFileFeatures(GetCBFeatures());
}

#ifdef _DEBUG
void CGamDoc::AssertValid() const
{
    mfcDoc->CDocument::AssertValid();
}

void CGamDoc::Dump(CDumpContext& dc) const
{
    mfcDoc->CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SerializeMoveSet(CArchive& ar, CHistRecord*& pHist)
{
    if (ar.IsStoring())
    {
        // Scenario file header
        ar.Write(FILEGMVSIGNATURE, 4);
        ar << value_preserving_cast<uint8_t>(fileGmvVerMajor);
        ar << value_preserving_cast<uint8_t>(fileGmvVerMinor);

        // leave space for pointer to feature list at end of file
        uint64_t offsetOffsetFeatureTable = UINT64_MAX;
        if (NumVersion(fileGmvVerMajor, fileGmvVerMinor) >= NumVersion(5, 0))
        {
            ar.Flush();     // ensure GetPosition() is current
            offsetOffsetFeatureTable = ar.GetFile()->GetPosition();
            ar << uint64_t(0);

            // initialize feature list
            c_fileFeatures = GetCBForcedFeatures();
            /* these features are global in the sense that, if
                any ID needs 32 bits, then we store all IDs in
                32 bit format */
            if (GetBoardManager().Needs32BitIDs() ||
                GetTileManager().Needs32BitIDs() ||
                GetPieceManager().Needs32BitIDs() ||
                GetMarkManager().Needs32BitIDs())
            {
                if (!GetCBFeatures().Check(ftrId32Bit))
                {
                    AfxThrowArchiveException(CArchiveException::badSchema);
                }
                c_fileFeatures.Add(ftrId32Bit);
            }
            if (GetPieceManager().Needs100SidePieces())
            {
                if (!GetCBFeatures().Check(ftrPiece100Sides))
                {
                    AfxThrowArchiveException(CArchiveException::badSchema);
                }
                c_fileFeatures.Add(ftrPiece100Sides);
            }
            /* TODO:  Check all size_t for 64bit vals.  For now,
                        use unless forbidden */
            if (GetCBFeatures().Check(ftrSizet64Bit))
            {
                c_fileFeatures.Add(ftrSizet64Bit);
            }
            c_fileFeatures.Add(ftrImgBMPZlib);
        }
        else if (NumVersion(fileGmvVerMajor, fileGmvVerMinor) == NumVersion(4, 0)) {
            c_fileFeatures = GetCBFile4Features();
        }
        else
        {
            wxASSERT(NumVersion(fileGmvVerMajor, fileGmvVerMinor) <= NumVersion(3, 90));
            c_fileFeatures = Features();
        }

        ar << value_preserving_cast<uint8_t>(progVerMajor);
        ar << value_preserving_cast<uint8_t>(progVerMinor);

        ar << m_dwScenarioID;
        pHist->Serialize(ar);

        // serialize done, so write features now
        if (NumVersion(fileGmvVerMajor, fileGmvVerMinor) >= NumVersion(5, 0))
        {
            ar.Flush();     // ensure GetPosition() is current
            uint64_t offsetFeatureTable = ar.GetFile()->GetPosition();
            ar << c_fileFeatures;
            // write data at current file pointer before changing it
            ar.Flush();
            ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
            ar << offsetFeatureTable;
        }
    }
    else
    {
        // Process the file header
        char szID[5];
        ar.Read(&szID, 4);
        szID[4] = 0;
        if (strcmp(szID, FILEGMVSIGNATURE) != 0)
        {
            wxMessageBox(CB::string::LoadString(IDS_ERR_NOTAMOVEFILE),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        uint8_t verMajor, verMinor;
        ar >> verMajor;
        ar >> verMinor;

        Features fileFeatures;
        if (NumVersion(verMajor, verMinor) >= NumVersion(5, 0))
        {
            try
            {
                ar.Flush();     // ensure GetPosition() is current
                uint64_t offsetOffsetFeatureTable = ar.GetFile()->GetPosition();
                uint64_t offsetFeatureTable;
                ar >> offsetFeatureTable;
                ar.Flush();
                ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetFeatureTable), CFile::begin);
                ar >> fileFeatures;
                ar.Flush();
                ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
                uint64_t dummy;
                ar >> dummy;
                wxASSERT(dummy == offsetFeatureTable);
            }
            catch (...)
            {
                wxASSERT(!"exception");
                // report file too new
                verMajor = value_preserving_cast<uint8_t>(fileGbxVerMajor + 1);
            }
        }
        else if (NumVersion(verMajor, verMinor) == NumVersion(4, 0))
        {
            fileFeatures = GetCBFile4Features();
        }
        else
        {
            wxASSERT(NumVersion(verMajor, verMinor) <= NumVersion(3, 90));
        }

        if (NumVersion(verMajor, verMinor) >
            NumVersion(fileGmvVerMajor, fileGmvVerMinor) &&
            // file 3.90 is the same as 3.10
            NumVersion(verMajor, verMinor) != NumVersion(3, 90))
        {
            wxMessageBox(CB::string::LoadString(IDS_ERR_GAMENEWER),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(verMajor, verMinor),
                                                        NumVersion(fileGamVerMajor, fileGamVerMinor));
        SetFileFeaturesGuard setFileFeaturesGuard(ar,
                                                    std::move(fileFeatures),
                                                    Features(GetCBFeatures()));

        uint8_t byteBucket;        // Place to dump unused bytes
        ar >> byteBucket;       // Eat the program version
        ar >> byteBucket;

        uint32_t dwScnID;
        ar >> dwScnID;          // load and check scenario ID
        if (dwScnID != m_dwScenarioID)
        {
            wxMessageBox(CB::string::LoadString(IDS_ERR_WRONGGAME),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        wxASSERT(pHist == NULL);
        pHist = new CHistRecord;

        pHist->Serialize(ar);
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))
        {
            pHist->m_pMList = MakeOwner<CMoveList>();
            pHist->m_pMList->Load(ar);             // before Ver2.90
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

        uint64_t offsetOffsetFeatureTable = UINT64_MAX;
        SerializeScenarioOrGame(ar, offsetOffsetFeatureTable);

        ar << m_dwCurrentPlayer;
        ar << m_dwPlayerHash;
        ar << m_strPlayerFileDescr;
        wxASSERT(m_eState != stateNotRecording);  // Shouldn't save this state
        ar << value_preserving_cast<uint16_t>(m_eState);
        ar << m_strCurMsg;
        ar << m_astrMsgHist;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            wxASSERT(m_nCurMove == Invalid_v<size_t> ||
                    m_nCurMove < size_t(0xFFFF));
            ar << (m_nCurMove == Invalid_v<size_t> ? uint16_t(0xFFFF) : value_preserving_cast<uint16_t>(m_nCurMove));
            wxASSERT(m_nFirstMove == Invalid_v<size_t> ||
                    m_nFirstMove < size_t(0xFFFF));
            ar << (m_nFirstMove == Invalid_v<size_t> ? uint16_t(0xFFFF) : value_preserving_cast<uint16_t>(m_nFirstMove));
            wxASSERT(m_nCurHist == Invalid_v<size_t> ||
                    m_nCurHist < size_t(0xFFFF));
            ar << (m_nCurHist == Invalid_v<size_t> ? uint16_t(0xFFFF) : value_preserving_cast<uint16_t>(m_nCurHist));
            wxASSERT(m_nMoveIdxAtBookMark == Invalid_v<size_t> ||
                    m_nMoveIdxAtBookMark < size_t(0xFFFF));
            ar << (m_nMoveIdxAtBookMark == Invalid_v<size_t> ? uint16_t(0xFFFF) : value_preserving_cast<uint16_t>(m_nMoveIdxAtBookMark));
        }
        else
        {
            CB::WriteCount(ar, m_nCurMove);
            CB::WriteCount(ar, m_nFirstMove);
            CB::WriteCount(ar, m_nCurHist);
            CB::WriteCount(ar, m_nMoveIdxAtBookMark);
        }

        ar << value_preserving_cast<uint16_t>(m_bStepToNextHist);
        ar << value_preserving_cast<uint16_t>(m_bKeepSkipInd);
        ar << value_preserving_cast<uint16_t>(m_bAutoStep);
        ar << value_preserving_cast<uint16_t>(m_bMsgWinVisible);

        if (GetCBFeatures().Check(ftrCRollState))
        {
            CB::AddFeature(ar, ftrCRollState);
            ar << bool(m_pRollState);
            if (m_pRollState)
            {
                ar << *m_pRollState;
            }
        }
        ar << m_nSeedCarryOver;

        ar << uint8_t(m_pRcdMoves != NULL ? 1 : 0);
        if (m_pRcdMoves)
            m_pRcdMoves->Store(ar);

        ar << uint8_t(m_pHistMoves != NULL ? 1 : 0);
        if (m_pHistMoves)
            m_pHistMoves->Store(ar);

        ar << uint8_t(m_pPlayHist != NULL ? 1 : 0);
        if (m_pPlayHist)
            m_pPlayHist->Serialize(ar);

        ar << uint8_t(m_pBookMark != NULL ? 1 : 0);
        if (m_pBookMark)
            m_pBookMark->Serialize(ar);

        ar << uint8_t(m_pHistTbl != NULL ? 1 : 0);
        if (m_pHistTbl)
            m_pHistTbl->Serialize(ar);

        // serialize done, so write features now
        if (NumVersion(fileGsnVerMajor, fileGsnVerMinor) >= NumVersion(5, 0))
        {
            ar.Flush();     // ensure GetPosition() is current
            uint64_t offsetFeatureTable = ar.GetFile()->GetPosition();
            ar << c_fileFeatures;
            // write data at current file pointer before changing it
            ar.Flush();
            ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
            ar << offsetFeatureTable;
        }
    }
    else
    {
        // Process the file header
        char szID[5];
        ar.Read(&szID, 4);
        szID[4] = 0;
        if (strcmp(szID, FILEGAMSIGNATURE) != 0)
        {
            wxMessageBox(CB::string::LoadString(IDS_ERR_NOTAGAME),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }

        uint64_t dummy;
        SerializeScenarioOrGame(ar, dummy);

        uint16_t wTmp;
        uint8_t cTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
            {
                ar >> wTmp;
                m_dwCurrentPlayer = PlayerMask(UPGRADE_OWNER_MASK(wTmp));
            }
            else
                ar >> m_dwCurrentPlayer;
            ar >> m_dwPlayerHash;
            ar >> m_strPlayerFileDescr;

            if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10) && m_dwCurrentPlayer != OWNER_MASK_SPECTATOR)
                m_dwPlayerHash = CalculateHashForCurrentPlayerMask(); // Recompute player hash
        }
        else
        {
            m_dwCurrentPlayer = OWNER_MASK_SPECTATOR;
            m_dwPlayerHash = 0;
            m_strPlayerFileDescr.clear();
        }

        ar >> wTmp; m_eState = static_cast<GameState>(wTmp);
        wxASSERT(m_eState != stateNotRecording);  // Shouldn't save this state
        ar >> m_strCurMsg;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))
            ar >> m_astrMsgHist;
        else
            MsgParseLegacyHistory(m_strCurMsg, m_astrMsgHist, m_strCurMsg);
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
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
            ar >> wTmp; m_bStepToNextHist = static_cast<BOOL>(wTmp); // Ver2.90
            ar >> wTmp; m_bKeepSkipInd = static_cast<BOOL>(wTmp);    // Ver2.90
            ar >> wTmp; m_bAutoStep = value_preserving_cast<BOOL>(wTmp);       // Ver2.90
            ar >> wTmp; m_bMsgWinVisible = value_preserving_cast<BOOL>(wTmp);  // Ver2.90
        }

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            if (CB::GetFeatures(ar).Check(ftrCRollState))
            {
                bool temp;
                ar >> temp;
                if (temp)
                {
                    m_pRollState = MakeOwner<CRollState>();
                    ar >> *m_pRollState;
                }
            }
            ar >> m_nSeedCarryOver;
        }

        // Process Move List
        ar >> cTmp;
        if (cTmp != 0)
        {
            m_pRcdMoves = MakeOwner<CMoveList>();
            m_pRcdMoves->Load(ar);
        }
        else
        {
            // If no recording is under way make sure the random number seed
            // is new in case the first recorded move record is a random number
            // operation.
            m_nSeedCarryOver = value_preserving_cast<uint32_t>(GetTickCount());
        }

        // Process History Playback Move List
        ar >> cTmp;
        if (cTmp != 0)
        {
            m_pHistMoves = MakeOwner<CMoveList>();
            m_pHistMoves->Load(ar);
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
            m_pBookMark = new CGameState();
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

        if (GetPBoardManager().FindPieceOnBoard(PieceID(0)) != NULL)
        {
            CTrayManager& pTMgr = GetTrayManager();
            while (pTMgr.FindPieceIDInTraySet(PieceID(0)) != NULL)
            {
                TRACE0("Removed bogus PieceID 0 from tray\n");
                pTMgr.RemovePieceIDFromTraySets(PieceID(0));
            }
        }
        GetTrayManager().PropagateOwnerMaskToAllPieces(this);
        GetPBoardManager().PropagateOwnerMaskToAllPieces();
        GetMainFrame()->GetMessageWindow().SetText(this);
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
            wxMessageBox(CB::string::LoadString(IDS_ERR_NOTASCENARIO),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
    }
    uint64_t offsetOffsetFeatureTable = UINT64_MAX;
    SerializeScenarioOrGame(ar, offsetOffsetFeatureTable);

    if (ar.IsStoring())
    {
        // serialize done, so write features now
        if (NumVersion(fileGsnVerMajor, fileGsnVerMinor) >= NumVersion(5, 0))
        {
            ar.Flush();     // ensure GetPosition() is current
            uint64_t offsetFeatureTable = ar.GetFile()->GetPosition();
            ar << c_fileFeatures;
            // write data at current file pointer before changing it
            ar.Flush();
            ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
            ar << offsetFeatureTable;
        }
    }

    // This code will repair a game containing extraneous ID 0 Pieces
    // in trays. It will only fix the gamebox if the actual piece is
    // on a board somewhere.
    if (GetPBoardManager().FindPieceOnBoard(PieceID(0)) != NULL)
    {
        CTrayManager& pTMgr = GetTrayManager();
        while (pTMgr.FindPieceIDInTraySet(PieceID(0)) != NULL)
        {
            CPP20_TRACE("Removed bogus PieceID 0 from tray\n");
            pTMgr.RemovePieceIDFromTraySets(PieceID(0));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// File signature already processed
void CGamDoc::SerializeScenarioOrGame(CArchive& ar, uint64_t& offsetOffsetFeatureTable)
{
    if (ar.IsStoring())
    {
        ASSERT(fileGsnVerMajor == fileGamVerMajor &&
                fileGsnVerMinor == fileGamVerMinor);
        ar << value_preserving_cast<uint8_t>(fileGsnVerMajor);
        ar << value_preserving_cast<uint8_t>(fileGsnVerMinor);

        if (NumVersion(fileGsnVerMajor, fileGsnVerMinor) >= NumVersion(5, 0))
        {
            // leave space for pointer to feature list at end of file
            ar.Flush();     // ensure GetPosition() is current
            offsetOffsetFeatureTable = ar.GetFile()->GetPosition();
            ar << uint64_t(0);

            // initialize feature list
            c_fileFeatures = GetCBForcedFeatures();
            /* these features are global in the sense that, if
                any ID needs 32 bits, then we store all IDs in
                32 bit format */
            if (GetBoardManager().Needs32BitIDs() ||
                GetTileManager().Needs32BitIDs() ||
                GetPieceManager().Needs32BitIDs() ||
                GetMarkManager().Needs32BitIDs())
            {
                if (!GetCBFeatures().Check(ftrId32Bit))
                {
                    AfxThrowArchiveException(CArchiveException::badSchema);
                }
                c_fileFeatures.Add(ftrId32Bit);
            }
            if (GetPieceManager().Needs100SidePieces())
            {
                if (!GetCBFeatures().Check(ftrPiece100Sides))
                {
                    AfxThrowArchiveException(CArchiveException::badSchema);
                }
                c_fileFeatures.Add(ftrPiece100Sides);
            }
            /* TODO:  Check all size_t for 64bit vals.  For now,
                        use unless forbidden */
            if (GetCBFeatures().Check(ftrSizet64Bit))
            {
                c_fileFeatures.Add(ftrSizet64Bit);
            }
        }
        else if (NumVersion(fileGsnVerMajor, fileGsnVerMinor) == NumVersion(4, 0) ||
            NumVersion(fileGsnVerMajor, fileGsnVerMinor) == NumVersion(104, 5)) {
            c_fileFeatures = GetCBFile4Features();
        }
        else
        {
            wxASSERT(NumVersion(fileGmvVerMajor, fileGmvVerMinor) == NumVersion(3, 10));
            c_fileFeatures = Features();
        }

        // Before we do the actual save see if the user desires to have
        // the state of the windows saved. If they do, get the state information.
        // The state data will be saved during the serialization
        // process.
        // Delete old data
        m_pWinState = NULL;
        if (m_bSaveWindowPositions)
        {
            if (GetCBFeatures().Check(ftrAuiLayout))
            {
                CB::AddFeature(ar, ftrAuiLayout);
                m_pWinState = new CGpWinStateMgr(*this);
                m_pWinState->GetStateOfOpenDocumentFrames(CB::GetFeatures(ar));
            }
        }

        ar << value_preserving_cast<uint8_t>(progVerMajor);
        ar << value_preserving_cast<uint8_t>(progVerMinor);

        ar << value_preserving_cast<uint16_t>(m_bDisableOwnerTips);
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

        ar << value_preserving_cast<uint16_t>(m_bKeepGamBackup);
        ar << value_preserving_cast<uint16_t>(m_bKeepMoveHist);
        ar << value_preserving_cast<uint16_t>(m_bVrfyGameState);
        ar << value_preserving_cast<uint16_t>(m_bVrfySaveState);
        ar << value_preserving_cast<uint16_t>(m_bShowObjTipText);          // V2.0
        m_mapStrings.Serialize(ar);             // V2.0

        if (m_pPlayerMgr != NULL)               // V2.0
        {
            ar << uint8_t(1);                      // 1 -> player accounts follow
            m_pPlayerMgr->Serialize(ar);
        }
        else
            ar << uint8_t(0);                      // 0 -> no player accounts

        ar << value_preserving_cast<uint16_t>(m_bSaveWindowPositions);     // V2.0

        if (m_pWinState != NULL)
        {
            ar << uint8_t(1);                      // 0 -> win state serialized   // V2.0
            m_pWinState->Serialize(ar);         // V2.0
        }
        else
            ar << uint8_t(0);                      // 0 -> no win state serialize // V2.0

        ar << value_preserving_cast<uint16_t>(m_bTrayAVisible);
        m_palTrayA->Serialize(ar);               // Save tray position on screen
        ar << value_preserving_cast<uint16_t>(m_bTrayBVisible);
        m_palTrayB->Serialize(ar);               // Save tray position on screen
        ar << value_preserving_cast<uint16_t>(m_bMarkPalVisible);
        m_palMark->Serialize(ar);                // Save tray position on screen

        // Main content serialization....

        m_pPBMgr->Serialize(ar);        // Board contents
        m_pYMgr->Serialize(ar);         // Tray contents
        m_pPTbl->Serialize(ar);         // Playing Piece state table
    }
    else
    {
        uint8_t verMajor, verMinor;
        uint16_t wTmp;
        uint32_t dwCurFileSlot = uint32_t(0xffffffff);

        ar >> verMajor;
        ar >> verMinor;

        Features fileFeatures;
        if (NumVersion(verMajor, verMinor) >= NumVersion(5, 0))
        {
            try
            {
                ar.Flush();     // ensure GetPosition() is current
                offsetOffsetFeatureTable = ar.GetFile()->GetPosition();
                uint64_t offsetFeatureTable;
                ar >> offsetFeatureTable;
                ar.Flush();
                ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetFeatureTable), CFile::begin);
                ar >> fileFeatures;
                ar.Flush();
                ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
                uint64_t dummy;
                ar >> dummy;
                wxASSERT(dummy == offsetFeatureTable);
            }
            catch (...)
            {
                wxASSERT(!"exception");
                // report file too new
                verMajor = value_preserving_cast<uint8_t>(fileGbxVerMajor + 1);
            }
        }
        else if (NumVersion(verMajor, verMinor) == NumVersion(4, 0))
        {
            fileFeatures = GetCBFile4Features();
        }
        else
        {
            wxASSERT(NumVersion(verMajor, verMinor) <= NumVersion(3, 90));
        }

        if (NumVersion(verMajor, verMinor) >
            NumVersion(fileGsnVerMajor, fileGsnVerMinor) &&
            // file 3.90 is the same as 3.10
            NumVersion(verMajor, verMinor) != NumVersion(3, 90))
        {
            wxMessageBox(CB::string::LoadString(IDS_ERR_SCENARIONEWER),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        if (NumVersion(verMajor, verMinor) < NumVersion(fileGbxVerMajor, fileGbxVerMinor))
        {
            if (wxMessageBox(CB::string::LoadString(IDS_WARN_FILE_UPGRADE),
                            CB::GetAppName(),
                            wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_WARNING) != wxOK)
            {
                AfxThrowArchiveException(CArchiveException::genericException);
            }
        }
        m_nLoadedFileVersion = NumVersion(verMajor, verMinor);
        SetLoadingVersion(m_nLoadedFileVersion);
        SetFileFeatures(std::move(fileFeatures));

        uint8_t byteBucket;        // Place to dump unused bytes
        ar >> byteBucket;       // Eat the program version
        ar >> byteBucket;

        ar >> wTmp; m_bDisableOwnerTips = value_preserving_cast<BOOL>(wTmp);
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
        CPP20_TRACE("Scenario Description length is {}\n", m_strScnDescr.a_size());

        ar >> wTmp; m_bKeepGamBackup = value_preserving_cast<BOOL>(wTmp);
        ar >> wTmp; m_bKeepMoveHist = value_preserving_cast<BOOL>(wTmp);
        ar >> wTmp; m_bVrfyGameState = value_preserving_cast<BOOL>(wTmp);
        ar >> wTmp; m_bVrfySaveState = value_preserving_cast<BOOL>(wTmp);

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            uint8_t bTmp;

            ar >> wTmp;
            m_bShowObjTipText = value_preserving_cast<BOOL>(wTmp);         // V2.0
            m_mapStrings.Serialize(ar);             // V2.0

            ar >> bTmp;                             // 1 -> player accounts follow // V2.0

            if (bTmp)
            {
                m_pPlayerMgr = new CPlayerManager;
                m_pPlayerMgr->Serialize(ar);        // V2.0
            }


            ar >> wTmp;
            m_bSaveWindowPositions = value_preserving_cast<BOOL>(wTmp);    // V2.0

            m_pWinState = NULL;

            ar >> bTmp;                             // State data follows flag // V2.0
            if (bTmp)
            {
                m_pWinState = new CGpWinStateMgr(*this);
                m_pWinState->Serialize(ar);
            }
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   // Ver 2.90
            {
                // Discard old save data because we can't use it.
                m_pWinState = NULL;
            }
        }

        ar >> wTmp; m_bTrayAVisible = value_preserving_cast<BOOL>(wTmp);
        wxASSERT(!m_palTrayA);
        m_palTrayA = new CTrayPalette(*this, ID_VIEW_TRAYA);
        m_palTrayA->Serialize(ar);                   // Restore tray position on screen
        ar >> wTmp; m_bTrayBVisible = value_preserving_cast<BOOL>(wTmp);
        wxASSERT(!m_palTrayB);
        m_palTrayB = new CTrayPalette(*this, ID_VIEW_TRAYB);
        m_palTrayB->Serialize(ar);                   // Restore tray position on screen
        ar >> wTmp; m_bMarkPalVisible = value_preserving_cast<BOOL>(wTmp);
        wxASSERT(!m_palMark);
        m_palMark = new CMarkerPalette(*this);
        m_palMark->Serialize(ar);                    // Restore tray position on screen

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
        m_pPTbl = new CPieceTable(m_pGbx->GetPieceManager(), *this);

        // Create the tray manager.
        m_pYMgr = new CTrayManager;
        m_pYMgr->SetTileManager(&m_pGbx->GetTileManager());

        // Finally set up the tray palettes
        m_palTrayA->Create(GetMainFrame()->GetDockingTrayAWindow());
        m_palTrayB->Create(GetMainFrame()->GetDockingTrayBWindow());
        m_palMark->Create(GetMainFrame()->GetDockingMarkerWindow());

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

#if 0
/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SerializeCurrentGameData(CFile* pFile, long lOffset, BOOL bSaving)
{
    if (lOffset != -1)          // If false use current position
        pFile->Seek(lOffset, CFile::begin);
    CArchive ar(pFile, (bSaving ? CArchive::store : CArchive::load) |
        CArchive::bNoFlushOnDelete);
    ar.m_pDocument = *this;
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
    ar.m_pDocument = *this;
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
    ar.m_pDocument = *this;
    ar.m_bForceFlat = FALSE;
    pLst->Serialize(ar);
    ar.Close();                 // Flushes and Detaches CFile.
    return (long)pFile->GetPosition();
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::LoadGameBoxFileForSerialize()
{
    CB::string szFPath;

    m_pGbx = new CGameBox;      // Create game box object

    // First see if the unmodifed gamebox file name contained in the
    // Game/Scenario file exists. If it does, just use it. Otherwise,
    // Check if the gamebox is stored where the game or scenario is.
    // If we can't find it there, prompt for the location.
    if (!std::filesystem::exists(m_strGBoxFile))
    {
        CB::string szPath = StrTruncatePath(m_strTmpPathName);
        CB::string szGbxRootName = StrExtractFilename(m_strGBoxFile);

        szFPath = StrBuildFullFilename(szPath, szGbxRootName);

        if (!std::filesystem::exists(szFPath))
        {
            // File doesn't exist where the game file is.
            // Use open file dialog to locate file.
            CB::string str = AfxFormatString1(IDP_ERR_NOGAMEBOX, szFPath);
            if (wxMessageBox(str,
                            CB::GetAppName(),
                            wxOK | wxCANCEL | wxICON_EXCLAMATION) != wxOK)
                AfxThrowArchiveException(CArchiveException::genericException);

            CB::string strFilter = CB::string::LoadString(IDS_GBOX_FILTER);
            CB::string strTitle = CB::string::LoadString(IDS_GBOX_SELECT);

            wxFileDialog dlg(&CB::GetMainWndWx(), strTitle,
                                wxEmptyString, wxEmptyString,
                                strFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

            if (dlg.ShowModal() != wxID_OK)
                AfxThrowArchiveException(CArchiveException::genericException);

            m_strGBoxFile = dlg.GetPath();      // Set new name
            szFPath = dlg.GetPath();
        }
    }
    else
        szFPath = m_strGBoxFile;
    CB::string strErr;
    if (!m_pGbx->Load(*this, szFPath, strErr, m_dwGBoxID))
    {
        wxMessageBox(strErr,
                        CB::GetAppName(),
                        wxOK | wxICON_EXCLAMATION);
        AfxThrowArchiveException(CArchiveException::genericException);
    }
    // Finally check the major revision number and warn if different.
    if (m_pGbx->m_dwMajorRevs != m_dwMajorRevs)
    {
        if (wxMessageBox(CB::string::LoadString(IDS_ERR_MAJORREV),
                        CB::GetAppName(),
                        wxOK | wxCANCEL | wxICON_EXCLAMATION) != wxOK)
        {
            AfxThrowArchiveException(CArchiveException::genericException);
        }
    }
}

