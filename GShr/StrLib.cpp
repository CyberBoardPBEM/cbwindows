// StrLib.CPP - Miscellaneous string manipulation functions.
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
#include    <filesystem>
#include    "StrLib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// --------------------------- //

CB::string SetFileExt(const CB::string& fname, const CB::string& ext)
{
    std::filesystem::path stdFname(fname.w_str());
    std::filesystem::path stdExt(ext.w_str());
    return stdFname.replace_extension(stdExt).wstring();
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

// --------------------------- //
// Removes the filename portion of a file path specification

CB::string StrTruncatePath(const CB::string& pszName)
{
    std::filesystem::path stdName(pszName.w_str());
    return stdName.remove_filename().wstring();
}

// Extract only the file name.

CB::string StrExtractFilename(const CB::string& pszFPath)
{
    std::filesystem::path stdFPath(pszFPath.w_str());
    return stdFPath.filename().wstring();
}

// --------------------------- //

CB::string StrBuildFullFilename(const CB::string& pszPath,
    const CB::string& pszName)
{
    std::filesystem::path stdPath(pszPath.w_str());
    std::filesystem::path stdName(pszName.w_str());
    return (stdPath / stdName).wstring();
}

