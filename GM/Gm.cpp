// Gm.cpp : Defines the class behaviors for the application.
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

#include    "stdafx.h"

#include    "io.h"
#include    "Gm.h"
#include    "GmDoc.h"

#include    "FrmMain.h"
#include    "FrmView.h"
#include    "FrmBxdoc.h"
#include    "FrmBited.h"
#include    "VwEdtbrd.h"
#include    "VwBitedt.h"
#include    "VwTilesl.h"
#include    "VwPrjgbx.h"

#include    "ResTbl.h"
#include    "StrLib.h"

#if 0
namespace CB
{
    std::string Sprintf(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int rc = vsnprintf(nullptr, 0, fmt, args);
        va_end(args);
        if (rc < 0)
        {
            AfxThrowInvalidArgException();
        }
        // +1 for null terminator
        std::string retval(rc + 1, '\0');
        va_start(args, fmt);
        rc = vsnprintf(&retval[0], retval.size(), fmt, args);
        va_end(args);
        if (rc < 0)
        {
            AfxThrowInvalidArgException();
        }
        retval.resize(rc);
        return retval;
    }
}

static class Test
{
public:
    Test()
    {
        int8_t one = 1, two = 2, three = 3, four = 4, five = 5, six = 6;
        size_t big = 0x100000000000;
        std::string s1 = CB::Sprintf("%u:%u:%u:%u:%u:%u:%u:%u", big, one, two, three, four, five, six, big);
        std::string s2 = std::format("{}:{}:{}:{}:{}:{}:{}:{}", big, one, two, three, four, five, six, big);
        TRACE("\n%s\n%s\n", s1.c_str(), s2.c_str());
        std::string s3 = std::format("{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}", big, one, two, three, four, five, six, big);
        std::wstring s4 = std::format(L"{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}:{:#08x}", big, one, two, three, four, five, six, big);
        TRACE("\n%s\n%ls\n", s3.c_str(), s4.c_str());

        TRACE("%s\n", std::format("{0:15}", s3).c_str());
        TRACE("%ls\n", std::format(L"{:15}", s4).c_str());

        TRACE("%s\n", std::format("{:3.15}", s4).c_str());
        TRACE("%s\n", std::format("{:3.15}", s4.c_str()).c_str());
        TRACE("%s\n", std::format("{:15}", L"test").c_str());

        TRACE("%ls\n", std::format(L"{:15}", s3).c_str());
        TRACE("%ls\n", std::format(L"{:15}", s3.c_str()).c_str());
        TRACE("%ls\n", std::format(L"{:15}", "test").c_str());

        TRACE("%ls\n", std::format(L"{1:3.15}", L"unused", s3).c_str());

        TRACE("%s\n", std::format("{:#010x}", CSize(15, 16)).c_str());
        TRACE("%ls\n", std::format(L"{:#010x}", CSize(15, 16)).c_str());
        TRACE("%s\n", std::format("{:#010x}", CPoint(15, 16)).c_str());
        TRACE("%ls\n", std::format(L"{:#010x}", CPoint(15, 16)).c_str());
        TRACE("%s\n", std::format("{:#010x}", CRect(15, 16, 17, 18)).c_str());
        TRACE("%s\n", std::format(L"{:#010x}", CRect(15, 16, 17, 18)).c_str());

        TRACE("%s", std::format("{}\n", GetCBFile4Features()).c_str());
        TRACE("%ls", std::format(L"{}\n", GetCBFile4Features()).c_str());
        TRACE("%s", std::format("{}\n", *GetCBFile4Features().begin()).c_str());
        TRACE("%ls", std::format(L"{}\n", *GetCBFile4Features().begin()).c_str());

#if 0   // this gets rejected at compile time due to type mismatch
        TRACE("%s\n", std::format("{:3.15x}", s3).c_str());
#endif
        exit(0);
    }
} test;
#endif

#if 1
namespace {
    // demonstrate MFC/wx discrepancy
    class RectDiscrepancy
    {
    public:
        RectDiscrepancy()
        {
            CRect cr(11, 11, 20, 20);
            wxASSERT(cr.TopLeft() == CPoint(11, 11));
            wxASSERT(cr.BottomRight() == CPoint(20, 20));
            wxRect wxr(wxPoint(11, 11), wxPoint(20, 20));
            wxASSERT(wxr.GetTopLeft() == wxPoint(11, 11));
            wxASSERT(wxr.GetBottomRight() == wxPoint(20, 20));
            wxASSERT(wxr.GetTopLeft() == CB::Convert(cr.TopLeft()));
            wxASSERT(wxr.GetBottomRight() == CB::Convert(cr.BottomRight()));
            wxASSERT(!cr.PtInRect(CPoint(10, 10)));
            wxASSERT(!wxr.Contains(wxPoint(10, 10)));
            wxASSERT(cr.PtInRect(CPoint(11, 11)));
            wxASSERT(wxr.Contains(wxPoint(11, 11)));
            wxASSERT(cr.PtInRect(CPoint(19, 19)));
            wxASSERT(wxr.Contains(wxPoint(19, 19)));
            wxASSERT(!cr.PtInRect(CPoint(21, 21)));
            wxASSERT(!wxr.Contains(wxPoint(21, 21)));

            // and now, trouble:
            wxASSERT(!cr.PtInRect(CPoint(20, 20)));
            wxASSERT(wxr.Contains(wxPoint(20, 20)));
            wxASSERT(wxr.GetWidth() == cr.Width() + 1);
            wxASSERT(wxr.GetHeight() == cr.Height() + 1);

            // on the other hand
            wxRect wxr2(wxPoint(11, 11), wxPoint(19, 19));
            wxASSERT(wxr2.GetTopLeft() == wxPoint(11, 11));
            wxASSERT(wxr2.GetBottomRight() == wxPoint(19, 19));
            wxASSERT(wxr2.GetTopLeft() == CB::Convert(cr.TopLeft()));
            wxASSERT(wxr2.GetBottomRight() != CB::Convert(cr.BottomRight()));
            wxASSERT(wxr2.GetWidth() == cr.Width());
            wxASSERT(wxr2.GetHeight() == cr.Height());
            wxASSERT(!wxr2.Contains(wxPoint(10, 10)));
            wxASSERT(wxr2.Contains(wxPoint(11, 11)));
            wxASSERT(wxr2.Contains(wxPoint(19, 19)));
            wxASSERT(!wxr2.Contains(wxPoint(20, 20)));
            wxASSERT(!wxr2.Contains(wxPoint(21, 21)));

            /* given the results above, I think the least bad
                result is for conversion to preserve which points
                are in the rectangles (and also size)
                rather than preserve bottom-right equality */
            wxRect wxr3 = CB::Convert(cr);
            wxASSERT(CB::Convert(wxr3) == cr);
            wxASSERT(wxr3 == wxr2);
            wxASSERT(wxr3.GetWidth() == cr.Width());
            wxASSERT(wxr3.GetHeight() == cr.Height());
            wxASSERT(!wxr3.Contains(wxPoint(10, 10)));
            wxASSERT(wxr3.Contains(wxPoint(11, 11)));
            wxASSERT(wxr3.Contains(wxPoint(19, 19)));
            wxASSERT(!wxr3.Contains(wxPoint(20, 20)));
            wxASSERT(!wxr3.Contains(wxPoint(21, 21)));

            CPP20_TRACE("cr {}\n", cr);
            CPP20_TRACE("wxr {}\n", wxr);
            CPP20_TRACE("wxr2 {}\n", wxr2);
            CPP20_TRACE("wxr3 {}\n", wxr3);
        }
    } rectDiscrepancy;
}
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxDEFINE_EVENT(WM_SETCOLOR_WX, SetColorEvent);
wxDEFINE_EVENT(WM_SETLINEWIDTH_WX, SetLineWidthEvent);
wxDEFINE_EVENT(WM_SETCUSTOMCOLOR_WX, SetCustomColorEvent);
wxDEFINE_EVENT(WM_PALETTE_HIDE_WX, wxCommandEvent);

/////////////////////////////////////////////////////////////////////////////
// Registry keys...

static const CB::string szSectSettings = "Settings";
static const CB::string szSectDisableHtmlHelp = "DisableHtmlHelp";

/////////////////////////////////////////////////////////////////////////////

const UINT WM_DRAGDROP = RegisterWindowMessage("msgDragDrop"_cbstring);
wxDEFINE_EVENT(WM_DRAGDROP_WX, DragDropEvent);

const wxDataFormat CF_TILEIMAGES(wxString("cfCyberBoardTileImages"_cbstring));
const wxDataFormat CF_TIDLIST(wxString("cfCyberBoardTileIDList"_cbstring));
const wxDataFormat CF_GBOXID(wxString("cfCyberBoardGameBoxID"_cbstring));

/////////////////////////////////////////////////////////////////////////////

static const CB::string HELP_TIP_CONTEXT_FILE = "::/gmcontext.txt";
static const CB::string HELP_FILE = "cboard.chm";

/////////////////////////////////////////////////////////////////////////////
// Registry keys to delete...

static const CB::string szGbxShellNew = ".gbx\\ShellNew";

static const CB::string szPrint        = "print";
static const CB::string szPrintTo      = "printto";
static const CB::string szGameBox      = "CyberBoardGameBox";
// std::format() requires constant expression format string
static constexpr const wchar_t* szShell        = L"{}\\shell\\{}";
static constexpr const wchar_t* szShellCommand = L"{}\\shell\\{}\\command";
static constexpr const wchar_t* szShellDdeexec = L"{}\\shell\\{}\\ddeexec";

static void DeletePrintKeys(const CB::string& szFileClass);

/////////////////////////////////////////////////////////////////////////////
// CGmApp

BEGIN_MESSAGE_MAP(CGmApp, CWinAppEx)
    //{{AFX_MSG_MAP(CGmApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_HELP_WEBSITE, OnHelpWebsite)
    ON_COMMAND(ID_HELP_RELEASES, OnHelpReleases)
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

static void DeletePrintStyleKeys(const CB::string& szFileClass, const CB::string& szPrintStyle)
{
    CB::string szKeyBfr = std::format(szShellCommand, szFileClass, szPrintStyle);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKeyBfr);
    szKeyBfr = std::format(szShellDdeexec, szFileClass, szPrintStyle);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKeyBfr);
    szKeyBfr = std::format(szShell, szFileClass, szPrintStyle);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKeyBfr);
}

static void DeletePrintKeys(const CB::string& szFileClass)
{
    DeletePrintStyleKeys(szFileClass, szPrint);
    DeletePrintStyleKeys(szFileClass, szPrintTo);
}

/////////////////////////////////////////////////////////////////////////////
// CGmApp construction

CGmApp::CGmApp()
{
    m_dwHtmlHelpCookie = 0;
    m_bDisableHtmlHelp = FALSE;
    m_hHtmlProcessHandle = NULL;
    m_pMapViewTmpl = NULL;
    m_pTileEditTmpl = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGmApp object

class CGmApp::CwxGmApp : public wxMFCApp<CGmApp>
{
protected:
    BOOL InitMainWnd() override
    {
        // Create main MDI Frame window
        CMainFrame* pMainFrame = new CMainFrame;
        if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
        {
            if (pMainFrame != NULL)
                delete pMainFrame;
            return FALSE;
        }
        m_pMainWnd = pMainFrame;

        // Enable drag/drop open
        m_pMainWnd->DragAcceptFiles();

        // The main window has been initialized, so show and update it.
        if (!ReloadWindowPlacement(pMainFrame))
        {
            pMainFrame->ShowWindow(m_nCmdShow);
            pMainFrame->UpdateWindow();
        }

        wxTheApp->SetTopWindow(&CB::GetMainWndWx());

        return TRUE;
    }
};

CGmApp& CGmApp::Get()
{
    /* make this a function variable to ensure
        it's constructed before use */
    static CwxGmApp theApp;
    return theApp;
}
CWinApp& CbGetApp()
{
    return CGmApp::Get();
}

namespace {
    class wxCGmApp : public wxAppWithMFC
    {
    public:
        virtual bool OnInit() override
        {
            // handling cmd line w/ MFC, so skip wxCmdLineParser

            wxXmlResource::Get()->InitAllHandlers();
            wxCHECK(wxXmlResource::Get()->LoadFile(wxStandardPaths::Get().GetDataDir() + "/CBDesign.xrc"), false);

            return true;
        }
    };
}
wxDECLARE_APP(wxCGmApp);
// Notice use of wxIMPLEMENT_APP_NO_MAIN() instead of the usual wxIMPLEMENT_APP!
wxIMPLEMENT_APP_NO_MAIN(wxCGmApp);

/////////////////////////////////////////////////////////////////////////////
// CGmApp initialization

BOOL CGmApp::InitInstance()
{
    // Make sure language files are located and loaded.
    if (!CWinAppEx::InitInstance())
        return FALSE;

    // Starting with the Vista release an application needs elevated access to
    // write into HKEY_CLASSES_ROOT. We can't assume the user has this level
    // of access so the application CLASSES_ROOT must be redirected to
    // "HKEY_CURRENT_USER\Software\Classes".
    AfxSetPerUserRegistration(TRUE);

    // Change the registry key under which our settings are stored.
    // You should modify this string to be something appropriate
    // such as the name of your company or organization.
    SetRegistryKey("CyberBoard V4.00"_cbstring);

    m_bDisableHtmlHelp = GetProfileInt(szSectSettings, szSectDisableHtmlHelp, 0);
    if (!m_bDisableHtmlHelp)
        ::HtmlHelp(NULL, NULL, HH_INITIALIZE, reinterpret_cast<uintptr_t>(&m_dwHtmlHelpCookie));

    g_gt.InitGdiTools();
    g_res.InitResourceTable(m_hInstance);

    // Load standard INI file options (including MRU)
    LoadStdProfileSettings(10);

    InitContextMenuManager();
    InitTooltipManager();

    CMFCToolTipInfo ttParams;
    ttParams.m_bVislManagerTheme = TRUE;
    CGmApp::Get().GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
        RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

    //////////////////////////////////////////////////////////////////////
    // Switch to our shared help file.

    if (m_pszHelpFilePath != NULL)
    {
        CB::string buff = m_pszHelpFilePath;

        size_t dip = buff.rfind('\\');
        if (dip != CB::string::npos)
        {
            dip++;
            buff.resize(dip);
            buff += HELP_FILE;
            if (_waccess(buff, 4) == 0)
            {
                free(const_cast<LPTSTR>(m_pszHelpFilePath));
                m_pszHelpFilePath = _tcsdup(buff);
            }
        }
    }

    // Register the application's document templates.  Document templates
    // serve as the connection between documents, frame windows and views.
    AddDocTemplate(new CMultiDocTemplate(IDR_GAMEBOX,
        RUNTIME_CLASS(CGamDoc),
        RUNTIME_CLASS(CDocFrame),
        RUNTIME_CLASS(CGbxProjView)));

    m_pMapViewTmpl = new CMultiDocTemplate(IDR_BOARDVIEW,
        RUNTIME_CLASS(CGamDoc), RUNTIME_CLASS(CViewFrame),
        RUNTIME_CLASS(CBrdEditView));

    m_pTileEditTmpl = new CMultiDocTemplate(IDR_BITEDITOR,
        RUNTIME_CLASS(CGamDoc), RUNTIME_CLASS(CBitEditFrame),
        RUNTIME_CLASS(CBitEditViewContainer));

    EnableLoadWindowPlacement(FALSE);

    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);

    // Remove the ShellNew key since we don't support it...
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szGbxShellNew);
    // Remove print and printto shell commands since we don't support them
    DeletePrintKeys(szGameBox);

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    // Don't allow the app to be hidden for DDE operations.
    // CB doesn't need to operate in that mode. On some people's
    // systems the app remains hidden!
    if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileDDE)
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

    // The following prevents the OnFileNew from being called
    // when the app is run without a filename.
    if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
        cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;

    // Dispatch commands specified on the command line
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    if (GetCurrentVideoResolution() < 16)
    {
        AfxMessageBox(IDP_ERR_MINCOLORS, MB_OK | MB_ICONSTOP);
        return FALSE;
    }
wxString test = wxString("hello, €") + "world\nline2\n";
wxASSERT(CB::string(test).wx_str() == test);
OutputDebugStringA(test);
OutputDebugStringW(test);
CPP20_TRACE("test:  {}line3\n", test);
CPP20_TRACE(L"test:  {}line3\n", test);

    return TRUE;
}

int CGmApp::ExitInstance()
{
    if (!m_bDisableHtmlHelp && m_dwHtmlHelpCookie != 0)
    {
        ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
        ::HtmlHelp(NULL, NULL, HH_UNINITIALIZE, m_dwHtmlHelpCookie);
    }

    CWinAppEx::ExitInstance();

    if (m_pMapViewTmpl != NULL) delete m_pMapViewTmpl;
    if (m_pTileEditTmpl != NULL) delete m_pTileEditTmpl;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CGmApp::DoHelpShellLaunch()
{
    if (m_hHtmlProcessHandle != NULL)
    {
        // Check if still running.
        DWORD dwStatus;
        if (GetExitCodeProcess(m_hHtmlProcessHandle, &dwStatus))
        {
            if (dwStatus == STILL_ACTIVE)
                return;
        }
        CloseHandle(m_hHtmlProcessHandle);
        m_hHtmlProcessHandle = NULL;

    }
    SHELLEXECUTEINFO execInfo;
    memset(&execInfo, 0, sizeof(execInfo));
    execInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    execInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    execInfo.lpVerb = "open"_cbstring;
    execInfo.lpFile = m_pszHelpFilePath;
    execInfo.nShow = SW_SHOWNORMAL;
    if (ShellExecuteEx(&execInfo))
        m_hHtmlProcessHandle = execInfo.hProcess;
}

void CGmApp::DoHelpContents()
{
    if (!m_bDisableHtmlHelp)
        ::HtmlHelp(0, m_pszHelpFilePath, HH_DISPLAY_TOC, NULL);
    else
        DoHelpShellLaunch();
}

void CGmApp::DoHelpContext(uintptr_t dwContextData)
{
    if (!m_bDisableHtmlHelp)
        ::HtmlHelp(0, m_pszHelpFilePath, HH_HELP_CONTEXT, dwContextData);
}

void CGmApp::DoHelpTopic(const CB::string& pszTopic)
{
    if (m_bDisableHtmlHelp)
        return;
    CB::string strHelpPath = m_pszHelpFilePath;
    strHelpPath += "::/";
    strHelpPath += pszTopic;
    CPP20_TRACE("HtmlHelp Topic: {}\n", strHelpPath);
    ::HtmlHelp(0, strHelpPath, HH_DISPLAY_TOPIC, NULL);
}

BOOL CGmApp::DoHelpTipHelp(HELPINFO* pHelpInfo, DWORD* dwIDArray)
{
    if (m_bDisableHtmlHelp)
        return TRUE;
    if ((DWORD)pHelpInfo->iCtrlId == IDC_STATIC)
        return TRUE;                            // Ignore static controls
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        CB::string strHelpPath = m_pszHelpFilePath;
        strHelpPath += HELP_TIP_CONTEXT_FILE;
        return ::HtmlHelp((HWND)pHelpInfo->hItemHandle,
            strHelpPath, HH_TP_HELP_WM_HELP, reinterpret_cast<uintptr_t>(dwIDArray)) != NULL;
    }
    return TRUE;
}

void CGmApp::DoHelpWhatIsHelp(CWnd* pWndCtrl, DWORD* dwIDArray)
{
    if (m_bDisableHtmlHelp)
        return;
    if (pWndCtrl->GetDlgCtrlID() == IDC_STATIC)
        return;                                     // Ignore static controls
    if (pWndCtrl->GetDlgCtrlID() == 0)
        return;                                     // Ignore windows with no ID
    CB::string strHelpPath = m_pszHelpFilePath;
    strHelpPath += HELP_TIP_CONTEXT_FILE;
    ::HtmlHelp(pWndCtrl->GetSafeHwnd(), strHelpPath,
        HH_TP_HELP_CONTEXTMENU, reinterpret_cast<uintptr_t>(dwIDArray));
}

/////////////////////////////////////////////////////////////////////////////
// Returns TRUE if WM_QUIT encountered.

BOOL CGmApp::DispatchMessages()
{
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            if (GetMainFrame() != NULL && ::IsWindow(GetMainFrame()->GetSafeHwnd()))
                GetMainFrame()->PostMessage(WM_QUIT, 0, 0);
            return TRUE;                // Inform caller the WM_QUIT was queued
        }
        if (!PreTranslateMessage(&msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    AfxGetApp()->OnIdle(0);   // updates user interface
    AfxGetApp()->OnIdle(1);   // frees temporary objects
    return FALSE;
}

///////////////////////////////////////////////////////////////////////
// This method transmits a hint to all open documents.

void CGmApp::UpdateAllViewsForAllDocuments(CView* pSender, LPARAM lHint /*  = 0 */, CObject* pHint /* = NULL */)
{
    POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();

    while (pos != NULL)
    {
        CDocTemplate* pTemplate = (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
        POSITION pos2 = pTemplate->GetFirstDocPosition();
        while (pos2)
        {
            CDocument* pDoc = (CDocument*)pTemplate->GetNextDoc(pos2);
            ASSERT(pDoc != NULL);
            ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CDocument)));
            pDoc->UpdateAllViews(pSender, lHint, pHint);
        }
    }
}


BOOL CGmApp::OnIdle(LONG lCount)
{
    // Don't process idle if main frame has been destroyed. This
    // will only happen if the WM_QUIT message has been lost such
    // as when a context menu is visible. Therefore we also
    // cause the quit message to be reposted so the message
    // pump will exit.
    if (GetMainFrame() == NULL)
    {
        PostQuitMessage(0);
        return TRUE;
    }

    // Inform all open documents of idle condition.
    CDocument* pCurDoc = GetMainFrame()->GetCurrentDocument();

    POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();

    BOOL bAppVisible = m_pMainWnd->IsWindowVisible() &&
        !m_pMainWnd->IsIconic();

    while (pos != NULL)
    {
        CDocTemplate* pTemplate =
            (CDocTemplate*)m_pDocManager->GetNextDocTemplate(pos);
        POSITION pos2 = pTemplate->GetFirstDocPosition();
        while (pos2)
        {
            CGamDoc* pDoc = (CGamDoc*)pTemplate->GetNextDoc(pos2);
            ASSERT(pDoc != NULL);
            ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CGamDoc)));
            pDoc->OnIdle(bAppVisible && pDoc == pCurDoc);

        }
    }
    // Main idle processing...
    ((CMainFrame*)m_pMainWnd)->OnIdle();
    // Finally MFC idle processing...
    return CWinAppEx::OnIdle(lCount);
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_GM_ABOUTBOX };
    CStatic m_staticGenDate;
    CStatic m_staticProgVer;
    CStatic m_staticFileVer;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //{{AFX_MSG(CAboutDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    DDX_Control(pDX, IDC_D_ABOUT_GENDATE, m_staticGenDate);
    DDX_Control(pDX, IDC_D_ABOUT_VERSION, m_staticProgVer);
    DDX_Control(pDX, IDC_D_ABOUT_FILEVER, m_staticFileVer);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CGmApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

#ifdef _DEBUG
static const CB::string szGenDate = __DATE__;
#endif

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CB::string str = CB::string::Format(IDP_PROGVER, progVerMajor, progVerMinor);
    m_staticProgVer.SetWindowText(str);

    str = CB::string::Format(IDP_FILEVER, fileGbxVerMajor, fileGbxVerMinor);
    m_staticFileVer.SetWindowText(str);
#ifdef _DEBUG
    m_staticGenDate.SetWindowText(szGenDate);
#else
    m_staticGenDate.ShowWindow(SW_HIDE);
#endif

    CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CGmApp commands

void CGmApp::OnHelpWebsite()
{
    CB::string strUrl = CB::string::LoadString(IDS_URL_CB_WEBSITE);
    ShellExecute(NULL, "open"_cbstring, strUrl, NULL, NULL, SW_SHOWNORMAL);
}

void CGmApp::OnHelpReleases()
{
    CB::string strUrl = CB::string::LoadString(IDS_URL_CB_RELEASES);
    ShellExecute(NULL, "open"_cbstring, strUrl, NULL, NULL, SW_SHOWNORMAL);
}

wxNativeContainerWindow& CB::GetMainWndWx()
{
    return *dynamic_cast<CMainFrame*>(AfxGetMainWnd());
}

CB::string CB::GetAppName()
{
    return AfxGetAppName();
}
