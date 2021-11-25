// StrLib.h
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

#ifndef _STRLIB_H
#define _STRLIB_H

//*** STRLIB.CPP ***//

BOOL StrToIntChecked(const char **psp, int *iVal);
char *StrToLong(const char *sp, long *lVal);
char *SetFileExt(char *fname, const char *ext);
BOOL StrIsIdentifierChar(int ch);
BOOL StrIsIdentifier(const char* sp);
void StrGetAAAFormat(char *szVal, size_t n);
void StrLeadZeros(char* szVal, size_t nWidth);
void StrTruncatePath(char *pszName);
void StrExtractFilename(char *pszFName, const char* pszFPath);
void StrBuildFullFilename(char* pszFull, const char* pszPath,
    const char* pszName);

#endif

