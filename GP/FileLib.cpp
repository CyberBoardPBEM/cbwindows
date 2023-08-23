// FileLib.cpp
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
#include    <io.h>
#include    "FileLib.h"

/////////////////////////////////////////////////////////////////////////////

const UINT COPY_BFR_LEN = 4096;

BOOL CloneFile(const CB::string& pszSrcPath, const CB::string& pszDestPath)
{
    TRY
    {
        CFile iFile(pszSrcPath, CFile::modeRead);
        CFile oFile(pszDestPath, CFile::modeWrite | CFile::modeCreate);
        DWORD dwLen = (DWORD)iFile.GetLength();
        iFile.Seek(0, CFile::begin);
        if (!CopyOpenFiles(&iFile, &oFile, dwLen))
            AfxThrowMemoryException();      // Any exception will do.
        oFile.Close();
        iFile.Close();
    }
    CATCH_ALL(e)
    {
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

BOOL CloneOpenFile(CFile* pSrcFile, const CB::string& pszDestPath)
{
    TRY
    {
        CFile oFile(pszDestPath, CFile::modeWrite | CFile::modeCreate);
        DWORD dwLen = (DWORD)pSrcFile->GetLength();
        pSrcFile->Seek(0, CFile::begin);
        if (!CopyOpenFiles(pSrcFile, &oFile, dwLen))
            AfxThrowMemoryException();      // Any exception will do.
        oFile.Close();
    }
    CATCH_ALL(e)
    {
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

BOOL CopyOpenFiles(CFile* pSrcFile, CFile* pDestFile, DWORD dwLen)
{
    std::vector<std::byte> pszBfr(COPY_BFR_LEN);
    TRY
    {
        while (TRUE)
        {
            UINT nRead = value_preserving_cast<UINT>(CB::min(dwLen, COPY_BFR_LEN));
            pSrcFile->Read(pszBfr.data(), nRead);
            pDestFile->Write(pszBfr.data(), nRead);
            dwLen -= nRead;
            if (dwLen == 0)
                break;
        }
    }
    CATCH_ALL(e)
    {
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}


