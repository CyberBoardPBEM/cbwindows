// GamDoc.h
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

#ifndef _GAMDOC_H
#define _GAMDOC_H

#ifndef     _MARKS_H
#include    "Marks.h"
#endif

#ifndef     _PALTRAY_H
#include    "PalTray.h"
#endif

#ifndef     _PALMARK_H
#include    "PalMark.h"
#endif

#ifndef     _PIECES_H
#include    "Pieces.h"
#endif

#ifndef     _GAMEBOX_H
#include    "GameBox.h"
#endif

#ifndef     _MAPFACE_H
#include    "MapFace.h"
#endif

#ifndef     _MAPSTRNG_H
#include    "Mapstrng.h"
#endif

#ifndef     _DRAWOBJ_H
#include    "DrawObj.h"
#endif

#ifndef     _BOARD_H
#include    "Board.h"
#endif

#ifndef     _MOVEMGR_H
#include    "MoveMgr.h"
#endif

#ifndef     __VERSIONS_H__
#include    "Versions.h"
#endif

////////////////////////////////////////////////////////////////

class CGameBox;
class CTrayManager;
class CPieceTable;
class CDrawObj;
class CPieceObj;
class CMarkObj;
class CLine;
class CPlayBoard;
class CPBoardManager;
class CTileManager;
class CGameState;
class CHistoryTable;
class CHistRecord;
class CSelList;
class CRollState;
class CGpWinStateMgr;
class CPlayerManager;
class CGamProjView;
class CSendMsgDialog;

class CModelessDialogCleaner
{
public:
    void operator()(CDialog* p) const;
};

////////////////////////////////////////////////////////////////

enum EGamDocHint
{
    HINT_ALWAYSUPDATE =             0,       // Must be zero!

    HINT_BOARDCHANGE =              0x0001,
    HINT_CLEARINDTIP =              0x0002, // Clear any tooltip based indicators
    HINT_GSNPROPCHANGE =            0x0004,
    HINT_GAMPROPCHANGE =            0x0008,
    HINT_TRAYCHANGE =               0x0010,
    HINT_UPDATEOBJECT =             0x0100,
    HINT_UPDATEOBJLIST =            0x0200,
    HINT_UPDATEBOARD =              0x0300,
    HINT_UPDATESELECT =             0x0400,
    HINT_UPDATESELECTLIST =         0x0500, // resync select list. sender ignores.
    HINT_INVALIDATERECT =           0x0600,
    HINT_GAMESTATEUSED =            0x1000,
    HINT_POINTINVIEW =              0x2000,
    HINT_SELECTOBJ =                0x4000,
    HINT_SELECTOBJLIST =            0x8000,

    HINT_INVALID =                  -1,     // uninitialized Args
};

class CGamDocHint : public CObject
{
    DECLARE_DYNCREATE(CGamDocHint);
public:
    CGamDocHint() : hint(HINT_INVALID) {}

    template<EGamDocHint HINT>
    struct Args
    {
    };

    template<>
    struct Args<HINT_TRAYCHANGE>
    {
        CTraySet*   m_pTray;
    };

    template<>
    struct Args<HINT_UPDATEOBJECT>
    {
        const CPlayBoard* m_pPBoard;
        CDrawObj*   m_pDrawObj;
    };

    template<>
    struct Args<HINT_UPDATEOBJLIST>
    {
        CPlayBoard* m_pPBoard;
        const std::vector<CB::not_null<CDrawObj*>>* m_pPtrList;
    };

    template<>
    struct Args<HINT_UPDATEBOARD>
    {
        CPlayBoard* m_pPBoard;
    };

    template<>
    struct Args<HINT_UPDATESELECT>
    {
        CPlayBoard* m_pPBoard;
        CSelList*   m_pSelList;
    };

    template<>
    struct Args<HINT_INVALIDATERECT>
    {
        CPlayBoard* m_pPBoard;
        const CRect* m_pRect;
    };

    template<>
    struct Args<HINT_POINTINVIEW>
    {
        CPlayBoard* m_pPBoard;
        POINT       m_point;
    };

    template<>
    struct Args<HINT_SELECTOBJ>
    {
        CPlayBoard* m_pPBoard;
        CDrawObj*   m_pDrawObj;
    };

    template<>
    struct Args<HINT_SELECTOBJLIST>
    {
        CPlayBoard* m_pPBoard;
        const std::vector<CB::not_null<CDrawObj*>>* m_pPtrList;
    };

    template<EGamDocHint HINT>
    Args<HINT>& GetArgs()
    {
        if (hint == HINT_INVALID)
        {
            hint = HINT;
        }
        else if (HINT != hint)
        {
            CbThrowBadCastException();
        }
        return reinterpret_cast<Args<HINT>&>(args);
    }

private:
    EGamDocHint hint;
    union {
        Args<HINT_TRAYCHANGE>       m_trayChange;
        Args<HINT_UPDATEOBJECT>     m_updateObject;
        Args<HINT_UPDATEOBJLIST>    m_updateObjList;
        Args<HINT_UPDATEBOARD>      m_updateBoard;
        Args<HINT_UPDATESELECT>     m_updateSelect;
        Args<HINT_INVALIDATERECT>   m_updateRect;
        Args<HINT_POINTINVIEW>      m_pointInView;
        Args<HINT_SELECTOBJ>        m_selectObj;
        Args<HINT_SELECTOBJLIST>    m_selectObjList;
    } args;
};

////////////////////////////////////////////////////////////////

enum PlacePos { placeBack = -1, placeDefault = 0, placeTop = 1 };

////////////////////////////////////////////////////////////////

class CGamDoc : public CDocument
{
protected: // create from serialization only
    CGamDoc();
    DECLARE_DYNCREATE(CGamDoc)

// Class vars and methods (used during deserialize)...
    // Version of file being loaded
    static int c_fileVersion;

public:
    static CFontTbl* GetFontManager()
        { return CGameBox::GetFontManager(); }
    static void SetLoadingVersion(int ver) { c_fileVersion = ver; }
    using SetLoadingVersionGuard = ::SetLoadingVersionGuard<CGamDoc>;
    static int GetLoadingVersion() { return c_fileVersion; }
    // The version of the loaded file is used to determine
    // if the game histories need to be upgraded and written
    // to the file. Normally the histories are simply copied
    // with the file.
    int m_nLoadedFileVersion;
    int GetLoadedVersion() { return m_nLoadedFileVersion; }

// Attributes
public:
    // This enumeration defines the state of the game. Not used
    // in scenario documents.
    enum GameState { stateNotRecording, stateRecording,
        stateMovePlay, stateHistPlay };

    GameState GetGameState() { return m_eState; }
    void SetGameState(GameState eState) { m_eState = eState; }
    BOOL IsRecording() { return m_eState == stateRecording; }
    BOOL IsAnyRecorded() { return m_eState == stateRecording && m_pRcdMoves != NULL; }
    BOOL IsPlaying() const
        { return m_eState == stateMovePlay || m_eState == stateHistPlay; }
    BOOL IsPlayingMoves() { return m_eState == stateMovePlay; }
    BOOL IsPlayingHistory() { return m_eState == stateHistPlay; }
    BOOL IsPlayingLastHistory();

    BOOL IsQuietPlayback() { return m_bQuietPlayback; }
    void SetQuietPlayback(BOOL bBeQuite) { m_bQuietPlayback = bBeQuite; }

    BOOL IsScenario() const { return m_bScenario; }
    BOOL IsShowingObjectTips() const { return m_bShowObjTipText; }

    BOOL IsRecordingCompoundMove();

    size_t GetCurrentHistoryRecNum() const { return m_nCurHist; }

    // Called by view being constructed...
    void* GetNewViewParameter() { return m_pvParam; }

    // Major game related objects...
    const CTileManager* GetTileManager() const;
    CTileManager* GetTileManager() { return const_cast<CTileManager*>(std::as_const(*this).GetTileManager()); }
    CMarkManager* GetMarkManager();
    const CBoardManager* GetBoardManager() const;
    CBoardManager* GetBoardManager()
    {
        return const_cast<CBoardManager*>(std::as_const(*this).GetBoardManager());
    }
    const CPieceManager* GetPieceManager() const;
    const CPieceTable* GetPieceTable() const { return m_pPTbl; }
    CPieceTable* GetPieceTable() { return const_cast<CPieceTable*>(std::as_const(*this).GetPieceTable()); }
    const CTrayManager* GetTrayManager() const { return m_pYMgr; }
    CTrayManager* GetTrayManager() { return const_cast<CTrayManager*>(std::as_const(*this).GetTrayManager()); }
    const CPBoardManager* GetPBoardManager() const { return m_pPBMgr; }
    CPBoardManager* GetPBoardManager() { return const_cast<CPBoardManager*>(std::as_const(*this).GetPBoardManager()); }
    CPlayerManager* GetPlayerManager() { return m_pPlayerMgr; }
    CMoveList* GetRecordMoveList() { return m_pRcdMoves.get(); }
    CHistoryTable* GetHistoryTable() { return m_pHistTbl; }
    const CGameElementStringMap& GetGameStringMap() const { return m_mapStrings; }
    CGameElementStringMap& GetGameStringMap() { return const_cast<CGameElementStringMap&>(std::as_const(*this).GetGameStringMap()); }

    // Fetch map of temporary tile rotations.
    CTileFacingMap* GetFacingMap();

    uint32_t GetRandomNumberSeed() { return m_nSeedCarryOver; }
    void SetRandomNumberSeed(uint32_t nSeedCarryOver) { m_nSeedCarryOver = nSeedCarryOver; }

// Operations
public:
    BOOL CreateNewFrame(CDocTemplate* pTemplate, LPCSTR pszTitle,
        LPVOID lpvCreateParam);
    CGamProjView& FindProjectView();
    CView* FindPBoardView(const CPlayBoard& pPBoard);
    CView* MakeSurePBoardVisible(CPlayBoard& pPBoard);
    void GetDocumentFrameList(std::vector<CB::not_null<CFrameWnd*>>& tblFrames);

    BOOL IsWindowStateAvailable() { return m_pWinState != NULL; }
    void RestoreWindowState();
    void DiscardWindowState();

    // Support for player accounts
    BOOL  HasPlayers() const { return m_pPlayerMgr != NULL; }
    DWORD  GetCurrentPlayerMask() const;
    void  SetCurrentPlayerMask(WORD dwMask) { m_dwCurrentPlayer = dwMask; }
    BOOL  IsCurrentPlayerReferee();
    void  ClearAllOwnership();
    void  GetPieceOwnerName(PieceID pid, CString& strName);

    BOOL  IsCurrentPlayer(DWORD dwMask) { return (BOOL)(GetCurrentPlayerMask() & dwMask); }

    DWORD CalculateHashForCurrentPlayerMask();
    BOOL  VerifyCurrentPlayerMask();

    BOOL  IsOwnerTipsDisabled() { return m_bDisableOwnerTips; }

    // Support for strings associated with game elements (pieces, markers)
    CString     GetGameElementString(GameElement gelem) const;
    BOOL        HasGameElementString(GameElement gelem) const;
    void        SetGameElementString(GameElement gelem, LPCTSTR pszString);
    void        GetTipTextForObject(const CDrawObj& pDObj, CString &strTip, CString* pStrTitle = NULL);
    // Invalid_v<size_t> = top
    GameElement GetGameElementCodeForObject(const CDrawObj& pDObj, size_t nSide = Invalid_v<size_t>);
    GameElement GetVerifiedGameElementCodeForObject(const CDrawObj& pDObj, size_t nSide = Invalid_v<size_t>);
    void        DoEditPieceText(PieceID pid);
    void        DoEditObjectText(const CDrawObj& pDObj);

    void DoInitialUpdate();
    // Forced override of this (NOTE not virtual)
    void UpdateAllViews(CView* pSender, LPARAM lHint = 0L,
        CObject* pHint = NULL);
    void CloseTrayPalettes();

    // Generate unique Object Identifier number.
    ObjectID CreateObjectID(CDrawObj::CDrawObjType nObjType);

    // Recording support methods...
    void SaveRecordedMoves();
    void SaveHistoryMovesInFile(size_t nHistRec);
    void TransferPlaybackToHistoryTable(BOOL bTruncateAtCurrentMove = FALSE);
    void AddMovesToGameHistoryTable(OwnerPtr<CHistRecord> pHist);
    BOOL DiscardCurrentRecording(BOOL bPrompt = TRUE);
    void AssignNewMoveGroup();
    void CreateRecordListIfRequired();

    void RecordPieceMoveToBoard(CPlayBoard* pPBrd, PieceID pid, CPoint pnt,
        PlacePos ePos = placeDefault);
    void RecordPieceMoveToTray(const CTraySet& pYGrp, PieceID pid, size_t nPos);
    void RecordPieceSetSide(PieceID pid, CPieceTable::Flip flip, size_t side, bool forceHide);
    void RecordPieceSetFacing(PieceID pid, uint16_t nFacingDegCW);
    void RecordPieceSetOwnership(PieceID pid, DWORD dwOwnerMask);
    void RecordMarkerSetFacing(ObjectID dwObjID, MarkID mid, uint16_t nFacingDegCW);
    void RecordMarkMoveToBoard(CPlayBoard* pPBrd, ObjectID dwObjID,
        MarkID mid, CPoint pnt, PlacePos ePos = placeDefault);
    void RecordPlotList(CPlayBoard* pPBrd);
    void RecordGameState();
    void RecordMessage(CString strMsg);
    void RecordObjectDelete(ObjectID dwObjID);
    void RecordCompoundMoveBegin();
    void RecordCompoundMoveEnd();
    void RecordCompoundMoveDiscard();
    void RecordObjectSetText(GameElement elem, LPCTSTR pszObjText);
    void RecordObjectLockdown(GameElement elem, BOOL bLockState);
    void RecordEventMessage(CString strMsg, BoardID nBoard,
        int x, int y);
    void RecordEventMessage(CString strMsg, size_t nTray,
        PieceID pid);

    // Document data manipulations methods...
    void PlacePieceOnBoard(CPoint pnt, PieceID pid, CPlayBoard *pPBrd);
    void PlacePieceInTray(PieceID pid, CTraySet& pYGrp, size_t nPos = Invalid_v<size_t>);
    void PlacePieceListOnBoard(CPoint pnt, const std::vector<PieceID>& pTbl,
        int xStagger, int yStagger, CPlayBoard *pPBrd);
    size_t PlacePieceListInTray(const std::vector<PieceID>& pTbl, CTraySet& pYGrp, size_t nPos = Invalid_v<size_t>);
    size_t PlaceObjectTableInTray(const std::vector<CB::not_null<CDrawObj*>> & pTbl, CTraySet& pYGrp, size_t nPos = Invalid_v<size_t>);
    void PlaceObjectTableOnBoard(const std::vector<CB::not_null<CDrawObj*>>& pLst, CPoint pntUpLeft,
        CPlayBoard* pPBrd, PlacePos ePos = placeDefault);
    void PlaceObjectTableOnBoard(CPoint pnt, const std::vector<CB::not_null<CDrawObj*>>& pTbl,
        int xStagger, int yStagger, CPlayBoard *pPBrd);
    void PlaceObjectTableOnBoard(const std::vector<CB::not_null<CDrawObj*>>& pTbl, CPlayBoard* pPBrd);
    void PlaceObjectOnBoard(CPlayBoard *pPBrd, CDrawObj::OwnerPtr pObj,
        CSize sizeDelta, PlacePos ePos = placeDefault);

    void InvertPlayingPieceInTray(PieceID pid, CPieceTable::Flip flip, size_t side, bool bOkToNotifyTray, bool forceHide);
    void InvertPlayingPieceOnBoard(CPieceObj& pObj, const CPlayBoard& pPBrd, CPieceTable::Flip flip, size_t side = Invalid_v<size_t>);
    void InvertPlayingPieceTableOnBoard(const std::vector<CB::not_null<CDrawObj*>>& pLst, const CPlayBoard& pPBrd, CPieceTable::Flip flip);

    void ChangePlayingPieceFacingOnBoard(CPieceObj& pObj, CPlayBoard* pPBrd,
        uint16_t nFacingDegCW);
    void ChangePlayingPieceFacingTableOnBoard(const std::vector<CB::not_null<CDrawObj*>>& pLst,
        CPlayBoard* pPBrd, uint16_t nFacingDegCW);
    void ChangePlayingPieceFacingInTray(PieceID pid, uint16_t nFacingDegCW);
    void ChangeMarkerFacingOnBoard(CMarkObj& pObj, CPlayBoard* pPBrd,
        uint16_t nFacingDegCW);
    void SetPieceOwnership(PieceID pid, DWORD dwOwnerMask);
    void SetPieceOwnershipTable(const std::vector<PieceID>& pTblPieces, DWORD dwOwnerMask);

    CDrawObj& CreateMarkerObject(CPlayBoard* pPBrd, MarkID mid, CPoint pnt,
        ObjectID dwObjID = ObjectID());
    CDrawObj& CreateLineObject(CPlayBoard* pPBrd, CPoint ptBeg,
        CPoint ptEnd, UINT nLineWd, COLORREF crLine, ObjectID dwObjID = ObjectID());
    void ModifyLineObject(CPlayBoard* pPBrd, CPoint ptBeg, CPoint ptEnd,
        UINT nLineWd, COLORREF crLine, CLine* pObj);
    void ReorgObjsInDrawList(CPlayBoard *pPBrd, std::vector<CB::not_null<CDrawObj*>>& pList, BOOL bToFront);
    void DeleteObjectsInTable(const std::vector<CB::not_null<CDrawObj*>>& pList);
    void SetObjectText(GameElement elem, LPCTSTR pszObjText);
    void SetObjectLockdownTable(const std::vector<CB::not_null<CDrawObj*>>& pLst, BOOL bLockState);
    void SetObjectLockdown(CDrawObj& pDObj, BOOL bLockState);

    BOOL RemovePieceFromCurrentLocation(PieceID pid, BOOL bDeleteIfBoard,
        BOOL bTrayHintAllowed = TRUE);
    void RemoveObjectFromCurrentLocation(CDrawObj* pObj);
    void ExpungeUnusedPiecesFromBoards();

    void FindObjectTableUnionRect(const std::vector<CB::not_null<CDrawObj*>>& pLst, CRect& rct);

    // Object and piece locator methods...
    BOOL FindPieceCurrentLocation(PieceID pid, const CTraySet*& pTraySet,
        const CPlayBoard*& pPBoard, const CPieceObj*& ppObj) const;
    BOOL FindPieceCurrentLocation(PieceID pid, CTraySet*& pTraySet,
        CPlayBoard*& pPBoard, CPieceObj*& ppObj)
    {
        return std::as_const(*this).FindPieceCurrentLocation(pid,
                                                            const_cast<const CTraySet*&>(pTraySet),
                                                            const_cast<const CPlayBoard*&>(pPBoard),
                                                            const_cast<const CPieceObj*&>(ppObj));
    }
    const CPlayBoard* FindPieceOnBoard(PieceID pid, const CPieceObj*& ppObj) const;
    CPlayBoard* FindPieceOnBoard(PieceID pid, CPieceObj*& ppObj)
    {
        return const_cast<CPlayBoard*>(std::as_const(*this).FindPieceOnBoard(pid, const_cast<const CPieceObj*&>(ppObj)));
    }
    const CTraySet* FindPieceInTray(PieceID pid) const;
    CTraySet* FindPieceInTray(PieceID pid)
    {
        return const_cast<CTraySet*>(std::as_const(*this).FindPieceInTray(pid));
    }
    const CPlayBoard* FindObjectOnBoard(ObjectID dwObjID, const CDrawObj*& ppObj) const;
    CPlayBoard* FindObjectOnBoard(ObjectID dwObjID, CDrawObj*& ppObj)
    {
        return const_cast<CPlayBoard*>(std::as_const(*this).FindObjectOnBoard(dwObjID, const_cast<const CDrawObj*&>(ppObj)));
    }
    CPlayBoard* FindObjectOnBoard(CDrawObj* pObj);

    // Support for playback...
    void EnsureBoardVisible(CPlayBoard& pPBoard);
    void EnsureBoardLocationVisible(CPlayBoard& pPBoard, CPoint point);
    void EnsureTrayIndexVisible(const CTraySet& pYSet, int nPos);
    void SelectObjectOnBoard(CPlayBoard& pPBoard, CDrawObj* pObj);
    void SelectObjectListOnBoard(CPlayBoard& pPBoard, const std::vector<CB::not_null<CDrawObj*>>& pList);
    void SelectTrayItem(const CTraySet& pYSet, PieceID pid, UINT nResourceID);
    void SelectTrayItem(const CTraySet& pYSet, PieceID pid, LPCTSTR pszNotificationTip = NULL);
    void SelectMarkerPaletteItem(MarkID mid);
    void RestartMoves();
    void FinishHistoryPlayback();

    // Support for play indicators...
    void IndicateBoardToBoardPieceMove(CPlayBoard& pPBFrom, CPlayBoard& pPBTo,
        CPoint ptCtrFrom, CPoint ptCtrTo, CSize size);
    void IndicateBoardPlotLine(CPlayBoard& pPBrd, CPoint ptA, CPoint ptB);
    void IndicateBoardPiece(CPlayBoard& pPBrd, CPoint ptCtr, CSize size);
    void Invalidate(CPlayBoard& pPBrd, const CRect& rect);
    void FlushAllSelections();
    void FlushAllIndicators();
    void UpdateAllBoardIndicators(CPlayBoard& pPBrd);
    void IndicateTextTipOnBoard(const CPlayBoard& pPBoard, CPoint pointWorkspace, UINT nResID);
    void IndicateTextTipOnBoard(const CPlayBoard& pPBoard, CPoint pointWorkspace,LPCTSTR pszStr);

    // Support for event notification messages (displayed as tool tips)
    void EventShowBoardNotification(BoardID nBrdSerNum, CPoint pntTipLoc, CString strMsg);
    void EventShowTrayNotification(size_t nTrayNum, PieceID pid, CString strMsg);

    // Support for player messages
    void MsgSetMessageText(const CString& str);
    const CString& MsgGetMessageText() const;
    const CStringArray& MsgGetMessageHistory() const;

    void MsgDialogSend(const CString& str, BOOL bCloseDialog = TRUE);
    BOOL MsgSendDialogOpen(BOOL bShowDieRoller = FALSE);
    void MsgDialogSend(const CString& str);
    void MsgDialogCancel(BOOL bDiscardHistory = FALSE);
    void MsgDialogClose(const CString& str);
    void MsgDialogForceDefer();

    static CString MsgEncodeFromPieces(CString strReadOnly, CString strEditable);
    static void MsgSeperateIntoPieces(CString strMsg, CString& strReadOnly,
        CString& strEditable);

    static void MsgSeperateLegacyMsgIntoPieces(CString strMsg, CString& strHistory,
        CString& strReadOnly, CString& strEditable);
    static void MsgParseLegacyHistory(CString strLegacyMsg,
        CStringArray& astrMsgHist, CString& strCurMsg);

    void SetDieRollState(OwnerOrNullPtr<CRollState> pRState);
    const CRollState* GetDieRollState() const;

    // Doc I/O Support
    void OnFileClose() { CDocument::OnFileClose(); }    // Expose protected

    void SerializeScenario(CArchive& ar);
    void SerializeGame(CArchive& ar);
    void SerializeMoveSet(CArchive& ar, CHistRecord*& pHist);
    void SerializeScenarioOrGame(CArchive& ar);
    void SerializeCurrentGameData(CFile* pFile, long lOffset, BOOL bSaving);
    long SerializeMovesToFile(CFile* pFile, long lOffset, CMoveList* pLst);
    CMoveList* DeserializeMovesFromFile(CFile* pFile, long lOffset);
    void LoadGameBoxFileForSerialize();
    void LoadAndActivateMoveFile(LPCSTR pszPathName);
    BOOL LoadAndActivateHistory(size_t nHistRec);
    BOOL LoadVintageHistoryMoveLists(CFile& file);
    BOOL LoadVintageHistoryRecord(CFile& file, CHistRecord& pHist);

    // Other doc level doc changes...
    void DoScenarioProperties() { OnEditScenarioProperties(); }
    void DoSelectBoards() { OnEditSelectBoards(); }
    void DoCreateTray() { OnEditCreateTray(); }
    void DoLoadMoveFile() { OnFileLoadMoveFile(); }
    void DoBoardProperties(size_t nBrd);
    void DoBoardProperties(CPlayBoard& pPBoard);
    void DoAcceptPlayback();              // (exposed for project window access)

// Implementation - methods
public:
    void SetThisDocumentType();
    DWORD IssueScenarioID();
    BOOL OnNewScenario();
    BOOL OnNewGame();

    // OnIdle is called by the App object to inform
    // documents of idle condition. A flag indicates if
    // this is the active document.
    void OnIdle(BOOL bActive);

// Implementation - variables
public:
    // The state of the die roller. (NOT CURRENTLY SERIALIZED!!!!)
    OwnerOrNullPtr<CRollState> m_pRollState;

    uint32_t    m_nSeedCarryOver;   // Current rand() seed

    // File object for the currently open Game document.
    // (This object isn't used for scenarios.)
    CFile       m_file;

    BOOL        m_bScenario;    // TRUE if this doc is a scenario

    // These are used to match a Game or Scenario to a GameBox
    DWORD       m_dwMajorRevs;  // GameBox Major revision number
    DWORD       m_dwMinorRevs;  // GameBox Minor revisions number
    DWORD       m_dwGBoxID;     // Required Game box signature
    CString     m_strGBoxFile;  // Probable Game box filename
    // The scenario ID is used to relate games to move files. All players
    // MUST start play with the same scenario file!
    DWORD       m_dwScenarioID; // Scenario ID (ramdomly gened at new)
    CString     m_strScnAuthor; // Scenario author
    CString     m_strScnTitle;  // Scenario title
    CString     m_strScnDescr;  // Scenario description
    CPlayerManager* m_pPlayerMgr;// Pointer to player manager (if has player accounts)
    DWORD       m_dwCurrentPlayer;// Mask for game file's player
    DWORD       m_dwPlayerHash; // MS 32 bits of MD5 hash of m_wCurrentPlayer ..
                                // .. and player file description text. Used as ..
                                // .. a tamper check.
    CString     m_strPlayerFileDescr; // Description of who owns file and other stuff..
                                // .. that happened when it was created.
    BOOL        m_bTrayAVisible;// Tray pallete A visible
    BOOL        m_bTrayBVisible;// Tray pallete B visible
    BOOL        m_bMarkPalVisible;// Marker palette visible
    BOOL        m_bMsgWinVisible;// Message window visible
    BOOL        m_bShowObjTipText;// Show data tips for game objects
    BOOL        m_bDisableOwnerTips;// Disable owner tool tip pop up.(was m_wReserved1)
    BOOL        m_bSaveWindowPositions; // Save the state of windows
    BOOL        m_bKeepGamBackup;// Keep backup copy of game file.
    BOOL        m_bKeepMoveHist;// Keep a move history during a game
    BOOL        m_bVrfyGameState;// Verify game state against move set state
    BOOL        m_bVrfySaveState;// Save game state in move set file
    // DWORD     m_dwCurFileSlot;// Offset to byte past move history data
    // WORD      m_wReserved1;   // For future need (set to 0) // Now m_bDisableOwnerTips
    WORD        m_wReserved2;   // For future need (set to 0)
    WORD        m_wReserved3;   // For future need (set to 0)
    WORD        m_wReserved4;   // For future need (set to 0)

    GameState   m_eState;       // Current state of game.

    CString     m_strCurMsg;    // Current user message under construction
    CStringArray m_astrMsgHist; // Current message history

    size_t      m_nCurMove;     // Index of current move (Invalid_v<size_t> is at end)
    POSITION    m_posCurMove;   // Shadow of m_nCurMove. (NOSAVE)
    size_t      m_nFirstMove;   // Index of first move record (usually 0 or 1)
    size_t      m_nCurHist;     // History being viewed if state is "hist"
    BOOL        m_bStepToNextHist; // If TRUE, step to next history record
    BOOL        m_bKeepSkipInd; // If TRUE skipped move indications aren't erased
    BOOL        m_bAutoStep;    // If TRUE next move kicks off auto playback

    // Move file playback history record. This record is only non-NULL
    // when a move file has been loaded for playback.
    CHistRecord* m_pPlayHist;

    OwnerOrNullPtr<CMoveList> m_pRcdMoves;    // Moves being recorded or move file playback
    OwnerOrNullPtr<CMoveList> m_pHistMoves;   // Currently loaded history from hist table

    // m_pMoves is a shadow of either m_pRctMoves or m_pHistMoves depending on the
    // gamestate m_eState.  (NOSAVE)
    CMoveList*  m_pMoves;


    CGameElementStringMap m_mapStrings; // Mapping of pieces and markers to strings.

    size_t          m_nMoveIdxAtBookMark;// Move list index at bookmark
    CGameState*     m_pBookMark;// The state of things at the bookmark

    CHistoryTable*  m_pHistTbl; // Table containing history of game

    CPBoardManager* m_pPBMgr;   // List of play boards in use.
    CTrayManager*   m_pYMgr;    // Tray manager
    CPieceTable*    m_pPTbl;    // The playing piece table.

    using CSendMsgDlgPtr = CB::propagate_const<std::unique_ptr<CSendMsgDialog, CModelessDialogCleaner>>;
    CSendMsgDlgPtr  m_pMsgDialog; // Pointer to message modeless dialog
    CPoint          m_pntMsgReadPos; // Position of read message dialog

    CGameBox*       m_pGbx;     // Holds the contents of gamebox

    CTileFacingMap* m_pTileFacingMap; // Map of temp tile rotations (NOT SERIALIZED)

// Some document related windows...
public:
    // Tray Palettes...
    CTrayPalette    m_palTrayA;
    CTrayPalette    m_palTrayB;
    // Marker Palette...
    CMarkerPalette  m_palMark;

// Implementation
protected:
    void*   m_pvParam;          // View init backdoor parameter pointer

    // This is only valid during saving and loading of files.
    CGpWinStateMgr* m_pWinState;

    // Misc internal...
    BOOL    m_bIsRecording;     // Moves are being recorded
    int     m_nMoveInterlock;   // Nonzero if move underway
    BOOL    m_bAutoPlayback;    // Autoplayback is active
    BOOL    m_bQuietPlayback;   // True if no visual feedback is to be shown
    WORD    m_wDocRand;         // Used to generate ObjectID's
    CString m_strTmpPathName;   // Used to pass filename to serialize

    BOOL    m_bSimulateSpectator;// If set, show everything as if spectator game

// Implementation - overrides
public:
    virtual ~CGamDoc();

    virtual void OnCloseDocument();
    // Serialization support...
    virtual void Serialize(CArchive& ar);   // Overridden for document I/O

protected:
    virtual BOOL OnNewDocument();
    virtual BOOL OnSaveDocument(const char* pszPathName);
    virtual BOOL OnOpenDocument(const char* pszPathName);
    virtual void DeleteContents();

    BOOL DoSaveGameFile(LPCTSTR pszFileName);
    BOOL CheckIfPlayerFilesExist(LPCTSTR pszBaseName, LPCTSTR pszExt,
        BOOL bCheckReferee, CString& strExist);

// Generated message map functions
protected:
    //{{AFX_MSG(CGamDoc)
    afx_msg void OnUpdateViewTrayA(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewTrayB(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewMarkPalette(CCmdUI* pCmdUI);
    afx_msg void OnViewTrayA();
    afx_msg void OnViewTrayB();
    afx_msg void OnUpdateIndicatorCompoundMove(CCmdUI* pCmdUI);
    afx_msg void OnViewMarkPalette();
    afx_msg void OnEditSetBookMark();
    afx_msg void OnUpdateEditSetBookMark(CCmdUI* pCmdUI);
    afx_msg void OnEditRestoreBookMark();
    afx_msg void OnUpdateEditRestoreBookMark(CCmdUI* pCmdUI);
    afx_msg void OnEditClearBookMark();
    afx_msg void OnPbckDiscard();
    afx_msg void OnUpdatePbckDiscard(CCmdUI* pCmdUI);
    afx_msg void OnPbckEnd();
    afx_msg void OnUpdatePbckEnd(CCmdUI* pCmdUI);
    afx_msg void OnPbckFinish();
    afx_msg void OnUpdatePbckFinish(CCmdUI* pCmdUI);
    afx_msg void OnPbckNext();
    afx_msg void OnUpdatePbckNext(CCmdUI* pCmdUI);
    afx_msg void OnPbckStart();
    afx_msg void OnUpdatePbckStart(CCmdUI* pCmdUI);
    afx_msg void OnActDoMessage();
    afx_msg void OnUpdateEditClearBookMark(CCmdUI* pCmdUI);
    afx_msg void OnUpdateActDoMessage(CCmdUI* pCmdUI);
    afx_msg void OnPbckReadMessage();
    afx_msg void OnUpdatePbckReadMessage(CCmdUI* pCmdUI);
    afx_msg void OnPbckNextHistory();
    afx_msg void OnUpdatePbckNextHistory(CCmdUI* pCmdUI);
    afx_msg void OnPbckCloseHistory();
    afx_msg void OnUpdatePbckCloseHistory(CCmdUI* pCmdUI);
    afx_msg void OnFileSendRecording2File();
    afx_msg void OnUpdateFileSendRecording2File(CCmdUI* pCmdUI);
    afx_msg void OnFileDiscardRecordedMoves();
    afx_msg void OnUpdateFileDiscardRecordedMoves(CCmdUI* pCmdUI);
    afx_msg void OnEditCreateTray();
    afx_msg void OnUpdateEditCreateTray(CCmdUI* pCmdUI);
    afx_msg void OnEditScenarioProperties();
    afx_msg void OnUpdateEditScenarioProperties(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectBoards();
    afx_msg void OnUpdateEditSelectBoards(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectGamePieces();
    afx_msg void OnUpdateEditSelectGamePieces(CCmdUI* pCmdUI);
    afx_msg void OnFileLoadMoveFile();
    afx_msg void OnUpdateFileLoadMoveFile(CCmdUI* pCmdUI);
    afx_msg void OnActCompoundMoveBegin();
    afx_msg void OnUpdateActCompoundMoveBegin(CCmdUI* pCmdUI);
    afx_msg void OnActCompoundMoveEnd();
    afx_msg void OnUpdateActCompoundMoveEnd(CCmdUI* pCmdUI);
    afx_msg void OnActCompoundMoveDiscard();
    afx_msg void OnUpdateActCompoundMoveDiscard(CCmdUI* pCmdUI);
    afx_msg void OnPbckPrevious();
    afx_msg void OnUpdatePbckPrevious(CCmdUI* pCmdUI);
    afx_msg void OnFileSaveGameAsScenario();
    afx_msg void OnUpdateFileSaveGameAsScenario(CCmdUI* pCmdUI);
    afx_msg void OnActRollDice();
    afx_msg void OnUpdateActRollDice(CCmdUI* pCmdUI);
    afx_msg void OnPbckStepCompoundMoves();
    afx_msg void OnUpdateStepCompoundMoves(CCmdUI* pCmdUI);
    afx_msg void OnEditImportPieceGroups();
    afx_msg void OnUpdateEditImportPieceGroups(CCmdUI* pCmdUI);
    afx_msg void OnViewShowTipText();
    afx_msg void OnUpdateViewShowTipText(CCmdUI* pCmdUI);
    afx_msg void OnViewSaveWinState();
    afx_msg void OnUpdateViewSaveWinState(CCmdUI* pCmdUI);
    afx_msg void OnEditCreatePlayers();
    afx_msg void OnUpdateEditCreatePlayers(CCmdUI* pCmdUI);
    afx_msg void OnEditEditPlayers();
    afx_msg void OnUpdateEditEditPlayers(CCmdUI* pCmdUI);
    afx_msg void OnActSimulateSpectator();
    afx_msg void OnUpdateActSimulateSpectator(CCmdUI* pCmdUI);
    afx_msg void OnEditCreateGeomorphic();
    afx_msg void OnUpdateEditCreateGeomorphic(CCmdUI* pCmdUI);
    afx_msg void OnFileCreateReferee();
    afx_msg void OnUpdateFileCreateReferee(CCmdUI* pCmdUI);
    afx_msg void OnFileChangeGameOwner();
    afx_msg void OnUpdateFileChangeGameOwner(CCmdUI* pCmdUI);
    afx_msg void OnPbckStepToNextHist();
    afx_msg void OnUpdatePbckStepToNextHist(CCmdUI* pCmdUI);
    afx_msg void OnPbckSkipKeepIndicators();
    afx_msg void OnUpdatePbckSkipKeepIndicators(CCmdUI* pCmdUI);
    afx_msg void OnPbckAutoStep();
    afx_msg void OnUpdatePbckAutoStep(CCmdUI* pCmdUI);
    afx_msg void OnViewShowTipOwner();
    afx_msg void OnUpdateViewShowTipOwner(CCmdUI* pCmdUI);
    //}}AFX_MSG
#ifdef _DEBUG
    afx_msg void OnDebugMoveList();
    afx_msg void OnDebugPieceTable();
#endif
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif  // _GAMDOC_H


