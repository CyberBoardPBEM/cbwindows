// Font.cpp
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
#include    "Font.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ----------------------------------------------------- //

CFontTbl::~CFontTbl(void)
{
    CbFont *opFLst = (CbFont *)opAList;
    while (opFLst != NULL)
    {
        CbFont *opFNxt = (CbFont *)opFLst->next;    // Pick up next since we'll delete
        Destroy(opFLst);
        opFLst = opFNxt;                            // Chain to next entry
    }
    opAList = NULL;
}

// ----------------------------------------------------- //

FontID CFontTbl::AddFont(int iSize, int taFlgs, int iFamily,
    const char *pszFName)
{
    FNameID fnID = oFName.AddFaceName(pszFName, iFamily);

    CbFont oFont(iSize, taFlgs, fnID);
    FontID id;
    if ((id = AddIfExists(&oFont)) == 0)
        id = AddAtom(new CbFont(iSize, taFlgs, fnID)); // (use fnID)
    else    // Delete local fnID since we didn't use it.
        oFName.DelFaceName(fnID);
    return id;
}

// ----------------------------------------------------- //

void CFontTbl::Destroy(Atom *opAtom)
{
    CbFont *opFnt = (CbFont*)opAtom;
    oFName.DelFaceName(opFnt->fnID);
    if (opFnt->hFnt)
        DeleteObject(opFnt->hFnt);
    delete opFnt;
}

// ----------------------------------------------------- //

void CFontTbl::FillLogFontStruct(FontID id, LPLOGFONT pLF)
{
    CbFont *opFnt = (CbFont *)id;
    const char *pszFace;

    memset(pLF, 0, sizeof(LOGFONT));
    pLF->lfHeight = -(opFnt->iTypeSize);
    pLF->lfWeight = opFnt->IsBold() ? FW_BOLD : FW_NORMAL;
    pLF->lfItalic = opFnt->IsItalic();
    pLF->lfUnderline = opFnt->IsULine();
    pLF->lfCharSet = DEFAULT_CHARSET;
    pLF->lfPitchAndFamily = oFName.GetFaceFamily(opFnt->fnID);
    if ((pszFace = oFName.GetFaceName(opFnt->fnID)) != NULL)
    {
        strcpy(pLF->lfFaceName, pszFace);
    }
}

// ----------------------------------------------------- //

HFONT CFontTbl::GetFontHandle(FontID id)
{
    if (id == 0)
        return NULL;
    CbFont *opFnt = (CbFont *)id;
    if (opFnt->hFnt)
        return opFnt->hFnt;

    LOGFONT lf;
    FillLogFontStruct(id, &lf);

    opFnt->hFnt = CreateFontIndirect(&lf);
    return opFnt->hFnt;
}

// ----------------------------------------------------- //

void CFontTbl::ReleaseFontHandle(FontID id)
{
    if (id == 0) return;

    CbFont *opFnt = (CbFont *)id;

    if (opFnt->hFnt)
    {
        DeleteObject(opFnt->hFnt);
        opFnt->hFnt = NULL;
    }
}
// ----------------------------------------------------- //

void CFontTbl::ReleaseAllFontHandles(void)
{
    CbFont *opFnt = (CbFont *)opAList;
    while (opFnt != NULL)
    {
        if (opFnt->hFnt)
        {
            DeleteObject(opFnt->hFnt);
            opFnt->hFnt = NULL;
        }
        opFnt = (CbFont *)(opFnt->next);
    }
}

void CFontTbl::Archive(CArchive& ar, FontID& rfontID)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetSize(rfontID);
        ar << (WORD)GetFlags(rfontID);
        ar << (WORD)GetFamily(rfontID);
        CString str = GetFaceName(rfontID);
        ar << str;
    }
    else
    {
        ASSERT(rfontID == 0);
        WORD wSize, wFlags, wFamily;
        ar >> wSize;
        ar >> wFlags;
        ar >> wFamily;
        CString str;
        ar >> str;
        rfontID = AddFont((int)wSize, (int)wFlags, (int)wFamily, str);
    }
}

// ===================================================== //
// (protected)
BOOL CbFont::AtomsEqual(Atom &atom)
{
    CbFont& oFnt = (CbFont&)atom;
    return oFnt.fnID == fnID && oFnt.iTypeSize == iTypeSize &&
            oFnt.taFlags == taFlags;
}

