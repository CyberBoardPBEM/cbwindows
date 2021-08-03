// versions.h - program version numbers.
//
// Copyright (c) 1994-2010 By Dale L. Larson, All Rights Reserved.
//
// wsu20210731
//      4.00 - 32-bit TileID/MarkID/PieceID/BoardID
//
// DLL20100103
//      3.90 - Stripped out XtremeToolkit C++ code. MORE TO COME.
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

inline int NumVersion(int major, int minor) { return major * 256 + minor; }
inline int VersionMajor(int numVer) { return numVer / 256; }
inline int VersionMinor(int numVer) { return numVer % 256; }

inline int GetSaveFileVersion()
{
    static const int retval = [] {
        struct FileFlagParser : public CCommandLineInfo
        {
            bool id32 = false;
            virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast) override
            {
                if (bFlag && strcmp(pszParam, "id32") == 0)
                {
                    id32 = true;
                }
            }
        };
        FileFlagParser ffp;
        CbGetApp().ParseCommandLine(ffp);
        return ffp.id32 ? NumVersion(4, 0) : NumVersion(3, 90);
    }();
    return retval;
}

// File versions
const int fileGbxVerMajor = VersionMajor(GetSaveFileVersion());      // Current GBOX file version supported
const int fileGbxVerMinor = VersionMinor(GetSaveFileVersion());

const int fileGtlVerMajor = VersionMajor(GetSaveFileVersion());      // Current GTLB file version supported
const int fileGtlVerMinor = VersionMinor(GetSaveFileVersion());

const int fileGsnVerMajor = VersionMajor(GetSaveFileVersion());      // Current GSCN file version supported
const int fileGsnVerMinor = VersionMinor(GetSaveFileVersion());

const int fileGamVerMajor = VersionMajor(GetSaveFileVersion());      // Current GAME file version supported
const int fileGamVerMinor = VersionMinor(GetSaveFileVersion());

const int fileGmvVerMajor = VersionMajor(GetSaveFileVersion());      // Current GMOV file version supported
const int fileGmvVerMinor = VersionMinor(GetSaveFileVersion());

#define FILEGBXSIGNATURE    "GBOX"  // File signature for game boxes
#define FILEGTLSIGNATURE    "GTLB"  // File signature for tile library files
#define FILEGSNSIGNATURE    "GSCN"  // File signature for scenarios
#define FILEGAMSIGNATURE    "GAME"  // File signature for games
#define FILEGMVSIGNATURE    "GMOV"  // File signature for move files

#define FILEEXT_GTLB        "gtl"   // File extension for tile library files

/* This is an RAII helper for setting the current file format
    version, and then changing it to a different version when
    the need for the current version is done.  Defaults to
    different version being the version in use before creating
    this object, but that version can be overridden to set a
    different version.  */
template<typename T>
class SetLoadingVersionGuard
{
public:
    SetLoadingVersionGuard(int ctorVer, int dtorVer = T::GetLoadingVersion()) :
        m_dtorVer(dtorVer)
    {
        T::SetLoadingVersion(ctorVer);
    }
    ~SetLoadingVersionGuard()
    {
        T::SetLoadingVersion(m_dtorVer);
    }
private:
    const int m_dtorVer;
};

// KLUDGE:  get access to CGamDoc::GetLoadingVersion
extern int GetLoadingVersion();

/* cope with varying file versions
by getting sizeof(XxxxID<>) for file */
/*  N.B.:  making this a template allows us the option to use
different sizes for different id types */
template<typename T>
size_t GetXxxxIDSize(const CArchive& ar)
{
    static_assert(std::is_same_v<T, XxxxIDExt<T::PREFIX, T::UNDERLYING_TYPE>>, "requires XxxxIDExt");
    /* if .gbx/.gsn/.gam/.gmv versions become unequal,
        the version logic here will need to rebuilt */
    ASSERT(NumVersion(fileGsnVerMajor, fileGsnVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
    ASSERT(NumVersion(fileGamVerMajor, fileGamVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
    ASSERT(NumVersion(fileGmvVerMajor, fileGmvVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
    int ver = ar.IsStoring() ?
                    NumVersion(fileGbxVerMajor, fileGbxVerMinor)
                :
                    GetLoadingVersion();
    if (ver <= NumVersion(3, 90))
    {
        // ASSERT(ver < NumVersion(3, 90) --> ar.IsLoading());
        ASSERT(ver == NumVersion(3, 90) || ar.IsLoading());
        return sizeof(XxxxID16<T::PREFIX>::UNDERLYING_TYPE);
    }
    else
    {
        ASSERT(ver == NumVersion(4, 0));
        if (sizeof(XxxxID<T::PREFIX>::UNDERLYING_TYPE) != sizeof(XxxxID32<T::PREFIX>::UNDERLYING_TYPE))
        {
            ASSERT(!"not ready for 32bit ids");
            AfxThrowNotSupportedException();
        }
        return sizeof(XxxxID32<T::PREFIX>::UNDERLYING_TYPE);
    }
}

template<char PREFIX, typename UNDERLYING_TYPE>
CArchive& operator<<(CArchive& ar, const XxxxIDExt<PREFIX, UNDERLYING_TYPE>& oid)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSize<CB::remove_cvref_t<decltype(oid)>>(ar);
    if (fileIDSize == sizeof(oid))
    {
        return ar << reinterpret_cast<const UNDERLYING_TYPE&>(oid);
    }
    else
    {
        switch (fileIDSize)
        {
            case 2:
                return ar << value_preserving_cast<XxxxID16<PREFIX>::UNDERLYING_TYPE>(static_cast<UNDERLYING_TYPE>(oid));
            case 4:
                return ar << value_preserving_cast<XxxxID32<PREFIX>::UNDERLYING_TYPE>(static_cast<UNDERLYING_TYPE>(oid));
            default:
                CbThrowBadCastException();
        }
    }
}

template<char PREFIX, typename UNDERLYING_TYPE>
CArchive& operator>>(CArchive& ar, XxxxIDExt<PREFIX, UNDERLYING_TYPE>& oid)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    size_t fileIDSize = GetXxxxIDSize<CB::remove_cvref_t<decltype(oid)>>(ar);
    if (fileIDSize == sizeof(oid))
    {
        return ar >> reinterpret_cast<UNDERLYING_TYPE&>(oid);
    }
    else
    {
        switch (fileIDSize)
        {
            case 2: {
                uint16_t temp;
                ar >> temp;
                oid = static_cast<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>(static_cast<XxxxID16<PREFIX>>(temp));
                return ar;
            }
            case 4: {
                uint32_t temp;
                ar >> temp;
                oid = static_cast<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>(static_cast<XxxxID32<PREFIX>>(temp));
                return ar;
            }
            default:
                CbThrowBadCastException();
        }
    }
}

#endif

