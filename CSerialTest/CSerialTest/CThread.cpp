//*****************************************************************************
// CThread�N���X �w�b�_�[�t�@�C��
//*****************************************************************************
#include "CThread.h"
#include <stdio.h>
#include <process.h>
#include <time.h>


#define THREAD_START_TIMEOUT					( 1000 )				// �X���b�h�J�n�҂��^�C���A�E�g(ms)
#define THREAD_END_TIMEOUT						( 1000 )				// �X���b�h�I���҂��^�C���A�E�g(ms)
#define THREAD_LOOP_TIMEOUT						( 100 )					// �X���b�h���[�v�^�C���A�E�g

//-----------------------------------------------------------------------------
// �R���X�g���N�^
//-----------------------------------------------------------------------------
CThread::CThread()
{
	BOOL			bRet = FALSE;


	// �����o�[�ϐ�������
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


	// ���O�C�����Ă��郆�[�U�[�����擾
	m_dwUserNameSize = 256;
	bRet = GetUserName(m_szUserName, &m_dwUserNameSize);
	if (bRet == FALSE)
	{
		return;
	}

	// ���[�U�[SID���擾
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

	// �Z�L�����e�B�L�q�q��������
	InitializeSecurityDescriptor(&m_Sd, SECURITY_DESCRIPTOR_REVISION);

	// DACL�i���ӃA�N�Z�X���䃊�X�g�j��������
	m_dwDaclSize = GetLengthSid(m_pPsid);
	m_pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, m_dwDaclSize);
	if (m_pDacl == NULL)
	{
		return;
	}
	InitializeAcl(m_pDacl, m_dwDaclSize, ACL_REVISION);

	// ACE(�A�N�Z�X����G���g��)��DACL�ɒǉ�����
	AddAccessAllowedAce(m_pDacl, ACL_REVISION, GENERIC_ALL, m_pPsid);

	// �Z�L�����e�B�L�q�q��DACL�̏���ݒ肷��
	SetSecurityDescriptorDacl(&m_Sd, TRUE, m_pDacl, FALSE);

	// SECURITY_ATTRIBUTES�i�Z�L�����e�B�����j�I�u�W�F�N�g�̊e�����o(�I�u�W�F�N�g�̃T�C�Y�A�n���h���p���̗L���A�Z�L�����e�B�L�q�q)�ɒl���Z�b�g����
	m_Sa.nLength = sizeof(m_Sa);
	m_Sa.bInheritHandle = FALSE;
	m_Sa.lpSecurityDescriptor = &m_Sd;


	// �X���b�h�J�n�C�x���g����
	m_hStartEvent = CreateEvent(&m_Sa, TRUE, FALSE, NULL);
	if (m_hStartEvent == NULL)
	{
		m_dwError = GetLastError();
		return;
	}

	// �X���b�h�I���v���C�x���g����
	m_hEndReqEvent = CreateEvent(&m_Sa, TRUE, FALSE, NULL);
	if (m_hEndReqEvent == NULL)
	{
		m_dwError = GetLastError();
		return;
	}

	m_bInit = TRUE;

}


//-----------------------------------------------------------------------------
// �f�X�g���N�^
//-----------------------------------------------------------------------------
CThread::~CThread()
{
	// �X���b�h�I���v���C�x���g���
	if (m_hEndReqEvent != NULL)
	{
		CloseHandle(m_hEndReqEvent);
		m_hEndReqEvent = NULL;
	}

	// �X���b�h�J�n�C�x���g���
	if (m_hStartEvent != NULL)
	{
		CloseHandle(m_hStartEvent);
		m_hStartEvent = NULL;
	}

	// DACL�i�[�p�|�C���^��NULL�ȊO�̏ꍇ
	if (m_pDacl != NULL)
	{
		HeapFree(GetProcessHeap(), 0, m_pDacl);
		m_pDacl = NULL;
	}

	// SID�i�[�p�|�C���^��NULL�ȊO�̏ꍇ
	if (m_pPsid != NULL)
	{
		HeapFree(GetProcessHeap(), 0, m_pPsid);
		m_pPsid = NULL;
	}

	m_bInit = FALSE;
}



//-----------------------------------------------------------------------------
// �X���b�h�J�n
//-----------------------------------------------------------------------------
BOOL CThread::Start(void)
{
	DWORD			dwRet = 0;
	BOOL			bRet = FALSE;


	// �������Ɏ��s���Ă���ꍇ
	if (m_bInit == FALSE)
	{
		return FALSE;
	}

	// ���ɋN�����Ă���ꍇ
	if (m_hThread != 0)
	{
		return TRUE;
	}

	// �X���b�h�J�n
	ResetEvent(m_hStartEvent);
	ResetEvent(m_hEndReqEvent);
	m_hThread = _beginthreadex(&m_Sa, 0, LauncherThreadFunc, this, 0, &m_ThreadID);
	if (m_hThread == 0)
	{
		return FALSE;
	}

	// �X���b�h�J�n�C�x���g������܂ő҂�
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
// �X���b�h�I��
//-----------------------------------------------------------------------------
BOOL CThread::End(void)
{
	DWORD			dwRet = 0;
	BOOL			bRet = FALSE;
	BOOL			bForcedEnd = FALSE;


	// �������Ɏ��s���Ă���ꍇ
	if (m_bInit == FALSE)
	{
		return FALSE;
	}

	// ���ɒ�~���Ă���ꍇ
	if (m_hThread == 0)
	{
		return TRUE;
	}

	// �X���b�h���I��������
	bRet = SetEvent(m_hEndReqEvent);
	if (bRet == FALSE)
	{
		//�@�����I��
		bForcedEnd = TRUE;
	}
	else
	{
		// �X���b�h�I���҂�
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

	// �����I���H
	if (bForcedEnd == TRUE)
	{
		// �X���b�h�����I��
		TerminateThread((HANDLE)m_hThread, 0xF9999999);
	}

	m_hThread = 0;
	m_ThreadID = 0;

	return bRet;
}


//-----------------------------------------------------------------------------
// �X���b�h���쒆���m�F����
//-----------------------------------------------------------------------------
BOOL CThread::IsActive(void)
{
	// �������Ɏ��s���Ă���ꍇ
	if (m_bInit == FALSE)
	{
		return FALSE;
	}

	// ���ɒ�~���Ă���ꍇ
	if (m_hThread == 0)
	{
		return FALSE;
	}

	return (WaitForSingleObject((HANDLE)m_hThread, 0) == WAIT_OBJECT_0 ? TRUE : FALSE);
}


//-----------------------------------------------------------------------------
// �X���b�h�����i���T���v�����j
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


	// �X���b�h�J�n�C�x���g�𑗐M
	bRet = SetEvent(m_hStartEvent);
	if (bRet == TRUE)
	{
		// �X���b�h�I���v���C�x���g������܂Ń��[�v
		while (bLoop)
		{
			dwRet = WaitForMultipleObjects(dwEventCount, Events, FALSE, THREAD_LOOP_TIMEOUT);
			switch (dwRet) {
			case (WAIT_OBJECT_0 + 0):			// �X���b�h�I���v���C�x���g
				bLoop = FALSE;
				break;

			case WAIT_TIMEOUT:					// �^�C���A�E�g�i�����p���j
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
// �X���b�h�ďo���p�֐�
//-----------------------------------------------------------------------------
unsigned __stdcall CThread::LauncherThreadFunc( void *pClass )
{
	return (static_cast<CThread*>(pClass))->ThreadFunc(pClass);
}


