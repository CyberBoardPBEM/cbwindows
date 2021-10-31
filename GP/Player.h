// Player.h
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

#ifndef _PLAYER_H
#define _PLAYER_H

const int   MAX_OWNER_ACCOUNTS   = 26;              // Max of 26 players (A..Z)
const DWORD PLAYER_REFEREE_FLAG  = 0x80000000;      // Top bit set is referee
const DWORD OWNER_MASK_REFEREE   = 0x03FFFFFF;      // 26 total players allowed
const DWORD OWNER_MASK_SPECTATOR = 0;               // No ownership is spectator

struct Player
{
    CString m_strName;      // The player's name
    DWORD   m_seqNum;       // Number used for special unlock messages

    Player() { m_seqNum = 0; }
    Player(LPCTSTR pszName) { m_strName = pszName; m_seqNum = 0; }

    void Serialize(CArchive& ar);
};

class CPlayerManager : public CArray< Player, Player& >
{
public:
    intptr_t AddPlayer(LPCTSTR pszName);
    Player& GetPlayerUsingMask(DWORD dwMask);

    static DWORD GetMaskFromPlayerNum(int nPlayerNumber);
    static int   GetPlayerNumFromMask(DWORD dwMask);
    static BOOL  IsReferee(DWORD dwMask) { return (BOOL)(dwMask & PLAYER_REFEREE_FLAG); }

    void Serialize(CArchive& ar);
};

#endif
