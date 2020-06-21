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
    FNameID id;
    if ((id = AddIfExists(&oFName)) == 0)
        id = AddAtom(new FName(pszFName, iFamily));
    return id;
}

// ----------------------------------------------------- //
// if iFaceName == -1, return number of face names
// if iFaceName >= 0 && pszFName == NULL, return stringlength.
// else return the face name and family id

int FNameTbl::GetFaceInfo(int iFaceNum, char *pszFName, int* iFamily)
{
    FName* pFAtom = (FName*)opAList;
    if (pFAtom == NULL) return 0;

    if (iFaceNum < 0)
    {
        int i;
        for (i = 0; pFAtom != NULL; pFAtom = (FName*)pFAtom->next, i++);
        return i;
    }
    else
    {
        for (int i=0; i < iFaceNum && pFAtom != NULL; pFAtom=(FName*)pFAtom->next, i++);
        ASSERT(pFAtom != NULL);
        if (pszFName)
        {
            lstrcpy(pszFName, pFAtom->szFName);
            *iFamily = pFAtom->iFamily;
            return TRUE;
        }
        return lstrlen(pFAtom->szFName);
    }
    return FALSE;
}

// ====================================================== //

FName::FName(const char *pszFName, int iFamily)
{
    strcpy(szFName, pszFName);
    this->iFamily = iFamily;
}

// ----------------------------------------------------- //
// (protected)
BOOL FName::AtomsEqual(Atom &atom)
{
    if (iFamily == ((FName&)atom).iFamily)
    {
        if (strcmp(szFName, ((FName&)atom).szFName) == 0)
            return TRUE;
    }
    return FALSE;
}
