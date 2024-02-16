// Font.cpp
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
void CFontTbl::FillLogFontStruct(FontID id, LPLOGFONT pLF, int angle /* = 0 */) const
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
        _tcscpy(pLF->lfFaceName, *pszFace);
    }
}

// ----------------------------------------------------- //

HFONT CFontTbl::GetFontHandle(FontID id, int angle /* = 0 */) const
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

wxFont ToWxFont(FontID fid)
{
    LOGFONT lf;
    CFontTbl& pFontTbl = CheckedDeref(CGamDoc::GetFontManager());
    pFontTbl.FillLogFontStruct(fid, &lf);

    wxNativeFontInfo nfi;

    wxASSERT(lf.lfHeight <= 0);
    nfi.SetPixelSize(wxSize(0, -lf.lfHeight));

    if (lf.lfWeight == FW_BOLD)
    {
        nfi.SetWeight(wxFONTWEIGHT_BOLD);
    }
    else
    {
        nfi.SetWeight(wxFONTWEIGHT_NORMAL);
    }

    nfi.SetStyle(lf.lfItalic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);

    nfi.SetUnderlined(lf.lfUnderline);

    wxASSERT((lf.lfPitchAndFamily & 0x3) == DEFAULT_PITCH);
    switch (lf.lfPitchAndFamily & 0xf)
    {
        case FF_DECORATIVE:
            nfi.SetFamily(wxFONTFAMILY_DECORATIVE);
            break;
        case FF_DONTCARE:
            nfi.SetFamily(wxFONTFAMILY_DEFAULT);
            break;
        case FF_MODERN:
            nfi.SetFamily(wxFONTFAMILY_MODERN);
            break;
        case FF_ROMAN:
            nfi.SetFamily(wxFONTFAMILY_ROMAN);
            break;
        case FF_SCRIPT:
            nfi.SetFamily(wxFONTFAMILY_SCRIPT);
            break;
        case FF_SWISS:
            nfi.SetFamily(wxFONTFAMILY_SWISS);
            break;
        default:
            wxASSERT(!"unknown family");
    }

    nfi.SetFaceName(lf.lfFaceName);

    return wxFont(nfi);
}

FontID ToFontID(wxFont f)
{
    int size = f.GetPixelSize().GetHeight();

    int flags = 0;
    if (f.GetWeight() >= wxFONTWEIGHT_SEMIBOLD)
    {
        flags |= taBold;
    }
    if (f.GetStyle() == wxFONTSTYLE_ITALIC ||
        f.GetStyle() == wxFONTSTYLE_SLANT)
    {
        flags |= taItalic;
    }
    if (f.GetUnderlined())
    {
        flags |= taULine;
    }

    uint8_t family;
    switch (f.GetFamily())
    {
        case wxFONTFAMILY_DECORATIVE:
            family = FF_DECORATIVE;
            break;
        case wxFONTFAMILY_DEFAULT:
            family = FF_DONTCARE;
            break;
        case wxFONTFAMILY_MODERN:
            family = FF_MODERN;
            break;
        case wxFONTFAMILY_ROMAN:
            family = FF_ROMAN;
            break;
        case wxFONTFAMILY_SCRIPT:
            family = FF_SCRIPT;
            break;
        case wxFONTFAMILY_SWISS:
            family = FF_SWISS;
            break;
        default:
            wxASSERT(!"unknown family");
            family = FF_DONTCARE;
    }

    CB::string name = f.GetFaceName();

    CFontTbl& pFontTbl = CheckedDeref(CGamDoc::GetFontManager());
    return pFontTbl.AddFont(size, flags, family, name);
}
