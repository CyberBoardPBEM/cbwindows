// WinState.h - classes used to manage window state.
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
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

#ifndef _WINSTATE_H
#define _WINSTATE_H

#include <list>

///////////////////////////////////////////////////////////////////////

// This message is sent to each frame than holds document views.
// The frame should serialize all the information required to restore
// the frame's *contents* to it's current visual state.

#define WM_WINSTATE   (WM_USER + 300)  // WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

///////////////////////////////////////////////////////////////////////
// Helper class for working with the Windows WINDOWPLACEMENT structure

struct CWinPlacement : public WINDOWPLACEMENT
{
    CWinPlacement();

    friend CArchive& AFXAPI operator<<(CArchive& ar, const CWinPlacement& wndPlace);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CWinPlacement& wndPlace);
};

///////////////////////////////////////////////////////////////////////

class CWinStateManager
{
public:
    CWinStateManager() { m_pDoc = NULL; }
    CWinStateManager(CDocument* pDoc) { m_pDoc = pDoc; }

    virtual ~CWinStateManager() = default;

public:
    void SetDocument(CDocument* pDoc) { m_pDoc = pDoc; }

    BOOL GetStateOfOpenDocumentFrames();
    void RestoreStateOfDocumentFrames();

    void Serialize(CArchive& ar);

    enum { wincodeUnknown = 0, wincodeMainFrame = 1, wincodeViewFrame = 2, wincodeToolPal = 3 };

protected:
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        ~Buffer() = default;

        size_t GetSize() const { return size; }
        operator const BYTE*() const
        {
            static_assert(sizeof(ptr) == sizeof(BYTE*), "wasted space");
            return ptr.get();
        }
        operator BYTE*() { return const_cast<BYTE*>(static_cast<const BYTE*>(std::as_const(*this))); }

        void Reset()
        {
            Reset(nullptr, size_t(0));
        }
        void Reset(BYTE* p, size_t s)
        {
            ptr.reset(p);
            size = ptr ? s : size_t(0);
        }

    private:
        class Free
        {
        public:
            void operator()(BYTE* p) const { free(p); }
        };
        std::unique_ptr<BYTE[], Free> ptr;
        size_t size = size_t(0);
    };

    struct CWinStateElement
    {
        WORD  m_wWinCode;           // Generic type of window
        WORD  m_wUserCode1;         // Used by subclass to refine WinCode
        WORD  m_wUserCode2;         // Used by subclass to refine WinCode
        CWinPlacement m_wndState;   // Window placement information

        Buffer m_pWinStateBfr;       // Serialized window data

        CWinStateElement();
        virtual ~CWinStateElement() = default;
        void Serialize(CArchive& ar);
    };

protected:
    OwnerPtr<CWinStateElement> GetWindowState(CWnd* pWnd);
    BOOL RestoreWindowState(CWnd* pWnd, CWinStateElement& pWse);
    void GetDocumentFrameList(std::vector<CB::not_null<CFrameWnd*>>& tblFrames);
    CWnd* GetDocumentFrameHavingRuntimeClass(CRuntimeClass* pClass);

    void ArrangeFrameListInZOrder(std::vector<CB::not_null<CFrameWnd*>>& tblFrames);
    static BOOL CALLBACK EnumFrames(HWND hWnd, LPARAM dwTblFramePtr);

    void SetUpListIfNeedTo();

    // Required override used to locate or, if necessary, recreate frames.
    // This is called when window states are being restored.
    virtual CWnd* OnGetFrameForWinStateElement(const CWinStateElement& pWse) /* override */ = 0;

    // Allow subclass to create specialized version of CWinStateElement
    virtual OwnerPtr<CWinStateElement> OnCreateWinStateElement() /* override */ { return MakeOwner<CWinStateElement>(); }

    // Allow subclass to add more information.
    virtual void OnAnnotateWinStateElement(CWinStateElement& pWse, CWnd* pWnd) /* override */ {}

protected:
    CDocument*  m_pDoc;
    typedef std::list<OwnerPtr<CWinStateElement>> CWinStateList;
    OwnerOrNullPtr<CWinStateList> m_pList;            // Win state element list
};

#endif

