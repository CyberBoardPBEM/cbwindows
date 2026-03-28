// GameBox.h
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

#ifndef _GAMEBOX_H
#define _GAMEBOX_H

//////////////////////////////////////////////////////////////

#ifndef     _FONT_H
#include    "Font.h"
#endif

#ifndef     _MAPSTRNG_H
#include    "MapStrng.h"
#endif

//////////////////////////////////////////////////////////////

class CGamDoc;
class CBoardManager;
class CTileManager;
class CPieceManager;
class CMarkManager;

//////////////////////////////////////////////////////////////

class CGameBox
{
    static CFontTbl m_fontTbl;
    // Version of file being loaded
    static int c_gbxFileVersion;

// Static class functions
public:
    static CFontTbl& GetFontManager() { return m_fontTbl; }

    static void SetLoadingVersion(int ver) { c_gbxFileVersion = ver; }
    static int GetLoadingVersion() { return c_gbxFileVersion; }

public:
    CGameBox();
    ~CGameBox() = default;
// Attributes
public:
    const CBoardManager& GetBoardManager() const { return CheckedDeref(m_pBMgr); }
    CBoardManager& GetBoardManager() { return const_cast<CBoardManager&>(std::as_const(*this).GetBoardManager()); }
    const CTileManager& GetTileManager() const { return CheckedDeref(m_pTMgr); }
    CTileManager& GetTileManager() { return const_cast<CTileManager&>(std::as_const(*this).GetTileManager()); }
    const CPieceManager& GetPieceManager() const { return CheckedDeref(m_pPMgr); }
    CPieceManager& GetPieceManager() { return const_cast<CPieceManager&>(std::as_const(*this).GetPieceManager()); }
    const CMarkManager& GetMarkManager() const { return CheckedDeref(m_pMMgr); }
    CMarkManager& GetMarkManager() { return const_cast<CMarkManager&>(std::as_const(*this).GetMarkManager()); }

    const CGameElementStringMap& GetGameBoxStringMap() const { return m_mapStrings; }
    CGameElementStringMap& GetGameBoxStringMap() { return const_cast<CGameElementStringMap&>(std::as_const(*this).GetGameBoxStringMap()); }

// Operations
public:
    BOOL Load(CGamDoc& pDoc, const CB::string& pszPathName, CB::string& strErr,
        DWORD dwGbxID = 0);

// Vars...
public:
    WORD            m_nBitsPerPixel;// Geometry of bitmaps (4bpp or 8bpp)
    DWORD           m_dwMajorRevs;  // Major revisions (stuff was deleted)
    DWORD           m_dwMinorRevs;  // Minor revisions (stuff was added)
    DWORD           m_dwGameID;     // Unique ID for this game box

    CGameElementStringMap m_mapStrings; // Mapping of pieces and markers to strings.


    // Note...later we'll need to discard things not needed by
    // game play such as tile sets...
    // Note2:  reordered members to preserve deletion order
    OwnerOrNullPtr<CMarkManager>   m_pMMgr;        // Annotation markers
    OwnerOrNullPtr<CPieceManager>  m_pPMgr;        // Playing pieces
    OwnerOrNullPtr<CBoardManager>  m_pBMgr;        // Playing boards
    OwnerOrNullPtr<CTileManager>   m_pTMgr;        // Tiles
};

#endif


