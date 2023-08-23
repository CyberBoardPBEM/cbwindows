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

#ifndef _FONT_H
#define _FONT_H

#include    <map>

#ifndef     _FONTNAME_H
#include    "FontName.h"
#endif

class CbFont;
typedef AtomList<CbFont>::AtomID FontID;

const int taBold   = 0x0001;
const int taItalic = 0x0002;
const int taULine  = 0x0004;

class CbFont
{
    friend class CFontTbl;
protected:
    FNameID fnID;
    int     iTypeSize;          // Type height in 0.5 pt units
    int     taFlags;            // See ta* defs above
    // mutable since it's approximately a lazy evaluation of ctor
    typedef std::map<int /* angle */, HFONT> HFonts;
    mutable HFonts hFnt;        // Handle of font in system
public:
    // ---------- //
    bool operator==(const CbFont& rhs) const;

    CbFont(void) : fnID(0), iTypeSize(0), taFlags(0) {}
    CbFont(int iSize, int taFlgs, FNameID id) :
        fnID(id), iTypeSize(iSize), taFlags(taFlgs) {}
    ~CbFont();
    // --------- //
    bool IsBold(void) const { return (taFlags & taBold) != 0; }
    bool IsItalic(void) const { return (taFlags & taItalic) != 0; }
    bool IsULine(void) const { return (taFlags & taULine) != 0; }
    CB::string ToString(int angle = 0) const;
};

template<typename CharT>
struct std::formatter<CbFont, CharT> : private std::formatter<CB::string, CharT>
{
private:
    using BASE = formatter<CB::string, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const CbFont& f, FormatContext& ctx)
    {
        return BASE::format(f.ToString(), ctx);
    }
};

class CFontTbl : private AtomList<CbFont>
{
protected:
    FNameTbl oFName;
public:
    virtual ~CFontTbl(void);
    // -------- //
    FNameTbl& GetFNameTbl(void) { return oFName; }
    // -------- //
    FontID AddFont(int iSize, int taFlgs, uint8_t iFamily, const CB::string& pszFName);
    // N.B.:  angle is clockwise per CB convention
    void FillLogFontStruct(FontID id, LPLOGFONT pLF, int angle = 0);
    HFONT GetFontHandle(FontID id, int angle = 0);
    void ReleaseAllFontHandles(void);
    // -------- //
    int GetFlags(FontID id) const { return id != 0 ? (*id)->taFlags : 0; }
    int GetSize(FontID id) const { return id != 0 ? (*id)->iTypeSize : 0; }
    int GetFamily(FontID id) const
        { return oFName.GetFaceFamily((*id)->fnID); }
    const CB::string* GetFaceName(FontID id) const
        { return oFName.GetFaceName((*id)->fnID); }
    FNameTbl* GetFontNameTable() { return &oFName; }
    // -------- //
    void Archive(CArchive& ar, FontID& rfontID);
};

#endif

