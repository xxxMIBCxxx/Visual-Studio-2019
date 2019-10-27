#pragma once
//*****************************************************************************
// CThreadクラス ヘッダーファイル
//*****************************************************************************
#include <Windows.h>



class CThread
{
private:
	BOOL								m_bInit;						// 初期化フラグ

	uintptr_t							m_hThread;						// スレッドハンドル
	unsigned							m_ThreadID;						// スレッド識別子


public:
	SECURITY_ATTRIBUTES					m_Sa;							// セキュリティ属性
	SECURITY_DESCRIPTOR					m_Sd;							// セキュリティ記述子

	DWORD								m_dwUserNameSize;				// ユーザー名格納サイズ
	char								m_szUserName[256 + 1];			// ユーザー名
	
	DWORD								m_dwPsidSize;					// SID格納サイズ
	PSID								m_pPsid;						// SID格納用ポインタ

	DWORD								m_dwDaclSize;					// DACL格納サイズ
	PACL								m_pDacl;						// DACL格納用ポインタ

	DWORD								m_dwDomainNameSize;				// ドメイン名格納サイズ
	char								m_szDomainName[256 + 1];		// ドメイン名

	SID_NAME_USE						m_eSidType;						// Sid Type

	DWORD								m_dwError;


	HANDLE								m_hStartEvent;					// スレッド開始イベント
	HANDLE								m_hEndReqEvent;					// スレッド終了要求イベント



	CThread();

	~CThread();

	BOOL Start(void);
	BOOL End(void);
	BOOL IsActive(void);

	virtual unsigned ThreadFunc(void* pUserData);
	static unsigned __stdcall LauncherThreadFunc(void* pUserData);
};
















