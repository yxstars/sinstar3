#include "StdAfx.h"
#include "Sinstar3Impl.h"

#include <initguid.h>

class CAutoContext
{
public:
	CAutoContext(void ** ppCtx, void * pValue)
	{
		m_ppCtx = ppCtx;
		*m_ppCtx = pValue;		
	}

	~CAutoContext()
	{
		*m_ppCtx = NULL;
	}

	void ** m_ppCtx;
};

CSinstar3Impl::CSinstar3Impl(ITextService *pTxtSvr)
:m_pTxtSvr(pTxtSvr)
,m_pInputWnd(NULL)
,m_pStatusWnd(NULL)
,m_pCurImeContext(NULL)
{
	theModule->AddRef();

 	m_pInputWnd = new CInputWnd(m_inputState.GetInputContext());
	m_pStatusWnd = new CStatusWnd();
	m_pStatusWnd->Create();
	m_pInputWnd->Create();

	m_inputState.SetInputListener(this);

	SLOG_INFO("status:"<<m_pStatusWnd->m_hWnd<<", input:"<<m_pInputWnd->m_hWnd);
	SOUI::CSimpleWnd::Create(_T("sinstar3_msg_recv"),WS_DISABLED|WS_POPUP,WS_EX_TOOLWINDOW,0,0,0,0,HWND_MESSAGE,NULL);
	ISComm_Login(m_hWnd);
}

CSinstar3Impl::~CSinstar3Impl(void)
{
	ISComm_Logout(m_hWnd);
	SOUI::CSimpleWnd::DestroyWindow();

	m_pInputWnd->DestroyWindow();
	m_pStatusWnd->DestroyWindow();
	delete m_pStatusWnd;
	delete m_pInputWnd;


	theModule->Release();
}


void CSinstar3Impl:: ProcessKeyStoke(LPVOID lpImeContext,UINT vkCode,LPARAM lParam,BOOL bKeyDown,BOOL *pbEaten)
{
	if(!bKeyDown)
	{
		*pbEaten = FALSE;
		return;
	}
	CAutoContext autoCtx(&m_pCurImeContext,lpImeContext);
	BYTE byKeyState[256];
	GetKeyboardState(byKeyState);
	*pbEaten = m_inputState.TestKeyDown(vkCode,lParam,byKeyState);
}

void CSinstar3Impl:: TranslateKey(LPVOID lpImeContext,UINT vkCode,UINT uScanCode,BOOL bKeyDown,BOOL *pbEaten)
{
	CAutoContext autoCtx(&m_pCurImeContext,lpImeContext);

	*pbEaten = TRUE;

	BYTE byKeyState[256];
	GetKeyboardState(byKeyState);
	if(m_inputState.HandleKeyDown(vkCode,uScanCode,byKeyState))
	{
		m_pInputWnd->UpdateUI();
	}
}

void CSinstar3Impl::OnIMESelect(BOOL bSelect)
{
	m_inputState.OnImeSelect(bSelect);
}

void CSinstar3Impl::OnSetCaretPosition(POINT pt,int nHei)
{
	m_pInputWnd->MoveTo(pt,nHei);
}

void CSinstar3Impl::OnSetFocusSegmentPosition(POINT pt,int nHei)
{
}

void CSinstar3Impl::OnCompositionStarted()
{
	SLOG_INFO("");
	m_pInputWnd->Show(TRUE);
}

void CSinstar3Impl::OnCompositionChanged()
{
}

void CSinstar3Impl::OnCompositionTerminated()
{
	SLOG_INFO("");
	m_inputState.ClearContext(CPC_ALL);
	
	m_pInputWnd->Show(FALSE);
}

void CSinstar3Impl::OnSetFocus(BOOL bFocus)
{
	SLOG_INFO("GetThreadID="<<GetCurrentThreadId()<<" focus="<<bFocus);
	if(bFocus) m_pTxtSvr->SetConversionMode(FullNative);
	m_pStatusWnd->Show(bFocus);
	
	if(m_inputState.IsTypeing()) m_pInputWnd->Show(bFocus);
}

int  CSinstar3Impl::GetCompositionSegments()
{
	return 0;
}

int  CSinstar3Impl::GetCompositionSegmentEnd(int iSeg)
{
	return 0;
}

int CSinstar3Impl::GetCompositionSegmentAttr(int iSeg)
{
	return 0;
}

BOOL CSinstar3Impl::OnHotkey(LPVOID lpImeContext,REFGUID guidHotKey)
{

	return FALSE;
}

void CSinstar3Impl::OnOpenStatusChanged(BOOL bOpen)
{
	m_pStatusWnd->Show(bOpen);
}

void CSinstar3Impl::OnConversionModeChanged(EInputMethod nMode)
{

}

void CSinstar3Impl::ShowHelp()
{
}

EInputMethod CSinstar3Impl::GetDefInputMode()
{
	return FullNative;
}

BOOL CSinstar3Impl::ShowCompWnd()
{
	return FALSE;
}

LRESULT CSinstar3Impl::OnWildMessage(WPARAM wParam,LPARAM lParam)
{
	return E_NOTIMPL;
}

void CSinstar3Impl::CheckDefIME()
{

}


void CSinstar3Impl::OnFinalRelease()
{

}

HRESULT CSinstar3Impl::OnQueryInterface(REFIID riid, void **ppvObject)
{
	return E_NOINTERFACE;
}

LRESULT CSinstar3Impl::OnSvrNotify(UINT uMsg, WPARAM wp, LPARAM lp)
{
	PMSGDATA pMsg=ISComm_OnSeverNotify(m_hWnd,wp,lp);
	if(wp==NT_COMPINFO)
	{
		CDataCenter::getSingleton().Lock();
		CMyData &myData = CDataCenter::getSingleton().GetData();
		myData.m_compInfo.SetSvrCompInfo(ISComm_GetCompInfo());
		CDataCenter::getSingleton().Unlock();

		EventSvrNotify *evt= new EventSvrNotify(this);
		evt->wp = wp;
		evt->lp = lp;
		SNotifyCenter::getSingleton().FireEventAsync(evt);
		evt->Release();
		return 1;
	}else
	{
		return m_inputState.OnSvrNotify((UINT)wp,pMsg)?1:0;
	}
}

HWND CSinstar3Impl::GetHwnd() const
{
	return m_hWnd;
}

void CSinstar3Impl::OnInputStart()
{
	if(!m_pCurImeContext) return;
	m_pTxtSvr->StartComposition(m_pCurImeContext);
}


void CSinstar3Impl::OnInputEnd()
{
	if(!m_pCurImeContext) return;
	m_pTxtSvr->EndComposition(m_pCurImeContext);
}

BOOL CSinstar3Impl::GoNextCandidatePage()
{
	return m_pInputWnd->GoNextCandidatePage();
}

BOOL CSinstar3Impl::GoPrevCandidatePage()
{
	return m_pInputWnd->GoPrevCandidatePage();
}

short CSinstar3Impl::SelectCandidate(short iCand)
{
	return m_pInputWnd->SelectCandidate(iCand);
}

void CSinstar3Impl::CloseInputWnd(BOOL bDelay)
{
	m_pInputWnd->Hide(bDelay?5000:0);
}

BOOL CSinstar3Impl::SetOpenStatus(BOOL bOpen)
{
	SASSERT(m_pCurImeContext);
	return m_pTxtSvr->SetOpenStatus(m_pCurImeContext,bOpen);
}

BOOL CSinstar3Impl::GetOpenStatus() const
{
	SASSERT(m_pCurImeContext);
	return m_pTxtSvr->GetOpenStatus(m_pCurImeContext);
}


void CSinstar3Impl::OnInputResult(const SStringT & strResult,const SStringT & strComp/*=SStringT() */)
{
	if(!m_pCurImeContext) return;
	SStringW strResultW = S_CT2W(strResult);
	SStringW strCompW = S_CT2W(strComp);
	m_pTxtSvr->UpdateResultAndCompositionStringW(m_pCurImeContext,strResult,strResult.GetLength(),strCompW,strCompW.GetLength());
}

void CSinstar3Impl::OpenInputWnd()
{
	m_pInputWnd->Show(TRUE);
}

void CSinstar3Impl::UpdateInputWnd()
{
	m_pInputWnd->UpdateUI();
}



