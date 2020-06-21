// GameBox.h
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
    // Current Tile Manager. Only valid when Serializing
    static CTileManager* c_pTileMgr;
    // Version of file being loaded
    static int c_gbxFileVersion;

// Static class functions
public:
    // Current Tile Manager. Only valid when Serializing
//  static void SetCurrentTileManager(CTileManager* pMgr) { c_pTileMgr = pMgr; }
//  static CTileManager* GetCurrentTileManager() { return c_pTileMgr; }

    static CFontTbl* GetFontManager() { return &m_fontTbl; }

    static void SetLoadingVersion(int ver) { c_gbxFileVersion = ver; }
    static int GetLoadingVersion() { return c_gbxFileVersion; }

public:
    CGameBox();
    ~CGameBox();
// Attributes
public:
    CBoardManager* GetBoardManager() { return m_pBMgr; }
    CTileManager* GetTileManager() { return m_pTMgr; }
    CPieceManager* GetPieceManager() { return m_pPMgr; }
    CMarkManager* GetMarkManager() { return m_pMMgr; }

    CGameElementStringMap& GetGameBoxStringMap() { return m_mapStrings; }

// Operations
public:
    BOOL Load(CGamDoc* pDoc, LPCSTR pszPathName, CString& strErr,
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
    CTileManager*   m_pTMgr;        // Tiles
    CBoardManager*  m_pBMgr;        // Playing boards
    CPieceManager*  m_pPMgr;        // Playing pieces
    CMarkManager*   m_pMMgr;        // Annotation markers
};

#endif


