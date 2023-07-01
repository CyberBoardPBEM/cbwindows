# GHelp

# apparently copy can't use cmake paths
cmake_path(NATIVE_PATH CMAKE_CURRENT_SOURCE_DIR NATIVE_SOURCE_DIR)
cmake_path(NATIVE_PATH CMAKE_RUNTIME_OUTPUT_DIRECTORY NATIVE_BINARY_DIR)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gmhelpidmap.h
	COMMENT "Generate gmhelpidmap.h"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../GM/resource.h
	COMMAND makehm IDR_,IDH_R_,0x20000 ${CMAKE_CURRENT_SOURCE_DIR}/../GM/resource.h >${CMAKE_CURRENT_SOURCE_DIR}/gmhelpidmap.h
	COMMAND makehm ID_,IDH_,0x10000 IDM_,IDH_M_,0x10000 ${CMAKE_CURRENT_SOURCE_DIR}/../GM/resource.h >>${CMAKE_CURRENT_SOURCE_DIR}/gmhelpidmap.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/makeidh ${CMAKE_CURRENT_SOURCE_DIR}/gmhelpidmap.h
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gmhelp.h
	COMMENT "Copy gmhelp.h"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../GM/gmhelp.h
	# apparently copy can't use cmake paths
	COMMAND copy ${NATIVE_SOURCE_DIR}\\..\\GM\\gmhelp.h ${NATIVE_SOURCE_DIR}\\gmhelp.h
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gphelpidmap.h
	COMMENT "Generate gphelpidmap.h"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../GP/resource.h
	COMMAND makehm IDR_,IDH_R_,0x20000 ${CMAKE_CURRENT_SOURCE_DIR}/../GP/resource.h >${CMAKE_CURRENT_SOURCE_DIR}/gphelpidmap.h
	COMMAND makehm ID_,IDH_,0x10000 IDM_,IDH_M_,0x10000 ${CMAKE_CURRENT_SOURCE_DIR}/../GP/resource.h >>${CMAKE_CURRENT_SOURCE_DIR}/gphelpidmap.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/makeidh ${CMAKE_CURRENT_SOURCE_DIR}/gphelpidmap.h
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gphelp.h
	COMMENT "Copy gphelp.h"
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../GP/gphelp.h
	# apparently copy can't use cmake paths
	COMMAND copy ${NATIVE_SOURCE_DIR}\\..\\GP\\gphelp.h ${NATIVE_SOURCE_DIR}\\gphelp.h
)

# hhc.exe seems to use inverse logic for exitvalue!
file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/fixhhc.bat "hhc.exe CBoard.hhp\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/fixhhc.bat "if errorlevel 1 exit /b 0\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/fixhhc.bat "exit /b 1\n")

add_custom_command(
	OUTPUT cboard.chm
	COMMENT "Make cboard.chm"
	DEPENDS
			${CMAKE_CURRENT_SOURCE_DIR}/gmhelp.h ${CMAKE_CURRENT_SOURCE_DIR}/gmhelpidmap.h
			${CMAKE_CURRENT_SOURCE_DIR}/gphelp.h ${CMAKE_CURRENT_SOURCE_DIR}/gphelpidmap.h
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/fixhhc.bat
	# apparently move can't use cmake paths
	COMMAND move ${NATIVE_SOURCE_DIR}\\CBoard.chm ${NATIVE_BINARY_DIR}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(CBHelp ALL DEPENDS cboard.chm)
