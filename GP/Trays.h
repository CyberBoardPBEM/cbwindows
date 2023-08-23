// Trays.cpp
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

#ifndef _TRAYS_H
#define _TRAYS_H

#ifndef     _PPIECES_H
#include    "PPieces.h"
#endif

//////////////////////////////////////////////////////////////////////

class CGamDoc;

//////////////////////////////////////////////////////////////////////

enum TrayViz : int            // Tray content visiblity options
{
    trayVizAllSides = 0,
    trayVizOneSide = 1,
    trayVizEachGeneric = 2,
    trayVizNone = 3,
    trayVizOwnEnforceFlag = 0x8000        // Set/Checked when serializing
};

class CTraySet
{
public:
    CTraySet();
    CTraySet(const CTraySet&) = delete;
    CTraySet& operator=(const CTraySet&) = delete;
    CTraySet(CTraySet&&) = default;
    CTraySet& operator=(CTraySet&&) = default;
    ~CTraySet() = default;

// Attributes
public:
    const std::vector<PieceID>& GetPieceIDTable() const { return m_pidTbl; }
    BOOL IsEmpty() const { return m_pidTbl.empty(); }
    BOOL HasPieceID(PieceID pid) const;
    size_t GetPieceIDIndex(PieceID pid) const;

    const CB::string& GetName() const { return m_strName; }
    void SetName(CB::string pszName) { m_strName = std::move(pszName); }

    BOOL IsRandomPiecePull() { return m_bRandomPull; }
    void SetRandPiecePull(BOOL bRandomPull) { m_bRandomPull = bRandomPull; }
    bool IsRandomSidePull() { return m_bRandomSidePull; }
    void SetRandSidePull(bool bRandomPull) { m_bRandomSidePull = bRandomPull; }

    TrayViz GetTrayContentVisibility() { return m_eTrayViz; }
    void SetTrayContentVisibility(TrayViz eTrayViz) { m_eTrayViz = eTrayViz; }

    void SetEnforceVisibilityForOwnerToo(BOOL bEnforce) { m_bEnforceVizForOwnerToo = bEnforce; }
    BOOL IsEnforcingVisibilityForOwnerToo() { return m_bEnforceVizForOwnerToo; }

    BOOL IsNonOwnerAccessAllowed() { return m_bNonOwnerAccess; }
    void SetNonOwnerAccess(BOOL bAllowAccess) { m_bNonOwnerAccess = bAllowAccess; }

    DWORD GetOwnerMask() { return m_dwOwnerMask; }
    void SetOwnerMask(DWORD dwMask) { m_dwOwnerMask = dwMask; }
    BOOL IsOwned() const { return m_dwOwnerMask != 0; }
    BOOL IsOwnedBy(DWORD dwMask) const { return (BOOL)(m_dwOwnerMask & dwMask); }
    BOOL IsOwnedButNotByCurrentPlayer(const CGamDoc& pDOc) const;

// Operations
public:
    void AddPieceID(PieceID pid, size_t nPos = Invalid_v<size_t>);
    void RemovePieceID(PieceID pid);

    void AddPieceList(const std::vector<PieceID>& pTbl, size_t nPos = Invalid_v<size_t>);
    void RemovePieceList(const std::vector<PieceID>& pTbl);

    void PropagateOwnerMaskToAllPieces(CGamDoc* pDoc);

    CTraySet Clone(CGamDoc *pDoc) const;
    void Restore(CGamDoc *pDoc, const CTraySet& pTbl);
    BOOL Compare(const CTraySet& pYGrp) const;

    void Serialize(CArchive& ar);

// Implementation
protected:
    CB::string m_strName;
    std::vector<PieceID> m_pidTbl;

    DWORD     m_dwOwnerMask;        // Who can change the tray (0=no owners)
    BOOL      m_bNonOwnerAccess;    // Allow non-owner access. Visiblity is still enforced.
    BOOL      m_bRandomPull;        // Pieces are randomly selected on drags
    bool      m_bRandomSidePull;    // Piece sidess are randomly selected on drags
    BOOL      m_bEnforceVizForOwnerToo; // m_eTrayViz applies to owner too
    TrayViz   m_eTrayViz;           // What can be seen
};

//////////////////////////////////////////////////////////////////////

class CTrayManager
{
public:
    CTrayManager();
    CTrayManager(const CTrayManager&) = delete;
    CTrayManager& operator=(const CTrayManager&) = delete;
    CTrayManager(CTrayManager&&) = default;
    CTrayManager& operator=(CTrayManager&&) = default;
    ~CTrayManager() = default;

// Attributes
public:
    size_t GetNumTraySets() const { return m_YSetTbl.size(); }
    const CTraySet& GetTraySet(size_t nYSet) const
        { return m_YSetTbl.at(nYSet); }
    CTraySet& GetTraySet(size_t nYSet)
    {
        return const_cast<CTraySet&>(std::as_const(*this).GetTraySet(nYSet));
    }
    void SetTileManager(CTileManager* pTMgr) { m_pTMgr = pTMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }

// Operations
public:
    size_t CreateTraySet(CB::string pszName);
    void DeleteTraySet(size_t nYSet);
    void RemovePieceIDFromTraySets(PieceID pid);
    CTraySet* FindPieceIDInTraySet(PieceID pid);
    size_t FindTrayByName(const CB::string& strName) const;
    size_t FindTrayByRef(const CTraySet& pYSet) const;

    void Clear();

    void ClearAllOwnership();
    void PropagateOwnerMaskToAllPieces(CGamDoc* pDoc);

    CTrayManager Clone(CGamDoc *pDoc) const;
    void Restore(CGamDoc *pDoc, const CTrayManager& pMgr);
    BOOL Compare(const CTrayManager& pYMgr) const;

    void Serialize(CArchive& ar);
    void SerializeTraySets(CArchive& ar);

// Implementation
protected:
    std::vector<CTraySet> m_YSetTbl;
    WORD        m_wReserved1;   // For future need (set to 0)
    WORD        m_wReserved2;   // For future need (set to 0)
    WORD        m_wReserved3;   // For future need (set to 0)
    WORD        m_wReserved4;   // For future need (set to 0)
    // ------- //
    CTileManager* m_pTMgr;      // Supporting tile manager
};

#endif

