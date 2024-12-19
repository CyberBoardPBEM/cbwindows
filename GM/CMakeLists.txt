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
	../GShr/LBoxVHScrl.cpp
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
	# Use manifest from wx.rc for dpiAware
	set_target_properties(CBDesign PROPERTIES
		LINK_FLAGS "/MANIFEST:NO"
	)
	target_link_libraries(CBDesign htmlhelp.lib)
	# Unicode MFC expects this entry point
	target_link_options(CBDesign PRIVATE /ENTRY:wWinMainCRTStartup)
endif()

target_include_directories(CBDesign PRIVATE . ../GShr)
target_precompile_headers(CBDesign PRIVATE StdAfx.h)
target_link_libraries(CBDesign wx::aui wx::core wx::xrc)

add_custom_command(
	OUTPUT "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CBDesign.xrc"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/CBDesign.xrc" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CBDesign.xrc"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/CBDesign.xrc"
	VERBATIM
)
add_custom_target(CBDesign_copy_files DEPENDS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CBDesign.xrc)
add_dependencies(CBDesign CBDesign_copy_files)
