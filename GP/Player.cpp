// Player.cpp
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

#include    "stdafx.h"
#include    "Versions.h"
#include    "Player.h"

intptr_t CPlayerManager::AddPlayer(CB::string pszName)
{
    Player player(std::move(pszName));
    return Add(player);
}

const Player& CPlayerManager::GetPlayerUsingMask(DWORD dwMask) const
{
    ASSERT(dwMask != 0);
    int nPlayerNum = GetPlayerNumFromMask(dwMask);
    return ElementAt(nPlayerNum);
}

DWORD CPlayerManager::GetMaskFromPlayerNum(int nPlayerNumber)
{
    if (nPlayerNumber < 0)
        return 0;
	return (DWORD)1 << nPlayerNumber;
}

// Returns -1 if no bit set. Otherwise, returns number of
// the rightmost set bit.
int CPlayerManager::GetPlayerNumFromMask(DWORD dwMask)
{
    if (dwMask == 0) return -1;          // For a bit more speed
    for (int i = 0; i < MAX_OWNER_ACCOUNTS; i++)
    {
        if ((dwMask & 1) != 0)
            return i;
        dwMask >>= 1;
    }
    return -1;
}

void CPlayerManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(GetSize());
        }
        else
        {
            CB::WriteCount(ar, value_preserving_cast<size_t>(GetSize()));
        }
        for (int i = 0; i < GetSize(); i++)
            GetAt(i).Serialize(ar);
    }
    else
    {
        RemoveAll();
        size_t nCount;
        WORD wTmp;

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar >> wTmp;
            nCount = wTmp;
        }
        else
        {
            nCount = CB::ReadCount(ar);
        }
        while (nCount--)
        {
            Player player;
            player.Serialize(ar);
            Add(player);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void Player::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_seqNum;
        ar << m_strName;
    }
    else
    {
        ar >> m_seqNum;
        ar >> m_strName;
    }
}

