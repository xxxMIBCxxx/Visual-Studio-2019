//*****************************************************************************
// CThreadクラス ヘッダーファイル
//*****************************************************************************
#include "CThread.h"
#include <stdio.h>
#include <process.h>
#include <time.h>


#define THREAD_START_TIMEOUT					( 1000 )				// スレッド開始待ちタイムアウト(ms)
#define THREAD_END_TIMEOUT						( 1000 )				// スレッド終了待ちタイムアウト(ms)
#define THREAD_LOOP_TIMEOUT						( 100 )					// スレッドループタイムアウト

//-----------------------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------------------
CThread::CThread()
{
	BOOL			bRet = FALSE;


	// メンバー変数初期化
	m_bInit = FALSE;
	m_hThread = 0;
	m_ThreadID = 0;

	m_dwUserNameSize = 0;
	memset(m_szUserName, 0x00, sizeof(m_szUserName));

	m_dwPsidSize = 0;
	m_pPsid = NULL;

	m_dwDaclSize = 0;
	m_pDacl = NULL;

	m_dwDomainNameSize = 0;
	memset(m_szDomainName, 0x00, sizeof(m_szDomainName));

	m_eSidType = SidTypeUser;
	
	m_dwError = 0;

	m_hStartEvent = NULL;
	m_hEndReqEvent = NULL;


	// ログインしているユーザー名を取得
	m_dwUserNameSize = 256;
	bRet = GetUserName(m_szUserName, &m_dwUserNameSize);
	if (bRet == FALSE)
	{
		return;
	}

	// ユーザーSIDを取得
	m_dwPsidSize = 0;
	m_dwDomainNameSize = 256;
	bRet = LookupAccountName(NULL, m_szUserName, NULL, &m_dwPsidSize, m_szDomainName, &m_dwDomainNameSize, &m_eSidType);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		if (m_dwError != ERROR_INSUFFICIENT_BUFFER)
		{
			return;
		}
	}

	m_pPsid = (PSID)HeapAlloc(GetProcessHeap(), 0, m_dwPsidSize);
	if (m_pPsid == NULL)
	{
		return;
	}

	bRet = LookupAccountName(NULL, m_szUserName, m_pPsid, &m_dwPsidSize, m_szDomainName, &m_dwDomainNameSize, &m_eSidType);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		return;
	}

	// セキュリティ記述子を初期化
	InitializeSecurityDescriptor(&m_Sd, SECURITY_DESCRIPTOR_REVISION);

	// DACL（随意アクセス制御リスト）を初期化
	m_dwDaclSize = GetLengthSid(m_pPsid);
	m_pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, m_dwDaclSize);
	if (m_pDacl == NULL)
	{
		return;
	}
	InitializeAcl(m_pDacl, m_dwDaclSize, ACL_REVISION);

	// ACE(アクセス制御エントリ)をDACLに追加する
	AddAccessAllowedAce(m_pDacl, ACL_REVISION, GENERIC_ALL, m_pPsid);

	// セキュリティ記述子にDACLの情報を設定する
	SetSecurityDescriptorDacl(&m_Sd, TRUE, m_pDacl, FALSE);

	// SECURITY_ATTRIBUTES（セキュリティ属性）オブジェクトの各メンバ(オブジェクトのサイズ、ハンドル継承の有無、セキュリティ記述子)に値をセットする
	m_Sa.nLength = sizeof(m_Sa);
	m_Sa.bInheritHandle = FALSE;
	m_Sa.lpSecurityDescriptor = &m_Sd;


	// スレッド開始イベント生成
	m_hStartEvent = CreateEvent(&m_Sa, TRUE, FALSE, NULL);
	if (m_hStartEvent == NULL)
	{
		m_dwError = GetLastError();
		return;
	}

	// スレッド終了要求イベント生成
	m_hEndReqEvent = CreateEvent(&m_Sa, TRUE, FALSE, NULL);
	if (m_hEndReqEvent == NULL)
	{
		m_dwError = GetLastError();
		return;
	}

	m_bInit = TRUE;

}


//-----------------------------------------------------------------------------
// デストラクタ
//-----------------------------------------------------------------------------
CThread::~CThread()
{
	// スレッド終了要求イベント解放
	if (m_hEndReqEvent != NULL)
	{
		CloseHandle(m_hEndReqEvent);
		m_hEndReqEvent = NULL;
	}

	// スレッド開始イベント解放
	if (m_hStartEvent != NULL)
	{
		CloseHandle(m_hStartEvent);
		m_hStartEvent = NULL;
	}

	// DACL格納用ポインタがNULL以外の場合
	if (m_pDacl != NULL)
	{
		HeapFree(GetProcessHeap(), 0, m_pDacl);
		m_pDacl = NULL;
	}

	// SID格納用ポインタがNULL以外の場合
	if (m_pPsid != NULL)
	{
		HeapFree(GetProcessHeap(), 0, m_pPsid);
		m_pPsid = NULL;
	}

	m_bInit = FALSE;
}



//-----------------------------------------------------------------------------
// スレッド開始
//-----------------------------------------------------------------------------
BOOL CThread::Start(void)
{
	DWORD			dwRet = 0;
	BOOL			bRet = FALSE;


	// 初期化に失敗している場合
	if (m_bInit == FALSE)
	{
		return FALSE;
	}

	// 既に起動している場合
	if (m_hThread != 0)
	{
		return TRUE;
	}

	// スレッド開始
	ResetEvent(m_hStartEvent);
	ResetEvent(m_hEndReqEvent);
	m_hThread = _beginthreadex(&m_Sa, 0, LauncherThreadFunc, this, 0, &m_ThreadID);
	if (m_hThread == 0)
	{
		return FALSE;
	}

	// スレッド開始イベントが来るまで待つ
	dwRet = WaitForSingleObject(m_hStartEvent, THREAD_START_TIMEOUT);
	ResetEvent(m_hStartEvent);
	switch (dwRet) {
	case WAIT_OBJECT_0:
		bRet = TRUE;
		break;

	case WAIT_TIMEOUT:
		bRet = FALSE;
		break;

	case WAIT_FAILED:
		m_dwError = GetLastError();
		bRet = FALSE;
		break;

	case WAIT_ABANDONED:
	default:
		bRet = FALSE;
		break;
	}

	return bRet;
}


//-----------------------------------------------------------------------------
// スレッド終了
//-----------------------------------------------------------------------------
BOOL CThread::End(void)
{
	DWORD			dwRet = 0;
	BOOL			bRet = FALSE;
	BOOL			bForcedEnd = FALSE;


	// 初期化に失敗している場合
	if (m_bInit == FALSE)
	{
		return FALSE;
	}

	// 既に停止している場合
	if (m_hThread == 0)
	{
		return TRUE;
	}

	// スレッドを終了させる
	bRet = SetEvent(m_hEndReqEvent);
	if (bRet == FALSE)
	{
		//　強制終了
		bForcedEnd = TRUE;
	}
	else
	{
		// スレッド終了待ち
		dwRet = WaitForSingleObject((HANDLE)m_hThread, THREAD_END_TIMEOUT);
		switch (dwRet) {
		case WAIT_OBJECT_0:
			bRet = TRUE;
			break;

		case WAIT_TIMEOUT:
			bForcedEnd = TRUE;
			bRet = FALSE;
			break;

		case WAIT_FAILED:
			m_dwError = GetLastError();
			bForcedEnd = TRUE;
			bRet = FALSE;
			break;

		case WAIT_ABANDONED:
		default:
			bForcedEnd = TRUE;
			bRet = FALSE;
			break;
		}
	}

	// 強制終了？
	if (bForcedEnd == TRUE)
	{
		// スレッド強制終了
		TerminateThread((HANDLE)m_hThread, 0xF9999999);
	}

	m_hThread = 0;
	m_ThreadID = 0;

	return bRet;
}


//-----------------------------------------------------------------------------
// スレッド動作中か確認する
//-----------------------------------------------------------------------------
BOOL CThread::IsActive(void)
{
	// 初期化に失敗している場合
	if (m_bInit == FALSE)
	{
		return FALSE;
	}

	// 既に停止している場合
	if (m_hThread == 0)
	{
		return FALSE;
	}

	return (WaitForSingleObject((HANDLE)m_hThread, 0) == WAIT_OBJECT_0 ? TRUE : FALSE);
}


//-----------------------------------------------------------------------------
// スレッド処理（※サンプル※）
//-----------------------------------------------------------------------------
unsigned CThread::ThreadFunc(void* pUserData)
{
	CThread				*pcThread = (CThread*)pUserData;
	HANDLE				Events[] = { m_hEndReqEvent };
	DWORD				dwEventCount = sizeof(Events) / sizeof(HANDLE);
	BOOL				bLoop = TRUE;
	DWORD				dwRet = 0;
	BOOL				bRet = FALSE;
	time_t				t;
	struct tm			tTm;


	// スレッド開始イベントを送信
	bRet = SetEvent(m_hStartEvent);
	if (bRet == TRUE)
	{
		// スレッド終了要求イベントが来るまでループ
		while (bLoop)
		{
			dwRet = WaitForMultipleObjects(dwEventCount, Events, FALSE, THREAD_LOOP_TIMEOUT);
			switch (dwRet) {
			case (WAIT_OBJECT_0 + 0):			// スレッド終了要求イベント
				bLoop = FALSE;
				break;

			case WAIT_TIMEOUT:					// タイムアウト（処理継続）
				time(&t);
				localtime_s(&tTm, &t);
				printf("[%04d/%02d/%02d %02d:%02d:%02d] Execute Thread Sample... \n", 
										(tTm.tm_year + 1900), tTm.tm_mon, tTm.tm_mday, tTm.tm_hour, tTm.tm_min, tTm.tm_sec);
				break;

			case WAIT_FAILED:
			default:
				bLoop = FALSE;
				break;
			}
		}
		ResetEvent(m_hEndReqEvent);
	}

	return 0;
}



//-----------------------------------------------------------------------------
// スレッド呼出し用関数
//-----------------------------------------------------------------------------
unsigned __stdcall CThread::LauncherThreadFunc( void *pClass )
{
	return (static_cast<CThread*>(pClass))->ThreadFunc(pClass);
}


