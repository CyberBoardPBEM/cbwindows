// FontName.cpp
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

// -------------------------------------------------- //
// These mimic ATOM management in Windows that store
// strings that would normally be redundant. Only we store
// font faces and family codes in a nonredundant manner.

#include    "stdafx.h"
#include    "FontName.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ====================================================== //

FNameID FNameTbl::AddFaceName(const CB::string& pszFName, uint8_t iFamily)
{
    FName oFName(pszFName, iFamily);
    return Register(std::move(oFName));
}

FNameID FNameTbl::operator[](size_t iFaceNum) const
{
    if (iFaceNum >= size())
    {
        AfxThrowInvalidArgException();
    }
    const_iterator it = std::next(begin(), value_preserving_cast<ptrdiff_t>(iFaceNum));
    ASSERT(!it->expired());
    return it->lock();
}

// ====================================================== //

FName::FName(CB::string pszFName, uint8_t iFamily)
{
    if (pszFName.a_size() >= size_t(LF_FACESIZE))
    {
        AfxThrowInvalidArgException();
    }
    szFName = std::move(pszFName);
    this->iFamily = iFamily;
}

// ----------------------------------------------------- //
bool FName::operator==(const FName& rhs) const
{
    if (iFamily == rhs.iFamily)
    {
        if (szFName == rhs.szFName)
            return TRUE;
    }
    return FALSE;
}
