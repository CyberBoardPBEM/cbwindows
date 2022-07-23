// GamState.h
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

#ifndef _GAMSTATE_H
#define _GAMSTATE_H

#include "MapStrng.h"
#include "Trays.h"

class CTrayManager;
class CPieceTable;
class CPBoardManager;

// A CGameState object saves a snapshot of the state of the game.
// This can be used for bookmarks and for state records in the move
// file.

class CGameState
{
public:
    CGameState();
    ~CGameState() = default;

// Operations
public:
    BOOL SaveState(CGamDoc& doc);
    BOOL RestoreState(CGamDoc& doc) const;
    // ------- //
    BOOL CompareState(const CGamDoc& doc) const;
    // ------- //
    void Clear();
    void Serialize(CArchive& ar);

// Implementation
protected:
    // --------- //
    CGameElementStringMap m_mapString; // Text set by player
    /* can't use std::default_delete<CPBoardManager> because
    default_delete<> requires a complete type, and we can't
    #include "PBoard.h" due to circular dependencies */
    class CPBoardManagerDelete
    {
    public:
        void operator()(CPBoardManager* p) const;
    };
    using OwnerOrNullCPBoardManager = CB::propagate_const<std::unique_ptr<CPBoardManager, CPBoardManagerDelete>>;
    OwnerOrNullCPBoardManager m_pPBMgr;   // Playing boards in use.
    CTrayManager    m_pYMgr;    // Content of trays manager
    OwnerOrNullPtr<CPieceTable> m_pPTbl;    // The playing piece table.
};

#endif

