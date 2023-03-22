// Gm.cpp : Defines the class behaviors for the application.
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
#include <format>

// unfortunately, these aren't provided by default (due to needing encoding conversion)
namespace std {
    template<>
    struct formatter<std::wstring_view, char> : formatter<std::string, char>
    {
        using BASE = formatter<std::string, char>;
    public:
        using BASE::parse;

        template<typename FormatContext>
        constexpr auto format(const std::wstring_view& s, FormatContext& ctx)
        {
            // TODO:  replace this with proper encoding conversion
            std::string temp;
            temp.reserve(s.size());
            for (size_t i = 0; i < s.size(); ++i) {
                temp += static_cast<char>(value_preserving_cast<uint8_t>(s[i]));
            }
            return BASE::format(temp, ctx);
        }
    };

    template<>
    struct formatter<std::wstring, char> : formatter<std::wstring_view, char>
    {
        using BASE = formatter<std::wstring_view, char>;
    public:
        using BASE::parse;
        using BASE::format;
    };

    template<>
    struct formatter<const wchar_t*, char> : formatter<std::wstring_view, char>
    {
        using BASE = formatter<std::wstring_view, char>;
    public:
        using BASE::parse;
        using BASE::format;
    };

    template<size_t n>
    struct formatter<wchar_t [n], char> : formatter<std::wstring_view, char>
    {
        using BASE = formatter<std::wstring_view, char>;
    public:
        using BASE::parse;
        using BASE::format;
    };

    template<>
    struct formatter<std::string_view, wchar_t> : formatter<std::wstring, wchar_t>
    {
        using BASE = formatter<std::wstring, wchar_t>;
    public:
        using BASE::parse;

        template<typename FormatContext>
        constexpr auto format(const std::string_view& s, FormatContext& ctx)
        {
            // TODO:  replace this with proper encoding conversion
            std::wstring temp;
            temp.reserve(s.size());
            for (size_t i = 0; i < s.size(); ++i) {
                temp += static_cast<wchar_t>(static_cast<uint8_t>(s[i]));
            }
            return BASE::format(temp, ctx);
        }
    };

    template<>
    struct formatter<std::string, wchar_t> : formatter<std::string_view, wchar_t>
    {
        using BASE = formatter<std::string_view, wchar_t>;
    public:
        using BASE::parse;
        using BASE::format;
    };

    template<>
    struct formatter<const char*, wchar_t> : formatter<std::string_view, wchar_t>
    {
        using BASE = formatter<std::string_view, wchar_t>;
    public:
        using BASE::parse;
        using BASE::format;
    };

    template<size_t n>
    struct formatter<char [n], wchar_t> : formatter<std::string_view, wchar_t>
    {
        using BASE = formatter<std::string_view, wchar_t>;
    public:
        using BASE::parse;
        using BASE::format;
    };
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

#if 0   // this gets rejected at compile time due to type mismatch
        TRACE("%s\n", std::format("{:3.15x}", s3).c_str());
#endif
        exit(0);
    }
} test;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Registry keys...

static char szSectSettings[] = "Settings";
static char szSectDisableHtmlHelp[] = "DisableHtmlHelp";

/////////////////////////////////////////////////////////////////////////////

const UINT WM_DRAGDROP = RegisterWindowMessage("msgDragDrop");

UINT CF_TILEIMAGES = RegisterClipboardFormat("cfCyberBoardTileImages");
UINT CF_TIDLIST = RegisterClipboardFormat("cfCyberBoardTileIDList");
UINT CF_GBOXID = RegisterClipboardFormat("cfCyberBoardGameBoxID");

/////////////////////////////////////////////////////////////////////////////

static const char HELP_TIP_CONTEXT_FILE[] = "::/gmcontext.txt";
static const char HELP_FILE[] = "cboard.chm";

/////////////////////////////////////////////////////////////////////////////
// Registry keys to delete...

static const TCHAR szGbxShellNew[] = ".gbx\\ShellNew";

static const TCHAR szPrint[]        = "print";
static const TCHAR szPrintTo[]      = "printto";
static const TCHAR szGameBox[]      = "CyberBoardGameBox";
static const TCHAR szShell[]        = "%s\\shell\\%s";
static const TCHAR szShellCommand[] = "%s\\shell\\%s\\command";
static const TCHAR szShellDdeexec[] = "%s\\shell\\%s\\ddeexec";

static void DeletePrintKeys(LPCTSTR szFileClass);

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
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

static void DeletePrintStyleKeys(LPCTSTR szFileClass, LPCTSTR szPrintStyle)
{
    TCHAR szKeyBfr[128];
    sprintf(szKeyBfr, szShellCommand, szFileClass, szPrintStyle);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKeyBfr);
    sprintf(szKeyBfr, szShellDdeexec, szFileClass, szPrintStyle);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKeyBfr);
    sprintf(szKeyBfr, szShell, szFileClass, szPrintStyle);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKeyBfr);
}

static void DeletePrintKeys(LPCTSTR szFileClass)
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

CGmApp& CGmApp::Get()
{
    /* make this a function variable to ensure
        it's constructed before use */
    static CGmApp theApp;
    return theApp;
}
CWinApp& CbGetApp()
{
    return CGmApp::Get();
}

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
    SetRegistryKey("CyberBoard V4.00");

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
        char buff[_MAX_PATH];
        strcpy(buff, m_pszHelpFilePath);

        char *dip = strrchr(buff, '\\');
        if (dip != NULL)
        {
            dip++;
            *dip = '\0';
            strcat(buff, HELP_FILE);
            if (_access(buff, 4) == 0)
            {
                free((char*)m_pszHelpFilePath);
                m_pszHelpFilePath = _strdup(buff);
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
        RUNTIME_CLASS(CBitEditView));

    EnableLoadWindowPlacement(FALSE);

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

    // The main window has been initialized, so show and update it.
    if (!ReloadWindowPlacement(pMainFrame))
    {
        pMainFrame->ShowWindow(m_nCmdShow);
        pMainFrame->UpdateWindow();
    }

    if (GetCurrentVideoResolution() < 16)
    {
        AfxMessageBox(IDP_ERR_MINCOLORS, MB_OK | MB_ICONSTOP);
        return FALSE;
    }

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
    execInfo.lpVerb = "open";
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

void CGmApp::DoHelpTopic(LPCTSTR pszTopic)
{
    if (m_bDisableHtmlHelp)
        return;
    CString strHelpPath = m_pszHelpFilePath;
    strHelpPath += "::/";
    strHelpPath += pszTopic;
    TRACE1("HtmlHelp Topic: %s\n", (LPCTSTR)strHelpPath);
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
        CString strHelpPath = m_pszHelpFilePath;
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
    CString strHelpPath = m_pszHelpFilePath;
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
static char szGenDate[] = __DATE__;
#endif

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    char szNum1[20];
    char szNum2[20];
    CString str;

    _itoa(progVerMajor, szNum1, 10);
    _itoa(progVerMinor + 100, szNum2, 10);

    AfxFormatString2(str, IDP_PROGVER, szNum1, szNum2 + 1);
    m_staticProgVer.SetWindowText(str);

    _itoa(fileGbxVerMajor, szNum1, 10);
    _itoa(fileGbxVerMinor, szNum2, 10);
    AfxFormatString2(str, IDP_FILEVER, szNum1, szNum2);
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
    CString strUrl;
    strUrl.LoadString(IDS_URL_CB_WEBSITE);
    ShellExecute(NULL, "open", strUrl, NULL, NULL, SW_SHOWNORMAL);
}

void CGmApp::OnHelpReleases()
{
    CString strUrl;
    strUrl.LoadString(IDS_URL_CB_RELEASES);
    ShellExecute(NULL, "open", strUrl, NULL, NULL, SW_SHOWNORMAL);
}

