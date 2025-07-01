// Player.h
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

#ifndef _PLAYER_H
#define _PLAYER_H

class alignas(uint8_t) PlayerId
{
public:
    PlayerId() = default;

    template<typename Integer,
            std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
    explicit PlayerId(Integer i) :
        id(value_preserving_cast<int>(i) != -1 ?
                value_preserving_cast<uint8_t>(i)
            :
                uint8_t(-1))
    {
        static_assert(sizeof(*this) == sizeof(uint8_t));
        static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(uint8_t));
        wxASSERT(id <= 25 || id == uint8_t(-1));
    }

    template<typename Integer,
            std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
    explicit operator Integer() const
    {
        return id != uint8_t(-1) ?
                    value_preserving_cast<Integer>(id)
                :
                    static_cast<Integer>(-1);
    }

private:
    uint8_t id;
};

inline bool operator==(const PlayerId& l, const PlayerId& r)
{
    return static_cast<uint8_t>(l) == static_cast<uint8_t>(r);
}

inline bool operator!=(const PlayerId& l, const PlayerId& r)
{
    return !(l == r);
}

class alignas(uint32_t) PlayerMask
{
public:
    PlayerMask() = default;
    explicit PlayerMask(uint32_t i) :
        mask(i)
    {
        static_assert(sizeof(*this) == sizeof(uint32_t));
        static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(uint32_t));
    }

    explicit operator uint32_t() const
    {
        return mask;
    }

    explicit operator bool() const
    {
        return bool(uint32_t(*this));
    }

private:
    uint32_t mask;
};

inline bool operator==(const PlayerMask& l, const PlayerMask& r)
{
    return static_cast<uint32_t>(l) == static_cast<uint32_t>(r);
}

inline bool operator!=(const PlayerMask& l, const PlayerMask& r)
{
    return !(l == r);
}

inline PlayerMask operator&(const PlayerMask& l, const PlayerMask& r)
{
    return PlayerMask(static_cast<uint32_t>(l) & static_cast<uint32_t>(r));
}

inline PlayerMask operator|(const PlayerMask& l, const PlayerMask& r)
{
    return PlayerMask(static_cast<uint32_t>(l) | static_cast<uint32_t>(r));
}

inline CArchive& operator<<(CArchive& ar, const PlayerMask& m)
{
    return ar << static_cast<uint32_t>(m);
}

inline CArchive& operator>>(CArchive& ar, PlayerMask& m)
{
    return ar >> reinterpret_cast<uint32_t&>(m);
}

template<typename CharT>
struct std::formatter<PlayerMask, CharT> : private std::formatter<uint32_t, CharT>
{
private:
    using BASE = std::formatter<uint32_t, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const PlayerMask& t, FormatContext& ctx) const
    {
        return BASE::format(static_cast<uint32_t>(t), ctx);
    }
};

const int   MAX_OWNER_ACCOUNTS   = 26;              // Max of 26 players (A..Z)
const PlayerId INVALID_PLAYER(-1);
const PlayerMask PLAYER_REFEREE_FLAG(uint32_t(0x80000000));      // Top bit set is referee
const PlayerMask OWNER_MASK_REFEREE(uint32_t(0x03FFFFFF));      // 26 total players allowed
const PlayerMask OWNER_MASK_SPECTATOR(uint32_t(0));               // No ownership is spectator

struct Player
{
    CB::string m_strName;   // The player's name
    uint32_t m_seqNum;       // Number used for special unlock messages

    Player() { m_seqNum = uint32_t(0); }
    Player(CB::string pszName) { m_strName = std::move(pszName); m_seqNum = uint32_t(0); }

    void Serialize(CArchive& ar);
};

class CPlayerManager : private std::vector<Player>
{
private:
    typedef std::vector<Player> BASE;
public:
    using BASE::begin;
    using BASE::end;
    using BASE::iterator;
    using BASE::const_iterator;

    using BASE::empty;
    using BASE::size;

    const Player& operator[](PlayerId i) const
    {
        return BASE::at(static_cast<size_t>(i));
    }
    Player& operator[](PlayerId i)
    {
        return BASE::at(static_cast<size_t>(i));
    }

    PlayerId AddPlayer(CB::string pszName);
    const Player& GetPlayerUsingMask(PlayerMask dwMask) const;

    static PlayerMask GetMaskFromPlayerNum(PlayerId nPlayerNumber);
    static PlayerId GetPlayerNumFromMask(PlayerMask dwMask);
    static bool IsReferee(PlayerMask dwMask) { return bool(dwMask & PLAYER_REFEREE_FLAG); }

    void Serialize(CArchive& ar);
};

#endif
