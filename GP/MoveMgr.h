// MoveMgr.h
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

#include <set>
#include "GamState.h"

class CPlayBoard;

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
    CMoveRecord() = default;
    virtual ~CMoveRecord() = default;

public:
    RcdType GetType() const { return m_eType; }
    // -------- //
    void   SetSeqNum(size_t nSeq) { m_nSeqNum = nSeq; }
    size_t GetSeqNum() const { return m_nSeqNum; }

public:
    virtual BOOL IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const /* override */
    {
        return m_hiddenByPrivate;
    }
    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const /* override */ {}
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const /* override */ {}
    virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const /* override */ {}

    virtual BOOL ValidatePieces(const CGamDoc& pDoc) const /* override */ { return TRUE; }

    virtual void Serialize(CArchive& ar);
    class BoardCmp
    {
    public:
        bool operator()(BoardID left, BoardID right) const
        {
            return static_cast<BoardID::UNDERLYING_TYPE>(left) < static_cast<BoardID::UNDERLYING_TYPE>(right);
        }
    };
    using BoardSet = std::set<BoardID, BoardCmp>;
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) /* override */;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const /* override */ = 0;
#endif
protected:
    static bool IsPrivateBoard(CGamDoc& pDoc, BoardID bid);
    static bool IsPrivateBoard(CGamDoc& pDoc, const CPlayBoard& pbrd);

    size_t  m_nSeqNum = Invalid_v<size_t>;   // Used to group move records
    RcdType m_eType = mrecUnknown;              // Type of record
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    bool m_hiddenByPrivate = false;
};

///////////////////////////////////////////////////////////////////////

enum PlacePos : int;

class CBoardPieceMove : public CMoveRecord
{
public:
    CBoardPieceMove() { m_eType = mrecPMove; }
    CBoardPieceMove(BoardID nBrdSerNum, PieceID pid, CPoint pnt, PlacePos ePos);

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual BOOL ValidatePieces(const CGamDoc& pDoc) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    CPoint      m_ptCtr;
    PieceID     m_pid;
    BoardID     m_nBrdNum;
    PlacePos    m_ePos;
};

///////////////////////////////////////////////////////////////////////

class CTrayPieceMove : public CMoveRecord
{
public:
    CTrayPieceMove() { m_eType = mrecTMove; }
    CTrayPieceMove(size_t nTrayNum, PieceID pid, size_t nPos);

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual BOOL ValidatePieces(const CGamDoc& pDoc) const override;

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    PieceID     m_pid;
    size_t      m_nTrayNum;
    size_t      m_nPos;
};

///////////////////////////////////////////////////////////////////////

class CPieceSetSide : public CMoveRecord
{
public:
    CPieceSetSide() : m_forceMoveHidden(false) { m_eType = mrecPSide; }
    CPieceSetSide(PieceID pid, CPieceTable::Flip flip, size_t side, bool forceHide) :
        m_forceMoveHidden(forceHide)
    {
        m_eType = mrecPSide;
        m_pid = pid;
        m_flip = flip;
        m_side = side;
    }

    virtual BOOL IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const override;
    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual BOOL ValidatePieces(const CGamDoc& pDoc) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    PieceID     m_pid;
    CPieceTable::Flip m_flip;
    size_t      m_side;
    // for hiding moves due to CTraySet::IsRandomSidePull()
    bool m_forceMoveHidden;
};

///////////////////////////////////////////////////////////////////////

class CPieceSetFacing : public CMoveRecord
{
public:
    CPieceSetFacing() { m_eType = mrecPFacing; }
    CPieceSetFacing(PieceID pid, uint16_t nFacingDegCW)
        { m_eType = mrecPFacing; m_pid = pid; m_nFacingDegCW = nFacingDegCW; }

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;
//  virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual BOOL ValidatePieces(const CGamDoc& pDoc) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    PieceID     m_pid;
    uint16_t    m_nFacingDegCW;
};

///////////////////////////////////////////////////////////////////////

class CPieceSetOwnership : public CMoveRecord
{
public:
    CPieceSetOwnership() { m_eType = mrecPOwner; }
    CPieceSetOwnership(PieceID pid, DWORD dwOwnerMask)
        { m_eType = mrecPOwner; m_pid = pid; m_dwOwnerMask = dwOwnerMask; }

    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual BOOL ValidatePieces(const CGamDoc& pDoc) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
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
    CMarkerSetFacing(ObjectID dwObjID, MarkID mid, uint16_t nFacingDegCW);

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;
//  virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    ObjectID    m_dwObjID;
    MarkID      m_mid;
    uint16_t    m_nFacingDegCW;
};

///////////////////////////////////////////////////////////////////////

class CBoardMarkerMove : public CMoveRecord
{
public:
    CBoardMarkerMove() { m_eType = mrecMMove; }
    CBoardMarkerMove(BoardID nBrdSerNum, ObjectID dwObjID, MarkID mid,
        CPoint pnt, PlacePos ePos);

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    CPoint      m_ptCtr;
    ObjectID    m_dwObjID;
    MarkID      m_mid;
    BoardID     m_nBrdNum;
    PlacePos    m_ePos;
};

///////////////////////////////////////////////////////////////////////

class CObjectDelete : public CMoveRecord
{
public:
    CObjectDelete() { m_eType = mrecDelObj; }
    CObjectDelete(ObjectID dwObjID);

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;
//  virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    ObjectID    m_dwObjID;
};

///////////////////////////////////////////////////////////////////////

class CObjectSetText : public CMoveRecord
{
public:
    CObjectSetText() { m_eType = mrecSetObjText; }
    CObjectSetText(GameElement elem, const CB::string* pszText);

    virtual BOOL IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const override;
//  virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;
//  virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    GameElement m_elem;
    CB::string  m_strObjText;
};

///////////////////////////////////////////////////////////////////////

class CObjectLockdown : public CMoveRecord
{
public:
    CObjectLockdown() { m_eType = mrecLockObj; }
    CObjectLockdown(GameElement elem, BOOL bLockState);

    virtual BOOL IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const override;
    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    GameElement m_elem;
    BOOL        m_bLockState;
};

///////////////////////////////////////////////////////////////////////

class CGameState;

class CGameStateRcd : public CMoveRecord
{
public:
    CGameStateRcd() { m_eType = mrecState; }
    CGameStateRcd(CGameState* pState) :
        m_pState(pState)
    { m_eType = mrecState; }
    ~CGameStateRcd() = default;
    // -------- //
    // m_pState should only be null in CMoveList::Serialize
    const CGameState& GetGameState() const { return *m_pState; }
    CGameState& GetGameState()
    {
        return const_cast<CGameState&>(std::as_const(*this).GetGameState());
    }

    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    OwnerOrNullPtr<CGameState> m_pState;
};

///////////////////////////////////////////////////////////////////////

class CDrawList;

class CMovePlotList : public CMoveRecord
{
public:
    CMovePlotList() { m_eType = mrecMPlot; }
    CMovePlotList(BoardID nBrdSerNum, const CDrawList& pDwg)
        { m_eType = mrecMPlot; m_nBrdNum = nBrdSerNum; SavePlotList(pDwg); }

    void SavePlotList(const CDrawList& pDwg);

    virtual void DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const override;
    virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    BoardID     m_nBrdNum;
    CDWordArray m_tblPlot;
};

///////////////////////////////////////////////////////////////////////

class CMessageRcd : public CMoveRecord
{
public:
    CMessageRcd() { m_eType = mrecMsg; }
    CMessageRcd(CB::string strMsg)
        { m_eType = mrecMsg; m_strMsg = std::move(strMsg); }

    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    CB::string m_strMsg;
};

///////////////////////////////////////////////////////////////////////

class CEventMessageRcd : public CMoveRecord
{
public:
    CEventMessageRcd() { m_eType = mrecEvtMsg; }
    CEventMessageRcd(CB::string strMsg,
        BoardID nBoard, int x, int y);
    CEventMessageRcd(CB::string strMsg,
        size_t nTray, PieceID pid);

    virtual void DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const override;

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    BOOL    m_bIsBoardEvent;        // If TRUE, message relates to board
    union
    {
        struct
        {
            BoardID m_nBoard;
            int     m_x;
            int     m_y;
        };
        struct
        {
            size_t  m_nTray;
            PieceID m_pieceID;
        };
    };
    CB::string m_strMsg;            // The message text
};

///////////////////////////////////////////////////////////////////////

class CCompoundMove : public CMoveRecord
{
public:
    CCompoundMove() { m_eType = mrecCompoundMove; m_bGroupBegin = TRUE; }
    CCompoundMove(BOOL bGroupBegin)
        { m_eType = mrecCompoundMove; m_bGroupBegin = bGroupBegin; }

    BOOL IsGroupBegin() const { return m_bGroupBegin; }

    virtual void Serialize(CArchive& ar);
    /* provide visual feedback for only the first move in a
        block of moves involving the same hidden board(s) */
    virtual void SerializeHiddenByPrivate(CGamDoc& doc,
                                        BoardSet& usedPrivateBoards) override;

#ifdef _DEBUG
    virtual void DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const override { ASSERT(FALSE); }
    virtual void DumpToTextFile(const CGamDoc& pDoc, CFile& file) const override;
#endif
protected:
    BOOL    m_bGroupBegin;
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// CMoveRecord is polymorphic
class CMoveList : private std::list<OwnerPtr<CMoveRecord>>
{
    typedef std::list<OwnerPtr<CMoveRecord>> BASE;
public:
    using BASE::iterator;
    using BASE::begin;
    using BASE::end;
    using BASE::erase;
    using BASE::front;
    using BASE::insert;
    using BASE::pop_front;

    CMoveList();
    CMoveList(const CMoveList&) = delete;
    CMoveList& operator=(const CMoveList&) = delete;
    ~CMoveList();

// Attributes
public:
    size_t GetCount() const { return size(); }

// Operations
public:
    static OwnerPtr<CMoveList> CloneMoveList(CGamDoc& pDoc, const CMoveList& pMoveList);

    void AssignNewMoveGroup() { m_nSeqNum++; }
    iterator AppendMoveRecord(OwnerPtr<CMoveRecord> pRec);
    iterator PrependMoveRecord(OwnerPtr<CMoveRecord> pRec, BOOL bSetSeqNum = TRUE);

    void PushAndSetState(CGamDoc& pDoc, size_t nIndex);
    void PopAndRestoreState(CGamDoc& pDoc);
    size_t SetStartingState(CGamDoc& doc);

    size_t FindPreviousMove(CGamDoc& pDoc, size_t nIndex);

    // Compound moves are a sequence of moves done one at a time.
    // For example: move piece, change facing, move again, change facing,
    // move again and flip over. Each move is played back completely one after
    // the other.
    void BeginRecordingCompoundMove(CGamDoc& pDoc);
    void CancelRecordingCompoundMove(CGamDoc& pDoc);
    void EndRecordingCompoundMove();
    BOOL IsRecordingCompoundMove() const { return m_bCompoundMove ; }

    BOOL IsSingleSteppingCompoundMoves() const { return m_bCompoundSingleStep; }
    bool IsThisMovePossible(size_t nIndex) const;
    void SetCompoundSingleStep(BOOL bSingleStep = TRUE)
        { m_bCompoundSingleStep = bSingleStep; }
    bool IsWithinCompoundMove(size_t nIndex) const;
    bool IsMoveHidden(const CGamDoc& pDoc, size_t nIndex) const;
    BOOL IsDoMoveActive() const { return m_nPlaybackLock != 0; }

    void PurgeAfter(size_t nIndex);

    BOOL ValidatePieces(const CGamDoc& pDoc) const;

    size_t DoMove(CGamDoc& pDoc, size_t nIndex, BOOL bAutoStepHiddenMove = TRUE);
    void IncrementSkipCount(BOOL bKeepInd) { m_nSkipCount++; m_bSkipKeepInd = bKeepInd; }

    const CMoveRecord& GetAt(const_iterator pos) const
        { return **pos; }
    const CMoveRecord& GetNext(const_iterator& pos) const
        { return **pos++; }
    const CMoveRecord& GetPrev(const_iterator& pos) const
    {
        const CMoveRecord& retval = **pos;
        // decrementing MFC head position --> null, so emulate that
        if (pos != begin())
        {
            --pos;
        }
        else
        {
            pos = end();
        }
        return retval;
    }
    const CMoveRecord& GetFirstRecord() const
        { return *front(); }

    void Clear();
    void Serialize(CArchive& ar, BOOL bSaveUndo = TRUE);

#ifdef _DEBUG
    void DumpToTextFile(const CGamDoc& pDoc, CFile& file);
#endif
// Implementation
protected:
    const_iterator FindIndex(size_t nIndex) const;

protected:
    int         m_nPlaybackLock;        // Lock count used to stop recursion
    int         m_nSkipCount;           // Number of moves to skip
    BOOL        m_bSkipKeepInd;         // Keep each skipped move's indicators
    BOOL        m_bCompoundSingleStep;  // If TRUE, single step compound moves

    OwnerOrNullPtr<CGameState> m_pStateSave;           // Used to push/pos state
    BOOL        m_bQuietPlaybackSave;

    // Serialize the following...
    size_t      m_nSeqNum;

    BOOL        m_bCompoundMove;        // Recording a compound move.
    OwnerOrNullPtr<CGameState> m_pCompoundBaseBookMark;// Game state at start of compound move
    size_t      m_nCompoundBaseIndex;   // Index of current compound move.
};

#endif

