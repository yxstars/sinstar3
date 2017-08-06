// sinstar3_core.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Sinstar3Impl.h"


EXTERN_C BAIDUJP3_API  ISinstar* BaiduJP3_Create(ITextService* pTxtSvr,HINSTANCE hInst)
{
	CSinstar3Impl*  pRet= new CUnknownImpl<CSinstar3Impl>(pTxtSvr,hInst);
	return pRet;
}

EXTERN_C BAIDUJP3_API  void BaiduJP3_Delete(ISinstar* pBaiduJP3)
{
	pBaiduJP3->Release();
}

EXTERN_C BAIDUJP3_API HRESULT BaiduJP3_CanUnloadNow()
{
	return E_NOTIMPL;
}

EXTERN_C BAIDUJP3_API BOOL BaiduJP3_Config(HWND hWnd)
{
	return FALSE;
}


