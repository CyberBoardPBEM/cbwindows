// MapStrng.cpp
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

#include    <stdafx.h>
#include    "MapStrng.h"

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

#if defined(GPLAY)
#if !defined(NDEBUG)
ObjectID32 GetObjectIDFromElementLegacyCheck(GameElementLegacyCheck elem)
{
    return static_cast<ObjectID32>(elem);
}
#endif
#endif

