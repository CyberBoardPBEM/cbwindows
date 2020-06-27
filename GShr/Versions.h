// versions.h - program version numbers.
//
// Copyright (c) 1994-2010 By Dale L. Larson, All Rights Reserved.
//
// DLL20100103
//      4.00 - Stripped out XtremeToolkit C++ code. MORE TO COME.
//
// DLL20091229
//      3.10 - Added multiplayer support far 32 players.
//
// DLL20050827
//      3.02 - Patch release to fix DDE launch problem
//
// DLL20050827
//      3.01 - Some fixes for final release
//
// DLL20050827
//      3.00 - Lock down new version for release.
//
// DLL20031008
//      2.91 - Expand size of sheet ID's from BYTE to WORD.
//
// DLL20010920
//      2.90 - prelude version to version 3.00
//
// DLL20010801
//      2.01 - File versions moved up to support geomorphic boards.
//          Support was added mid-beta for version 2.0 of the program.
//
// DLL20010626
//      Program version set to 1.12. No changes in file formats.
//
// DLL20010416
//      Program version set to 1.11. No changes in file formats.
//
// DLL20000416
//      Program version set to 1.10. No changes in file formats.
//
// DLL19990831
//      2.00 - All file versions and program rev'ed to 2.00 for 
//          multiplayer support. A big jump but it is
//          a very significant addition to the program and
//          I intended to rev the file versions to 1.00 on
//          to official release. I just didn't have the
//          guts to make everyone load and save all their
//          files!
//
// DLL990313
//      Program version set to 1.00. No changes were made to the 
//      file formats.
//
// DLL971222
//      All formats update:
//      0.60 - Added support for compound moves in move files and 
//          recorded history.
//
// DLL970911
//      All formats update:
//      0.59 - Added hacks and fixes for nasty problem of not 
//          storing the version number of the game when a move
//          file is absored. The net result is the history has
//          many versions of file formats with no way to tell the
//          difference. The solution is to store said version
//          number. Eventually I may just upgrade all the data 
//          rewrite the game file.
//
// DLL970901
//      All formats update:
//      0.58 - Now allow decimal snap grid pixels and offsets.
//
// DLL970901
//      fileGbxVerMinor updates:
//      0.57 - Added bitmapped drawing objects. (DrawObj.*)
//

#ifndef __VERSIONS_H__
#define __VERSIONS_H__

const int progVerMajor = 3;         // Current program version
const int progVerMinor = 50;        // (Number is divided by 100. ex: 10 is .10)

// File versions
const int fileGbxVerMajor = 3;      // Current GBOX file version supported
const int fileGbxVerMinor = 90;

const int fileGtlVerMajor = 3;      // Current GTLB file version supported
const int fileGtlVerMinor = 90;

const int fileGsnVerMajor = 3;      // Current GSCN file version supported
const int fileGsnVerMinor = 90;

const int fileGamVerMajor = 3;      // Current GAME file version supported
const int fileGamVerMinor = 90;

const int fileGmvVerMajor = 3;      // Current GMOV file version supported
const int fileGmvVerMinor = 90;

inline int NumVersion(int major, int minor) { return major * 256 + minor; }

#define FILEGBXSIGNATURE    "GBOX"  // File signature for game boxes
#define FILEGTLSIGNATURE    "GTLB"  // File signature for tile library files
#define FILEGSNSIGNATURE    "GSCN"  // File signature for scenarios
#define FILEGAMSIGNATURE    "GAME"  // File signature for games
#define FILEGMVSIGNATURE    "GMOV"  // File signature for move files

#define FILEEXT_GTLB        "gtl"   // File extension for tile library files

#endif

