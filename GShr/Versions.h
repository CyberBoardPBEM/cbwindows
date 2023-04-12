// versions.h - program version numbers.
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
//
#ifndef __VERSIONS_H__
#define __VERSIONS_H__

#include <regex>

const int progVerMajor = 4;         // Current program version
const int progVerMinor = 0;        // (Number is divided by 100. ex: 10 is .10)

inline int NumVersion(int major, int minor) { return major * 256 + minor; }
inline int VersionMajor(int numVer) { return numVer / 256; }
inline int VersionMinor(int numVer) { return numVer % 256; }

// wsu20230331
//      5.00 - use feature lists instead of version numbers
//              The goal of this version is to make one final
//              file version NUMBER change, and then never
//              change the file version NUMBER again.The problem
//              with version numbers is there isn't a good way
//              to make file format changes in parallel, and no
//              good way to distinguish the evolution of the
//              file format between releases. This makes it
//              difficult to read files written at intermediate
//              points in time between releases because the
//              version number is the same, but the actual
//              format is different.
//
//              This version replaces the storage-efficient but
//              not-very-informative version NUMBER with a
//              much-larger storage but more informative version
//              FEATURE LIST that a particular file requires its
//              reader to support.There is also a requirement
//              that readers that know about features NOT
//              specified by a file will correctly interpret that
//              file in a backward-compatible pre-feature mode.
//
//              A "feature" has value semantics
//              (https://en.wikipedia.org/wiki/Value_semantics),
//              and is represented by a name of string type.  It
//              is of course each developer's responsibility not
//              to use the same feature name for different
//              purposes. For convenience (see below), the
//              string names are case-insensitive. These names
//              are represented by class Feature. Feature is
//              implemented as a thin wrapper on std::string
//              that provides the very small interface needed to
//              represent a feature (identity comparison, and
//              save/load).
class Feature
{
public:
    Feature() noexcept = default;
    explicit Feature(std::string f) : feature(std::move(f)) {}
    bool operator==(const Feature& other) const noexcept;
    friend CArchive& operator<<(CArchive& ar, const Feature& f);
    friend CArchive& operator>>(CArchive& ar, Feature& f);

private:
    std::string feature;

    // KLUDGE:  friend can't refer to template partial specialization
    friend std::formatter<Feature, char>;
    friend std::formatter<Feature, wchar_t>;
    friend std::formatter<class Features, char>;
    friend std::formatter<class Features, wchar_t>;
};
//
//              A collection of Features is represented by
//              class Features.  Again, this has value semantics
//              and is just a thin wrapper on
//              std::vector<Feature> providing the very small
//              interface needed (iterate, add/remove feature,
//              test whether it contains a feature, and
//              save/load).
class Features : private std::vector<Feature>
{
    using BASE = std::vector<Feature>;
public:
    Features() noexcept = default;
    Features(std::initializer_list<Feature> l) : BASE(l) {}
    using BASE::begin;
    using BASE::end;
    bool Check(const Feature& f) const noexcept;
    void Add(const Feature& f);
    void Remove(const Feature& f);
    friend CArchive& operator<<(CArchive& ar, const Features& fs);
    friend CArchive& operator>>(CArchive& ar, Features& fs);

private:
    typename BASE::const_iterator Find(const Feature& f) const noexcept;
};
//
//              Some features (e.g., the size of an ID) are
//              relatively widely scattered throughout the file,
//              but some (e.g., the presence of CRollState) are
//              very local.To avoid needing the have these local
//              features checked at the beginning of writing a
//              file (i.e., to reduce code coupling), the
//              feature list is appended to throughout the save
//              operation, and then the list is written at the
//              end.  (This does have the disadvantage that
//              writing and reading the file is not purely
//              sequential.)
//
//              For testing purposes, there is command line
//              switch support to force using a file Feature
//              even if it isn't actually required,
//              e.g., /filever:force-id-32bit (see
//              GetCBForcedFeatures()). Also, there is support
//              to forbid saving a file with a particular
//              Feature, e.g., /filever:no-id-32bit
//              (see GetCBFeatures()). Currently, this is only
//              checked at save time, not continuously during
//              editing, but at least the user can verify that
//              he hasn't unintentionally written a file that is
//              no longer compatible with another player's CB.
//              (If the additional work seems warranted, nothing
//              prevents editing actions from checking the
//              GetCBFeatures() list.) fFor convenience of
//              typing these command line switches, Feature
//              names are case-insensitive.
//
// wsu20210731
//      4.00 - 32-bit TileID/MarkID/PieceID/BoardID
inline const Feature ftrId32Bit("id-32bit");
//              treat size_t as 64bit (even in 32bit builds)
inline const Feature ftrSizet64Bit("size_t-64bit");
//              geomorphic boards with square cells
inline const Feature ftrGeoSquareCell("geo-square-cell");
//              geomorphic boards with rotated unit boards
inline const Feature ftrGeoRotateUnit("geo-rotate-unit");
//              pieces with <= 100 sides
inline const Feature ftrPiece100Sides("piece-100-sides");
//              completely private boards
inline const Feature ftrPrivatePlayerBoard("private-player-board");
//              serialize CGamDoc::m_pRollState
inline const Feature ftrCRollState("CRollState");
//              update old object ID CDrawObj::drawMarkObj tag
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

inline int GetSaveFileVersion()
{
    static const int retval = [] {
        struct FileFlagParser : public CCommandLineInfo
        {
            int version = NumVersion(5, 0);
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

const Features& GetCBFeatures();

inline bool Feature::operator==(const Feature& other) const noexcept
{
    return _stricmp(feature.c_str(), other.feature.c_str()) == 0;
}

inline CArchive& operator<<(CArchive& ar, const Feature& f)
{
    /* At this point in the file, the reader doesn't yet
        know whether feature "size_t-64bit" is active, so
        make a rule that size is always 8 bytes for
        Feature/Features */
    ar << uint64_t(f.feature.size());
    ar.Write(f.feature.c_str(), value_preserving_cast<uint32_t>(f.feature.size()));
    return ar;
}

inline CArchive& operator>>(CArchive& ar, Feature& f)
{
    /* At this point in the file, the reader doesn't yet
        know whether feature "size_t-64bit" is active, so
        make a rule that size is always 8 bytes for
        Feature/Features */
    uint64_t size;
    ar >> size;
    f.feature.resize(value_preserving_cast<size_t>(size));
    ar.Read(&f.feature.front(), value_preserving_cast<uint32_t>(size));
    return ar;
}

template<typename CharT>
struct std::formatter<Feature, CharT> : private std::formatter<std::basic_string<CharT>, CharT>
{
private:
    using BASE = formatter<std::basic_string<CharT>, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const Feature& f, FormatContext& ctx)
    {
        return BASE::format("Feature("_cbstring + f.feature + ")", ctx);
    }
};

inline bool Features::Check(const Feature& f) const noexcept
{
    return Find(f) != end();
}

inline void Features::Add(const Feature& f)
{
    if (!Check(f))
    {
        push_back(f);
    }
}

inline void Features::Remove(const Feature& f)
{
    auto it = Find(f);
    if (it != end())
    {
        erase(it);
    }
}

inline CArchive& operator<<(CArchive& ar, const Features& fs)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }

    /* At this point in the file, the reader doesn't yet
        know whether feature "size_t-64bit" is active, so
        make a rule that size is always 8 bytes for
        Feature/Features */
    ar << uint64_t(fs.size());
    for (const Feature& f : fs)
    {
        ar << f;
    }

    return ar;
}

inline CArchive& operator>>(CArchive& ar, Features& fs)
{
    if (!ar.IsLoading())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }

    /* At this point in the file, the reader doesn't yet
        know whether feature "size_t-64bit" is active, so
        make a rule that size is always 8 bytes for
        Feature/Features */
    uint64_t size;
    ar >> size;
    fs.resize(value_preserving_cast<size_t>(size));
    for (Feature& feature : fs)
    {
        ar >> feature;
        if (!GetCBFeatures().Check(feature))
        {
            AfxThrowArchiveException(CArchiveException::badSchema);
        }
    }

    return ar;
}

inline typename Features::BASE::const_iterator Features::Find(const Feature & f) const noexcept
{
    return std::find(begin(), end(), f);
}

template<typename CharT>
struct std::formatter<Features, CharT> : private std::formatter<std::basic_string<CharT>, CharT>
{
private:
    using BASE = formatter<std::basic_string<CharT>, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const Features& fs, FormatContext& ctx)
    {
        CB::string accum;
        for (const Feature& f : fs)
        {
            if (!accum.empty())
            {
                accum += ", ";
            }
            accum += f.feature;
        }
        return BASE::format("Features(" + accum + ")", ctx);
    }
};

/* mostly for testing, features to be used even if not required
    (e.g., id-32bit) */
inline const Features& GetCBForcedFeatures()
{
    static const Features features = [] {
        struct FileFlagParser : public CCommandLineInfo
        {
            Features features;
            virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast) override
            {
                static const std::regex re(R"(filever:force-(.+))", std::regex_constants::icase);
                std::cmatch m;
                if (bFlag && std::regex_match(pszParam, m, re))
                {
                    features.Add(Feature(m[1]));
                }
            }
        };
        FileFlagParser ffp;
        CbGetApp().ParseCommandLine(ffp);
        TRACE("%ls", std::format(L"{}:  {}\n", CB::string(__func__), ffp.features).c_str());
        return ffp.features;
    }();
    return features;
}

// CB file format 4.0 added these features
inline const Features& GetCBFile4Features()
{
    static const Features features = {
        ftrId32Bit,
        ftrSizet64Bit,
        ftrGeoSquareCell,
        ftrGeoRotateUnit,
        ftrPiece100Sides,
        ftrPrivatePlayerBoard,
        ftrCRollState,
    };
    return features;
}

inline const Features& GetCBFeatures()
{
    static const Features features = [] {
        struct FileFlagParser : public CCommandLineInfo
        {
            Features features;
            virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast) override
            {
                static const std::regex re(R"(filever:no-(.+))", std::regex_constants::icase);
                std::cmatch m;
                if (bFlag && std::regex_match(pszParam, m, re))
                {
                    Feature f(m[1]);
                    features.Remove(f);
                }
            }
        };
        FileFlagParser ffp;
        if (GetSaveFileVersion() >= NumVersion(4, 0))
        {
            ffp.features = GetCBFile4Features();
            CbGetApp().ParseCommandLine(ffp);
        }
        const Features& forced = GetCBForcedFeatures();
        for (const Feature& f : forced)
        {
            if (!ffp.features.Check(f))
            {
                ASSERT(!"conflicting feature settings");
                AfxThrowInvalidArgException();
            }
        }
        TRACE("%ls", std::format(L"{}:  {}\n", CB::string(__func__), ffp.features).c_str());
        return ffp.features;
    }();
    return features;
}

/* TODO:  when we have our own CArchive replacement, make
            Features a member of it rather than static CGamDoc
            member */
// KLUDGE:  get access to CGamDoc::Get/SetFileFeatures
extern const Features& GetFileFeatures();
extern void SetFileFeatures(Features&& fs);
namespace CB
{
    inline const Features& GetFeatures(const CArchive& ar)
    {
        return GetFileFeatures();
    }

    inline void SetFeatures(CArchive& ar, Features&& fs)
    {
        return SetFileFeatures(std::move(fs));
    }

    inline void AddFeature(CArchive& ar, const Feature& f)
    {
        Features fs = GetFeatures(ar);
        fs.Add(f);
        SetFeatures(ar, std::move(fs));
    }
}

/* This is an RAII helper for setting the current file format
    version, and then changing it to a different version when
    the need for the current version is done.  Defaults to
    different version being the version in use before creating
    this object, but that version can be overridden to set a
    different version.  */
class SetFileFeaturesGuard
{
public:
    SetFileFeaturesGuard(CArchive& ar, Features&& ctorFeat, Features&& dtorFeat) noexcept :
        ar(ar), m_dtorFeat(std::move(dtorFeat))
    {
        CB::SetFeatures(ar, std::move(ctorFeat));
    }
    SetFileFeaturesGuard(CArchive& ar, const Features& ctorFeat, const Features& dtorFeat) :
        SetFileFeaturesGuard(ar, Features(ctorFeat), Features(dtorFeat))
    {
    }
    SetFileFeaturesGuard(CArchive& ar, const Features& ctorFeat) :
        SetFileFeaturesGuard(ar, Features(ctorFeat), Features(CB::GetFeatures(ar)))
    {
    }
    ~SetFileFeaturesGuard()
    {
        CB::SetFeatures(ar, std::move(m_dtorFeat));
    }
private:
    CArchive& ar;
    Features m_dtorFeat;
};

/* cope with varying file versions
by getting sizeof(XxxxID<>) for file */
/*  N.B.:  making this a template allows us the option to use
different sizes for different id types */
namespace CB { namespace Impl
{
    template<typename T>
    size_t GetXxxxIDSerializeSize(const CArchive& ar)
    {
        static_assert(std::is_same_v<T, XxxxIDExt<T::PREFIX, typename T::UNDERLYING_TYPE>>, "requires XxxxIDExt");
        /* if .gbx/.gsn/.gam/.gmv versions become unequal,
            the version logic here will need to rebuilt */
        ASSERT(NumVersion(fileGsnVerMajor, fileGsnVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        ASSERT(NumVersion(fileGamVerMajor, fileGamVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        ASSERT(NumVersion(fileGmvVerMajor, fileGmvVerMinor) == NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        if (!CB::GetFeatures(ar).Check(ftrId32Bit))
        {
            return sizeof(XxxxID16<T::PREFIX>::UNDERLYING_TYPE);
        }
        else
        {
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

template<wchar_t PREFIX, typename UNDERLYING_TYPE>
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

template<wchar_t PREFIX, typename UNDERLYING_TYPE>
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

template<wchar_t PREFIX, typename UNDERLYING_TYPE>
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

template<typename T, std::enable_if_t<std::is_same_v<T, CWordArray> ||
                                        std::is_same_v<T, CDWordArray> ||
                                        std::is_same_v<T, CArray<int, int>>, bool> = true>
CArchive& operator<<(CArchive& ar, const T& v);

template<typename T, std::enable_if_t<std::is_same_v<T, CWordArray> ||
                                        std::is_same_v<T, CDWordArray> ||
                                        std::is_same_v<T, CArray<int, int>>, bool> = true>
CArchive& operator>>(CArchive& ar, T& v);

template<wchar_t PREFIX, typename UNDERLYING_TYPE>
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

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            // handle 32/64bit difference in Invalid_v<size_t>
#if defined(_WIN64)
            if (s == Invalid_v<size_t>)
            {
                s = std::numeric_limits<uint32_t>::max();
            }
#endif
            if (s > uint32_t(0xFFFFFFFF))
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
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

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
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
                                        std::is_same_v<T, CArray<int, int>>, bool> /*= true*/>
inline CArchive& operator<<(CArchive& ar, const T& v)
{
    CB::WriteCount(ar, value_preserving_cast<size_t>(v.GetSize()));
    intptr_t expected = v.GetSize()*int(sizeof(v[0]));
    ar.Write(v.GetData(), value_preserving_cast<unsigned>(expected));
    return ar;
}

template<typename T, std::enable_if_t<std::is_same_v<T, CWordArray> ||
                                        std::is_same_v<T, CDWordArray> ||
                                        std::is_same_v<T, CArray<int, int>>, bool> /*= true*/>
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

inline CArchive& operator<<(CArchive& ar, const std::vector<CB::string>& v)
{
    CB::WriteCount(ar, v.size());
    for (size_t i = size_t(0) ; i < v.size() ; ++i)
    {
        ar << v[i];
    }
    return ar;
}

inline CArchive& operator>>(CArchive& ar, std::vector<CB::string>& v)
{
    size_t size = CB::ReadCount(ar);
    v.resize(size);
    for (size_t i = size_t(0) ; i < v.size() ; ++i)
    {
        ar >> v[i];
    }
    return ar;
}
#endif

