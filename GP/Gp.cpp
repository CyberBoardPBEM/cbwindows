//  Gp.cpp : Defines the class behaviors for the application.
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
#include    "io.h"          // for _access()
#include    "Gp.h"
#include    "afxpriv.h"

#include    "ResTbl.h"
#include    "GdiTools.h"
#include    "LibMfc.h"

#include    "FrmMain.h"
#include    "FrmPbrd.h"
#include    "FrmProj.h"
#include    "GamDoc.h"
#include    "VwPbrd.h"
#include    "VwPrjgsn.h"
#include    "VwPrjgam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Registry keys...

static const CB::string szSectSettings = "Settings";
static const CB::string szSectDisableHtmlHelp = "DisableHtmlHelp";

/////////////////////////////////////////////////////////////////////////////

const UINT WM_DRAGDROP = RegisterWindowMessage("msgDragDrop"_cbstring);
wxDEFINE_EVENT(WM_DRAGDROP_WX, DragDropEvent);

/////////////////////////////////////////////////////////////////////////////

static const CB::string HELP_TIP_CONTEXT_FILE = "::/gpcontext.txt";
static const CB::string HELP_FILE = "cboard.chm";

/////////////////////////////////////////////////////////////////////////////
// Registry keys to delete...

static const CB::string szGsnShellNew = ".gsn\\ShellNew";
static const CB::string szGamShellNew = ".gam\\ShellNew";

static const CB::string szPrint        = "print";
static const CB::string szPrintTo      = "printto";
static const CB::string szGame         = "CyberBoardGame";
static const CB::string szScenario     = "CyberBoardScenario";
static constexpr const wchar_t* szShell        = L"{}\\shell\\{}";
static constexpr const wchar_t* szShellCommand = L"{}\\shell\\{}\\command";
static constexpr const wchar_t* szShellDdeexec = L"{}\\shell\\{}\\ddeexec";

static void DeletePrintKeys(const CB::string& szFileClass);

/////////////////////////////////////////////////////////////////////////////
// Registry related string...

static const CB::string szGmvExtension = ".gmv";
static const CB::string szGmvType = "CyberBoardRecording";
static const CB::string szGmvTypeDesc = "CyberBoard Recording";
static const CB::string szGmvIconKey = "CyberBoardRecording\\DefaultIcon";

/////////////////////////////////////////////////////////////////////////////
// CGpApp

BEGIN_MESSAGE_MAP(CGpApp, CWinAppEx)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_HELP_WEBSITE, OnHelpWebsite)
    ON_COMMAND(ID_HELP_RELEASES, OnHelpReleases)
    ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGpApp construction

CGpApp::CGpApp()
{
    m_dwHtmlHelpCookie = 0;
    m_bDisableHtmlHelp = FALSE;
    m_pBrdViewTmpl = NULL;
    m_pScnDocTemplate = NULL;
    m_hHtmlProcessHandle = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGpApp object

class CGpApp::CwxGpApp : public wxMFCApp<CGpApp>
{
protected:
    BOOL InitMainWnd() override
    {
        // Create main MDI Frame window
        CMainFrame* pMainFrame = new CMainFrame;
        if (!pMainFrame || !pMainFrame->LoadFrame(IDR_GP_MAINFRAME))
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

        OnIdle(0);   // Run idle so control bars are in sync prior to message boxes

        return TRUE;
    }
};

CGpApp& CGpApp::Get()
{
    /* make this a function variable to ensure
        it's constructed before use */
    static CwxGpApp theApp;
    return theApp;
}
CWinApp& CbGetApp()
{
    return CGpApp::Get();
}

namespace {
    class wxCGpApp : public wxAppWithMFC
    {
    public:
        virtual bool OnInit() override
        {
            // handling cmd line w/ MFC, so skip wxCmdLineParser
            return true;
        }
    };
}
wxDECLARE_APP(wxCGpApp);
// Notice use of wxIMPLEMENT_APP_NO_MAIN() instead of the usual wxIMPLEMENT_APP!
wxIMPLEMENT_APP_NO_MAIN(wxCGpApp);

/////////////////////////////////////////////////////////////////////////////
// CGpApp initialization

BOOL CGpApp::InitInstance()
{
    // Make sure language files are located and loaded.
    if (!CWinAppEx::InitInstance())
        return FALSE;

    // Starting with the Vista release an application needs elevated access to
    // write into HKEY_CLASSES_ROOT. We can't assume the user has this level
    // of access so the application's CLASSES_ROOT must be redirected to
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
    CGpApp::Get().GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
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

    CMultiDocTemplate* pDocTemplate;

    pDocTemplate = new CMultiDocTemplate(
        IDR_GAMETYPE,
        RUNTIME_CLASS(CGamDoc),
        RUNTIME_CLASS(CProjFrame),
        RUNTIME_CLASS(CGamProjView));
    AddDocTemplate(pDocTemplate);

    m_pScnDocTemplate = new CMultiDocTemplate(
        IDR_GSCNTYPE,
        RUNTIME_CLASS(CGamDoc),
        RUNTIME_CLASS(CProjFrame),
        RUNTIME_CLASS(CGsnProjView));
    AddDocTemplate(m_pScnDocTemplate);

    m_pBrdViewTmpl = new CMultiDocTemplate(
        IDR_GP_BOARDVIEW,
        RUNTIME_CLASS(CGamDoc),
        RUNTIME_CLASS(CPlayBoardFrame),
        RUNTIME_CLASS(CPlayBoardView));

    EnableLoadWindowPlacement(FALSE);

    // Enable DDE Execute open
    EnableShellOpen();
    RegisterShellFileTypes(TRUE);

    // Add an icon association for move files.
    CB::string szFName = CB::string::GetModuleFileName(NULL);
    szFName += ",4";
    AfxRegSetValue(HKEY_CLASSES_ROOT, szGmvExtension, REG_SZ, szGmvType, value_preserving_cast<DWORD>(szGmvType.v_size()));
    AfxRegSetValue(HKEY_CLASSES_ROOT, szGmvType, REG_SZ, szGmvTypeDesc, value_preserving_cast<DWORD>(szGmvTypeDesc.v_size()));
    AfxRegSetValue(HKEY_CLASSES_ROOT, szGmvIconKey, REG_SZ, szFName, value_preserving_cast<DWORD>(szFName.v_size()));

    // Remove the ShellNew keys since we don't support them...
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szGsnShellNew);
    AfxRegDeleteKey(HKEY_CLASSES_ROOT, szGamShellNew);

    // Remove print and printto shell commands since we don't support them
    DeletePrintKeys(szGame);
    DeletePrintKeys(szScenario);

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

    return TRUE;
}

int CGpApp::ExitInstance()
{
    if (!m_bDisableHtmlHelp && m_dwHtmlHelpCookie != 0)
    {
        ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, 0);
        ::HtmlHelp(NULL, NULL, HH_UNINITIALIZE, m_dwHtmlHelpCookie);
    }

    CWinAppEx::ExitInstance();

    if (m_pBrdViewTmpl != NULL) delete m_pBrdViewTmpl;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// This little hack is used to translate spacebar hits into playback
// next move commands.
BOOL CGpApp::PreTranslateMessage(MSG *pMsg)
{
    if (pMsg->message == WM_CHAR && pMsg->wParam == ' ')
        m_pMainWnd->PostMessage(WM_COMMAND, MAKEWPARAM(uint16_t(ID_PBCK_NEXT), uint16_t(0)));
    return CWinAppEx::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// Intercept this call so we can force the current directory to the
// one contained in the LRU.

BOOL CGpApp::OnOpenRecentFile(UINT nID)
{
    ASSERT(nID >= ID_FILE_MRU_FILE1);
    ASSERT(nID < ID_FILE_MRU_FILE1 + (UINT)m_pRecentFileList->GetSize());
    int nIndex = nID - ID_FILE_MRU_FILE1;

    CB::string strFileName = (*m_pRecentFileList)[nIndex];
    if (!strFileName.empty())
    {
        size_t nPathEnd = strFileName.rfind('\\');
        if (nPathEnd != CB::string::npos)
        {
            strFileName = strFileName.substr(size_t(0), nPathEnd);
            SetCurrentDirectory(strFileName);
        }
    }
    return CWinAppEx::OnOpenRecentFile(nID);
}

/////////////////////////////////////////////////////////////////////////////
// lParam is ptr to ProcID number. Set to window handle if found.

void CGpApp::DoHelpShellLaunch()
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

void CGpApp::DoHelpContents()
{
    if (!m_bDisableHtmlHelp)
        ::HtmlHelp(0, m_pszHelpFilePath, HH_DISPLAY_TOC, NULL);
    else
        DoHelpShellLaunch();
}

void CGpApp::DoHelpContext(uintptr_t dwContextData)
{
    if (!m_bDisableHtmlHelp)
        ::HtmlHelp(0, m_pszHelpFilePath, HH_HELP_CONTEXT, dwContextData);
}

void CGpApp::DoHelpTopic(const CB::string& pszTopic)
{
    if (m_bDisableHtmlHelp)
        return;
    CB::string strHelpPath = m_pszHelpFilePath;
    strHelpPath += "::/";
    strHelpPath += pszTopic;
    CPP20_TRACE("HtmlHelp Topic: {}\n", strHelpPath);
    ::HtmlHelp(0, strHelpPath, HH_DISPLAY_TOPIC, NULL);
}

BOOL CGpApp::DoHelpTipHelp(HELPINFO* pHelpInfo, DWORD* dwIDArray)
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

void CGpApp::DoHelpWhatIsHelp(CWnd* pWndCtrl, DWORD* dwIDArray)
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

BOOL CGpApp::OnIdle(LONG lCount)
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
    CDocument* pCurDoc = GetCurrentDocument();

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

//////////////////////////////////////////////////////////////////////////////

// Returns TRUE if WM_QUIT encountered.
BOOL CGpApp::DispatchMessages()
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

void CGpApp::Delay(int nMilliSecs, BOOL* pbBailOut /* = NULL */)
{
    DWORD dwTick = GetTickCount();
    while (GetTickCount() - dwTick < (DWORD)nMilliSecs)
    {
        DispatchMessages();
        if (pbBailOut != NULL && *pbBailOut)
            return;                             // Bail out
    }
}

//////////////////////////////////////////////////////////////////////////////

CDocument* CGpApp::GetCurrentDocument()
{
    return GetMainFrame()->GetCurrentDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CStatic m_staticGenDate;
    CStatic m_staticGSNVer;
    CStatic m_staticProgVer;
    CStatic m_staticGMVVer;
    CStatic m_staticGBXVer;
    CStatic m_staticGAMVer;
    //}}AFX_DATA

// Implementation
protected:
    void SetupVersion(CStatic& s, int major, int minor, UINT nRes);

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
    DDX_Control(pDX, IDC_D_ABOUT_GSNVER, m_staticGSNVer);
    DDX_Control(pDX, IDC_D_ABOUT_PROGVER, m_staticProgVer);
    DDX_Control(pDX, IDC_D_ABOUT_GMVVER, m_staticGMVVer);
    DDX_Control(pDX, IDC_D_ABOUT_GBXVER, m_staticGBXVer);
    DDX_Control(pDX, IDC_D_ABOUT_GAMVER, m_staticGAMVer);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAboutDlg::SetupVersion(CStatic& s, int major, int minor, UINT nRes)
{
    CB::string str = CB::string::Format(nRes, major, minor);
    s.SetWindowText(str);
}

#ifdef _DEBUG
static const CB::string szGenDate = __DATE__;
#endif

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetupVersion(m_staticProgVer, progVerMajor, progVerMinor, IDP_PROGVER);
    SetupVersion(m_staticGBXVer, fileGbxVerMajor, fileGbxVerMinor, IDP_GBXVER);
    SetupVersion(m_staticGSNVer, fileGsnVerMajor, fileGsnVerMinor, IDP_GSNVER);
    SetupVersion(m_staticGAMVer, fileGamVerMajor, fileGamVerMinor, IDP_GAMVER);
    SetupVersion(m_staticGMVVer, fileGmvVerMajor, fileGmvVerMinor, IDP_GMVVER);
#ifdef _DEBUG
    m_staticGenDate.SetWindowText(szGenDate);
#else
    m_staticGenDate.ShowWindow(SW_HIDE);
#endif

    CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CGpApp commands

// App command to run the about dialog
void CGpApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CGpApp::OnHelpWebsite()
{
    CB::string strUrl = CB::string::LoadString(IDS_URL_CB_WEBSITE);
    ShellExecute(NULL, "open"_cbstring, strUrl, NULL, NULL, SW_SHOWNORMAL);
}

void CGpApp::OnHelpReleases()
{
    CB::string strUrl = CB::string::LoadString(IDS_URL_CB_RELEASES);
    ShellExecute(NULL, "open"_cbstring, strUrl, NULL, NULL, SW_SHOWNORMAL);
}

