// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "FileHelper.h"
#include "base64.h"
#include <shellapi.h>
#include "../include/compbuilder-i.h"
#include <fstream>
#include "TxtReader.h"

typedef ICompBuilder* (*FunICompBuilder_Create)();
typedef void(*FunICompBuilder_Destroy)(ICompBuilder* pCompBuilder);
typedef ICompReaderEx * (*FunICompReader_Create)();
typedef void(*FunICompReader_Destroy)(ICompReaderEx* pCompReader);

class CompBuilderLoader
{
public:
	CompBuilderLoader()
	{
		m_hMod = LoadLibrary(_T("..\\program\\iscore.dll"));
		m_funCreate = (FunICompBuilder_Create)GetProcAddress(m_hMod, "ICompBuilder_Create");
		m_funDestroy = (FunICompBuilder_Destroy)GetProcAddress(m_hMod, "ICompBuilder_Destroy");

		m_funCreateReader = (FunICompReader_Create)GetProcAddress(m_hMod, "ICompReader_Create");
		m_funDestroyReader = (FunICompReader_Destroy)GetProcAddress(m_hMod, "ICompReader_Destroy");

	}

	ICompBuilder * Create() {
		return m_funCreate();
	}

	void  Destroy(ICompBuilder * pCompBuilder) {
		return m_funDestroy(pCompBuilder);
	}

	ICompReaderEx *CreateReader()
	{
		return m_funCreateReader();
	}

	void DestroyReader(ICompReaderEx *pReader)
	{
		m_funDestroyReader(pReader);
	}

	~CompBuilderLoader()
	{
		if (m_hMod) FreeLibrary(m_hMod);
	}
private:
	FunICompBuilder_Create m_funCreate;
	FunICompBuilder_Destroy m_funDestroy;

	FunICompReader_Create  m_funCreateReader;
	FunICompReader_Destroy m_funDestroyReader;

	HMODULE m_hMod;
};


CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_pLoader = new CompBuilderLoader();
}

CMainDlg::~CMainDlg()
{
	delete m_pLoader;
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	return 0;
}

void CMainDlg::OnBroweComp()
{
	CFileDialogEx openDlg(TRUE, _T("txt"), 0, 6, _T("码表文件(*.txt)\0*.txt\0All files (*.*)\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		FindChildByID(R.id.edit_comp)->SetWindowText(openDlg.m_szFileName);

		if (GetFileAttributes(openDlg.m_szFileName) != 0xffffffff)
		{
			int nSymbolFirst = GetPrivateProfileInt(_T("Description"), _T("SymbolFirst"), 1, openDlg.m_szFileName);
			FindChildByID(R.id.chk_symbol_first)->SetCheck(nSymbolFirst != 0);
			int nYinXingMa = GetPrivateProfileInt(_T("Description"), _T("YinXingMa"), 0, openDlg.m_szFileName);
			FindChildByID(R.id.chk_yinxingma)->SetCheck(nYinXingMa != 0);

			int nAutoSelect = GetPrivateProfileInt(_T("Description"), L"AutoSelect", 1, openDlg.m_szFileName);//唯一重码自动上屏属性
			FindChildByID(R.id.chk_auto_select)->SetCheck(nAutoSelect !=0);

			int nPhraseCompPart = GetPrivateProfileInt(_T("Description"), L"PhraseCompPart", 0, openDlg.m_szFileName) ;//词组可以不是全码标志
			FindChildByID(R.id.chk_phrase_full_code)->SetCheck(nPhraseCompPart==0);

			TCHAR szWebSite[200];
			GetPrivateProfileString(_T("Description"), _T("WebSite"), _T(""), szWebSite, 200, openDlg.m_szFileName);
			FindChildByID(R.id.edit_url)->SetWindowText(szWebSite);
			char *szIcon = (char*)malloc(32 * 1024);//max to 32k
			int nLen = GetPrivateProfileSectionA("Icon2", szIcon, 32 * 1024, S_CT2A(openDlg.m_szFileName));
			if (nLen)
			{
				SStringT strIcon = openDlg.m_szFileName;
				strIcon += _T(".icon");

				std::string tmp;
				char *p = szIcon;
				char *pEnd = p + nLen;

				while (p<pEnd)
				{
					tmp += p;
					p += strlen(p) + 1;
				}

				std::string result;
				result.resize(nLen);
				size_t nUsed = 0;
				Base64::DecodeFromArray(&tmp[0], tmp.length(), Base64::DO_LAX, &result, &nUsed);

				FILE *f = _tfopen(strIcon, _T("wb"));
				fwrite(&result[0], 1, result.length(), f);
				fclose(f);
				FindChildByID(R.id.edit_comp_logo)->SetWindowText(strIcon);
				UpdateLogo();
			}
			free(szIcon);
		}
	}
}


void CMainDlg::OnBroweCit()
{
	CFileDialogEx openDlg(FALSE, _T("cit"), 0, 6, _T("启程码表(*.cit)\0*.cit\0All files (*.*)\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		FindChildByID(R.id.edit_cit_save)->SetWindowText(openDlg.m_szFileName);
	}
}

void CMainDlg::OnBroweLogo()
{
	CFileDialogEx openDlg(TRUE, _T("png"), 0, 6, _T("码表logo(*.png)\0*.png\0All files (*.*)\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		FindChildByID(R.id.edit_comp_logo)->SetWindowText(openDlg.m_szFileName);
		UpdateLogo();
	}
}

void CMainDlg::OnBroweKeymap()
{
	CFileDialogEx openDlg(TRUE, _T("png"), 0, 6, _T("字根图(*.png,*.jpg,*.gif)\0*.png;*.jpg;*.gif\0All files (*.*)\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		FindChildByID(R.id.edit_keymap)->SetWindowText(openDlg.m_szFileName);
		IBitmap *pKeyMap = SResLoadFromFile::LoadImage(openDlg.m_szFileName);
		if (pKeyMap)
		{
			FindChildByID2<SImageWnd>(R.id.img_keymap)->SetImage(pKeyMap);
			pKeyMap->Release();
		}
	}
}

void CMainDlg::UpdateLogo()
{
	SStringT strIconFile = FindChildByID(R.id.edit_comp_logo)->GetWindowText();
	IBitmap *pLogo = SResLoadFromFile::LoadImage(strIconFile);
	if (pLogo)
	{
		FindChildByID2<SImageWnd>(R.id.img_logo)->SetImage(pLogo);
		pLogo->Release();
	}

}



void CMainDlg::OnBuildComp()
{
	COMPHEAD head = { 0 };
	SStringT strComp, strKeyMap, strLogo, strSave;

	strComp = FindChildByID(R.id.edit_comp)->GetWindowText();
	strKeyMap = FindChildByID(R.id.edit_keymap)->GetWindowText();
	strSave = FindChildByID(R.id.edit_cit_save)->GetWindowText();
	strLogo = FindChildByID(R.id.edit_comp_logo)->GetWindowText();

	SStringT strUrl = FindChildByID(R.id.edit_url)->GetWindowText();

	head.bSymbolFirst = FindChildByID(R.id.chk_symbol_first)->IsChecked();
	head.dwAutoSelect = FindChildByID(R.id.chk_auto_select)->IsChecked();
	head.dwYinXingMa = FindChildByID(R.id.chk_yinxingma)->IsChecked();
	head.dwPhraseCompPart = !FindChildByID(R.id.chk_phrase_full_code)->IsChecked();
	if(strSave.IsEmpty())
	{
		SMessageBox(m_hWnd, _T("没有指定CIT保存路径!"), _T("错误"), MB_OK | MB_ICONSTOP);
		return;
	}
	if (strComp.IsEmpty() || GetFileAttributes(strComp) == INVALID_FILE_ATTRIBUTES)
	{
		SMessageBox(m_hWnd, _T("请先指定码表文件"), _T("错误"), MB_OK | MB_ICONSTOP);
		return;
	}
	if (!strKeyMap.IsEmpty() && GetFileAttributes(strKeyMap) == INVALID_FILE_ATTRIBUTES)
	{
		SMessageBox(m_hWnd, _T("字根表文件不存在"), _T("错误"), MB_OK | MB_ICONSTOP);
		return;
	}
	if(!strLogo.IsEmpty() && GetFileAttributes(strLogo) == INVALID_FILE_ATTRIBUTES)
	{
		SMessageBox(m_hWnd, _T("码表图标文件不存在"), _T("错误"), MB_OK | MB_ICONSTOP);
		return;
	}
	wchar_t szWildChar[2];
	BYTE byNumRules;
	WCHAR szRules[RULE_MAX * 100];

	TCHAR buf[100];
	GetPrivateProfileString(_T("Description"),_T("Name"),L"",buf,50,strComp);
	_tcscpy_s(head.szName, 50, buf);
	GetPrivateProfileString(_T("Description"),L"UsedCodes",L"",buf,50,strComp);
	_tcscpy_s(head.szCode, 50, buf);
	GetPrivateProfileString(_T("Description"), L"WildChar", L"", szWildChar, 2, strComp);
	head.cWildChar = (char)szWildChar[0];
	head.cCodeMax = GetPrivateProfileInt(_T("Description"), L"MaxCodes", 0, strComp);
	_tcscpy_s(head.szUrl,50,strUrl);
	byNumRules = GetPrivateProfileInt(_T("Description"), L"NumRules", 0, strComp);

	GetPrivateProfileSectionW(L"Rule", szRules, RULE_MAX * 100, strComp);

	ICompBuilder * pCompBuilder = m_pLoader->Create();
	ICodingRule * pCodingRule = pCompBuilder->CreateCodingRule();

	if (!pCodingRule->ParseRules(head.cCodeMax, byNumRules, szRules))
	{
		pCompBuilder->DestroyCodingRule(pCodingRule);
		m_pLoader->Destroy(pCompBuilder);
		SMessageBox(m_hWnd, _T("解析[Rule]失败"), _T("错误"), MB_OK | MB_ICONSTOP);
		return;
	}

	CTxtReader txtReader;
	if(!txtReader.Open(strComp))
		return;
	WCHAR szLine[1000];
	bool bText = false;
	for(;!bText;){
		if(!txtReader.getline(szLine,1000))
			break;
		bText = _wcsnicmp(szLine, L"[Text]", 6) == 0;
	}
	if (!bText)
	{
		txtReader.Close();
		SMessageBox(m_hWnd, _T("码表中没有发现[Text]数据段"), _T("错误"), MB_OK | MB_ICONSTOP);
		return;
	}
	SStringT strLogFile = strComp+_T(".log");
	FILE *flog=_tfopen(strLogFile,_T("wb"));
	if(flog) fwrite("\xff\xfe",1,2,flog);
	//import composition parts
	while (txtReader.getline(szLine,1000))
	{
		if (_wcsicmp(szLine, L"[MPCode]") == 0)
			break;
		if(szLine[0]>=0xd800 && szLine[0]<0xDBFF)
		{
			if(flog)
			{
				fwprintf(flog,L"%s\r\n",szLine);
			}
			continue;
		}
		if (szLine[0]>128)
		{
			WCHAR *pszWord = szLine;
			WCHAR *pszComp = szLine + 1;
			if (pszComp[0]<128)
			{//只支持单字
				char cCompLen = 0;
				WCHAR *pszCompTemp = pszComp;
				int i = 0;
				while (pszComp[i])
				{
					if (pszComp[i] == ' ')
					{
						pCompBuilder->AddWord(pszWord[0], pszCompTemp, cCompLen);
						pszCompTemp += cCompLen + 1;
						cCompLen = 0;
					}
					else
						cCompLen++;
					i++;
				}
				if (cCompLen != 0) 
				{
					BOOL bAdd=pCompBuilder->AddWord(pszWord[0], pszCompTemp, cCompLen);
				}
			}
		}else
		{
			if(flog)
			{
				fwprintf(flog,L"%s\r\n",szLine);
			}
		}
	}
	if(flog) fclose(flog);
	if (_wcsicmp(szLine, L"[MPCode]") == 0)
	{//parse make phrase code table.
		while (txtReader.getline(szLine, 1000))
		{
			if (szLine[0] >128)
			{
				WCHAR *pszWord = szLine;
				WCHAR *pszComp = szLine + 1;
				if (pszComp[0]<128)
				{//只支持单字
					pCompBuilder->AddMakePhraseCode(pszWord[0], pszComp, -1);
				}
			}
		}
	}
	txtReader.Close();
	if (pCompBuilder->GetWords()<2000)
	{
		pCompBuilder->DestroyCodingRule(pCodingRule);
		m_pLoader->Destroy(pCompBuilder);
		SMessageBox(m_hWnd, _T("码表中编码字符太少，请检查您的码表!"), _T("error"), MB_OK | MB_ICONSTOP);
		return;
	}
	BOOL bSuccess = pCompBuilder->Make(strSave, head,
		pCodingRule,
		strLogo.IsEmpty() ? NULL : (LPCTSTR)strLogo,
		0,
		strKeyMap.IsEmpty() ? NULL : (LPCTSTR)strKeyMap);

	pCompBuilder->DestroyCodingRule(pCodingRule);
	m_pLoader->Destroy(pCompBuilder);
	
	if(bSuccess)
		SMessageBox(m_hWnd, _T("码表创建成功!"), _T("恭喜"), MB_OK);
	else
		SMessageBox(m_hWnd, _T("未知错误，码表创建失败!"), _T("错误"), MB_OK);

}

void CMainDlg::OnExportComp()
{
	CFileDialogEx openDlg(TRUE, _T("cit"), 0, 6, _T("启程码表(*.cit)\0*.cit\0All files (*.*)\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		ICompReaderEx *pReader = m_pLoader->CreateReader();
		bool bOk = false;
		SStringT strCit = openDlg.m_szFileName;
		if (pReader->Load(strCit))
		{
			TCHAR szDrive[5], szDir[MAX_PATH];
			_tsplitpath(strCit, szDrive, szDir, NULL, NULL);
			TCHAR szSave[MAX_PATH * 2];
			_stprintf(szSave, _T("%s%s%s.txt"), szDrive, szDir, pReader->GetProps()->szName);
			if (pReader->Export(szSave))
			{
				ShellExecute(NULL, _T("open"), szSave, NULL, NULL, SW_SHOWDEFAULT);
				bOk = true;
			}
		}
		m_pLoader->DestroyReader(pReader);
	}

}

//TODO:消息映射
void CMainDlg::OnClose()
{
	GetNative()->DestroyWindow();
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

