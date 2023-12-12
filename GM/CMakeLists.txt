# Copyright (c) 2023 By William Su, All Rights Reserved.

# GM CBDesign

set(SRCS
	ClipBrd.cpp
	DlgBmask.cpp
	DlgBrdp.cpp
	DlgBrdsz.cpp
	DlgGboxp.cpp
	DlgGPass.cpp
	DlgMakts.cpp
	DlgMEditMulti.cpp
	DlgMedt.cpp
	DlgMgrpn.cpp
	DlgMkbrd.cpp
	DlgMnew.cpp
	DlgMrkp.cpp
	DlgNtile.cpp
	DlgNum.cpp
	DlgPaste.cpp
	DlgPcep.cpp
	DlgPEditMulti.cpp
	DlgPedt.cpp
	DlgPgrpn.cpp
	DlgPnew.cpp
	DlgSPass.cpp
	DlgSvisi.cpp
	DlgTexto.cpp
	DlgTilsz.cpp
	DlgTsetp.cpp
	FrmBited.cpp
	FrmBxdoc.cpp
	FrmDockTile.cpp
	FrmMain.cpp
	FrmView.cpp
	Gm.cpp
	GmDoc.cpp
	LBoxTile.cpp
	PalColor.cpp
	PalTile.cpp
	SelObjs.cpp
	StdAfx.cpp
	ToolImag.cpp
	ToolObjs.cpp
	VwBitedt.cpp
	VwEdtbrd.cpp
	VwPrjgb1.cpp
	VwPrjgbx.cpp
	VwTilesl.cpp
)

set(SRCS
	${SRCS}
	../GShr/Arclib.cpp
	../GShr/Board.cpp
	../GShr/BrdCell.cpp
	../GShr/CalcLib.cpp
	../GShr/CDib.cpp
	../GShr/CellForm.cpp
	../GShr/Color.cpp
	../GShr/DragDrop.cpp
	../GShr/DrawObj.cpp
	../GShr/Font.cpp
	../GShr/FontName.cpp
	../GShr/GdiTools.cpp
	../GShr/LBoxGrfx.cpp
	../GShr/LBoxMark.cpp
	../GShr/LBoxPiec.cpp
	../GShr/LBoxProj.cpp
	../GShr/LBoxTileBase.cpp
	../GShr/LibMfc.cpp
	../GShr/MapStrng.cpp
	../GShr/Marks.cpp
	../GShr/MD5.cpp
	../GShr/Pieces.cpp
	../GShr/ResTbl.cpp
	../GShr/Rotate.cpp
	../GShr/StrLib.cpp
	../GShr/Tile.cpp
	../GShr/TileMgr.cpp
	../GShr/TileSet.cpp
	../GShr/TileSht.cpp
)

if (MSVC)
	set(SRCS
		${SRCS}
		Cbdesign.rc
	)
endif()

add_executable(CBDesign WIN32 ${SRCS})

if (MSVC)
	target_compile_definitions(CBDesign PUBLIC
		UNICODE _UNICODE
		_AFX_SECURE_NO_WARNINGS _ATL_SECURE_NO_WARNINGS
		_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
	)
	target_link_libraries(CBDesign htmlhelp)
	# Unicode MFC expects this entry point
	target_link_options(CBDesign PRIVATE /ENTRY:wWinMainCRTStartup)
endif()

target_include_directories(CBDesign PRIVATE . ../GShr)
target_precompile_headers(CBDesign PRIVATE StdAfx.h)
target_link_libraries(CBDesign wx::core)
