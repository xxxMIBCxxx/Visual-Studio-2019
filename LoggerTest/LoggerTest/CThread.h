#pragma once
//*****************************************************************************
// CThread�N���X �w�b�_�[�t�@�C��
//*****************************************************************************
#include <Windows.h>



class CThread
{
private:
	BOOL								m_bInit;						// �������t���O

	uintptr_t							m_hThread;						// �X���b�h�n���h��
	unsigned							m_ThreadID;						// �X���b�h���ʎq


public:
	SECURITY_ATTRIBUTES					m_Sa;							// �Z�L�����e�B����
	SECURITY_DESCRIPTOR					m_Sd;							// �Z�L�����e�B�L�q�q

	DWORD								m_dwUserNameSize;				// ���[�U�[���i�[�T�C�Y
	char								m_szUserName[256 + 1];			// ���[�U�[��
	
	DWORD								m_dwPsidSize;					// SID�i�[�T�C�Y
	PSID								m_pPsid;						// SID�i�[�p�|�C���^

	DWORD								m_dwDaclSize;					// DACL�i�[�T�C�Y
	PACL								m_pDacl;						// DACL�i�[�p�|�C���^

	DWORD								m_dwDomainNameSize;				// �h���C�����i�[�T�C�Y
	char								m_szDomainName[256 + 1];		// �h���C����

	SID_NAME_USE						m_eSidType;						// Sid Type

	DWORD								m_dwError;


	HANDLE								m_hStartEvent;					// �X���b�h�J�n�C�x���g
	HANDLE								m_hEndReqEvent;					// �X���b�h�I���v���C�x���g



	CThread();

	~CThread();

	BOOL Start(void);
	BOOL End(void);
	BOOL IsActive(void);

	virtual unsigned ThreadFunc(void* pUserData);
	static unsigned __stdcall LauncherThreadFunc(void* pUserData);
};
















