// Player.cpp
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
#include    "Versions.h"
#include    "Player.h"

PlayerId CPlayerManager::AddPlayer(CB::string pszName)
{
    emplace_back(std::move(pszName));
    return PlayerId(size() - size_t(1));
}

const Player& CPlayerManager::GetPlayerUsingMask(PlayerMask dwMask) const
{
    wxASSERT(dwMask);
    PlayerId nPlayerNum = GetPlayerNumFromMask(dwMask);
    return (*this)[nPlayerNum];
}

PlayerMask CPlayerManager::GetMaskFromPlayerNum(PlayerId nPlayerNumber)
{
    if (nPlayerNumber == INVALID_PLAYER)
        return OWNER_MASK_SPECTATOR;
	return PlayerMask(uint32_t(1) << static_cast<uint8_t>(nPlayerNumber));
}

// Returns -1 if no bit set. Otherwise, returns number of
// the rightmost set bit.
PlayerId CPlayerManager::GetPlayerNumFromMask(PlayerMask dwMask)
{
    if (!dwMask) return INVALID_PLAYER;          // For a bit more speed
    uint32_t u32(dwMask);
    for (int i = 0; i < MAX_OWNER_ACCOUNTS; i++)
    {
        if ((u32 & uint32_t(1)) != uint32_t(0))
            return PlayerId(i);
        u32 >>= 1;
    }
    return INVALID_PLAYER;
}

void CPlayerManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<uint16_t>(size());
        }
        else
        {
            CB::WriteCount(ar, size());
        }
        for (Player& player : *this)
        {
            player.Serialize(ar);
        }
    }
    else
    {
        clear();
        size_t nCount;
        uint16_t wTmp;

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
            push_back(std::move(player));
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

