// Trays.cpp
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

#ifndef _TRAYS_H
#define _TRAYS_H

#ifndef     _PPIECES_H
#include    "PPieces.h"
#endif

//////////////////////////////////////////////////////////////////////

class CGamDoc;

//////////////////////////////////////////////////////////////////////

enum TrayViz            // Tray content visiblity options
{
    trayVizTwoSide = 0,
    trayVizOneSide = 1,
    trayVizEachGeneric = 2,
    trayVizNone = 3,
    trayVizOwnEnforceFlag = 0x8000        // Set/Checked when serializing
};

class CTraySet
{
public:
    CTraySet();

// Attributes
public:
    CWordArray* GetPieceIDTable() { return &m_pidTbl; }
    BOOL IsEmpty() { return m_pidTbl.GetSize() == 0; }
    BOOL HasPieceID(PieceID pid);
    int GetPieceIDIndex(PieceID pid);

    const char* GetName() const { return m_strName; }
    void SetName(const char *pszName) { m_strName = pszName; }

    BOOL IsRandomPiecePull() { return m_bRandomPull; }
    void SetRandPiecePull(BOOL bRandomPull) { m_bRandomPull = bRandomPull; }

    TrayViz GetTrayContentVisibility() { return m_eTrayViz; }
    void SetTrayContentVisibility(TrayViz eTrayViz) { m_eTrayViz = eTrayViz; }

    void SetEnforceVisibilityForOwnerToo(BOOL bEnforce) { m_bEnforceVizForOwnerToo = bEnforce; }
    BOOL IsEnforcingVisibilityForOwnerToo() { return m_bEnforceVizForOwnerToo; }

    BOOL IsNonOwnerAccessAllowed() { return m_bNonOwnerAccess; }
    void SetNonOwnerAccess(BOOL bAllowAccess) { m_bNonOwnerAccess = bAllowAccess; }

    DWORD GetOwnerMask() { return m_dwOwnerMask; }
    void SetOwnerMask(DWORD dwMask) { m_dwOwnerMask = dwMask; }
    BOOL IsOwned() { return m_dwOwnerMask != 0; }
    BOOL IsOwnedBy(DWORD dwMask) { return (BOOL)(m_dwOwnerMask & dwMask); }
    BOOL IsOwnedButNotByCurrentPlayer(CGamDoc* pDOc);

// Operations
public:
    void AddPieceID(PieceID pid, int nPos = -1);
    void RemovePieceID(PieceID pid);

    void AddPieceList(CWordArray *pTbl, int nPos = -1);
    void RemovePieceList(CWordArray *pTbl);

    void PropagateOwnerMaskToAllPieces(CGamDoc* pDoc);

    CTraySet* Clone(CGamDoc *pDoc);
    void Restore(CGamDoc *pDoc, CTraySet* pTbl);
    BOOL Compare(CTraySet* pYGrp);

    void Serialize(CArchive& ar);

// Implementation
protected:
    CString     m_strName;
    CWordArray  m_pidTbl;           // PieceIDs in this set.

    DWORD     m_dwOwnerMask;        // Who can change the tray (0=no owners)
    BOOL      m_bNonOwnerAccess;    // Allow non-owner access. Visiblity is still enforced.
    BOOL      m_bRandomPull;        // Pieces are randomly selected on drags
    BOOL      m_bEnforceVizForOwnerToo; // m_eTrayViz applies to owner too
    TrayViz   m_eTrayViz;           // What can be seen
};

//////////////////////////////////////////////////////////////////////

class CTrayManager
{
public:
    CTrayManager();
    ~CTrayManager();

// Attributes
public:
    int GetNumTraySets() const { return m_YSetTbl.GetSize(); }
    CTraySet* GetTraySet(int nYSet)
        { return (CTraySet*)m_YSetTbl.GetAt(nYSet); }
    void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }

// Operations
public:
    int CreateTraySet(const char* pszName);
    void DeleteTraySet(int nYSet);
    void RemovePieceIDFromTraySets(PieceID pid);
    CTraySet* FindPieceIDInTraySet(PieceID pid);
    int FindTrayByName(const char* strName);
    int FindTrayByPtr(CTraySet* pYSet);

    void Clear();

    void ClearAllOwnership();
    void PropagateOwnerMaskToAllPieces(CGamDoc* pDoc);

    CTrayManager* Clone(CGamDoc *pDoc);
    void Restore(CGamDoc *pDoc, CTrayManager* pMgr);
    BOOL Compare(CTrayManager* pYMgr);

    void Serialize(CArchive& ar);
    void SerializeTraySets(CArchive& ar);

// Implementation
protected:
    CPtrArray   m_YSetTbl;      // Table of tray set pointers
    WORD        m_wReserved1;   // For future need (set to 0)
    WORD        m_wReserved2;   // For future need (set to 0)
    WORD        m_wReserved3;   // For future need (set to 0)
    WORD        m_wReserved4;   // For future need (set to 0)
    // ------- //
    CTileManager* m_pTMgr;      // Supporting tile manager
};

#endif

