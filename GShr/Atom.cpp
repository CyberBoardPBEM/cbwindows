// Atom.cpp
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

// -------------------------------------------------- //
// These mimic ATOM management in Windows that store
// strings that would normally be redundant.

#include    "stdafx.h"
#include    "atom.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ====================================================== //

AtomList::~AtomList(void)
{
    Atom *opALst = opAList;
    while (opALst != NULL)
    {
        Atom *opANxt = opALst->next;    // Pick up next sice we'll delete
        Destroy(opALst);
        opALst = opANxt;                // Chain to next entry
    }
}

// ------------------------------------------------------ //

AtomID AtomList::AddAtom(AtomID aID)
{
    if (aID != 0)
    {
        ASSERT(opAList != NULL);
        ((Atom *)aID)->nRefCnt++;
    }
    return aID;
}

// ------------------------------------------------------ //

AtomID AtomList::AddAtom(Atom *opAtom)
{
    opAtom->next = opAList;
    opAList = opAtom;
    return (AtomID)opAtom;
}

// ------------------------------------------------------ //

AtomID AtomList::AddIfExists(Atom *opAtom)
{
    ASSERT(opAtom != NULL);

    Atom *opALst = opAList;
    while (opALst != NULL)
    {
        if (opAtom->AtomsEqual(*opALst))
        {
            opALst->nRefCnt++;
            return (AtomID)opALst;
        }
        opALst = opALst->next;
    }
    return (AtomID)0;
}

// ------------------------------------------------------ //

void AtomList::DestroyAtom(AtomID id)
{
    if (id == 0) return;
    ASSERT(opAList != NULL);
    ((Atom *)id)->nRefCnt = 1;
    DeleteAtom(id);
}

// ------------------------------------------------------ //

void AtomList::DeleteAtom(AtomID id)
{
    if (id == 0) return;
    ASSERT(opAList != NULL);

    Atom *opALst = opAList;
    if ((AtomID)opALst == id)               // Checks head of list
    {
        if (--(opALst->nRefCnt)) return;
        opAList = opALst->next;             // Unlink
        Destroy(opALst);
    }
    else
    {
        Atom *opAPrv = opAList;             // Previous atom object
        opALst = opAPrv->next;
        while (opALst != NULL)
        {
            if ((AtomID)opALst == id)
            {
                if (--(opALst->nRefCnt)) return;
                opAPrv->next = opALst->next;// Unlink
                Destroy(opALst);
                return;
            }
            opAPrv = opALst;
            opALst = opALst->next;          // Chain to next entry
        }
    }
}
