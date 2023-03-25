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
#include    <sstream>
#include    "Font.h"

#ifdef      GPLAY
    #include    "GamDoc.h"
#else
    #include    "GmDoc.h"
#endif

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
}

// ----------------------------------------------------- //

FontID CFontTbl::AddFont(int iSize, int taFlgs, uint8_t iFamily,
    const CB::string& pszFName)
{
    FNameID fnID = oFName.AddFaceName(pszFName, iFamily);

    CbFont oFont(iSize, taFlgs, fnID);
    return Register(std::move(oFont));
}

// ----------------------------------------------------- //

CbFont::~CbFont()
{
    for (HFonts::value_type& v : hFnt)
    {
        DeleteObject(v.second);
    }
}

CB::string CbFont::ToString(int angle /* = 0 */) const
{
    std::wostringstream str;
    str << L"CbFont(" <<
        (*fnID)->ToString() << L", "
        L"size:" << std::to_wstring(iTypeSize) << L", "
        L"flags:";

    const static struct
    {
        int flag;
        CB::string name;
    } flags[] =
    {
        taBold, L"Bold",
        taItalic, L"Italic",
        taULine, L"ULine",
    };
    if (taFlags)
    {
        int temp = taFlags;
        for (ptrdiff_t i = 0; i < static_cast<ptrdiff_t>(sizeof(flags) / sizeof(flags[0])); ++i)
        {
            if (temp & flags[i].flag)
            {
                temp &= ~flags[i].flag;
                str << flags[i].name;
                if (temp)
                {
                    str << L'|';
                }
            }
        }
        ASSERT(!temp);
    }
    else
    {
        str << L"<none>";
    }

    if (angle)
    {
        str << L' ' << angle << L'°';
    }

    str << L')';
    return str.str();
}

// ----------------------------------------------------- //

// N.B.:  angle is clockwise per CB convention
void CFontTbl::FillLogFontStruct(FontID id, LPLOGFONT pLF, int angle /* = 0 */)
{
    const CbFont *opFnt = &**id;
    const CB::string* pszFace;

    memset(pLF, 0, sizeof(LOGFONT));
    // need TrueType for rotated text
    pLF->lfOutPrecision = angle == 0 ? 0 : OUT_TT_ONLY_PRECIS;
    pLF->lfOrientation = pLF->lfEscapement = -angle*10;
    pLF->lfHeight = -(opFnt->iTypeSize);
    pLF->lfWeight = opFnt->IsBold() ? FW_BOLD : FW_NORMAL;
    pLF->lfItalic = opFnt->IsItalic();
    pLF->lfUnderline = opFnt->IsULine();
    pLF->lfCharSet = DEFAULT_CHARSET;
    pLF->lfPitchAndFamily = oFName.GetFaceFamily(opFnt->fnID);
    if ((pszFace = oFName.GetFaceName(opFnt->fnID)) != NULL)
    {
        strcpy(pLF->lfFaceName, *pszFace);
    }
}

// ----------------------------------------------------- //

HFONT CFontTbl::GetFontHandle(FontID id, int angle /* = 0 */)
{
    if (id == 0)
        return NULL;
    const CbFont* opFnt = &**id;
    CbFont::HFonts::iterator it = opFnt->hFnt.find(angle);
    if (it != opFnt->hFnt.end())
        return it->second;

    LOGFONT lf;
    FillLogFontStruct(id, &lf, angle);

    it = opFnt->hFnt.insert(std::make_pair(angle, CreateFontIndirect(&lf))).first;
    return it->second;
}

// ----------------------------------------------------- //

void CFontTbl::ReleaseAllFontHandles(void)
{
    clear();
}

void CFontTbl::Archive(CArchive& ar, FontID& rfontID)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetSize(rfontID);
        ar << (WORD)GetFlags(rfontID);
        ar << (WORD)GetFamily(rfontID);
        const CB::string& str = CheckedDeref(GetFaceName(rfontID));
        ar << str;
    }
    else
    {
        ASSERT(rfontID == 0);
        WORD wSize, wFlags, wFamily;
        ar >> wSize;
        ar >> wFlags;
        ar >> wFamily;
        CB::string str;
        ar >> str;
        rfontID = AddFont((int)wSize, (int)wFlags, value_preserving_cast<uint8_t>(wFamily), str);
    }
}

// ===================================================== //
bool CbFont::operator==(const CbFont& rhs) const
{
    return rhs.fnID == fnID && rhs.iTypeSize == iTypeSize &&
            rhs.taFlags == taFlags;
}
