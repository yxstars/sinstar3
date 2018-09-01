#include "stdafx.h"
#include "IsSvrProxy.h"
#include <helper\smenuex.h>
#include "AboutDlg.h"
#include "KeyMapDlg.h"
#include "../helper/helper.h"
#include "../include/reg.h"
#include <helper/STime.h>
#include <process.h>
#include "UpdateInfoDlg.h"

#define TIMERID_DELAY_EXIT	200
#define SPAN_DELAY_EXIT		5000
#define TIMERID_CHECK_UPDATE 300
#define SPAN_CHECK_UPDATE	1000

#define TIMERID_DATA_REPORT 400
#define SPAN_DATA_REPORT1	50
#define SPAN_DATA_REPORT2	24*60*60*1000	//1 day

static void DoSomething()
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

CIsSvrProxy::CIsSvrProxy(const SStringT & strDataPath)
	:m_strDataPath(strDataPath)
	, m_worker(strDataPath)
	,m_hCoreModule(NULL)
	,m_pCore(NULL)
	,m_funIsCore_Create(NULL)
	,m_funIsCore_Destroy(NULL)
	, m_pKeyMapDlg(NULL)
	, m_pBuildIndexProg(NULL)
{
	m_uMsgTaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));
}


CIsSvrProxy::~CIsSvrProxy()
{
	if (m_pKeyMapDlg) m_pKeyMapDlg->DestroyWindow();
	if (m_pBuildIndexProg) m_pBuildIndexProg->DestroyWindow();
}

int CIsSvrProxy::OnCreate(LPCREATESTRUCT pCS)
{
	m_hCoreModule = LoadLibrary(_T("iscore.dll"));
	if (!m_hCoreModule) return -1;
	int nRet = 0;
	do {
		m_funIsCore_Create = (funIscore_Create)GetProcAddress(m_hCoreModule, "Iscore_Create");
		m_funIsCore_Destroy = (funIscore_Destroy)GetProcAddress(m_hCoreModule, "Iscore_Destroy");
		if (!m_funIsCore_Create || !m_funIsCore_Destroy)
		{
			nRet = -1;
			break;
		}
		m_pCore = m_funIsCore_Create();
		if (!m_pCore)
		{
			nRet = -1;
			break;
		}
		if (!m_pCore->Init(m_hWnd, this, S_CT2A(m_strDataPath)))
		{
			m_funIsCore_Destroy(m_pCore);
			m_pCore = NULL;
			nRet = -1;
			break;
		}
	} while (0);
	if (nRet != 0)
	{
		FreeLibrary(m_hCoreModule);
		m_hCoreModule = NULL;
	}
	else
	{
		DWORD dwVer = 0;
		SStringT strTip = CAboutDlg::GetVersionInfo(dwVer);
		strTip += _T("\n�汾:");
		TCHAR szVer[100];
		Helper_VersionString(dwVer, szVer);
		strTip += szVer;
		m_trayIcon.Init(m_hWnd, strTip);
		if(m_pCore->IsShowTray()) m_trayIcon.Show();

		char szConfig[MAX_PATH];
		m_pCore->GetConfigIni(szConfig, MAX_PATH);
		m_nUpdateInterval = GetPrivateProfileIntA("update", "interval", 30, szConfig);

		CSimpleWnd::SetTimer(TIMERID_CHECK_UPDATE, SPAN_CHECK_UPDATE, NULL);
		CSimpleWnd::SetTimer(TIMERID_DATA_REPORT, SPAN_DATA_REPORT1, NULL);
	}
	return nRet;
}

void CIsSvrProxy::OnDestroy()
{
	m_trayIcon.Hide();
	if (m_pCore)
	{
		m_funIsCore_Destroy(m_pCore);
		m_pCore = NULL;
	}
	if (m_hCoreModule)
	{
		FreeLibrary(m_hCoreModule);
		m_hCoreModule = NULL;
	}
}

LRESULT CIsSvrProxy::OnBuildIndexProg(UINT uMsg, WPARAM wp, LPARAM lp)
{
	int iPage = uMsg - UM_BUILD_INDEX_PROG0;
	PROGTYPE uType = (PROGTYPE)wp;
	int nProg = (int)lp;
	if (uType == PT_MAX)
	{
		m_pBuildIndexProg = new CBuildIndexProgWnd();
		m_pBuildIndexProg->Create(NULL, WS_POPUP, WS_EX_TOOLWINDOW, 0, 0, 0, 0);
		m_pBuildIndexProg->CenterWindow(GetDesktopWindow());
		m_pBuildIndexProg->SetPage(iPage,nProg);
		m_pBuildIndexProg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
	else if (uType == PT_PROG)
	{
		m_pBuildIndexProg->SetProg(nProg);
	}
	else if (uType == PT_END)
	{
		m_pBuildIndexProg->DestroyWindow();
		m_pBuildIndexProg = NULL;
	}
	return 0;
}


void CIsSvrProxy::OnBuildShapePhraseIndex(PROGTYPE uType, unsigned int dwData) {
	_OnBuildIndexProg(0, uType, dwData);
}

void CIsSvrProxy::OnBuildSpellPhraseIndex(PROGTYPE uType, unsigned int dwData) {
	_OnBuildIndexProg(1, uType, dwData);
}
void CIsSvrProxy::OnBuildSpellPhraseIndex2(PROGTYPE uType, unsigned int dwData) {
	_OnBuildIndexProg(2, uType, dwData);
}

void CIsSvrProxy::OnImportUserDict(PROGTYPE uType, unsigned int dwData) {
	_OnBuildIndexProg(3, uType, dwData);
}


void CIsSvrProxy::_OnBuildIndexProg(int indexMode, PROGTYPE uType, unsigned int dwData)
{
	static int nMax = 0, nPrevProg = 0;
	if (uType == PT_MAX) nMax = dwData, nPrevProg = 0;
	if (uType == PT_PROG)
	{
		if (dwData - nPrevProg < nMax / 200)
		{
			return;
		}
		nPrevProg = dwData;
	}
	SendMessage(UM_BUILD_INDEX_PROG0+ indexMode, uType, dwData);
	DoSomething();
}

void CIsSvrProxy::OnClientActive() {
	m_trayIcon.AnimateIcon();
}

void CIsSvrProxy::OnClientLogin() {
	CSimpleWnd::KillTimer(TIMERID_DELAY_EXIT);
}

void CIsSvrProxy::OnClientLogout() {
	if (m_pCore->CanQuitNow())
	{
		CSimpleWnd::SetTimer(TIMERID_DELAY_EXIT, SPAN_DELAY_EXIT, NULL);
	}
}

void CIsSvrProxy::OnShowTray(bool bTray) {
	if (bTray)
	{
		m_trayIcon.Show();
	}
	else
	{
		m_trayIcon.Hide();
	}
}

void CIsSvrProxy::OnKeyMapFree(CKeyMapDlg * pWnd)
{
	m_pKeyMapDlg = NULL;
}

int CIsSvrProxy::GetUpdateInterval() const
{
	return m_nUpdateInterval;
}

void CIsSvrProxy::OnUpdateIntervalChanged(int nInterval)
{
	m_nUpdateInterval = nInterval;
	char szConfig[MAX_PATH];
	m_pCore->GetConfigIni(szConfig, MAX_PATH);
	WritePrivateProfileStringA("update", "interval", SStringA().Format("%d",m_nUpdateInterval), szConfig);
}

void CIsSvrProxy::OnShowKeyMap(IDataBlock * pCompData, LPCSTR pszName, LPCSTR pszUrl) {
	SOUI::IBitmap *pBmp = SResLoadFromMemory::LoadImage(pCompData->GetData(), pCompData->GetLength());
	if (pBmp)
	{
		if (m_pKeyMapDlg)
		{
			m_pKeyMapDlg->DestroyWindow();
		}
		m_pKeyMapDlg = new CKeyMapDlg(pBmp, pszName, pszUrl);
		m_pKeyMapDlg->SetListener(this);
		m_pKeyMapDlg->Create(NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST, 0, 0, 0, 0);
		m_pKeyMapDlg->SendMessage(WM_INITDIALOG);
		m_pKeyMapDlg->CenterWindow(GetDesktopWindow());
		pBmp->Release();

	}
}

int CIsSvrProxy::TtsGetSpeed()
{
	return m_worker.GetSpeed();
}

int CIsSvrProxy::TtsGetVoice(bool bCh)
{
	return m_worker.GetVoice(bCh);
}

void CIsSvrProxy::TtsSetSpeed(int nSpeed) {
	m_worker.SetSpeed(nSpeed);
}

void CIsSvrProxy::TtsSpeakText(const wchar_t* pText, int nLen, bool bChinese) {
	m_worker.SpeakWText(pText, nLen, bChinese);
}

void CIsSvrProxy::TtsSetVoice(bool bCh, int iToken) {
	m_worker.SetVoice(bCh, iToken);
}

int CIsSvrProxy::TtsGetTokensInfo(bool bCh, wchar_t token[][MAX_TOKEN_NAME_LENGHT], int nBufSize) { 
	return m_worker.GetTokensInfo(bCh, token, nBufSize);
}

DWORD CIsSvrProxy::OnQueryVersion() const
{
	DWORD dwVer;
	CAboutDlg::GetVersionInfo(dwVer);
	return dwVer;
}


LRESULT CIsSvrProxy::OnTrayNotify(UINT uMsg, WPARAM wp, LPARAM lp)
{
	switch (lp)
	{
	case WM_LBUTTONDBLCLK:
		break;
	case WM_RBUTTONUP:
		{
			SMenuEx tmenuex;
			if (tmenuex.LoadMenu(UIRES.smenu.menu_context))
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(m_hWnd);
				tmenuex.CheckMenuItem(R.id.menu_auto, MF_BYCOMMAND | (m_pCore->IsAutoQuit()?MF_CHECKED:0));
				tmenuex.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
				PostMessage(WM_NULL);
			}
		}
		break;
	}
	return 0;
}

LRESULT CIsSvrProxy::OnTaskbarCreated(UINT uMsg, WPARAM wp, LPARAM lp)
{
	if (m_pCore->IsShowTray())
	{
		m_trayIcon.Show();
	}
	return 0;
}

void CIsSvrProxy::OnTimer(UINT_PTR uID)
{
	if (uID == TIMERID_DELAY_EXIT)
	{
		PostQuitMessage(0);
	}
	else if (uID == TIMERID_CHECK_UPDATE)
	{
		CSimpleWnd::KillTimer(uID);
		CheckUpdate(FALSE);
	}
	else if (uID == TIMERID_DATA_REPORT)
	{
		m_worker.PostMessage(UM_FUN_DATA_REPORT);
		KillTimer(uID);
		CSimpleWnd::SetTimer(uID, SPAN_DATA_REPORT2, NULL);
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

void CIsSvrProxy::OnMenuExit(UINT uNotifyCode, int nID, HWND wndCtl)
{
	PostQuitMessage(0);
}

void CIsSvrProxy::OnMenuAbout(UINT uNotifyCode, int nID, HWND wndCtl)
{
	CAboutDlg aboutDlg(this);
	if (ID_CHECK_UPDATE_NOW == aboutDlg.DoModal())
	{
		CheckUpdate(TRUE);
	}
}

void CIsSvrProxy::OnMenuAutoExit(UINT uNotifyCode, int nID, HWND wndCtl)
{
	m_pCore->SetAutoQuit(!m_pCore->IsAutoQuit());
}


void CIsSvrProxy::CheckUpdate(BOOL bManual)
{
	char szConfig[MAX_PATH];
	char szDate[100];
	CTime timeToday = CTime::GetCurrentTime();
	m_pCore->GetConfigIni(szConfig, MAX_PATH);
	if (!bManual)
	{
		if (m_nUpdateInterval == 0) return;
		int nDay, nMonth, nYear;
		GetPrivateProfileStringA("update", "date", "0-0-0", szDate, 100, szConfig);
		sscanf(szDate, "%d-%d-%d", &nMonth, &nDay, &nYear);
		int nDays = 360 * (timeToday.GetYear() - nYear);
		nDays += 30 * (timeToday.GetMonth() - nMonth);
		nDays += timeToday.GetDay() - nDay;
		if (nDays<m_nUpdateInterval) return;
	}

	//update date
	sprintf(szDate, "%d-%d-%d", timeToday.GetMonth(), timeToday.GetDay(), timeToday.GetYear());
	WritePrivateProfileStringA("update", "date", szDate, szConfig);

	m_worker.PostMessage(UM_FUN_CHECK_UPDATE, bManual,(LPARAM)this);
}


LRESULT CIsSvrProxy::OnCheckUpdateResult(UINT uMsg, WPARAM wp, LPARAM lp)
{
	BOOL bManual = (BOOL)wp;
	CheckUpdateResult *pResult = (CheckUpdateResult*)lp;
	char szConfig[MAX_PATH];
	m_pCore->GetConfigIni(szConfig, MAX_PATH);
	WritePrivateProfileStringA("update", "url", S_CW2A(pResult->strNewUpdateUrl), szConfig);

	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_READ | KEY_WOW64_64KEY);
	if (ret == ERROR_SUCCESS)
	{
		TCHAR szClientPath[MAX_PATH];
		ULONG len = MAX_PATH;
		reg.QueryStringValue(_T("path_client"), szClientPath, &len);
		reg.Close();

		SStringT strClient = szClientPath;
		strClient += _T("\\program\\sinstar3_core.dll");
		TCHAR szSvrPath[MAX_PATH];
		GetModuleFileName(NULL, szSvrPath, MAX_PATH);
		DWORD dwVerSvr = 0, dwVerClient = 0;
		Helper_PEVersion(szSvrPath, &dwVerSvr, NULL, NULL);
		Helper_PEVersion(strClient, &dwVerClient, NULL, NULL);

		if (pResult->dwClientVer > dwVerClient || pResult->dwServerVer > dwVerSvr)
		{//found new ver
			pResult->dwClientCurVer = dwVerClient;
			pResult->dwServerCurVer = dwVerSvr;

			CUpdateInfoDlg updateDlg(pResult);
			updateDlg.DoModal(GetDesktopWindow());
		}
		else if (bManual)
		{
			SMessageBox(GetDesktopWindow(), _T("û�з����°汾!"), _T("��ʾ"), MB_OK | MB_ICONINFORMATION);
		}
	}
	delete pResult;
	return 0;
}