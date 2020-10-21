// MoveMgr.h
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

#ifndef _MOVEMGR_H
#define _MOVEMGR_H

///////////////////////////////////////////////////////////////////////

class CMoveRecord
{
public:
    // Record types...
    enum RcdType { mrecUnknown, mrecState, mrecPMove, mrecTMove,
        mrecPSide, mrecMMove, mrecMPlot, mrecMsg, mrecDelObj,
        mrecPFacing,
        mrecCompoundMove,   // For moves spanning multiple sequence groupings
        mrecMFacing,        // Ver2.0
        mrecSetObjText,     // Ver2.0
        mrecLockObj,        // Ver2.0
        mrecEvtMsg,         // Ver2.0
        mrecPOwner,         // Ver2.0
        mrecMax };

public:
    CMoveRecord() { m_nSeqNum = -1; m_eType = mrecUnknown; }
    virtual ~CMoveRecord() {}

public:
    RcdType GetType() { return m_eType; }
    // -------- //
    void SetSeqNum(int nSeq) { m_nSeqNum = nSeq; }
    int  GetSeqNum() { return m_nSeqNum; }

public:
    virtual BOOL IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup) { return FALSE; }
    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup) {}
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup) {}
    virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup) {}

    virtual BOOL ValidatePieces(CGamDoc* pDoc) { return TRUE; }

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file) = 0;
#endif
protected:
    int     m_nSeqNum;              // Used to group move records
    RcdType m_eType;                // Type of record
};

///////////////////////////////////////////////////////////////////////

enum PlacePos;

class CBoardPieceMove : public CMoveRecord
{
public:
    CBoardPieceMove() { m_eType = mrecPMove; }
    CBoardPieceMove(int nBrdSerNum, PieceID pid, CPoint pnt, PlacePos ePos);

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual BOOL ValidatePieces(CGamDoc* pDoc);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    CPoint      m_ptCtr;
    PieceID     m_pid;
    int         m_nBrdNum;
    PlacePos    m_ePos;
};

///////////////////////////////////////////////////////////////////////

class CTrayPieceMove : public CMoveRecord
{
public:
    CTrayPieceMove() { m_eType = mrecTMove; }
    CTrayPieceMove(int nTrayNum, PieceID pid, int nPos);

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual BOOL ValidatePieces(CGamDoc* pDoc);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    PieceID     m_pid;
    int         m_nTrayNum;
    int         m_nPos;
};

///////////////////////////////////////////////////////////////////////

class CPieceSetSide : public CMoveRecord
{
public:
    CPieceSetSide() { m_eType = mrecPSide; }
    CPieceSetSide(PieceID pid, BOOL bTopUp)
        { m_eType = mrecPSide; m_pid = pid; m_bTopUp = bTopUp; }

    virtual BOOL IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual BOOL ValidatePieces(CGamDoc* pDoc);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    PieceID     m_pid;
    BOOL        m_bTopUp;
};

///////////////////////////////////////////////////////////////////////

class CPieceSetFacing : public CMoveRecord
{
public:
    CPieceSetFacing() { m_eType = mrecPFacing; }
    CPieceSetFacing(PieceID pid, int nFacingDegCW)
        { m_eType = mrecPFacing; m_pid = pid; m_nFacingDegCW = nFacingDegCW; }

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);
//  virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual BOOL ValidatePieces(CGamDoc* pDoc);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    PieceID     m_pid;
    int         m_nFacingDegCW;
};

///////////////////////////////////////////////////////////////////////

class CPieceSetOwnership : public CMoveRecord
{
public:
    CPieceSetOwnership() { m_eType = mrecPOwner; }
    CPieceSetOwnership(PieceID pid, DWORD dwOwnerMask)
        { m_eType = mrecPOwner; m_pid = pid; m_dwOwnerMask = dwOwnerMask; }

    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual BOOL ValidatePieces(CGamDoc* pDoc);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    PieceID     m_pid;
    DWORD       m_dwOwnerMask;
};

///////////////////////////////////////////////////////////////////////

class CMarkerSetFacing : public CMoveRecord
{
public:
    CMarkerSetFacing() { m_eType = mrecMFacing; }
    CMarkerSetFacing(ObjectID dwObjID, MarkID mid, int nFacingDegCW);

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);
//  virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    ObjectID    m_dwObjID;
    MarkID      m_mid;
    int         m_nFacingDegCW;
};

///////////////////////////////////////////////////////////////////////

class CBoardMarkerMove : public CMoveRecord
{
public:
    CBoardMarkerMove() { m_eType = mrecMMove; }
    CBoardMarkerMove(int nBrdSerNum, ObjectID dwObjID, MarkID mid,
        CPoint pnt, PlacePos ePos);

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    CPoint      m_ptCtr;
    ObjectID    m_dwObjID;
    MarkID      m_mid;
    int         m_nBrdNum;
    PlacePos    m_ePos;
};

///////////////////////////////////////////////////////////////////////

class CObjectDelete : public CMoveRecord
{
public:
    CObjectDelete() { m_eType = mrecDelObj; }
    CObjectDelete(ObjectID dwObjID);

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);
//  virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    ObjectID    m_dwObjID;
};

///////////////////////////////////////////////////////////////////////

class CObjectSetText : public CMoveRecord
{
public:
    CObjectSetText() { m_eType = mrecSetObjText; }
    CObjectSetText(GameElement elem, LPCTSTR pszText);

    virtual BOOL IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup);
//  virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);
//  virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    DWORD       m_elem;
    CString     m_strObjText;
};

///////////////////////////////////////////////////////////////////////

class CObjectLockdown : public CMoveRecord
{
public:
    CObjectLockdown() { m_eType = mrecLockObj; }
    CObjectLockdown(GameElement elem, BOOL bLockState);

    virtual BOOL IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    DWORD       m_elem;
    BOOL        m_bLockState;
};

///////////////////////////////////////////////////////////////////////

class CGameState;

class CGameStateRcd : public CMoveRecord
{
public:
    CGameStateRcd() { m_eType = mrecState; m_pState = NULL; }
    CGameStateRcd(CGameState* pState)
        { m_eType = mrecState; m_pState = pState; }
    ~CGameStateRcd();
    // -------- //
    CGameState* GetGameState() { return m_pState; }

    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    CGameState*     m_pState;
};

///////////////////////////////////////////////////////////////////////

class CDrawList;

class CMovePlotList : public CMoveRecord
{
public:
    CMovePlotList() { m_eType = mrecMPlot; }
    CMovePlotList(int nBrdSerNum, CDrawList* pDwg)
        { m_eType = mrecMPlot; m_nBrdNum = nBrdSerNum; SavePlotList(pDwg); }

    void SavePlotList(CDrawList* pDwg);

    virtual void DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup);
    virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    int         m_nBrdNum;
    CDWordArray m_tblPlot;
};

///////////////////////////////////////////////////////////////////////

class CMessageRcd : public CMoveRecord
{
public:
    CMessageRcd() { m_eType = mrecMsg; }
    CMessageRcd(CString strMsg)
        { m_eType = mrecMsg; m_strMsg = strMsg; }

    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    CString m_strMsg;
};

///////////////////////////////////////////////////////////////////////

class CEventMessageRcd : public CMoveRecord
{
public:
    CEventMessageRcd() { m_eType = mrecEvtMsg; }
    CEventMessageRcd(CString strMsg, BOOL bIsBoardEvent,
        int nID, int nVal1, int nVal2 = 0);

    virtual void DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup);

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    BOOL    m_bIsBoardEvent;        // If TRUE, message relates to board
    int     m_nID;                  // Serial num of board or tray number
    int     m_nVal1;                // X coord if board, tray item index if tray
    int     m_nVal2;                // Y coord if board, 0 if tray
    CString m_strMsg;               // The message text
};

///////////////////////////////////////////////////////////////////////

class CCompoundMove : public CMoveRecord
{
public:
    CCompoundMove() { m_eType = mrecCompoundMove; m_bGroupBegin = TRUE; }
    CCompoundMove(BOOL bGroupBegin)
        { m_eType = mrecCompoundMove; m_bGroupBegin = bGroupBegin; }

    BOOL IsGroupBegin() { return m_bGroupBegin; }

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DoMove(CGamDoc* pDoc, int nMoveWithinGroup) { ASSERT(FALSE); }
    virtual void DumpToTextFile(CFile& file);
#endif
protected:
    BOOL    m_bGroupBegin;
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

class CMoveList : public CPtrList
{
public:
    CMoveList();
    ~CMoveList();

// Attributes
public:

// Operations
public:
    static CMoveList* CloneMoveList(CGamDoc* pDoc, CMoveList* pMoveList);

    void AssignNewMoveGroup() { m_nSeqNum++; }
    POSITION AppendMoveRecord(CMoveRecord* pRec);
    POSITION PrependMoveRecord(CMoveRecord* pRec, BOOL bSetSeqNum = TRUE);

    void PushAndSetState(CGamDoc* pDoc, int nIndex);
    void PopAndRestoreState(CGamDoc* pDoc);
    int  SetStartingState();

    int FindPreviousMove(CGamDoc* pDoc, int nIndex);

    // Compound moves are a sequence of moves done one at a time.
    // For example: move piece, change facing, move again, change facing,
    // move again and flip over. Each move is played back completely one after
    // the other.
    void BeginRecordingCompoundMove(CGamDoc* pDoc);
    void CancelRecordingCompoundMove(CGamDoc* pDoc);
    void EndRecordingCompoundMove();
    BOOL IsRecordingCompoundMove() { return m_bCompoundMove ; }

    BOOL IsSingleSteppingCompoundMoves() { return m_bCompoundSingleStep; }
    BOOL IsThisMovePossible(int nIndex);
    void SetCompoundSingleStep(BOOL bSingleStep = TRUE)
        { m_bCompoundSingleStep = bSingleStep; }
    BOOL IsWithinCompoundMove(int nIndex);
    BOOL IsMoveHidden(CGamDoc* pDoc, int nIndex);
    BOOL IsDoMoveActive() { return m_nPlaybackLock != 0; }

    void PurgeAfter(int nIndex);

    BOOL ValidatePieces(CGamDoc* pDoc);

    int  DoMove(CGamDoc* pDoc, int nIndex, BOOL bAutoStepHiddenMove = TRUE);
    void IncrementSkipCount(BOOL bKeepInd) { m_nSkipCount++; m_bSkipKeepInd = bKeepInd; }

    CMoveRecord* GetAt(POSITION pos)
        { return (CMoveRecord*)CPtrList::GetAt(pos); }
    CMoveRecord* GetNext(POSITION& pos)
        { return (CMoveRecord*)CPtrList::GetNext(pos); }
    CMoveRecord* GetPrev(POSITION& pos)
        { return (CMoveRecord*)CPtrList::GetPrev(pos); }
    CMoveRecord* GetFirstRecord()
        { return (CMoveRecord*)CPtrList::GetHead(); }

    void Clear();
    void Serialize(CArchive& ar, BOOL bSaveUndo = TRUE);

#ifdef _DEBUG
    void DumpToTextFile(CFile& file);
#endif
// Implementation
protected:
    int  DoMoveGroup(CGamDoc* pDoc, POSITION posFirst);

protected:
    int         m_nPlaybackLock;        // Lock count used to stop recursion
    int         m_nSkipCount;           // Number of moves to skip
    BOOL        m_bSkipKeepInd;         // Keep each skipped move's indicators
    BOOL        m_bCompoundSingleStep;  // If TRUE, single step compound moves

    CGameState* m_pStateSave;           // Used to push/pos state
    BOOL        m_bQuietPlaybackSave;

    // Serialize the following...
    int         m_nSeqNum;

    BOOL        m_bCompoundMove;        // Recording a compound move.
    CGameState* m_pCompoundBaseBookMark;// Game state at start of compound move
    int         m_nCompoundBaseIndex;   // Index of current compound move.
};

#endif

