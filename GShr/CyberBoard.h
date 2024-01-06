// CyberBoard.h
//
// Copyright (c) 2020-2023 By William Su, All Rights Reserved.
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

#ifndef _CYBERBOARD_H
#define _CYBERBOARD_H

// use explicitly sized ints for portable file format control
#include <cinttypes>
#include <cstdarg>
#include <filesystem>
#include <format>
// KLUDGE:  see https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2508r1.html
#if __cpp_lib_format < 202207
    #pragma message("WARNING:  out-of-date STL does not provide std::basic_format_string")
    namespace std {
        template<typename CharT, typename... Args>
        struct basic_format_string : private basic_string_view<CharT>
        {
        private:
            using BASE = basic_string_view<CharT>;
        public:
            using BASE::BASE;
            const basic_string_view<CharT>& get() const { return *this; }
        };

        template<typename... Args>
        using format_string = basic_format_string<char, type_identity_t<Args>...>;

        template<typename... Args>
        using wformat_string = basic_format_string<wchar_t, type_identity_t<Args>...>;
    }
#endif
#include <limits>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <WinExt.h>

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clipbrd.h>
#include <wx/clrpicker.h>
#include <wx/dataobj.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dialog.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/nativewin.h>
#include <wx/rawbmp.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/textbuf.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/valtext.h>
#include <wx/vlbox.h>
#include <wx/xrc/xmlres.h>
#include <wx/zstream.h>
#include <wx/msw/mfc.h>

static_assert(std::is_same_v<uint8_t, BYTE>, "wrong standard replacement for BYTE");
static_assert(std::is_same_v<uint16_t, WORD>, "wrong standard replacement for WORD");
static_assert(std::is_same_v<unsigned int, UINT>, "wrong standard replacement for UINT");
static_assert(std::is_same_v<long, LONG>, "wrong standard replacement for LONG");
static_assert(std::is_same_v<intptr_t, INT_PTR>, "wrong standard replacement for INT_PTR");
static_assert(std::is_same_v<uintptr_t, UINT_PTR>, "wrong standard replacement for UINT_PTR");
#if defined(_WIN64)
// unfortunately, these aren't true in 32bit
static_assert(std::is_same_v<UINT_PTR, DWORD_PTR>, "inconsistent unsigned *_PTR types");
static_assert(std::is_same_v<uintptr_t, DWORD_PTR>, "wrong standard replacement for DWORD_PTR");
#else
/* best we can do in 32bit, but this means we can't always use
    uintptr_t, e.g., virtual function override doesn't work when
    types aren't same */
static_assert(std::is_unsigned_v<uintptr_t> == std::is_unsigned_v<DWORD_PTR> &&
                    sizeof(uintptr_t) == sizeof(DWORD_PTR),
                "wrong standard replacement for DWORD_PTR");
#endif
#if 0
// unfortunately, these aren't true
static_assert(std::is_same_v<uint32_t, DWORD>, "wrong standard replacement for DWORD");
static_assert(std::is_same_v<int32_t, LONG>, "wrong standard replacement for LONG");
#else
// best we can do
static_assert(std::is_unsigned_v<uint32_t> == std::is_unsigned_v<DWORD> &&
                    sizeof(uint32_t) == sizeof(DWORD),
                "wrong standard replacement for DWORD");
static_assert(std::is_unsigned_v<int32_t> == std::is_unsigned_v<LONG> &&
                    sizeof(int32_t) == sizeof(LONG),
                "wrong standard replacement for DWORD");
#endif

#if defined(_MSC_VER)
// warning C4172 : returning address of local variable or temporary
#pragma warning(error:  4172)
// warning C4239 : nonstandard extension used : 'argument' : conversion from 'xxx' to 'xxx &'
#pragma warning(error:  4239)
// warning C4244 : 'initializing' : conversion from 'xxx' to 'yyy', possible loss of data
#pragma warning(error:  4244)
// warning C4245 : 'initializing' : conversion from 'xxx' to 'yyy', signed / unsigned mismatch
#pragma warning(error:  4245)
// warning C4267 : 'initializing' : conversion from 'xxx' to 'yyy', possible loss of data
#pragma warning(error:  4267)
// warning C4302 : 'type cast' : truncation from 'xxx' to 'yyy'
#pragma warning(error:  4302)
// warning C4310 : cast truncates constant value
#pragma warning(error:  4310)
// warning C4311 : 'type cast' : pointer truncation from 'xxx' to 'yyy'
#pragma warning(error:  4311)
// warning C4312 : 'type cast' : conversion from 'xxx' to 'yyy' of greater size
#pragma warning(error:  4312)
// warning C4471 : 'xxx' : a forward declaration of an unscoped enumeration must have an underlying type
#pragma warning(error:  4471)
// warning C4477 : 'sprintf' : format string 'xxx' requires an argument of type 'yyy', but variadic argument 1 has type 'zzz'
#pragma warning(error:  4477)
// warning C4701 : potentially uninitialized local variable 'xxx' used
#pragma warning(error:  4701)
// warning C4703 : potentially uninitialized local pointer variable 'xxx' used
#pragma warning(error:  4703)
// warning C4715 : 'xxx' : not all control paths return a value
#pragma warning(error:  4715)
// warning C4717 : 'xxx' : recursive on all control paths, function will cause runtime stack overflow
#pragma warning(error:  4717)
// warning C4805 : '|=' : unsafe mix of type 'bool' and type 'BOOL' in operation
#pragma warning(error:  4805)
// warning C4834 : discarding return value of function with 'nodiscard' attribute
#pragma warning(error:  4834)
// warning C4840 : non - portable use of class 'xxx' as an argument to a variadic function
#pragma warning(error:  4840)
// warning C4927 : illegal conversion; more than one user - defined conversion has been implicitly applied
#pragma warning(error:  4927)
// warning C4996 : 'xxx' : The POSIX name for this item is deprecated.Instead, use the ISO Cand C++ conformant name : yyy.See online help for details.
#pragma warning(error:  4996)
// warning C5205 : delete of an abstract class 'xxx' that has a non - virtual destructor results in undefined behavior
// WARNING:  for some reason, this won't change level!
#pragma warning(1:  5205)
// WARNING:  for some reason, this won't become an error, even at level 4!
#pragma warning(error:  5205)

// warning C4100 : 'xxx' : unreferenced formal parameter
#pragma warning(disable:  4100)
// warning C4189 : 'xxx' : local variable is initialized but not referenced
#pragma warning(disable:  4189)
// warning C4456 : declaration of 'xxx' hides previous local declaration
#pragma warning(disable:  4456)
#endif

#if defined(max)
    #undef max
#endif
#if defined(min)
    #undef min
#endif

namespace CB
{
    // unfortunately, some, but not all, systems declare ssize_t
    using ssize_t = std::make_signed_t<size_t>;
}

/////////////////////////////////////////////////////////////////////////////

template<typename CharT>
struct std::formatter<SIZE, CharT> : private std::formatter<decltype(SIZE::cx), CharT>
{
private:
    using BASE = formatter<decltype(SIZE::cx), CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const SIZE& s, FormatContext& ctx) const
    {
        std::format_to(ctx.out(), "(");
        BASE::format(s.cx, ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(s.cy, ctx);
        return std::format_to(ctx.out(), ")");
    }
};

template<typename CharT>
struct std::formatter<CSize, CharT> : public std::formatter<SIZE, CharT>
{
};

template<typename CharT>
struct std::formatter<POINT, CharT> : private std::formatter<decltype(POINT::x), CharT>
{
private:
    using BASE = formatter<decltype(POINT::x), CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const POINT& p, FormatContext& ctx) const
    {
        std::format_to(ctx.out(), "(");
        BASE::format(p.x, ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(p.y, ctx);
        return std::format_to(ctx.out(), ")");
    }
};

template<typename CharT>
struct std::formatter<CPoint, CharT> : public std::formatter<POINT, CharT>
{
};

template<typename CharT>
struct std::formatter<RECT, CharT> : private std::formatter<decltype(RECT::left), CharT>
{
private:
    using BASE = formatter<decltype(RECT::left), CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const RECT& r, FormatContext& ctx) const
    {
        std::format_to(ctx.out(), "(");
        BASE::format(r.left, ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(r.top, ctx);
        std::format_to(ctx.out(), " - ");
        BASE::format(r.right, ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(r.bottom, ctx);
        return std::format_to(ctx.out(), ")");
    }
};

template<typename CharT>
struct std::formatter<CRect, CharT> : public std::formatter<RECT, CharT>
{
};

template<typename CharT>
struct std::formatter<wxPoint, CharT> : private std::formatter<decltype(wxPoint::x), CharT>
{
private:
    using BASE = formatter<decltype(wxPoint::x), CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const wxPoint& p, FormatContext& ctx) const
    {
        std::format_to(ctx.out(), "(");
        BASE::format(p.x, ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(p.y, ctx);
        return std::format_to(ctx.out(), ")");
}
};

template<typename CharT>
struct std::formatter<wxRect, CharT> : private std::formatter<decltype(std::declval<wxRect>().GetLeft()), CharT>
{
private:
    using BASE = formatter<decltype(std::declval<wxRect>().GetLeft()), CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const wxRect& r, FormatContext& ctx) const
    {
        std::format_to(ctx.out(), "(");
        BASE::format(r.GetLeft(), ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(r.GetTop(), ctx);
        std::format_to(ctx.out(), " - ");
        BASE::format(r.GetRight(), ctx);
        std::format_to(ctx.out(), ",");
        BASE::format(r.GetBottom(), ctx);
        return std::format_to(ctx.out(), ")");
    }
};

/////////////////////////////////////////////////////////////////////////////

// emulate c++20 std::remove_cvref_t
#if !defined(__cpp_lib_remove_cvref)
namespace CB
{
    template<typename T>
    struct remove_cvref {
        typedef std::remove_cv_t<std::remove_reference_t<T>> type;
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;
}
#else
    // untested
namespace CB
{
    using std::remove_cvref;
    using std::remove_cvref_t;
}
#endif

//#define MAKELONG(a, b) ***poison***      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#undef MAKELONG
template<typename ARG1, typename ARG2>
constexpr int32_t MAKELONG(ARG1 a, ARG2 b)
{
    static_assert(sizeof(a) <= sizeof(uint16_t), "arg too large");
    static_assert(sizeof(b) <= sizeof(uint16_t), "arg too large");
    // avoid sign extension
    return static_cast<int32_t>(
            static_cast<uint32_t>(std::make_unsigned_t<CB::remove_cvref_t<decltype(a)>>(a)) |
            (static_cast<uint32_t>(std::make_unsigned_t<CB::remove_cvref_t<decltype(b)>>(b)) << 16)
        );
}

/////////////////////////////////////////////////////////////////////////////

namespace CB
{
    // adapters for different container types
    // TODO:  add support for std::vector::reserve?
    // TODO:  avoid need for these since they're runtime overhead
    template<typename LEFT, typename RIGHT>
    void Copy(LEFT& left, const RIGHT& right)
    {
        left.clear();
        for (typename RIGHT::const_iterator it = right.begin() ; it != right.end() ; ++it)
        {
            left.push_back(*it);
        }
    }

    template<typename LEFT, typename RIGHT>
    void Move(LEFT& left, RIGHT&& right)
    {
        left.clear();
        for (typename RIGHT::iterator it = right.begin() ; it != right.end() ; ++it)
        {
            left.push_back(std::move(*it));
        }
    }
}

/* simple(r) version of TS v2 experimental::propagate_const
    (see http://open-std.org/JTC1/SC22/WG21/docs/papers/2015/n4388.html
    and https://gcc.gnu.org/onlinedocs/gcc-6.2.0/libstdc++/api/a01497_source.html) */
namespace CB
{
    template<typename PT>
    class propagate_const
    {
    public:
        typedef std::remove_reference_t<decltype(*PT())> element_type;

        propagate_const() = default;
        propagate_const(nullptr_t) : p(nullptr) {}
        propagate_const(int i) : p(nullptr)
        {
            if (i) {
                AfxThrowInvalidArgException();
            }
        }

        template<typename PU>
        propagate_const(PU* pu) : p(pu) {}

        template<typename PU>
        propagate_const(propagate_const<PU>&& pu) : p(get_underlying(std::move(pu))) {}

        propagate_const(const propagate_const&) = delete;
        propagate_const& operator=(propagate_const&) = delete;
        propagate_const(propagate_const&&) = default;
        propagate_const& operator=(propagate_const&&) = default;
        ~propagate_const() = default;

        propagate_const& operator=(nullptr_t)
        {
            return *this = propagate_const(nullptr);
        }
        propagate_const& operator=(int i)
        {
            return *this = propagate_const(i);
        }

        template<typename PU>
        propagate_const& operator=(PU* pu)
        {
            return *this = propagate_const(pu);
        }

        template<typename PU>
        propagate_const& operator=(propagate_const<PU>&& pu)
        {
            p = get_underlying(std::move(pu));
            return *this;
        }

        explicit operator bool() const { return bool(p); }

        element_type* get() { return raw(p); }
        const element_type* get() const { return raw(p); }

        element_type* operator->() { return get(); }
        const element_type* operator->() const { return get(); }

        element_type& operator*() { return *get(); }
        const element_type& operator*() const { return *get(); }

    private:
        template<typename U>
        static U* raw(U* p) { return p; }

        template<typename U, typename DELETER>
        static U* raw(const std::unique_ptr<U, DELETER>& p) { return p.get(); }

        PT p = nullptr;

        template<typename PU>
        friend const PU& get_underlying(const propagate_const<PU>& p);

        template<typename PU>
        friend PU&& get_underlying(propagate_const<PU>&& p);
    };

    template<typename PT>
    const PT& get_underlying(const propagate_const<PT>& p)
    {
        return p.p;
    }

    template<typename PT>
    PT&& get_underlying(propagate_const<PT>&& p)
    {
        return std::move(const_cast<PT&>(get_underlying(std::as_const(p))));
    }

    template<typename T, typename U>
    bool operator==(const CB::propagate_const<T>& left, const CB::propagate_const<U>& right)
    {
        return left.get() == right.get();
    }

    template<typename T>
    bool operator==(const CB::propagate_const<T>& left, int i)
    {
        if (i) {
            AfxThrowInvalidArgException();
        }
        return left.get() == nullptr;
    }

    template<typename T>
    bool operator==(int i, const CB::propagate_const<T>& right)
    {
        return right == i;
    }

    template<typename T, typename U>
    bool operator==(const CB::propagate_const<T>& left, const U& right)
    {
        return left.get() == right;
    }

    template<typename T, typename U>
    bool operator==(const U& left, const CB::propagate_const<T>& right)
    {
        return right == left;
    }

    template<typename T, typename U>
    bool operator!=(const CB::propagate_const<T>& left, const U& right)
    {
        return !(left == right);
    }

    template<typename T, typename U>
    bool operator!=(const U& left, const CB::propagate_const<T>& right)
    {
        return right != left;
    }

    // like c++14 std::make_unique<>
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

/////////////////////////////////////////////////////////////////////////////

/* NOTE:  this is significantly different from gsl::not_null
    (see https://github.com/microsoft/GSL/blob/main/include/gsl/pointers)
    because gsl::not_null and std::experimental::propagate_const
    don't nest well, and gsl::not_null provides no non-const ref
    to wrapped pointer */
namespace CB
{
    template<typename PT>
    class not_null
    {
    public:
        typedef std::remove_reference_t<decltype(*std::declval<PT>())> deref_ptr_type;
        typedef std::remove_reference_t<decltype(*std::declval<const PT>())> deref_ptr_const_type;

        not_null(PT&& pt) :
            p(std::forward<PT>(pt))
        {
            CheckValid();
        }

        template<typename PU>
        not_null(PU* pu) :
            p(pu)
        {
            CheckValid();
        }

        /* N.B.:  causes pu to violate not_null, but moved-from
                objects are in an undefined state, so let's
                allow this */
        template<typename PU>
        not_null(not_null<PU>&& pu) :
            p(std::forward<PU>(get_underlying(std::move(pu))))
        {
            CheckValid();
        }

        deref_ptr_const_type* get() const
        {
            CheckValid();
            return &*p;
        }
        deref_ptr_type* get()
        {
            CheckValid();
            return &*p;
        }

        deref_ptr_const_type& operator*() const
        {
            return *get();
        }
        deref_ptr_type& operator*()
        {
            return *get();
        }

        deref_ptr_const_type* operator->() const
        {
            return get();
        }
        deref_ptr_type* operator->()
        {
            return get();
        }

        // never makes sense to check for nullptr
        operator bool() const = delete;
        bool operator==(nullptr_t) const = delete;
        bool operator==(int) const = delete;
        bool operator!=(nullptr_t) const = delete;
        bool operator!=(int) const = delete;

        template<typename U>
        bool operator==(const U& u) const
        {
            if (!u) {
                AfxThrowInvalidArgException();
            }
            return get() == u;
        }

        template<typename U>
        bool operator==(const CB::not_null<U>& u) const
        {
            return get() == u.get();
        }

        template<typename U>
        bool operator!=(const U& u) const
        {
            return !(*this == u);
        }

    private:
        void CheckValid() const
        {
            ASSERT(p);
            if (!p) {
                AfxThrowInvalidArgException();
            }
        }

        PT p;

        template<typename PU>
        friend const PU& get_underlying(const not_null<PU>& p);
    };

    template<typename PT>
    const PT& get_underlying(const not_null<PT>& p)
    {
        return p.p;
    }

    /* NOTE:  This allows violating p.p != nullptr, so is
                restricted to rvalue refs */
    template<typename PT>
    PT&& get_underlying(not_null<PT>&& p)
    {
        return std::move(const_cast<PT&>(get_underlying(std::as_const(p))));
    }

    template<typename T, typename U>
    bool operator==(const U& u, const not_null<T>& t)
    {
        return t == u;
    }
}

template<typename T>
using OwnerPtr = CB::not_null<CB::propagate_const<std::unique_ptr<T>>>;

// like c++14 std::make_unique<>
template<typename T, typename... Args>
OwnerPtr<T> MakeOwner(Args&&... args)
{
    return OwnerPtr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
class OwnerOrNullPtr : public CB::propagate_const<std::unique_ptr<T>>
{
    using BASE = CB::propagate_const<std::unique_ptr<T>>;

public:
    using BASE::BASE;
    using BASE::operator=;

    OwnerOrNullPtr() = default;

    template<typename U>
    OwnerOrNullPtr(OwnerPtr<U>&& pu) : BASE(CB::get_underlying(std::move(pu))) {}

    template<typename U>
    OwnerOrNullPtr& operator=(OwnerPtr<U>&& pu)
    {
        *this = CB::get_underlying(std::move(pu));
        return *this;
    }
};

/* Like pointers, references do not propagate const.
    However, unlike operator->(), it is not possible to declare
    operator.(), so a wrapper like propagate_const can't be
    created for refs.  Therefore, let's define RefPtr,
    i.e., a pointer that is not-null, has a constant value,
    and doesn't own the target, to approximate a ref, and
    use propagate_const on it.  */
template<typename T>
using RefPtr = CB::not_null<CB::propagate_const<T* const>>;

/////////////////////////////////////////////////////////////////////////////

/* Invalid_v<T> is used as the "no-value" value for type T.
    It will probably be set to std::numeric_limits<T>::max(). */

/* N.B.:  semantically, std::optional<T> is a better way to
    represent the absence of a T, but T values sometimes get used
    in C-style functions, so std::optional<T> is unavailable */

/* KLUDGE:  bool handles uintptr_t being same type as size_t.
    Code using Invalid<> should never specify second template
    argument; let its default value be used.

    Additional notes:
    In this code, size_t is used all over because that's the index
    type for std::vector.  (I am too lazy to use std::vector::size_type.)
    The original code frequently used -1 to indicate "no-value" for an
    index parameter.  I wanted to use std::numeric_limits<size_t>::max()
    for this purpose, but that was too long, so I declared a
    constexpr Invalid_size_t or something like that.  However, I
    found a spot in the code where uintptr_t seems more
    appropriate.  I could have declared Invalid_uintptr_t, but I
    felt like a situation like this cried out for using template
    Invalid.  The problem is that both size_t and uintptr_t are
    specified by the MSVC compiler as typedefs to the same C++
    fundamental type.  That means that attempting to explicitly
    instantiate Invalid<size_t> and Invalid<uintptr_t> is viewed
    by the compiler as conflicting instantiations.  To get
    around this problem, I added a second default template
    argument to Invalid<>, and have the default value be
    different for size_t and uintptr_t.  By making that second
    argument different, the compiler sees them as separate
    instantiations, and is satisfied.  */
template<typename T, bool = true>
struct Invalid
{
};

template<>
struct Invalid<uintptr_t>
{
    static constexpr uintptr_t value = std::numeric_limits<uintptr_t>::max();
};

// this explicit instantiation is only relevant if size_t != uintptr_t
template<>
struct Invalid<size_t, !std::is_same_v<size_t, uintptr_t>>
{
    static constexpr size_t value = std::numeric_limits<size_t>::max();
};

// convenience helper
template<typename T>
constexpr T Invalid_v = Invalid<T>::value;

/////////////////////////////////////////////////////////////////////////////

template<typename T>
decltype(*std::declval<T>()) CheckedDeref(T& p)
{
    ASSERT(p);
    if (!p) {
        AfxThrowInvalidArgException();
    }
    return *p;
}

template<typename T>
decltype(*std::declval<const T>()) CheckedDeref(const T& p)
{
    ASSERT(p);
    if (!p) {
        AfxThrowInvalidArgException();
    }
    return *p;
}

template<typename T>
decltype(*std::declval<CB::not_null<T>>()) CheckedDeref(CB::not_null<T>& p) = delete;

template<typename T>
decltype(*std::declval<const CB::not_null<T>>()) CheckedDeref(const CB::not_null<T>& p) = delete;

/////////////////////////////////////////////////////////////////////////////

/* test whether DEST can represent the exact value of src,
    not just the bit pattern
    (e.g., sint8_t can't represent 130) */

template<typename DEST, typename SRC>
struct is_always_value_preserving
{
    static_assert(std::is_arithmetic_v<DEST>,
                    "only arithmetic types supported");
    static constexpr bool value = (std::is_integral_v<DEST> &&
                                    std::is_integral_v<SRC> &&
                                    (
                                        (std::is_signed_v<DEST> == std::is_signed_v<SRC> &&
                                        sizeof(DEST) >= sizeof(SRC))
                                    ||
                                        (std::is_unsigned_v<SRC> &&
                                        sizeof(DEST) > sizeof(SRC))
                                    ))
                                ||
                                    (std::is_floating_point_v<DEST> &&
                                        std::is_floating_point_v<SRC> &&
                                        sizeof(DEST) >= sizeof(SRC))
                                ||
                                    (std::is_floating_point_v<DEST> &&
                                        std::is_integral_v<SRC> &&
                                        std::numeric_limits<DEST>::digits >= std::numeric_limits<SRC>::digits);
};

// convenience helper
template<typename DEST, typename SRC>
constexpr bool is_always_value_preserving_v = is_always_value_preserving<DEST, SRC>::value;

template<typename DEST, typename SRC>
constexpr std::enable_if_t<is_always_value_preserving_v<DEST, SRC>, bool> is_value_preserving(SRC /*src*/)
{
    return true;
}

template<typename DEST, typename SRC>
constexpr std::enable_if_t<!is_always_value_preserving_v<DEST, SRC>, bool> is_value_preserving(SRC src)
{
    if (std::is_signed_v<SRC> &&
        std::is_unsigned_v<DEST> &&
        src < SRC(0))
    {
        return false;
    }
    else if (std::is_unsigned_v<SRC> &&
        std::is_signed_v<DEST> &&
        static_cast<DEST>(src) < DEST(0))
    {
        return false;
    }
    else if (static_cast<SRC>(static_cast<DEST>(src)) != src)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/////////////////////////////////////////////////////////////////////////////

namespace CB
{
    /* long term plans involve Unicode Windows API due to
        wxWidgets dropping ANSI support
        (https://github.com/wxWidgets/wxWidgets/issues/22682),
        but storing UTF-8 strings
        (https://github.com/CyberBoardPBEM/cbwindows/pull/71#discussion_r725491054) */
    /* currently:  std::string assumed to be windows-1252 string,
        use wx for encoding conversions */
    // TODO:  make std::string utf8
    /*  WARNING:  switching to utf8 will require changing all
            indexing and sizing operations to work in terms of
            iterators due to varying-sized chars */
    // implementation in LibMfc.cpp
    class string
    {
    public:
#if !defined(_UNICODE)
        using value_type = char;
#else
        using value_type = wchar_t;
#endif
        static constexpr const size_t npos = std::string::npos;

        // nodiscard to avoid confusing with BOOL CString::LoadString(), etc
        [[nodiscard]] static string LoadString(UINT nID);
        [[nodiscard]] static string LoadString(int nID);

        template<typename... Args>
        [[nodiscard]] static string Format(UINT fmt, Args&&... args)
        {
            string sFmt = LoadString(fmt);
            return std::vformat(sFmt, std::make_wformat_args(args...));
        }

        [[nodiscard]] static string GetModuleFileName(HMODULE hModule);
        [[nodiscard]] static string GetWindowText(const CWnd& wnd);
        [[nodiscard]] static string GetText(const CListBox& wnd, int nIndex);
        [[nodiscard]] static string GetLBText(const CComboBox& wnd, int nIndex);
        [[nodiscard]] static string GetMenuString(const CMenu& menu, UINT nIDItem, UINT nFlags);
        [[nodiscard]] static string GetDocString(const CDocTemplate& docTempl, CDocTemplate::DocStringIndex index);
        // TODO:  std::optional<string> is probably better
        [[nodiscard]] static std::unique_ptr<string> DoPromptFileName(CWinApp& app, UINT nIDSTitle,
            DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);

        string() = default;     // empty string
        string(const string& other) : cp1252(other.cp1252) {}
        string(string&&) = default;
        string& operator=(const string&);
        string& operator=(string&&) = default;
//        ~string() = default;
~string();

        string(std::string_view s) : cp1252(s) {} // windows-1252 string
        // CString accepts !s
        string(const char* s) : cp1252(s ? s : "") {} // windows-1252 string
        string(const char* s, size_t n) : cp1252(s, n) {} // windows-1252 string
        string(const std::string& s) : string(s.c_str()) {}
        string(size_t n, char c) : cp1252(n, c) {}
        // not impl yet
        string(size_t n, wchar_t c) = delete;
        string(const CString& s) : string(s.GetString()) {}
        string(const wxString& s);
        string(std::wstring_view s);
        string(const wchar_t* s) : string(std::wstring_view(s)) {}
        string(const std::wstring& s) : string(std::wstring_view(s)) {}
        // resolve ambiguous string(NULL)
        string(int i) : string(nullptr) { ASSERT(!i); }
        string(nullptr_t) : string(static_cast<const char*>(nullptr)) {}

        // nodiscard to avoid confusing with void CString::Empty()
        [[nodiscard]] bool empty() const { return cp1252.empty(); }
        size_t a_size() const { return cp1252.size(); }
        const char* a_str() const { return cp1252.c_str(); }
        operator const char*() const { return a_str(); }
        inline CString mfc_str() const;
        operator CString() const { return mfc_str(); }
        const wxString& wx_str() const;
        operator const wxString&() const { return wx_str(); }
        size_t w_size() const { return wx_str().size(); }
        const wchar_t* w_str() const { return std_wstr().c_str(); }
        operator const wchar_t*() const { return w_str(); }
        std::string std_str() const { return std::string(a_str(), a_size()); }
        std::string_view std_strv() const { return std::string_view(a_str(), a_size()); }
        operator std::string_view() const { return std_strv(); }
        operator std::string() const { return std_str(); }
        const std::wstring& std_wstr() const;
        std::wstring_view std_wstrv() const { return std_wstr(); }
        operator std::wstring_view() const { return std_wstrv(); }
        operator const std::wstring&() const { return std_wstr(); }
#if !defined(_UNICODE)
        size_t v_size() const { return a_size(); }
#else
        size_t v_size() const { return w_size(); }
#endif
        const value_type* v_str() const { return static_cast<const value_type*>(*this); }
        const char& operator[](size_t s) const { return cp1252[s]; }
        const wchar_t& front() const { return *w_str(); }

        string& operator=(const wxString& s) { return *this = string(s); }

        operator std::filesystem::path() const { return std::filesystem::path(std_wstr()); }

        size_t find(char _Ch, size_t _Off = size_t(0)) const
        {
            return cp1252.find(_Ch, _Off);
        }
        size_t find(const string& _Right, size_t _Off = size_t(0)) const
        {
            return cp1252.find(_Right.cp1252, _Off);
        }
        size_t rfind(char _Ch, size_t _Off = npos) const
        {
            return cp1252.rfind(_Ch, _Off);
        }
        size_t rfind(const string& _Right, size_t _Off = npos) const
        {
            return cp1252.rfind(_Right.cp1252, _Off);
        }

        string substr(size_t _Off = size_t(0), size_t _Count = npos) const
        {
            return cp1252.substr(_Off, _Count);
        }

        int CompareNoCase(const string& rhs) const { return _wcsicmp(*this, rhs); }

        void clear() { wxstr.reset(); stdwide.reset(); cp1252.clear(); }
        // reserve doesn't change cp1252's value, so no need for wide.reset()
        void reserve(size_t s) { cp1252.reserve(s); }
        void resize(size_t s) { wxstr.reset(); stdwide.reset(); cp1252.resize(s); }

        string& operator+=(const string& rhs) { wxstr.reset(); stdwide.reset(); cp1252 += rhs.cp1252; return *this; }
        string& operator+=(char c) { wxstr.reset(); stdwide.reset(); cp1252 += c; return *this; }
        string& operator+=(wchar_t c) { return *this += CB::string(std::wstring_view(&c, size_t(1))); }

        void Serialize(CArchive& ar) const;
        void Serialize(CArchive& ar);

    private:
        std::string cp1252;
        /* need to keep object long-term so char* doesn't become
            orphaned */
        /* need separate wxString and std::wstring because
            wx edit controls expect \n, but MFC edit controls
            expect \r\n */
        mutable std::unique_ptr<wxString> wxstr;
        mutable std::unique_ptr<std::wstring> stdwide;
    };

    inline bool operator==(const string& lhs, const string& rhs) { return lhs.CompareNoCase(rhs) == 0; }
    inline bool operator==(const string& lhs, const char* rhs) { return lhs == CB::string(rhs); }
    inline bool operator!=(const string& lhs, const string& rhs) { return !(lhs == rhs); }
    inline bool operator!=(const string& lhs, const char* rhs) { return !(lhs == rhs); }
    inline std::string& operator+=(std::string& lhs, const string& rhs) { return lhs += rhs.a_str(); }
    inline string operator+(string lhs, const string& rhs) { return lhs += rhs; }
    inline string operator+(string lhs, const char* rhs) { return lhs += static_cast<string>(rhs); }
    inline string operator+(const char* lhs, const string& rhs) { return static_cast<string>(lhs) + rhs; }
    inline string operator+(string lhs, const CString& rhs) { return lhs += static_cast<string>(rhs); }
    inline string operator+(char lhs, const string& rhs) { return string(size_t(1), lhs) += rhs; }
    // not impl yet
    inline string operator+(wchar_t lhs, const string& rhs) = delete;
    inline string operator+(string lhs, char rhs) { return lhs += string(size_t(1), rhs); }
    // not impl yet
    inline string operator+(string lhs, wchar_t rhs) = delete;
    inline CArchive& operator<<(CArchive& ar, const string& s) { s.Serialize(ar); return ar; }
    inline CArchive& operator>>(CArchive& ar, string& s) { s.Serialize(ar); return ar; }

    using regex = std::basic_regex<string::value_type>;
    using cmatch = std::match_results<const string::value_type*>;
}

template<typename CharT>
struct std::formatter<CB::string, CharT> : private std::formatter<std::basic_string<CharT>, CharT>
{
private:
    using BASE = formatter<std::basic_string<CharT>, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const CB::string& s, FormatContext& ctx) const
    {
        return BASE::format(static_cast<const CharT*>(s), ctx);
    }
};

inline CB::string operator""_cbstring(const char* s, size_t len)
{
    return CB::string(s, len);
}

template<typename CharT>
struct std::formatter<CString, CharT> : private std::formatter<CB::string, CharT>
{
private:
    using BASE = formatter<CB::string, CharT>;
public:
    using BASE::parse;
    using BASE::format;
};

// unfortunately, these aren't provided by default (due to needing encoding conversion)
template<>
struct std::formatter<wchar_t, char> : private std::formatter<std::string, char>
{
    using BASE = formatter<std::string, char>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(wchar_t c, FormatContext& ctx) const
    {
        return BASE::format(CB::string(std::wstring_view(&c, size_t(1))), ctx);
    }
};

template<>
struct std::formatter<std::wstring_view, char> : private std::formatter<std::string, char>
{
    using BASE = formatter<std::string, char>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(std::wstring_view s, FormatContext& ctx) const
    {
        return BASE::format(CB::string(s), ctx);
    }
};

template<size_t n>
struct std::formatter<wchar_t[n], char> : private std::formatter<std::wstring_view, char>
{
    using BASE = formatter<std::wstring_view, char>;
public:
    using BASE::parse;
    using BASE::format;
};

template<>
struct std::formatter<const wchar_t*, char> : private std::formatter<std::wstring_view, char>
{
    using BASE = formatter<std::wstring_view, char>;
public:
    using BASE::parse;
    using BASE::format;
};

template<>
struct std::formatter<std::wstring, char> : private std::formatter<std::wstring_view, char>
{
    using BASE = std::formatter<std::wstring_view, char>;
public:
    using BASE::parse;
    using BASE::format;
};

template<>
struct std::formatter<char, wchar_t> : private std::formatter<std::wstring, wchar_t>
{
    using BASE = formatter<std::wstring, wchar_t>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(char c, FormatContext& ctx) const
    {
        return BASE::format(CB::string(std::string_view(&c, size_t(1))), ctx);
    }
};

template<>
struct std::formatter<std::string_view, wchar_t> : private std::formatter<std::wstring, wchar_t>
{
    using BASE = formatter<std::wstring, wchar_t>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(std::string_view s, FormatContext& ctx) const
    {
        return BASE::format(CB::string(s), ctx);
    }
};

template<size_t n>
struct std::formatter<char[n], wchar_t> : private std::formatter<std::string_view, wchar_t>
{
    using BASE = formatter<std::string_view, wchar_t>;
public:
    using BASE::parse;
    using BASE::format;
};

template<>
struct std::formatter<const char*, wchar_t> : private std::formatter<std::string_view, wchar_t>
{
    using BASE = formatter<std::string_view, wchar_t>;
public:
    using BASE::parse;
    using BASE::format;
};

template<>
struct std::formatter<std::string, wchar_t> : private std::formatter<std::string_view, wchar_t>
{
    using BASE = std::formatter<std::string_view, wchar_t>;
public:
    using BASE::parse;
    using BASE::format;
};

template<typename CharT>
struct std::formatter<wxString, CharT> : private std::formatter<std::basic_string<CharT>, CharT>
{
private:
    using BASE = formatter<std::basic_string<CharT>, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const wxString& s, FormatContext& ctx) const
    {
        CB::string temp(s);
        return BASE::format(static_cast<const CharT*>(temp), ctx);
    }
};

/////////////////////////////////////////////////////////////////////////////

// TODO:  when drop MFC THROW/AfxThrow, use std::bad_cast
class CBadCastException : public CException
{
};

inline void __declspec(noreturn) CbThrowBadCastException()
{
    ASSERT(!"bad cast");
    THROW(new CBadCastException());
}

/////////////////////////////////////////////////////////////////////////////

// exception if DEST can't represent src
template<typename DEST, typename SRC>
constexpr std::enable_if_t<is_always_value_preserving_v<DEST, SRC>, DEST> value_preserving_cast(SRC src)
{
    return static_cast<DEST>(src);
}

template<typename DEST, typename SRC>
constexpr std::enable_if_t<!is_always_value_preserving_v<DEST, SRC>, DEST> value_preserving_cast(SRC src)
{
    if (!is_value_preserving<DEST>(src))
    {
        CbThrowBadCastException();
    }
    return static_cast<DEST>(src);
}

/////////////////////////////////////////////////////////////////////////////

namespace CB
{
    /* defining Invalid<> required removing the MS min/max
        macros.  On the other hand, std::min/max don't allow
        different argument types.  So provide our own min/max,
        and check for value preservation while we're at it. */
    template<typename LEFT, typename RIGHT>
    std::common_type_t<LEFT, RIGHT> max(LEFT left, RIGHT right)
    {
        typedef std::common_type_t<LEFT, RIGHT> CommonType;
        static_assert(is_always_value_preserving_v<CommonType, LEFT> &&
                        is_always_value_preserving_v<CommonType, RIGHT>,
                        "unsafe conversion not supported");
        CommonType left2(static_cast<CommonType>(left));
        CommonType right2(static_cast<CommonType>(right));
        return !(left2 < right2) ? left2 : right2;
    }

    template<typename LEFT, typename RIGHT>
    std::common_type_t<LEFT, RIGHT> min(LEFT left, RIGHT right)
    {
        typedef std::common_type_t<LEFT, RIGHT> CommonType;
        static_assert(is_always_value_preserving_v<CommonType, LEFT> &&
                        is_always_value_preserving_v<CommonType, RIGHT>,
                        "unsafe conversion not supported");
        CommonType left2(static_cast<CommonType>(left));
        CommonType right2(static_cast<CommonType>(right));
        return left2 < right2 ? left2 : right2;
    }
}

/////////////////////////////////////////////////////////////////////////////

template<wchar_t PREFIX_, typename UNDERLYING_TYPE_, std::enable_if_t<std::is_integral_v<UNDERLYING_TYPE_> && std::is_unsigned_v<UNDERLYING_TYPE_>, bool> = true>
class XxxxIDExt
{
public:
    static constexpr wchar_t PREFIX = PREFIX_;
    using UNDERLYING_TYPE = UNDERLYING_TYPE_;

    XxxxIDExt() = default;
    explicit constexpr XxxxIDExt(uint32_t i) : id(value_preserving_cast<UNDERLYING_TYPE>(i)) {}

    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    explicit constexpr XxxxIDExt(T i) : XxxxIDExt(value_preserving_cast<uint32_t>(i)) {}

    XxxxIDExt(const XxxxIDExt&) = default;
    XxxxIDExt& operator=(const XxxxIDExt&) = default;
    ~XxxxIDExt() = default;

    explicit constexpr operator UNDERLYING_TYPE() const { return id; }

    bool operator==(const XxxxIDExt& rhs) const { return id == rhs.id; }
    bool operator!=(const XxxxIDExt& rhs) const { return !operator==(rhs); }

private:
    UNDERLYING_TYPE id;

    // serialize conversion helpers
    template<typename OTHER_UL>
    explicit constexpr XxxxIDExt(XxxxIDExt<PREFIX, OTHER_UL> other) :
        XxxxIDExt(other != Invalid_v<XxxxIDExt<PREFIX, OTHER_UL>> ?
                        value_preserving_cast<UNDERLYING_TYPE>(static_cast<OTHER_UL>(other))
                    :
                        static_cast<UNDERLYING_TYPE>(Invalid_v<XxxxIDExt>))
    {
    }

    friend class SerializeBackdoor;
};

template<wchar_t PREFIX, typename UNDERLYING_TYPE, typename CharT>
struct std::formatter<XxxxIDExt<PREFIX, UNDERLYING_TYPE>, CharT> : private std::formatter<UNDERLYING_TYPE, CharT>
{
private:
    using BASE = formatter<UNDERLYING_TYPE, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(const XxxxIDExt<PREFIX, UNDERLYING_TYPE>& id, FormatContext& ctx) const
    {
        return BASE::format(static_cast<UNDERLYING_TYPE>(id), ctx);
    }
};

// goal is 32bit ids, but currently ids are 16 bit
template<wchar_t PREFIX>
using XxxxID16 = XxxxIDExt<PREFIX, uint16_t>;

template<wchar_t PREFIX>
using XxxxID32 = XxxxIDExt<PREFIX, uint32_t>;

template<wchar_t PREFIX>
using XxxxID = XxxxID32<PREFIX>;

// KLUDGE:  use OwnerPtr since CWordArray doesn't have move operators
template<wchar_t PREFIX, typename UNDERLYING_TYPE>
const OwnerPtr<CWordArray> ToCWordArray(const std::vector<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>& v)
{
    OwnerPtr<CWordArray> retval = MakeOwner<CWordArray>();
    retval->SetSize(value_preserving_cast<INT_PTR>(v.size()));
    for (INT_PTR i = 0 ; i < retval->GetSize() ; ++i)
    {
        (*retval)[i] = value_preserving_cast<uint16_t>(static_cast<UNDERLYING_TYPE>(v[value_preserving_cast<size_t>(i)]));
    }
    return retval;
}

template<typename T>
std::vector<T> ToVector(const CWordArray& a)
{
    static_assert(std::is_same_v<T, XxxxIDExt<T::PREFIX, typename T::UNDERLYING_TYPE>>, "requires XxxxIDExt<>");
    std::vector<T> retval;
    retval.reserve(value_preserving_cast<size_t>(a.GetSize()));
    for (INT_PTR i = 0 ; i < a.GetSize() ; ++i)
    {
        retval.push_back(static_cast<T>(a[i]));
    }
    return retval;
}

// KLUDGE:  use OwnerPtr since CWordArray doesn't have move operators
template<wchar_t PREFIX, typename UNDERLYING_TYPE>
const OwnerPtr<CDWordArray> ToCDWordArray(const std::vector<XxxxIDExt<PREFIX, UNDERLYING_TYPE>>& v)
{
    OwnerPtr<CDWordArray> retval = MakeOwner<CDWordArray>();
    retval->SetSize(value_preserving_cast<INT_PTR>(v.size()));
    for (INT_PTR i = 0 ; i < retval->GetSize() ; ++i)
    {
        (*retval)[i] = value_preserving_cast<uint32_t>(static_cast<UNDERLYING_TYPE>(v[value_preserving_cast<size_t>(i)]));
    }
    return retval;
}

template<typename T>
std::vector<T> ToVector(const CDWordArray& a)
{
    static_assert(std::is_same_v<T, XxxxIDExt<T::PREFIX, typename T::UNDERLYING_TYPE>>, "requires XxxxIDExt<>");
    std::vector<T> retval;
    retval.reserve(value_preserving_cast<size_t>(a.GetSize()));
    for (INT_PTR i = 0; i < a.GetSize(); ++i)
    {
        retval.push_back(static_cast<T>(a[i]));
    }
    return retval;
}

template<typename DEST, wchar_t PREFIX, typename UNDERLYING_TYPE>
constexpr std::enable_if_t<is_always_value_preserving_v<DEST, UNDERLYING_TYPE>, DEST> value_preserving_cast(XxxxIDExt<PREFIX, UNDERLYING_TYPE> src)
{
    return static_cast<DEST>(static_cast<UNDERLYING_TYPE>(src));
}

template<typename DEST, wchar_t PREFIX, typename UNDERLYING_TYPE>
constexpr std::enable_if_t<!is_always_value_preserving_v<DEST, UNDERLYING_TYPE>, DEST> value_preserving_cast(XxxxIDExt<PREFIX, UNDERLYING_TYPE> src)
{
    if (!is_value_preserving<DEST>(static_cast<UNDERLYING_TYPE>(src)))
    {
        CbThrowBadCastException();
    }
    return static_cast<DEST>(static_cast<UNDERLYING_TYPE>(src));
}

/////////////////////////////////////////////////////////////////////////////

/* Factor some repetitive code for tables indexed by ID values
    into a new XxxxIDTable<>. */
template<typename KEY, typename ELEMENT,
    size_t baseSize, size_t incrSize,
    bool saveInGPlay,
    size_t (*calcAllocSize)(size_t uiDesired, size_t uiBaseSize, size_t uiExtendSize),
    bool triviallyCopyable = std::is_trivially_copyable_v<ELEMENT>>
class XxxxIDTable;

template<typename KEY, typename ELEMENT,
        size_t baseSize, size_t incrSize,
        bool saveInGPlay,
        size_t (*calcAllocSize)(size_t uiDesired, size_t uiBaseSize, size_t uiExtendSize)>
class XxxxIDTable<KEY, ELEMENT, baseSize, incrSize, saveInGPlay, calcAllocSize, true>
{
    static_assert(std::is_same_v<KEY, XxxxIDExt<KEY::PREFIX, KEY::UNDERLYING_TYPE>>, "requires XxxxIDExt<>");
    static_assert(std::is_trivially_copyable_v<ELEMENT>, "ELEMENT must be trivially copyable");

public:
    size_t maxSize() const
    {
        return std::min(value_preserving_cast<size_t>(std::numeric_limits<ptrdiff_t>::max() / sizeof(ELEMENT)), value_preserving_cast<size_t>(std::numeric_limits<KEY::UNDERLYING_TYPE>::max()));
    }

    XxxxIDTable() noexcept
    {
        m_pTbl = NULL;
        m_nTblSize = 0;
    }

    XxxxIDTable(const XxxxIDTable& other) :
        XxxxIDTable()
    {
        *this = other;
    }

    XxxxIDTable& operator=(const XxxxIDTable& other)
    {
        ResizeTable(other.GetSize(), nullptr);
        /* ELEMENT is trivially copyable, so this is safe
            (and more efficient than for loop with assignments) */
        memcpy(m_pTbl, other.m_pTbl, GetSize()*sizeof(ELEMENT));
        return *this;
    }

    XxxxIDTable(XxxxIDTable&& other) noexcept
    {
        m_pTbl = other.m_pTbl;
        m_nTblSize = other.m_pTbl;
        other.m_pTbl = NULL;
        other.m_pTbl = 0;
    }

    XxxxIDTable& operator=(XxxxIDTable&& other) noexcept
    {
        if (m_pTbl != NULL) free(m_pTbl);
        m_pTbl = other.m_pTbl;
        m_nTblSize = other.m_pTbl;
        other.m_pTbl = NULL;
        other.m_pTbl = 0;
    }

    ~XxxxIDTable()
    {
        Clear();
    }

    bool operator==(nullptr_t) const { return Empty(); }
    bool operator!=(nullptr_t) const { return !operator==(nullptr); }
    bool Empty() const { return !m_pTbl; }
    size_t GetSize() const { return m_nTblSize; }
    bool Valid(KEY tid) const { return static_cast<KEY::UNDERLYING_TYPE>(tid) < m_nTblSize; }

    const ELEMENT& operator[](KEY tid) const { return m_pTbl[value_preserving_cast<ptrdiff_t>(static_cast<KEY::UNDERLYING_TYPE>(tid))]; }
    ELEMENT& operator[](KEY tid) { return const_cast<ELEMENT&>(std::as_const(*this)[tid]); }

    void Clear()
    {
        if (m_pTbl != NULL) free(m_pTbl);
        m_pTbl = NULL;
        m_nTblSize = 0;
    }

    /* WARNING:  will need to be generalized if an ELEMENT
        without IsEmpty() wants to use this */
    KEY CreateIDEntry(void (ELEMENT::*initializer)())
    {
        ASSERT(initializer);
        // Allocate from empty entry if possible
        for (size_t i = 0; i < m_nTblSize; i++)
        {
            if (m_pTbl[i].IsEmpty())
                return static_cast<KEY>(i);
        }
        // Get TileID from end of table.
        if (m_nTblSize >= maxSize())
        {
            AfxThrowMemoryException();
        }
        KEY newXid = static_cast<KEY>(m_nTblSize);
        ResizeTable(m_nTblSize + 1, initializer);
        return newXid;
    }

    void ResizeTable(size_t nEntsNeeded, void (ELEMENT::*initializer)())
    {
        if (nEntsNeeded == 0)
        {
            Clear();
            return;
        }

        size_t nNewSize = (*calcAllocSize)(nEntsNeeded, baseSize, incrSize);
        if (m_pTbl != NULL)
        {
            ELEMENT* pNewTbl = (ELEMENT*)realloc(
                m_pTbl, nNewSize * sizeof(ELEMENT));
            if (pNewTbl == NULL)
                AfxThrowMemoryException();
            m_pTbl = pNewTbl;
        }
        else
        {
            m_pTbl = (ELEMENT*)malloc(nNewSize * sizeof(ELEMENT));
            if (m_pTbl == NULL)
                AfxThrowMemoryException();
        }
        if (initializer)
        {
            for (size_t i = m_nTblSize; i < nNewSize; i++)
                (m_pTbl[i].*initializer)();
        }
        m_nTblSize = nNewSize;
    }

    void Serialize(CArchive& ar)
    {
        /* m_nTblSize is serialized in key format to match
            file v3.90, and size will become new key whenever
            adding to a full table, so treating it as a key
            here doesn't seem all that peculiar */
        if (ar.IsStoring())
        {
            constexpr bool inGplay =
#ifdef GPLAY
                true
#else
                false
#endif
                ;
            if (!inGplay || saveInGPlay)
            {
                ar << static_cast<KEY>(m_nTblSize);
                for (size_t i = 0; i < m_nTblSize; i++)
                    m_pTbl[i].Serialize(ar);
            }
        }
        else
        {
            KEY wTmp;
            ar >> wTmp;
            if (static_cast<KEY::UNDERLYING_TYPE>(wTmp) > size_t(0))
            {
                ResizeTable(value_preserving_cast<size_t>(static_cast<KEY::UNDERLYING_TYPE>(wTmp)), nullptr);
                for (size_t i = size_t(0); i < m_nTblSize; i++)
                    m_pTbl[value_preserving_cast<ptrdiff_t>(i)].Serialize(ar);
            }
        }
    }

private:
    ELEMENT*    m_pTbl;         // Global def'ed
    size_t      m_nTblSize;     // Number of alloc'ed ents in tile table
};

template<typename KEY, typename ELEMENT,
    size_t baseSize, size_t incrSize,
    bool saveInGPlay,
    size_t(*calcAllocSize)(size_t uiDesired, size_t uiBaseSize, size_t uiExtendSize)>
class XxxxIDTable<KEY, ELEMENT, baseSize, incrSize, saveInGPlay, calcAllocSize, false>
{
    static_assert(std::is_same_v<KEY, XxxxIDExt<KEY::PREFIX, KEY::UNDERLYING_TYPE>>, "requires XxxxIDExt<>");
    static_assert(!std::is_trivially_copyable_v<ELEMENT>, "inefficient; use other XxxxIDTable impl instead");

public:
    size_t maxSize() const
    {
        return std::min(m_pTbl.max_size(), value_preserving_cast<size_t>(std::numeric_limits<KEY::UNDERLYING_TYPE>::max()));
    }

    XxxxIDTable() noexcept = default;

    XxxxIDTable(const XxxxIDTable& other) :
        XxxxIDTable()
    {
        *this = other;
    }

    XxxxIDTable& operator=(const XxxxIDTable& other) = default;

    XxxxIDTable(XxxxIDTable&& other) noexcept = default;

    XxxxIDTable& operator=(XxxxIDTable&& other) noexcept = default;

    ~XxxxIDTable() = default;

    bool operator==(nullptr_t) const { return Empty(); }
    bool operator!=(nullptr_t) const { return !operator==(nullptr); }
    bool Empty() const { return m_pTbl.empty(); }
    size_t GetSize() const { return m_pTbl.size(); }
    bool Valid(KEY tid) const { return static_cast<KEY::UNDERLYING_TYPE>(tid) < GetSize(); }

    const ELEMENT& operator[](KEY tid) const { return m_pTbl[value_preserving_cast<size_t>(static_cast<KEY::UNDERLYING_TYPE>(tid))]; }
    ELEMENT& operator[](KEY tid) { return const_cast<ELEMENT&>(std::as_const(*this)[tid]); }

    void Clear()
    {
        m_pTbl.clear();
    }

    /* WARNING:  will need to be generalized if an ELEMENT
        without IsEmpty() wants to use this */
    KEY CreateIDEntry(void (ELEMENT::* initializer)())
    {
        ASSERT(initializer);
        // Allocate from empty entry if possible
        for (size_t i = size_t(0) ; i < m_pTbl.size() ; ++i)
        {
            if (m_pTbl[i].IsEmpty())
                return static_cast<KEY>(i);
        }
        // Get TileID from end of table.
        if (m_pTbl.size() >= maxSize())
        {
            AfxThrowMemoryException();
        }
        KEY newXid = static_cast<KEY>(GetSize());
        ResizeTable(GetSize() + size_t(1), initializer);
        return newXid;
    }

    void ResizeTable(size_t nEntsNeeded, void (ELEMENT::* initializer)())
    {
        if (nEntsNeeded == size_t(0))
        {
            Clear();
            return;
        }

        size_t oldSize = m_pTbl.size();
        size_t nNewSize = (*calcAllocSize)(nEntsNeeded, baseSize, incrSize);
        m_pTbl.resize(nNewSize);
        if (initializer)
        {
            for (size_t i = oldSize ; i < nNewSize ; ++i)
                (m_pTbl[i].*initializer)();
        }
    }

    void Serialize(CArchive& ar)
    {
        /* m_nTblSize is serialized in key format to match
            file v3.90, and size will become new key whenever
            adding to a full table, so treating it as a key
            here doesn't seem all that peculiar */
        if (ar.IsStoring())
        {
            constexpr bool inGplay =
#ifdef GPLAY
                true
#else
                false
#endif
                ;
ASSERT(!inGplay || saveInGPlay);
            if (!inGplay || saveInGPlay)
            {
                ar << static_cast<KEY>(m_pTbl.size());
                for (size_t i = size_t(0) ; i < m_pTbl.size() ; ++i)
                    m_pTbl[i].Serialize(ar);
            }
        }
        else
        {
            KEY wTmp;
            ar >> wTmp;
            if (static_cast<KEY::UNDERLYING_TYPE>(wTmp) > size_t(0))
            {
                ResizeTable(value_preserving_cast<size_t>(static_cast<KEY::UNDERLYING_TYPE>(wTmp)), nullptr);
                for (size_t i = size_t(0) ; i < m_pTbl.size() ; ++i)
                    m_pTbl[i].Serialize(ar);
            }
        }
    }

private:
    std::vector<ELEMENT> m_pTbl;
};

template<typename KEY, typename ELEMENT,
        size_t baseSize, size_t incrSize,
        bool saveInGPlay,
        size_t (*calcAllocSize)(size_t uiDesired, size_t uiBaseSize, size_t uiExtendSize),
        bool triviallyCopyable>
CArchive& operator<<(CArchive& ar, const XxxxIDTable<KEY, ELEMENT, baseSize, incrSize, saveInGPlay, calcAllocSize, triviallyCopyable>& v)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    const_cast<XxxxIDTable<KEY, ELEMENT, baseSize, incrSize, saveInGPlay, calcAllocSize, triviallyCopyable>&>(v).Serialize(ar);
    return ar;
}

template<typename KEY, typename ELEMENT,
        size_t baseSize, size_t incrSize,
        bool saveInGPlay,
        size_t(*calcAllocSize)(size_t uiDesired, size_t uiBaseSize, size_t uiExtendSize),
        bool triviallyCopyable>
CArchive& operator>>(CArchive& ar, XxxxIDTable<KEY, ELEMENT, baseSize, incrSize, saveInGPlay, calcAllocSize, triviallyCopyable>& v)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    v.Serialize(ar);
    return ar;
}

/////////////////////////////////////////////////////////////////////////////

/* Some operations should only be used to support handling file
    format differences.  Make them available here. */
class SerializeBackdoor
{
public:
    // use RAII to turn on/off these back doors
    SerializeBackdoor()
    {
        ++Depth();
    }
    ~SerializeBackdoor()
    {
        --Depth();
    }

    template<wchar_t PREFIX, typename UNDERLYING_TYPE_SRC, typename UNDERLYING_TYPE_DEST = std::conditional_t<std::is_same_v<UNDERLYING_TYPE_SRC, uint16_t>, uint32_t, uint16_t>>
    static XxxxIDExt<PREFIX, UNDERLYING_TYPE_DEST> Convert(const XxxxIDExt<PREFIX, UNDERLYING_TYPE_SRC>& xid)
    {
        if (!Depth())
        {
            ASSERT(!"only for serialize use");
            AfxThrowNotSupportedException();
        }
        return static_cast<XxxxIDExt<PREFIX, UNDERLYING_TYPE_DEST>>(xid);
    }

protected:
    static int& Depth()
    {
        static int retval = 0;
        return retval;
    }
};

/////////////////////////////////////////////////////////////////////////////

namespace CB { namespace Impl {
    template<typename... Args>
    void Cpp20Trace(const char* file, int line, std::format_string<Args...> fmt, Args&&... args)
    {
        CB::string temp = std::vformat(fmt.get(), std::make_format_args(args...));
        CB::string temp2 = std::format(L"{}({}):  {}", file, line, temp);
        OutputDebugStringW(temp2);
    }

    template<typename... Args>
    void Cpp20Trace(const char* file, int line, std::wformat_string<Args...> fmt, Args&&... args)
    {
        CB::string temp = std::vformat(fmt.get(), std::make_wformat_args(args...));
        CB::string temp2 = std::format(L"{}({}):  {}", file, line, temp);
        OutputDebugStringW(temp2);
    }

    // TODO:  pay attention to cat and level
    template<typename... Args>
    void Cpp20Trace(const char* file, int line, DWORD cat, int level, std::format_string<Args...> fmt, Args&&... args)
    {
        CB::string temp = std::vformat(fmt.get(), std::make_format_args(args...));
        CB::string temp2 = std::format(L"{}({}):  {}", file, line, temp);
        OutputDebugStringW(temp2);
    }

    // TODO:  pay attention to cat and level
    template<typename... Args>
    void Cpp20Trace(const char* file, int line, DWORD cat, int level, std::wformat_string<Args...> fmt, Args&&... args)
    {
        CB::string temp = std::vformat(fmt.get(), std::make_wformat_args(args...));
        CB::string temp2 = std::format(L"{}({}):  {}", file, line, temp);
        OutputDebugStringW(temp2);
    }
}}
#define DBGREL_CPP20_TRACE(fmt, ...) CB::Impl::Cpp20Trace(__FILE__, __LINE__, fmt, __VA_ARGS__)
#if !defined(NDEBUG)
    #define CPP20_TRACE(fmt, ...) DBGREL_CPP20_TRACE(fmt, __VA_ARGS__)
#else
    #define CPP20_TRACE(fmt, ...) true ? void(0) : CB::Impl::Cpp20Trace(__FILE__, __LINE__, fmt, __VA_ARGS__)
#endif

/////////////////////////////////////////////////////////////////////////////

// N.B.:  don't mix MS clipboard and wx clipboard functions
class LockWxClipboard
{
public:
    LockWxClipboard() :
        LockWxClipboard(std::try_to_lock)
    {
        if (!open)
        {
            throw std::runtime_error("open clipboard failed");
        }
    }
    LockWxClipboard(std::try_to_lock_t) { TryLock(); }
    LockWxClipboard(std::defer_lock_t) {}
    ~LockWxClipboard()
    {
        if (open)
        {
            Unlock();
        }
    }
    explicit operator bool() const { return open; }
    bool TryLock()
    {
        ASSERT(!open);
        open = wxTheClipboard->Open();
        ASSERT(open);
        return open;
    }
    void Unlock()
    {
        ASSERT(open);
        wxTheClipboard->Close();
    }
private:
    bool open = false;
};

/////////////////////////////////////////////////////////////////////////////

namespace CB {
    inline CString string::mfc_str() const { return CString(v_str(), value_preserving_cast<int>(v_size())); }
}

/////////////////////////////////////////////////////////////////////////////

static_assert(std::is_same_v<std::vector<int>::size_type, size_t>, "std::vector index is not size_t");
static_assert(std::is_same_v<std::vector<int>::iterator::difference_type, ptrdiff_t>, "std::vector iterator offset is not ptrdiff_t");

/////////////////////////////////////////////////////////////////////////////

/* AfxGetApp() docs say it may return NULL during process
    startup, so declare a function that we will guarantee returns
    an object during startup */
CWinApp& CbGetApp();
namespace CB
{
    wxNativeContainerWindow& GetMainWndWx();
    string GetAppName();
}

// helpers for setting up wxDialog instances
namespace CB
{
    // provide a more convenient way to set up wxNumValidator<> objects
    template<typename T,
        typename RETVAL = std::conditional_t<std::is_floating_point_v<T>, wxFloatingPointValidator<T>, wxIntegerValidator<T>>>
    RETVAL MakeValidator(T& var, T min, T max, int style = wxNUM_VAL_DEFAULT)
    {
        RETVAL retval(&var, style);
        retval.SetMin(min);
        retval.SetMax(max);
        if constexpr (std::is_floating_point_v<T>)
        {
            retval.SetPrecision(3);
        }
        return retval;
    }

    inline wxTextCtrl& SetValidator(wxTextCtrl& ctrl, wxString& var,
                                    wxTextValidatorStyle style = wxFILTER_NONE, unsigned long maxLen = 0)
    {
        ctrl.SetValidator(wxTextValidator(style, &var));
        ctrl.SetMaxLength(maxLen);
        return ctrl;
    }

    template<typename VAR, typename = std::enable_if_t<std::is_arithmetic_v<VAR>>>
    wxTextCtrl& SetValidator(wxTextCtrl& ctrl, VAR& var, VAR min, VAR max)
    {
        ctrl.SetValidator(CB::MakeValidator(var, min, max));
        return ctrl;
    }

    template<typename CTRL, typename VAR>
    CTRL& SetValidator(CTRL& ctrl, VAR& var)
    {
        ctrl.SetValidator(wxGenericValidator(&var));
        return ctrl;
    }
}

/* m_dummy is a way to call LoadDialog()
    before the Refs are initialized */
#define CB_XRC_BEGIN_CTRLS_DECL() RefPtr<wxObject> m_dummy;
#define CB_XRC_END_CTRLS_DECL()

#define CB_XRC_BEGIN_CTRLS_DEFN(parent, id) m_dummy(wxXmlResource::Get()->LoadDialog(this, parent, #id) ? this : nullptr)
#define CB_XRC_END_CTRLS_DEFN()

#define CB_XRC_CTRL(ctrl) , ctrl(XRCCTRL(*this, #ctrl, std::remove_reference_t<decltype(*ctrl)>))

/* second arg is data var
    CB_XRC_CTRL_VAL(ctrl*&, bool&)
    CB_XRC_CTRL_VAL(ctrl*&, wxString&, wxTextValidatorStyle, unsigned long maxlen)
    CB_XRC_CTRL_VAL(ctrl*&, int&, min, max)
*/
#define CB_XRC_CTRL_VAL(ctrl, ...) , ctrl(&CB::SetValidator(*XRCCTRL(*this, #ctrl, std::remove_reference_t<decltype(*ctrl)>), __VA_ARGS__))

// provide conversions between COLORREF and wxColour
namespace CB
{
    inline wxColour Convert(COLORREF c)
    {
        return wxColour(GetRValue(c), GetGValue(c), GetBValue(c));
    }

    inline COLORREF Convert(wxColour c)
    {
        return RGB(c.Red(), c.Green(), c.Blue());
    }
}

#endif
