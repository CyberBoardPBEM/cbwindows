// versions.h - program version numbers.
//
// Copyright (c) 1994-2010 By Dale L. Larson, All Rights Reserved.
//
// wsu20210731
//      4.00 - 32-bit TileID/MarkID/PieceID/BoardID
//              geomorphic boards with square cells
//              geomorphic boards with rotated unit boards
//              pieces with <= 100 sides
//              completely private boards
//              serialize CGamDoc::m_pRollState
//
// wsu20220208:  This version has been reverted.
//                  (It is equivalent to 3.10, so we will no
//                  longer create v 3.90 files to preserve
//                  compatibility with released CB 3.10 unless
//                  later features are enabled.)
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

#include <regex>

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
            int version = NumVersion(4, 0);
            virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast) override
            {
                static const std::regex re(R"(filever:([[:digit:]]+)\.([[:digit:]]+))");
                std::cmatch m;
                if (bFlag && std::regex_match(pszParam, m, re))
                {
                    version = NumVersion(stoi(m[1]), stoi(m[2]));
                }
            }
        };
        FileFlagParser ffp;
        CbGetApp().ParseCommandLine(ffp);
        return ffp.version;
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
namespace CB
{
    inline int GetVersion(const CArchive& ar)
    {
        return ar.IsStoring() ?
                    NumVersion(fileGbxVerMajor, fileGbxVerMinor)
                :
                    GetLoadingVersion();
    }
}

/* cope with varying file versions
by getting sizeof(XxxxID<>) for file */
/*  N.B.:  making this a template allows us the option to use
different sizes for different id types */
namespace CB { namespace Impl
{
    template<typename T>
    size_t GetXxxxIDSerializeSize(const CArchive& ar)
    {
        static_assert(std::is_same_v<T, XxxxIDExt<T::PREFIX, T::UNDERLYING_TYPE>>, "requires XxxxIDExt");
        /* if .gbx/.gsn/.gam/.gmv versions become unequal,
            the version logic here will need to rebuilt */
        ASSERT(NumVersion(fileGsnVerMajor, fileGsnVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        ASSERT(NumVersion(fileGamVerMajor, fileGamVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        ASSERT(NumVersion(fileGmvVerMajor, fileGmvVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        if (GetVersion(ar) <= NumVersion(3, 90))
        {
            // ASSERT(GetVersion(ar) < NumVersion(3, 10) --> ar.IsLoading());
            ASSERT(GetVersion(ar) == NumVersion(3, 10) || ar.IsLoading());
            return sizeof(XxxxID16<T::PREFIX>::UNDERLYING_TYPE);
        }
        else
        {
            ASSERT(GetVersion(ar) == NumVersion(4, 0));
            if (sizeof(XxxxID<T::PREFIX>::UNDERLYING_TYPE) != sizeof(XxxxID32<T::PREFIX>::UNDERLYING_TYPE))
            {
                ASSERT(!"not ready for 32bit ids");
                AfxThrowNotSupportedException();
            }
            return sizeof(XxxxID32<T::PREFIX>::UNDERLYING_TYPE);
        }
    }
}}

template<typename T>
size_t GetXxxxIDSerializeSize(const CArchive& ar)
{
    return CB::Impl::GetXxxxIDSerializeSize<CB::remove_cvref_t<T>>(ar);
}

template<char PREFIX, typename UNDERLYING_TYPE>
CArchive& operator<<(CArchive& ar, const XxxxIDExt<PREFIX, UNDERLYING_TYPE>& oid)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<decltype(oid)>(ar);
    ASSERT(fileIDSize == 2 || fileIDSize == 4);
    if (fileIDSize == sizeof(oid))
    {
        return ar << reinterpret_cast<const UNDERLYING_TYPE&>(oid);
    }
    else
    {
        SerializeBackdoor sb;
        return ar << SerializeBackdoor::Convert(oid);
    }
}

template<char PREFIX, typename UNDERLYING_TYPE>
CArchive& operator>>(CArchive& ar, XxxxIDExt<PREFIX, UNDERLYING_TYPE>& oid)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<decltype(oid)>(ar);
    ASSERT(fileIDSize == 2 || fileIDSize == 4);
    if (fileIDSize == sizeof(oid))
    {
        return ar >> reinterpret_cast<UNDERLYING_TYPE&>(oid);
    }
    else
    {
        SerializeBackdoor sb;
        XxxxIDExt<PREFIX, std::conditional_t<sizeof(oid) == 2, uint32_t, uint16_t>> temp;
        ar >> temp;
        oid = SerializeBackdoor::Convert(temp);
        return ar;
    }
}

template<char PREFIX, typename UNDERLYING_TYPE>
CArchive& operator<<(CArchive& ar, const std::vector<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>& v)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>(ar);
    switch (fileIDSize)
    {
        case 2:
            // CB3.1 file format uses MFC CWordArray
            ar << *ToCWordArray(v);
            return ar;
        case 4:
            // CB3.1 file format uses MFC CWordArray
            ar << *ToCDWordArray(v);
            return ar;
        default:
            CbThrowBadCastException();
    }
}

template<char PREFIX, typename UNDERLYING_TYPE>
CArchive& operator>>(CArchive& ar, std::vector<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>& v)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    size_t fileIDSize = GetXxxxIDSerializeSize<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>(ar);
    switch (fileIDSize)
    {
        case 2: {
            // CB3.1 file format uses MFC CWordArray
            CWordArray temp;
            ar >> temp;
            v = ToVector<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>(temp);
            return ar;
        }
        case 4: {
            // CB3.1 file format uses MFC CWordArray
            CDWordArray temp;
            ar >> temp;
            v = ToVector<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>(temp);
            return ar;
        }
        default:
            CbThrowBadCastException();
    }
}

namespace CB
{
    inline void WriteCount(CArchive& ar, size_t s)
    {
        if (!ar.IsStoring())
        {
            AfxThrowArchiveException(CArchiveException::readOnly);
        }

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            /* this matches the
            MFC CArchive::WriteCount()/ReadCount() format.  Note
            that MFC 32bit and 64bit processes use the same
            format for size < 0xFFFFFFFF, but have different
            formats for 0xFFFFFFFF.  This uses the 64bit format
            for both 32bit and 64bit since we might pass files
            between computers with different bit sizes, so we
            need a single format.  */
            if (s < uint16_t(0xFFFF))
            {
                ar << static_cast<uint16_t>(s);
                return;
            }
            ASSERT(!"untested code");

            ar << uint16_t(0xFFFF);
            if (s < uint32_t(0xFFFFFFFF))
            {
                ar << static_cast<uint32_t>(s);
                return;
            }

            ar << uint32_t(0xFFFFFFFF);
            ar << static_cast<uint64_t>(s);
            return;
        }
        else
        {
            // handle 32/64bit difference in Invalid_v<size_t>
            uint64_t u64 = static_cast<uint64_t>(s);
#if !defined(_WIN64)
            if (u64 == Invalid_v<size_t>)
            {
                u64 = std::numeric_limits<uint64_t>::max();
            }
#endif
            ar << u64;
        }
    }

    inline size_t ReadCount(CArchive& ar)
    {
        if (ar.IsStoring())
        {
            AfxThrowArchiveException(CArchiveException::readOnly);
        }

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            uint16_t u16;
            ar >> u16;
            if (u16 != 0xFFFF)
            {
                return u16;
            }
            ASSERT(!"untested code");

            uint32_t u32;
            ar >> u32;
            if (u32 != uint32_t(0xFFFFFFFF))
            {
                return u32;
            }

            uint64_t u64;
            ar >> u64;
            return value_preserving_cast<size_t>(u64);
        }
        else
        {
            uint64_t u64;
            ar >> u64;
            // handle 32/64bit difference in Invalid_v<size_t>
#if !defined(_WIN64)
            if (u64 == std::numeric_limits<uint64_t>::max())
            {
                u64 = Invalid_v<size_t>;
            }
#endif
            return value_preserving_cast<size_t>(u64);
        }
    }
}

template<typename T, std::enable_if_t<std::is_same_v<T, CWordArray> ||
                                        std::is_same_v<T, CDWordArray> ||
                                        std::is_same_v<T, CArray<int, int>>, bool> = true>
inline CArchive& operator<<(CArchive& ar, const T& v)
{
    CB::WriteCount(ar, value_preserving_cast<size_t>(v.GetSize()));
    intptr_t expected = v.GetSize()*int(sizeof(v[0]));
    ar.Write(v.GetData(), value_preserving_cast<unsigned>(expected));
    return ar;
}

template<typename T, std::enable_if_t<std::is_same_v<T, CWordArray> ||
                                        std::is_same_v<T, CDWordArray> ||
                                        std::is_same_v<T, CArray<int, int>>, bool> = true>
inline CArchive& operator>>(CArchive& ar, T& v)
{
    size_t size = CB::ReadCount(ar);
    v.SetSize(value_preserving_cast<intptr_t>(size));
    unsigned expected = value_preserving_cast<unsigned>(size*sizeof(v[0]));
    unsigned actual = ar.Read(v.GetData(), expected);
    if (actual != expected)
    {
        CFile* file = ar.GetFile();
        AfxThrowArchiveException(CArchiveException::endOfFile, file ? file->GetFilePath() : nullptr);
    }
    return ar;
}

inline CArchive& operator<<(CArchive& ar, const CStringArray& v)
{
    CB::WriteCount(ar, value_preserving_cast<size_t>(v.GetSize()));
    for (intptr_t i = 0 ; i < v.GetSize() ; ++i)
    {
        ar << v[i];
    }
    return ar;
}

inline CArchive& operator>>(CArchive& ar, CStringArray& v)
{
    size_t size = CB::ReadCount(ar);
    v.SetSize(value_preserving_cast<intptr_t>(size));
    for (intptr_t i = 0 ; i < v.GetSize() ; ++i)
    {
        ar >> v[i];
    }
    return ar;
}
#endif

