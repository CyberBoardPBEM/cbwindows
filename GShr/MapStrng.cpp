// MapStrng.cpp
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

#include    <stdafx.h>
#include    "MapStrng.h"
#include    "Versions.h"

///////////////////////////////////////////////////////////////////////

/* porting check:  verify that compiler generates GameElement
that is binary-identical to Microsoft one */
namespace {
    class GameElementCheck
    {
    public:
        GameElementCheck()
        {
            {
                GameElement32 test(MarkID16(0x1234));
                static_assert(sizeof(test) == sizeof(uint32_t), "size mismatch");
                ASSERT(reinterpret_cast<uint32_t&>(test) == 0xF0001234 || !"non-Microsoft field layout");
            }
            {
                GameElement32 test = Invalid_v<GameElement32>;
                ASSERT(!test.IsAPiece() && !test.IsAMarker() && !test.IsAnObject());
                static_assert(sizeof(int) == sizeof(test), "need to adjust cast");
                ASSERT(reinterpret_cast<int&>(test) == -1);
            }
            {
                GameElement64 test(MarkID32(0x12345678));
                static_assert(sizeof(test) == sizeof(uint64_t), "size mismatch");
                ASSERT(reinterpret_cast<uint64_t&>(test) == UINT64_C(0xF000000012345678) || !"non-Microsoft field layout");
            }
            {
                GameElement64 test = Invalid_v<GameElement64>;
                ASSERT(!test.IsAPiece() && !test.IsAMarker() && !test.IsAnObject());
                static_assert(sizeof(int64_t) == sizeof(test), "need to adjust cast");
                ASSERT(reinterpret_cast<int64_t&>(test) == INT64_C(-1));
            }
        }
    } gameElementCheck;
}

void GameElement32::Serialize(CArchive& ar) const
{
    static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(u.buf), "size mismatch");
    static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(decltype(u.buf)), "align mismatch");
    static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(*this)>>, "needs more complex serialize");
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<decltype(u.pieceElement.pid)>(ar);
    switch (fileIDSize)
    {
        case 2:
            ar << u.buf;
            break;
        case 4:
        {
            ASSERT(!"will probably never be used");
            SerializeBackdoor sb;
            ar << SerializeBackdoorGameElement::Convert(*this);
            break;
        }
        default:
            CbThrowBadCastException();
    }
}

void GameElement32::Serialize(CArchive& ar)
{
    static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(u.buf), "size mismatch");
    static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(decltype(u.buf)), "align mismatch");
    static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(*this)>>, "needs more complex serialize");
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<decltype(u.pieceElement.pid)>(ar);
    switch (fileIDSize)
    {
        case 2:
            ar >> u.buf;
            break;
        case 4:
        {
            ASSERT(!"will probably never be used");
            SerializeBackdoor sb;
            GameElement64 temp;
            ar >> temp;
            *this = SerializeBackdoorGameElement::Convert(temp);
            break;
        }
        default:
            CbThrowBadCastException();
    }
}

GameElement32::operator GameElement64() const
{
    if (IsAPiece())
    {
        return GameElement64(SerializeBackdoor::Convert(static_cast<PieceID16>(*this)), GetSide());
    }
    else if (IsAMarker())
    {
        return GameElement64(SerializeBackdoor::Convert(static_cast<MarkID16>(*this)));
    }
#if defined(GPLAY)
    else if (IsAnObject())
    {
        return GameElement64(SerializeBackdoorObjectID::Convert(static_cast<ObjectID32>(*this)));
    }
#endif
    else
    {
        // Serialize shouldn't produce uninitialized objects
        CbThrowBadCastException();
    }
}

#if defined(GPLAY)
#if !defined(NDEBUG)
ObjectID32 GetObjectIDFromElementLegacyCheck(GameElementLegacyCheck elem)
{
    return static_cast<ObjectID32>(elem);
}
#endif
#endif

void GameElement64::Serialize(CArchive& ar) const
{
    static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(u.buf), "size mismatch");
    static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(decltype(u.buf)), "align mismatch");
    static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(*this)>>, "needs more complex serialize");
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<decltype(u.pieceElement.pid)>(ar);
    // piece-100-sides can't fit in GameElement32, so use GameElement64
    if (CB::GetFeatures(ar).Check(ftrPiece100Sides))
    {
        fileIDSize = 4;
    }
    switch (fileIDSize)
    {
        case 2:
        {
            SerializeBackdoor sb;
            ar << SerializeBackdoorGameElement::Convert(*this);
            break;
        }
        case 4:
            ar << u.buf;
            break;
        default:
            CbThrowBadCastException();
    }
}

void GameElement64::Serialize(CArchive& ar)
{
    static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(u.buf), "size mismatch");
    static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(decltype(u.buf)), "align mismatch");
    static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(*this)>>, "needs more complex serialize");
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<decltype(u.pieceElement.pid)>(ar);
    // piece-100-sides can't fit in GameElement32, so use GameElement64
    if (CB::GetFeatures(ar).Check(ftrPiece100Sides))
    {
        fileIDSize = 4;
    }
    switch (fileIDSize)
    {
        case 2:
        {
            SerializeBackdoor sb;
            GameElement32 temp;
            ar >> temp;
            *this = SerializeBackdoorGameElement::Convert(temp);
            break;
        }
        case 4:
            ar >> u.buf;
            break;
        default:
            CbThrowBadCastException();
    }
}

GameElement64::operator GameElement32() const
{
    if (IsAPiece())
    {
        return GameElement32(SerializeBackdoor::Convert(static_cast<PieceID32>(*this)), GetSide());
    }
    else if (IsAMarker())
    {
        return GameElement32(SerializeBackdoor::Convert(static_cast<MarkID32>(*this)));
    }
#if defined(GPLAY)
    else if (IsAnObject())
    {
        return GameElement32(SerializeBackdoorObjectID::Convert(static_cast<ObjectID64>(*this)));
    }
#endif
    else
    {
        // Serialize shouldn't produce uninitialized objects
        CbThrowBadCastException();
    }
}

GameElement64 SerializeBackdoorGameElement::Convert(const GameElement32& ge)
{
    if (!Depth())
    {
        ASSERT(!"only for serialize use");
        AfxThrowNotSupportedException();
    }
    return static_cast<GameElement64>(ge);
}

GameElement32 SerializeBackdoorGameElement::Convert(const GameElement64& ge)
{
    if (!Depth())
    {
        ASSERT(!"only for serialize use");
        AfxThrowNotSupportedException();
    }
    return static_cast<GameElement32>(ge);
}

