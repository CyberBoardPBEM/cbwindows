// GmDoc.h : interface of the CGamDoc class
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

/////////////////////////////////////////////////////////////////////////////

#ifndef _GMDOC_H
#define _GMDOC_H

#include    <array>

#ifndef     _FONT_H
#include    "Font.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _BOARD_H
#include    "Board.h"
#endif

#ifndef     _PALTILE_H
#include    "PalTile.h"
#endif

#ifndef     _MAPSTRNG_H
#include    "MapStrng.h"
#endif

#ifndef     __VERSIONS_H__
#include    "Versions.h"
#endif

//////////////////////////////////////////////////////////////////////

#define     KEY_PASS_POSTFIX        "BaDkArMa"

//////////////////////////////////////////////////////////////////////
// Hints for UpdateAllViews/OnUpdate

enum CGamDocHint
{
    HINT_ALWAYSUPDATE =     0,      // Must be zero!

    HINT_TILECREATED =      1,      // pHint->m_tid
    HINT_TILEMODIFIED =     2,      // pHint->m_tid
    HINT_TILEDELETED =      3,      // pHint->m_tid
    HINT_TILEGROUP =        0x0F,   // Mask for all tile hints

    HINT_BOARDDELETED =     0x10,   // pHint->m_pBoard;
    HINT_TILESETDELETED =   0x11,   // pHint->m_nVal = tset num
    HINT_PIECESETDELETED =  0x12,   // pHint->m_nVal = pset num
    HINT_MARKSETDELETED =   0x13,   // pHint->m_nVal = mset num
    HINT_PIECEDELETED =     0x14,
    HINT_MARKERDELETED =    0x15,

    HINT_DELETEGROUP =      0x10,   // Set if delete hint group

    HINT_TILESETPROPCHANGE =0x20,
    HINT_BOARDPROPCHANGE =  0x21,   // pHint->m_pBoard
    HINT_PIECESETPROPCHANGE=0x22,
    HINT_MARKERSETPROPCHANGE=0x23,

    HINT_FORCETILEUPDATE =  0x40,   // Used before a save is done

    HINT_UPDATEPROJVIEW =   0x0100, // Used to reload prject window

    HINT_INVALID =          -1,     // uninitialized Args
};

class CGmBoxHint : public CObject
{
    DECLARE_DYNCREATE(CGmBoxHint);
public:
    CGmBoxHint() : hint(HINT_INVALID) {}

    template<CGamDocHint HINT>
    struct Args
    {
    };

    template<>
    struct Args<HINT_TILECREATED>
    {
        TileID m_tid;
    };

    template<>
    struct Args<HINT_TILEMODIFIED>
    {
        TileID m_tid;
    };

    template<>
    struct Args<HINT_TILEDELETED>
    {
        TileID m_tid;
    };

    template<>
    struct Args<HINT_BOARDDELETED>
    {
        CBoard*     m_pBoard;
    };

    template<>
    struct Args<HINT_TILESETDELETED>
    {
        size_t      m_tileSet;
    };

    template<>
    struct Args<HINT_PIECESETDELETED>
    {
        size_t      m_pieceSet;
    };

    template<>
    struct Args<HINT_MARKSETDELETED>
    {
        size_t      m_markSet;
    };

    template<>
    struct Args<HINT_BOARDPROPCHANGE>
    {
        CBoard*     m_pBoard;
    };

private:
    CGamDocHint hint;
    union {
        Args<HINT_TILECREATED> m_tileCreated;
        Args<HINT_TILEMODIFIED> m_tileModified;
        Args<HINT_TILEDELETED> m_tileDeleted;
        Args<HINT_BOARDDELETED> m_boardDeleted;
        Args<HINT_TILESETDELETED> m_tileSetDeleted;
        Args<HINT_PIECESETDELETED> m_pieceSetDeleted;
        Args<HINT_MARKSETDELETED> m_markSetDeleted;
        Args<HINT_BOARDPROPCHANGE> m_boardPropChange;
    } args;

public:
    template<CGamDocHint HINT>
    Args<HINT>& GetArgs()
    {
        if (hint == HINT_INVALID)
        {
            hint = HINT;
        }
        else if (HINT != hint)
        {
            CbThrowBadCastException();
        }
        return reinterpret_cast<Args<HINT>&>(args);
    }
};

//////////////////////////////////////////////////////////////////////

class CDib;
class CPieceManager;
class CMarkManager;

class CGamDoc : public CDocument
{
    friend class CGbxProjView;

protected:
    CGamDoc();
    DECLARE_DYNCREATE(CGamDoc)

// Class Global Attributes
public:
    static CFontTbl m_fontTbl;
    static CFontTbl* GetFontManager() { return &m_fontTbl; }
    // Version of file being loaded
    static int c_fileVersion;
    // load and save can't be simultaneous, so use for both
    static Features c_fileFeatures;

// Attributes
public:
    void ExportGamebox(const CB::string& pszPathName);

    static void SetLoadingVersion(int ver) { c_fileVersion = ver; }
    using SetLoadingVersionGuard = ::SetLoadingVersionGuard<CGamDoc>;
    static int GetLoadingVersion() { return c_fileVersion; }
    static void SetFileFeatures(Features&& feat) noexcept { c_fileFeatures = std::move(feat); }
    static void SetFileFeatures(const Features& feat) { SetFileFeatures(Features(feat)); }
    static const Features& GetFileFeatures() { return c_fileFeatures; }
    // -------- //
    DWORD GetGameBoxID() { return m_dwGameID; }
    // -------- //
    CBoardManager* GetBoardManager() { return m_pBMgr; }
    const CTileManager* GetTileManager() const { return &*m_pTMgr; }
    CTileManager* GetTileManager() { return const_cast<CTileManager*>(std::as_const(*this).GetTileManager()); }
    const CPieceManager* GetPieceManager() const { return m_pPMgr; }
    CPieceManager* GetPieceManager() { return const_cast<CPieceManager*>(std::as_const(*this).GetPieceManager()); }
    CMarkManager* GetMarkManager() { return m_pMMgr; }
    const CTilePalette* GetTilePalWnd() const { return &m_palTile; }
    // -------- //
    BOOL GetStickyDrawTools() { return m_bStickyDrawTools; }
    // -------- //
    void IncrMajorRevLevel();
    DWORD IssueGameBoxID();

    // If you need to pass a pointer to the views to be created,
    // bracket the view can call the GetCreateParameter() method.
    // It is only valid during the InitialUpdate() method.
    LPVOID GetCreateParameter() { return m_lpvCreateParam; }

// Operations
public:
    BOOL SetupBlankBoard();

    BOOL CreateNewFrame(CDocTemplate* pTemplate, const CB::string& pszTitle,
        LPVOID lpvCreateParam = NULL);

    BOOL NotifyTileDatabaseChange(BOOL bPurgeScan = TRUE);
    BOOL PurgeMissingTileIDs();
    BOOL QueryTileInUse(TileID tid) const;
    BOOL QueryAnyOfTheseTilesInUse(const std::vector<TileID>& tbl) const;

    TileID CreateTileFromDib(CDib* pDib, size_t nTSet);
    CView* FindTileEditorView(TileID tid);
    CView* FindBoardEditorView(const CBoard& pBoard);

    // Support for strings associated with game elements (pieces, markers)
    CB::string  GetGameElementString(GameElement gelem) const;
    BOOL        HasGameElementString(GameElement gelem) const;

    BOOL DoBoardPropertyDialog(CBoard& pBoard);

    void DoGbxProperties() { OnEditGbxProperties(); }
    void DoCreateBoard() { OnEditCreateBoard(); }
    void DoCreateTileGroup() { OnEditCreateTileGroup(); }
    void DoCreatePieceGroup() { OnEditCreatePieceGroup(); }
    void DoCreateMarkGroup() { OnEditCreateMarkGroup(); }

    int  GetCompressLevel() { return (int)m_wCompressLevel; }
    void SetCompressLevel(int nCompressLevel) { m_wCompressLevel = (WORD)nCompressLevel; }

    LPVOID GetCustomColors();
    void   SetCustomColors(LPVOID pCustColors);

    const CGameElementStringMap& GetGameStringMap() const { return m_mapStrings; }
    CGameElementStringMap& GetGameStringMap() { return const_cast<CGameElementStringMap&>(std::as_const(*this).GetGameStringMap()); }

    std::array<std::byte, 16> ComputeGameboxPasskey(const CB::string& pszPassword);
    void ClearGameboxPasskey();

    void OnFileClose() { CDocument::OnFileClose(); }    // Expose protected

    // OnIdle is called by the App object to inform
    // documents of idle condition. A flag indicates if
    // this is the active document.
    void OnIdle(BOOL bActive);

    // Forced override of this (note not virtual)
    void UpdateAllViews(CView* pSender, LPARAM lHint = 0L,
        CObject* pHint = NULL);

// Implementation
protected:
    BYTE            m_abyteBoxID[16];// Special hashed UUID assigned to this gamebox.
    WORD            m_wReserved1;   // For future need (set to 0)
    WORD            m_wReserved2;   // For future need (set to 0)
    WORD            m_wReserved3;   // For future need (set to 0)
    WORD            m_wReserved4;   // For future need (set to 0)
    WORD            m_nBitsPerPixel;// Geometry of bitmaps (4bpp or 8bpp)
    DWORD           m_dwMajorRevs;  // Major revisions (stuff was deleted)
    DWORD           m_dwMinorRevs;  // Minor revisions (stuff was added)
    DWORD           m_dwGameID;     // Unique autogened signature for gamebox
    CB::string      m_strAuthor;    // Game box author (<= 32 chars)
    CB::string      m_strTitle;     // Game box tile (<= 64 chars)
    CB::string      m_strDescr;     // Game box description (<= 64 chars)
    std::array<std::byte, 16> m_abytePass;// MD5 Hash of password and uuid based box ID
    WORD            m_wCompressLevel;// Amount of compression to apply to bitmaps and such
    CGameElementStringMap m_mapStrings; // Mapping of pieces and markers to strings.
    LPVOID          m_pCustomColors; // Container for custom edit colors
    CB::propagate_const<CTileManager*>   m_pTMgr;        // Tiles
    CBoardManager*  m_pBMgr;        // Playing boards
    CPieceManager*  m_pPMgr;        // Playing pieces
    CMarkManager*   m_pMMgr;        // Annotation markers
    BOOL            m_bStickyDrawTools; // If TRUE, don't select the select tool after drawing

    BOOL            m_bMajorRevIncd;// Major rev number was increased.

    CTilePalette    m_palTile;      // Tile palette child window is in document

    LPVOID  m_lpvCreateParam;       // Used to pass parameters to new views

public:
    virtual ~CGamDoc();
    virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
    virtual BOOL OnSaveDocument(LPCTSTR pszPathName) override;
    virtual void DeleteContents();
    virtual void OnCloseDocument();

// Generated message map functions
protected:
    afx_msg void OnEditGbxProperties();
    afx_msg void OnEditCreateBoard();
    afx_msg void OnEditCreateTileGroup();
    afx_msg void OnEditCreatePieceGroup();
    afx_msg void OnEditCreateMarkGroup();
    afx_msg void OnProjectChangeFingerPrint();
    afx_msg void OnStickyDrawTools();
    afx_msg void OnUpdateStickyDrawTools(CCmdUI* pCmdUI);
    afx_msg void OnDumpTileData();
    afx_msg void OnBugFixDumpBadTiles();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnExportGamebox();
};

#endif

