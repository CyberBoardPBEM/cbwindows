// Font.cpp
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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

namespace {
    wxFontFamily ToWxFontFamily(WIN32_FF ff);
    WIN32_FF FromWxFontFamily(wxFontFamily ff);
}

// ----------------------------------------------------- //

CFontTbl::~CFontTbl(void)
{
}

// ----------------------------------------------------- //

FontID CFontTbl::AddFont(int iSize, int taFlgs, wxFontFamily iFamily,
    const CB::string& pszFName)
{
    FNameID fnID = oFName.AddFaceName(pszFName, iFamily);

    CbFont oFont(iSize, taFlgs, fnID);
    return Register(std::move(oFont));
}

// ----------------------------------------------------- //

CB::string CbFont::ToString() const
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

    str << L')';
    return str.str();
}

// ----------------------------------------------------- //

void CFontTbl::FillLogFontStruct(FontID id, LPLOGFONT pLF) const
{
    const CbFont *opFnt = &**id;
    const CB::string* pszFace;

    memset(pLF, 0, sizeof(LOGFONT));
    // need TrueType for rotated text
    pLF->lfOutPrecision = 0;
    pLF->lfOrientation = pLF->lfEscapement = 0;
    pLF->lfHeight = -(opFnt->iTypeSize);
    pLF->lfWeight = opFnt->IsBold() ? FW_BOLD : FW_NORMAL;
    pLF->lfItalic = opFnt->IsItalic();
    pLF->lfUnderline = opFnt->IsULine();
    pLF->lfCharSet = DEFAULT_CHARSET;
    pLF->lfPitchAndFamily = static_cast<std::underlying_type_t<WIN32_FF>>(FromWxFontFamily(oFName.GetFaceFamily(opFnt->fnID)));
    if ((pszFace = oFName.GetFaceName(opFnt->fnID)) != NULL)
    {
        _tcscpy(pLF->lfFaceName, *pszFace);
    }
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
        ar << value_preserving_cast<uint16_t>(GetSize(rfontID));
        ar << value_preserving_cast<uint16_t>(GetFlags(rfontID));
        ar << value_preserving_cast<uint16_t>(static_cast<std::underlying_type_t<WIN32_FF>>(FromWxFontFamily(GetFamily(rfontID))));
        const CB::string& str = CheckedDeref(GetFaceName(rfontID));
        ar << str;
    }
    else
    {
        ASSERT(rfontID == 0);
        uint16_t wSize, wFlags, wFamily;
        ar >> wSize;
        ar >> wFlags;
        ar >> wFamily;
        CB::string str;
        ar >> str;
        rfontID = AddFont((int)wSize, (int)wFlags, ToWxFontFamily(static_cast<WIN32_FF>(value_preserving_cast<uint8_t>(wFamily))), str);
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
    CFontTbl& pFontTbl = CGamDoc::GetFontManager();
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
    nfi.SetFamily(ToWxFontFamily(static_cast<WIN32_FF>(lf.lfPitchAndFamily & 0xf0)));

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

    CB::string name = f.GetFaceName();

    CFontTbl& pFontTbl = CGamDoc::GetFontManager();
    return pFontTbl.AddFont(size, flags, f.GetFamily(), name);
}

namespace {
    wxFontFamily ToWxFontFamily(WIN32_FF ff)
    {
        switch (ff)
        {
            case WIN32_FF::DECORATIVE:
                return wxFONTFAMILY_DECORATIVE;
            case WIN32_FF::DONTCARE:
                return wxFONTFAMILY_DEFAULT;
            case WIN32_FF::MODERN:
                return wxFONTFAMILY_MODERN;
            case WIN32_FF::ROMAN:
                return wxFONTFAMILY_ROMAN;
            case WIN32_FF::SCRIPT:
                return wxFONTFAMILY_SCRIPT;
            case WIN32_FF::SWISS:
                return wxFONTFAMILY_SWISS;
            default:
                wxASSERT(!"unknown family");
                return wxFONTFAMILY_UNKNOWN;
        }
    }

    WIN32_FF FromWxFontFamily(wxFontFamily  ff)
    {
        switch (ff)
        {
            case wxFONTFAMILY_DECORATIVE:
                return WIN32_FF::DECORATIVE;
            case wxFONTFAMILY_DEFAULT:
                return WIN32_FF::DONTCARE;
            case wxFONTFAMILY_MODERN:
                return WIN32_FF::MODERN;
            case wxFONTFAMILY_ROMAN:
                return WIN32_FF::ROMAN;
            case wxFONTFAMILY_SCRIPT:
                return WIN32_FF::SCRIPT;
            case wxFONTFAMILY_SWISS:
                return WIN32_FF::SWISS;
            default:
                wxASSERT(!"unknown family");
                return WIN32_FF::DONTCARE;
        }
    }
}
