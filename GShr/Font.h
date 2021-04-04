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

#ifndef _FONT_H
#define _FONT_H

#ifndef     _FONTNAME_H
#include    "FontName.h"
#endif

typedef AtomID FontID;          // Redefine for clarity

const int taBold   = 0x0001;
const int taItalic = 0x0002;
const int taULine  = 0x0004;

class CbFont : public Atom
{
    friend class CFontTbl;
protected:
    FNameID fnID;
    int     iTypeSize;          // Type height in 0.5 pt units
    int     taFlags;            // See ta* defs above
    HFONT   hFnt;               // Handle of font in system
    // ---------- //
    virtual BOOL AtomsEqual(Atom &atom);
public:
    CbFont(void) : fnID(0), iTypeSize(0), taFlags(0), hFnt(NULL) {}
    CbFont(int iSize, int taFlgs, FNameID id) :
        fnID(id), iTypeSize(iSize), taFlags(taFlgs), hFnt(NULL) {}
    // --------- //
    BOOL IsBold(void) { return (taFlags & taBold) != 0; }
    BOOL IsItalic(void) { return (taFlags & taItalic) != 0; }
    BOOL IsULine(void) { return (taFlags & taULine) != 0; }
};

class UniqueFontID
{
public:
    UniqueFontID() noexcept = default;
    UniqueFontID(const UniqueFontID&) = delete;
    UniqueFontID& operator=(const UniqueFontID&) = delete;
    UniqueFontID(UniqueFontID&& other) noexcept { fid = other.fid; other.fid = 0; }
    UniqueFontID& operator=(UniqueFontID&& other) noexcept { std::swap(fid, other.fid); return *this; }
    ~UniqueFontID() { Reset(); }
    void Reset(FontID f = 0);
    FontID Get() const { return fid; }
private:
    FontID fid = 0;
};

class CFontTbl : public AtomList
{
protected:
    FNameTbl oFName;
    // -------- //
    virtual void Destroy(Atom *opAtom);
public:
    virtual ~CFontTbl(void);
    // -------- //
    FNameTbl& GetFNameTbl(void) { return oFName; }
    // -------- //
    void DeleteFont(FontID id) { DeleteAtom((AtomID)id); }
    FontID AddFont(FontID id) { return AddAtom((AtomID)id); }
    FontID AddFont(int iSize, int taFlgs, int iFamily, const char *pszFName);
    void FillLogFontStruct(FontID id, LPLOGFONT pLF);
    HFONT GetFontHandle(FontID id);
    void ReleaseFontHandle(FontID id);
    void ReleaseAllFontHandles(void);
    // -------- //
    int GetFlags(FontID id) const { return id != 0 ? ((CbFont *)id)->taFlags : 0; }
    int GetSize(FontID id) const { return id != 0 ? ((CbFont *)id)->iTypeSize : 0; }
    int GetFamily(FontID id) const
        { return oFName.GetFaceFamily(((CbFont *)id)->fnID); }
    const char* GetFaceName(FontID id) const
        { return oFName.GetFaceName(((CbFont *)id)->fnID); }
    FNameTbl* GetFontNameTable() { return &oFName; }
    // -------- //
    void Archive(CArchive& ar, FontID& rfontID);
    void Archive(CArchive& ar, UniqueFontID& rfontID);
};

#endif

