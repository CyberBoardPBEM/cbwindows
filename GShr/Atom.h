// Atom.h
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

#ifndef _ATOM_H
#define _ATOM_H

typedef unsigned long AtomID;

class Atom
{
    friend class AtomList;
protected:
    Atom *next;             // Pointer to next atom in list
    int nRefCnt;            // Number of references to this atom
    // ------ //
    Atom(void) : nRefCnt(1), next(NULL) {}
    virtual ~Atom(void) {}  // Virtual for proper destruction
    // ------ //
    virtual BOOL AtomsEqual(Atom &atom) = 0;
};

class AtomList
{
protected:
    Atom *opAList;
    // ----------- //
    virtual void Destroy(Atom *opAtom) { delete opAtom; }
    // ----------- //
    AtomID AddIfExists(Atom *opAtom);
    AtomID AddAtom(Atom *opAtom);
    AtomID AddAtom(AtomID aID);
    void DeleteAtom(AtomID aID);
    void DestroyAtom(AtomID aID);
    // ----------- //
    AtomList(void) : opAList(NULL) {}
    virtual ~AtomList(void);
};

#endif
