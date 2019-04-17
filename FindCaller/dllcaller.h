#ifndef _DLLCALLER_
#define _DLLCALLER_


#ifdef __cplusplus
extern "C"{
#endif//__cplusplus

typedef struct tagCALLERINFO
{
	DWORD dwProcID;
	TCHAR szFile[MAX_PATH];
	TCHAR szPath[1000];
}CALLERINFO,*PCALLERINFO;

BYTE * GetCaller(LPCTSTR pszDllPath);
void FreeCallerInfo(BYTE * pBuf);
BOOL Kill_Process(DWORD dwProcID);
BOOL GetProcessInfo(DWORD dwProcID, HWND &hWnd, DWORD &dwThreadID);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif//_DLLCALLER_