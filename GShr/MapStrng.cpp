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
                GameElement test(MarkID(0x1234));
                static_assert(sizeof(test) == sizeof(uint32_t), "size mismatch");
                ASSERT(reinterpret_cast<uint32_t&>(test) == 0xF0001234 || !"non-Microsoft field layout");
            }
            {
                GameElement test = Invalid_v<GameElement>;
                ASSERT(!test.IsAPiece() && !test.IsAMarker() && !test.IsAnObject());
                static_assert(sizeof(int) == sizeof(test), "need to adjust cast");
                ASSERT(reinterpret_cast<int&>(test) == -1);
            }
        }
    } gameElementCheck;
}

void CGameElementStringMap::Clone(CGameElementStringMap* pMapToCopy)
{
    RemoveAll();

    POSITION pos = pMapToCopy->GetStartPosition();
    while (pos != NULL)
    {
        CString str;
        GameElement elem;
        pMapToCopy->GetNextAssoc(pos, elem, str);
        SetAt(elem, str);
    }
}

///////////////////////////////////////////////////////////////////////

BOOL CGameElementStringMap::Compare(CGameElementStringMap* pMapToCompare)
{
    if (GetCount() != pMapToCompare->GetCount())
        return FALSE;                       // Different sizes so no match
    POSITION pos = pMapToCompare->GetStartPosition();
    while (pos != NULL)
    {
        CString strToCompare;
        CString strOurs;
        GameElement elem;
        pMapToCompare->GetNextAssoc(pos, elem, strToCompare);
        if (!Lookup(elem, strOurs))
            return FALSE;                   // Not found so no match
        if (strToCompare != strOurs)
            return FALSE;                   // values don't match
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////

void CGameElementStringMap::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        DWORD dwCount = GetCount();
        ar << dwCount;
        POSITION pos = GetStartPosition();
        while (pos != NULL)
        {
            GameElement elem;
            CString str;
            GetNextAssoc(pos, elem, str);
            ar << elem;
            ar << str;
        }
    }
    else
    {
        this->RemoveAll();
        DWORD dwCount;
        ar >> dwCount;
        while (dwCount--)
        {
            GameElement dwElem;
            ar >> dwElem;
            CString str;
            ar >> str;
            SetAt(dwElem, str);
        }
    }
}

