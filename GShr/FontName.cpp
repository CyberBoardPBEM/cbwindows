// FontName.cpp
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

FNameID FNameTbl::AddFaceName(const char *pszFName, int iFamily)
{
    FName oFName(pszFName, iFamily);
    return Register(std::move(oFName));
}

// ----------------------------------------------------- //
// if iFaceName == -1, return number of face names
// if iFaceName >= 0 && pszFName == NULL, return stringlength.
// else return the face name and family id

size_t FNameTbl::GetFaceInfo(int iFaceNum, char *pszFName, int* iFamily)
{
    if (iFaceNum < 0)
    {
        return size();
    }
    else
    {
        if (value_preserving_cast<size_t>(iFaceNum) >= size())
        {
            AfxThrowInvalidArgException();
        }
        iterator it = std::next(begin(), iFaceNum);
        ASSERT(!it->expired());
        const FName& fname = **it->lock();
        if (pszFName)
        {
            lstrcpy(pszFName, fname.szFName);
            *iFamily = fname.iFamily;
            return size_t(TRUE);
        }
        return value_preserving_cast<size_t>(lstrlen(fname.szFName));
    }
    return size_t(FALSE);
}

// ====================================================== //

FName::FName(const char *pszFName, int iFamily)
{
    strcpy(szFName, pszFName);
    this->iFamily = iFamily;
}

// ----------------------------------------------------- //
bool FName::operator==(const FName& rhs) const
{
    if (iFamily == rhs.iFamily)
    {
        if (strcmp(szFName, rhs.szFName) == 0)
            return TRUE;
    }
    return FALSE;
}
