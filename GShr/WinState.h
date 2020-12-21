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
    CWinStateManager() { m_pDoc = NULL; m_pList = NULL; }
    CWinStateManager(CDocument* pDoc) { m_pDoc = pDoc; m_pList = NULL; }

    virtual ~CWinStateManager();

public:
    void SetDocument(CDocument* pDoc) { m_pDoc = pDoc; }

    BOOL GetStateOfOpenDocumentFrames();
    void RestoreStateOfDocumentFrames();

    void Serialize(CArchive& ar);

    enum { wincodeUnknown = 0, wincodeMainFrame = 1, wincodeViewFrame = 2, wincodeToolPal = 3 };

protected:
    struct CWinStateElement
    {
        WORD  m_wWinCode;           // Generic type of window
        WORD  m_wUserCode1;         // Used by subclass to refine WinCode
        WORD  m_wUserCode2;         // Used by subclass to refine WinCode
        CWinPlacement m_wndState;   // Window placement information

        DWORD m_dwWinStateBfrSize;  // Size of serialized window data
        BYTE* m_pWinStateBfr;       // Serialized window data

        CWinStateElement();
        virtual ~CWinStateElement();
        void Serialize(CArchive& ar);
    };

protected:
    CWinStateElement* GetWindowState(CWnd* pWnd);
    BOOL RestoreWindowState(CWnd* pWnd, CWinStateElement* pWse);
    void GetDocumentFrameList(std::vector<CFrameWnd*>& tblFrames);
    CWnd* GetDocumentFrameHavingRuntimeClass(CRuntimeClass* pClass);

    void ArrangeFrameListInZOrder(std::vector<CFrameWnd*>& tblFrames);
    static BOOL CALLBACK EnumFrames(HWND hWnd, LPARAM dwTblFramePtr);

    void SetUpListIfNeedTo();
    void DestroyList();

    // Required override used to locate or, if necessary, recreate frames.
    // This is called when window states are being restored.
    virtual CWnd* OnGetFrameForWinStateElement(CWinStateElement* pWse) = 0;

    // Allow subclass to create specialized version of CWinStateElement
    virtual CWinStateElement* OnCreateWinStateElement() { return new CWinStateElement; }

    // Allow subclass to add more information.
    virtual void OnAnnotateWinStateElement(CWinStateElement* pWse, CWnd* pWnd) {}

protected:
    CDocument*  m_pDoc;
    CPtrList*   m_pList;            // Win state element list
};

#endif

