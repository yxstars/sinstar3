#include "StdAfx.h"
#include "DataCenter.h"

namespace SOUI
{
	template<>
	CDataCenter * SSingleton<CDataCenter>::ms_Singleton = NULL;

	CDataCenter::CDataCenter(void)
	{
		InitializeCriticalSection(&m_cs);
	}

	CDataCenter::~CDataCenter(void)
	{
		DeleteCriticalSection(&m_cs);
	}

	void CDataCenter::Lock()
	{
		EnterCriticalSection(&m_cs);
	}

	void CDataCenter::Unlock()
	{
		LeaveCriticalSection(&m_cs);
	}

	CMyData::CMyData()
	{
		m_reg.Create(HKEY_CURRENT_USER,L"Software\\Setoutsoft\\sinstar3",NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE|KEY_READ|KEY_WOW64_64KEY,0,NULL);

		m_cInputCount = 0;
		m_compInfo.cWild = 'z';
		m_compInfo.strCompName = _T("����...");

		TCHAR szSkin[MAX_PATH]={0};
		ULONG nSize = MAX_PATH;
		m_reg.QueryStringValue(_T("skin"),szSkin,&nSize);
		m_strSkin = szSkin;

		DWORD dwPos=-1;
		m_reg.QueryDWORDValue(_T("status_pos"),dwPos);
		m_ptStatus = CPoint(GET_X_LPARAM(dwPos),GET_Y_LPARAM(dwPos));

		CRegKey keySvr;
		if(ERROR_SUCCESS == keySvr.Open(HKEY_LOCAL_MACHINE,L"Software\\Setoutsoft\\sinstar3",KEY_READ|KEY_WOW64_64KEY))
		{
			TCHAR szSvrPath[MAX_PATH]={0};
			ULONG nSize = MAX_PATH;
			keySvr.QueryStringValue(_T("path_svr"),szSvrPath,&nSize);
			ISComm_SetSvrPath(szSvrPath);
		}

	}

	CMyData::~CMyData()
	{
		m_reg.SetDWORDValue(_T("status_pos"),MAKELPARAM(m_ptStatus.x,m_ptStatus.y));
		m_reg.SetStringValue(_T("skin"),m_strSkin);
		m_reg.Close();
	}


	static int CharCmp(const void * p1, const void * p2)
	{
		const char *c1 = (const char*)p1;
		const char *c2 = (const char*)p2;
		return (*c1) - (*c2);
	}

	void CompInfo::SetSvrCompInfo(const COMPINFO * compInfo)
	{
		cWild = compInfo->cWildChar;
		strCompName = S_CA2T(compInfo->szName);
		strcpy(szCode,compInfo->szCode);
		nCodeNum = (int)strlen(szCode);
		qsort(szCode,nCodeNum,sizeof(char),CharCmp);
	}

	BOOL CompInfo::IsCompChar(char cInput)
	{
		if(cWild == cInput) return TRUE;
		return NULL != bsearch(&cInput,szCode,nCodeNum,1,CharCmp);
	}

}


