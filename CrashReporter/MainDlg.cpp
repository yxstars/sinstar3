// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "../include/reg.h"
#include <ShellAPI.h>
#include <helper/SDpiHelper.hpp>

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_szSvrPath[0]=0;
}

CMainDlg::~CMainDlg()
{
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\SetoutSoft\\sinstar3"),KEY_READ|KEY_WOW64_64KEY);
	if(ret == ERROR_SUCCESS)
	{
		ULONG len = MAX_PATH;
		reg.QueryStringValue(_T("path_svr"),m_szSvrPath,&len);
		reg.Close();
		WORD wVer[4];
		SDpiHelper::PEVersion(m_szSvrPath,wVer[0],wVer[1],wVer[2],wVer[3]);
		SStringT strVer=SStringT().Format(_T("%u.%u.%u.%u"),wVer[0],wVer[1],wVer[2],wVer[3]);
		FindChildByName(L"txt_ver")->SetWindowText(strVer);
	}
	return 0;
}

void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::OnBtnRestart()
{
	ShellExecute(NULL,_T("open"),m_szSvrPath,NULL,NULL,SW_HIDE);
}

void CMainDlg::OnBtnOpenDir()
{
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\SetoutSoft\\sinstar3"),KEY_READ|KEY_WOW64_64KEY);
	if(ret == ERROR_SUCCESS)
	{
		ULONG len = MAX_PATH;
		TCHAR szPathClient[MAX_PATH];
		reg.QueryStringValue(_T("path_client"),szPathClient,&len);
		reg.Close();
		ShellExecute(NULL,_T("open"),szPathClient,NULL,NULL,SW_SHOWNORMAL);
	}
}

