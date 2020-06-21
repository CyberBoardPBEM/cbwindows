// FontName.cpp
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

#ifndef _FONTNAME_H
#define _FONTNAME_H

#ifndef     _ATOM_H
#include    "Atom.h"
#endif

typedef AtomID FNameID;     // Redefine for clarity

class FName : public Atom
{
    friend class FNameTbl;
protected:
    int iFamily;                    // See LOGFONT FF_*
    char szFName[LF_FACESIZE];      // Name of font
    // ---------- //
    virtual BOOL FName::AtomsEqual(Atom &atom);
    // ---------- //
public:
    FName(void) { iFamily = 0; *szFName = 0; }
    FName(const char *pszFName, int iFamily);
};

// ------------------------------------------------------ //

class FNameTbl : public AtomList
{
public:
    FNameID AddFaceName(FNameID id) { return AddAtom((AtomID)id); }
    FNameID AddFaceName(const char *pszFName, int iFamily);
    const char *GetFaceName(FNameID id) const
        { return id == NULL ? NULL : ((FName *)id)->szFName;}
    int GetFaceFamily(FNameID id) const
        { return id == NULL ? 0 : ((FName *)id)->iFamily;}
    void DelFaceName(FNameID id) { DeleteAtom((AtomID)id); }
    void DestroyFaceName(FNameID id) { DestroyAtom((AtomID)id); }
    // if iFaceName == -1, return number of face names
    // if iFaceName >= 0 && pszFName == NULL, return stringlength.
    // else return the face name and family id
    int GetFaceInfo(int iFaceNum, char *pszFName=NULL, int* iFamily=0);
};

#endif

