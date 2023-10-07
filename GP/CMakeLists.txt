# Copyright (c) 2023 By William Su, All Rights Reserved.

# GP CBPlay

set(SRCS
	DlgChgGameOwner.cpp
	DlgDice.cpp
	DlgEdtEl.cpp
	DlgEPly.cpp
	DlgItray.cpp
	DlgMarkCount.cpp
	DlgMdesc.cpp
	DlgMPly.cpp
	DlgNewGeoBoard.cpp
	DlgNPly.cpp
	DlgPass.cpp
	DlgPbprp.cpp
	DlgRot.cpp
	DlgRotpc.cpp
	DlgScnp.cpp
	DlgSelOwner.cpp
	DlgSlbrd.cpp
	DlgSmsg.cpp
	DlgSpece.cpp
	DlgState.cpp
	DlgTrunc.cpp
	DlgYnew.cpp
	DlgYprop.cpp
	Editnocv.cpp
	FileLib.cpp
	FrmDockMark.cpp
	FrmDockTray.cpp
	FrmMain.cpp
	FrmPbrd.cpp
	FrmProj.cpp
	GamDoc.cpp
	GamDoc1.cpp
	GamDoc2.cpp
	GamDoc3.cpp
	GamDoc4.cpp
	GamDoc5.cpp
	GameBox.cpp
	GamState.cpp
	GeoBoard.cpp
	Gp.cpp
	LBoxSlct.cpp
	LBoxTray.cpp
	MapFace.cpp
	MoveHist.cpp
	MoveMgr.cpp
	PalMark.cpp
	PalReadMsg.cpp
	PalTray.cpp
	PBoard.cpp
	Player.cpp
	PPieces.cpp
	SelOPlay.cpp
	StdAfx.cpp
	ToolPlay.cpp
	Trays.cpp
	VwPbrd.cpp
	VwPbrd1.cpp
	VwPrjga1.cpp
	VwPrjgam.cpp
	VwPrjgs1.cpp
	VwPrjgsn.cpp
	VwSelpce.cpp
	VwTbrd.cpp
	WinDynspl.cpp
	WinMyspl.cpp
	WinPoptb.cpp
	WStateGp.cpp
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
	../GShr/LBoxGfx2.cpp
	../GShr/LBoxGrfx.cpp
	../GShr/LBoxMark.cpp
	../GShr/LBoxPiec.cpp
	../GShr/LBoxProj.cpp
	../GShr/LBoxTileBase.cpp
	../GShr/LBoxTileBase2.cpp
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
	../GShr/WinState.cpp
)

if (MSVC)
	set(SRCS
		${SRCS}
		Cbplay.rc
	)
endif()

add_executable(CBPlay WIN32 ${SRCS})

if (MSVC)
	target_compile_definitions(CBPlay PUBLIC
		UNICODE _UNICODE
		_AFX_SECURE_NO_WARNINGS _ATL_SECURE_NO_WARNINGS
		_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
	)
	target_link_libraries(CBPlay htmlhelp)
	# Unicode MFC expects this entry point
	target_link_options(CBPlay PRIVATE /ENTRY:wWinMainCRTStartup)
endif()

target_compile_definitions(CBPlay PUBLIC GPLAY)
target_include_directories(CBPlay PRIVATE . ../GShr)
# include order matters
if (MSVC)
	target_include_directories(CBPlay PRIVATE $ENV{VCToolsInstallDir}atlmfc/src/mfc)
endif()
target_precompile_headers(CBPlay PRIVATE StdAfx.h)
target_link_libraries(CBPlay wx::core)
