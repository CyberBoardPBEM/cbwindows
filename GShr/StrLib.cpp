// StrLib.CPP - Miscellaneous string manipulation functions.
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

#include    "stdafx.h"
#include    "StrLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// --------------------------- //

char *SetFileExt(char *fname, const char *ext)
{
    char *sp = strrchr(fname, '.');
    if (sp != NULL)
        *sp = 0;
    strcat(fname, ".");
    strcat(fname, ext);
    return fname;
}

// --------------------------- //

CB::string StrGetAAAFormat(size_t n)
{
    CB::string retval;
    wchar_t ch = value_preserving_cast<wchar_t>(((n - size_t(1)) % size_t(26)) + size_t(L'A'));
    for ( ; ; )
    {
        retval += ch;
        if (n > size_t(26))
        {
            n -= size_t(26);
        }
        else
        {
            break;
        }
    }
    return retval;
}

void StrLeadZeros(char* szVal, size_t nWidth)
{
    char szTmp[_MAX_I64TOSTR_BASE10_COUNT];
    size_t i;
    ASSERT(nWidth < sizeof(szTmp));
    for (i = size_t(0) ; i < nWidth ; ++i)
        szTmp[value_preserving_cast<ptrdiff_t>(i)] = '0';
    szTmp[value_preserving_cast<ptrdiff_t>(i)] = 0;           // Terminate
    size_t nLen = strlen(szVal);
    ASSERT(nLen < sizeof(szTmp));
    memcpy(szTmp + value_preserving_cast<ptrdiff_t>(nWidth - nLen), szVal, nLen);
    strcpy(szVal, szTmp);
}

// --------------------------- //
// Removes the filename portion of a file path specification

void StrTruncatePath(char *pszName)
{
    if (strlen(pszName) == 0)
        return;

    char* sp;
    for (sp = pszName + strlen(pszName) - 1; sp != pszName; sp--)
    {
        if (*sp == '\\')
        {
            if (sp[-1] == ':')
                sp++;           // leave backslash on drive
            break;
        }
        else if (*sp == ':')
        {
            // If no backslash encountered but the drive name was,
            // force a backslash after the drive.
            sp++;
            *sp++ = '\\';
        }
    }
    *sp = 0;
}

// Extract only the file name.

void StrExtractFilename(char *pszFName, const char* pszFPath)
{
    if (strlen(pszFPath) == 0)
    {
        *pszFName = 0;
        return;
    }
    const char* sp;
    for (sp = pszFPath + strlen(pszFPath) - 1; sp != pszFPath; sp--)
    {
        if (*sp == '\\' || *sp == ':')
        {
            sp++;
            break;
        }
    }
    strcpy(pszFName, sp);
}

// --------------------------- //

void StrBuildFullFilename(char* pszFull, const char* pszPath,
    const char* pszName)
{
    strcpy(pszFull, pszPath);
    if (pszFull[strlen(pszFull)-1] != '\\')
        strcat(pszFull, "\\");
    strcat(pszFull, pszName);
}

