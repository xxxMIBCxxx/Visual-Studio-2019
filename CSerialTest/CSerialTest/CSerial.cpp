//*****************************************************************************
// CSerial�N���X �\�[�X�t�@�C��
//*****************************************************************************
#include "CSerial.h"
#include <setupapi.h>

#pragma comment( lib, "Setupapi.lib" )

#define	THREAD_LOOP_TIMEOUT						( 100 )						// ��M�X���b�h�̑҂�����(ms)
#define RECV_BUFF_SIZE							( 1024 )					// ��M�o�b�t�@�T�C�Y


//-----------------------------------------------------------------------------
// �R���X�g���N�^
//-----------------------------------------------------------------------------
CSerial::CSerial()
{
	m_hCom = INVALID_HANDLE_VALUE;
	memset(&m_tComPortInfo, 0x00, sizeof(m_tComPortInfo));
	memset(&m_tDcb, 0x00, sizeof(m_tDcb));
	m_tDcb.DCBlength = sizeof(DCB);

	m_hMutex = NULL;
	m_dwRecvLength = 0;
	m_pRecvBuff = NULL;


	m_dwError = 0;
	m_eThreadRet = SERIAL_RET_SUCCESS;
}


//-----------------------------------------------------------------------------
// �f�X�g���N�^
//-----------------------------------------------------------------------------
CSerial::~CSerial()
{
	// COM�|�[�g�������Ă��Ȃ��ꍇ���l��
	Close();
}


//-----------------------------------------------------------------------------
// �V���A���|�[�g�I�[�v��
//-----------------------------------------------------------------------------
SERIAL_RET_ENUM CSerial::Open(COM_PORT_INFO_TABLE& tComPortInfo, COM_PORT_SETTING_INFO_TABLE& tComPortSettingInfo)
{
	BOOL			bRet = FALSE;
	

	// ���ɃI�[�v�����Ă���ꍇ
	if (m_hCom != INVALID_HANDLE_VALUE)
	{
		return SERIAL_RET_SUCCESS;
	}

	// �~���[�e�b�N�X�n���h�����擾
	m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutex == NULL)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_MUTEX;
	}


	// �V���A���|�[�g���I�[�v��
	m_hCom = CreateFile(tComPortInfo.szComName, (GENERIC_READ | GENERIC_WRITE), 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_OPEN;
	}
	memcpy(&m_tComPortInfo, &tComPortInfo, sizeof(COM_PORT_INFO_TABLE));

	// COM�|�[�g�̃f�o�C�X����u���b�N�����擾
	bRet = GetCommState(m_hCom, &m_tDcb);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_SETTING;
	}

	// COM�|�[�g�̃f�o�C�X����u���b�N����ݒ�
	DCB					tDcb;
	memcpy(&tDcb, &m_tDcb, sizeof(DCB));
	tDcb.BaudRate = (DWORD)tComPortSettingInfo.eBaudRate;		// �{�[ ���[�g
	tDcb.fBinary = TRUE;										// �o�C�i�� ���[�h�F�L��(���TRUE�Œ�j
//	tDcb.fOutxCtsFlow = TRUE;									// CTS(���M�N���A)�M���̏o�̓t���[�Ď��Ď�
//	tDcb.fOutxDsrFlow = TRUE;									// DSR(�f�[�^�E�Z�b�g�Ή�)�M���͏o�̓t���[����Ď�
//	tDcb.fDtrControl = DTR_CONTROL_DISABLE;						// DTR(�f�[�^�[���Ή�)�t���[����
//	tDcb.fDsrSensitivity = FALSE;								// DSR �M���̏�Ԃɕq��(DSR ���f�����͍s�������ꍇ�������A�h���C�o�[�͎�M�����o�C�g�𖳎�)
//	tDcb.fTXContinueOnXoff = TRUE;								// �h���C�o�[��XoffChar�����𑗐M�������Ƃ̓���
//	tDcb.fOutX = FALSE;											// �`������ XON/XOFF�t���[����(TRUE�̏ꍇ��XoffChar�����̎�M���ɑ��M����~���AXonChar�����̎�M���ɍĂъJ�n�����)
//	tDcb.fInX = FALSE;											// ��M���� XON/XOFF�t���[����(TRUE�̏ꍇ�͓��̓o�b�t�@�[�����S��XoffLim�o�C�g���ɓ����XoffChar���������M����A���̓o�b�t�@�[��XonLim�o�C�g���ɓ����XonChar���������M����܂�)
//	tDcb.fNull = TRUE;											// null �o�C�g�͎�M���ɔj�������
//	tDcb.fRtsControl = RTS_CONTROL_DISABLE;						// RTS(���M�v��) �t���[����
//	tDcb.fAbortOnError = FALSE;									// �h���C�o�[�̓G���[�����������ꍇ�ɃG���[�X�e�[�^�X�ł��ׂĂ̓ǂݎ�肨��я������ݑ�����I�����邩�̐ݒ�
//	tDcb.XonLim = 0;											// �t���[���䂪�A�N�e�B�u�������O�ɓ��̓o�b�t�@�[�ŋ�����Ă���g�p�o�C�g���̍ŏ���
//	tDcb.XoffLim = 0;											// ���M�҂��֎~���邽�߂Ƀt���[���䂪�A�N�e�B�u�������O�ɁA���̓o�b�t�@�[�ŋ������󂫃o�C�g�̍ŏ���
	tDcb.ByteSize = tComPortSettingInfo.ByteSize;				// ����M���ꂽ�o�C�g���̃r�b�g��
	tDcb.fParity = TRUE;										// �p���e�B�`�F�b�N�̗L��
	tDcb.Parity = (BYTE)tComPortSettingInfo.eParityCheck;		// �p���e�B�`�F�b�N�̎��
	tDcb.StopBits = (BYTE)tComPortSettingInfo.eStopBits;		// �X�g�b�v�r�b�g
//	tDcb.XonChar = '';											// ���M�Ǝ�M�̗����� XON �����̒l
//	tDcb.XoffChar = '';											// ���M�Ǝ�M�̗����� XOFF �����̒l
//	tDcb.fErrorChar = FALSE;									// �p���e�B �G���[�Ŏ�M�����o�C�g���AErrorChar�����o�[�Ŏw�肳�ꂽ�����ɒu���������邩�ǂ����������܂�
//	tDcb.ErrorChar = '';										// �󂯎�����o�C�g���p���e�B �G���[�Œu�������邽�߂Ɏg�p����镶���̒l
//	tDcb.EofChar = '';											// �f�[�^�̏I����ʒm���邽�߂Ɏg�p����镶���̒l
//  tDcb.EvtChar = '';											// �C�x���g�̃V�O�i���𑗐M���邽�߂Ɏg�p����镶���̒l
	bRet = SetCommState(m_hCom, &tDcb);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_SETTING;
	}
	memcpy(&m_tDcb, &tDcb, sizeof(DCB));
	
	// ��M�X���b�h���J�n������
	bRet = Start();
	if (bRet == FALSE)
	{
		Close();
		return SERIAL_RET_ERROR_RECV_THREAD;
	}

	return SERIAL_RET_SUCCESS;
}


//-----------------------------------------------------------------------------
// �V���A���|�[�g�N���[�Y
//-----------------------------------------------------------------------------
void CSerial::Close(void)
{
	DWORD			dwRet = 0;

	// ���ɕ����Ă���ꍇ
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// ������������������������������������������������������������
	dwRet = WaitForSingleObject(m_hMutex, (1000 * 5));

	// ��M�X���b�h���I��������
	End();

	// ��M�o�b�t�@�N���A
	if (m_pRecvBuff != NULL)
	{
		free(m_pRecvBuff);
		m_pRecvBuff = NULL;
	}
	m_dwRecvLength = 0;

	// COM�|�[�g�����
	if (m_hCom != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
		memset(&m_tComPortInfo, 0x00, sizeof(m_tComPortInfo));
	}

	if (dwRet == (WAIT_OBJECT_0 + 0))
	{
		ReleaseMutex(m_hMutex);
	}
	// ������������������������������������������������������������

	// �~���[�e�b�N�X�n���h�������
	if (m_hMutex != NULL)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}


//-----------------------------------------------------------------------------
// �V���A���|�[�g�����擾
//-----------------------------------------------------------------------------
BOOL CSerial::GetComPortInfo(std::list<COM_PORT_INFO_TABLE>& tComPortInfoList)
{
	BOOL					bRet = FALSE;
	GUID					cGuid;
	DWORD					dwRequiredSize = 0;
	HDEVINFO 				hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA			tDevInfoData;
	DWORD					dwMemberIndex = 0;
	COM_PORT_INFO_TABLE		tComPortInfo;
	HKEY					hKey = (HKEY)INVALID_HANDLE_VALUE;
	long					lRet = 0;
	BOOL					bResult = TRUE;



	// ���X�g��������
	tComPortInfoList.clear();


	// PORTS�N���X��GUID���擾
	bRet = SetupDiClassGuidsFromName("PORTS", &cGuid, 1, &dwRequiredSize);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		return FALSE;
	}

	// GUID�������N���X�̃f�o�C�X���n���h�����擾
	hDevInfo = SetupDiGetClassDevs(&cGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_PROFILE));
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		m_dwError = GetLastError();
		return FALSE;
	}


	// �f�o�C�X��񂪖����Ȃ�܂Ń��[�v
	dwMemberIndex = 0;
	while (1)
	{
		// �f�o�C�X�����擾
		memset(&tDevInfoData, 0x00, sizeof(tDevInfoData));
		memset(&tComPortInfo, 0x00, sizeof(tComPortInfo));
		tDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		bRet = SetupDiEnumDeviceInfo(hDevInfo, dwMemberIndex, &tDevInfoData);
		if (bRet == FALSE)
		{
			// �G���[���e���u�f�[�^�͂���ȏ゠��܂���v�̏ꍇ�́A�G���[�Ƃ��Ȃ�
			m_dwError = GetLastError();
			if (m_dwError == ERROR_NO_MORE_ITEMS)
			{
				bResult = TRUE;
			}
			else
			{
				bResult = FALSE;
			}
			break;
		}

		// �f�o�C�X���̃t�����h���[�����擾
		bRet = SetupDiGetDeviceRegistryProperty(hDevInfo, &tDevInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)tComPortInfo.szFriendlyName, MAX_PATH, NULL);
		if (bRet == FALSE)
		{
			m_dwError = GetLastError();
			bResult = FALSE;
			break;
		}

		// �f�o�C�X�̃��W�X�g���L�[�I�[�v��
		hKey = SetupDiOpenDevRegKey(hDevInfo, &tDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
		if (hKey != INVALID_HANDLE_VALUE)
		{
			// ���W�X�g������"PortName"������H
			dwRequiredSize = 32;
			lRet = RegQueryValueEx(hKey, "PortName", 0, NULL, (PBYTE)tComPortInfo.szComName, &dwRequiredSize);
			if (lRet == ERROR_SUCCESS)
			{
				// �V���A���|�[�g�Ȃ̂����ׂ�
				if (_strnicmp(tComPortInfo.szComName, "COM", 3) == 0)
				{
					// ���X�g�ɓo�^
					tComPortInfoList.push_back(tComPortInfo);
				}
			}

			// ���W�X�g���L�[�����
			RegCloseKey(hKey);
		}
		else
		{
			m_dwError = GetLastError();
			bResult = FALSE;
			break;
		}

		// ���̃f�o�C�X����
		dwMemberIndex++;
	}

	// �f�o�C�X���n���h�������
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return bResult;
}


//-----------------------------------------------------------------------------
// �f�[�^���M
//-----------------------------------------------------------------------------
SERIAL_RET_ENUM CSerial::Send(const char *pszSendData, DWORD dwSendDataLength )
{
	BOOL				bRet = FALSE;
	DWORD				dwWriteLength = 0;


	// COM�|�[�g���I�[�v�����Ă��Ȃ��ꍇ
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return SERIAL_RET_ERROR_NOT_OPEN;
	}

	// �p�����[�^�`�F�b�N
	if ((pszSendData == NULL) || (dwSendDataLength == 0))
	{
		return SERIAL_RET_ERROR_PARAM;
	}
	
	// ���M�f�[�^��S�đ��M����܂Ń��[�v
	while (dwSendDataLength != 0)
	{
		bRet = WriteFile(m_hCom, pszSendData, dwSendDataLength, &dwWriteLength, NULL);
		if (bRet == FALSE)
		{
			return SERIAL_RET_ERROR_SEND;
		}

		dwSendDataLength -= dwWriteLength;
	}

	return SERIAL_RET_SUCCESS;
}


//-----------------------------------------------------------------------------
// ��M�f�[�^�L���`�F�b�N
//-----------------------------------------------------------------------------
BOOL CSerial::IsRecv(void)
{
	return ((m_dwRecvLength != 0) ? TRUE : FALSE);
}


//-----------------------------------------------------------------------------
// ��M�f�[�^�擾
//-----------------------------------------------------------------------------
SERIAL_RET_ENUM CSerial::Recv(char* pszBuff, DWORD dwBuffSize)
{
	SERIAL_RET_ENUM			eRet = SERIAL_RET_SUCCESS;
	DWORD					dwRet = 0;


	// COM�|�[�g���I�[�v�����Ă��Ȃ��ꍇ
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return SERIAL_RET_ERROR_NOT_OPEN;
	}

	// �p�����[�^�`�F�b�N
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return SERIAL_RET_ERROR_PARAM;
	}

	// ������������������������������������������������������������
	dwRet = WaitForSingleObject(m_hMutex, 5000);
	switch (dwRet) {
	case (WAIT_OBJECT_0 + 0):			// �X���b�h�I���v���C�x���g
		
		// ��M�f�[�^���Ȃ��ꍇ
		if (m_dwRecvLength == 0)
		{
		}
		else
		{
			// �ێ����Ă����M�f�[�^���o�b�t�@�T�C�Y�փR�s�[
			if (dwBuffSize >= m_dwRecvLength)
			{
				memcpy(pszBuff, m_pRecvBuff, m_dwRecvLength);
				free(m_pRecvBuff);
				m_pRecvBuff = NULL;
				m_dwRecvLength = 0;
			}
			else
			{
				memcpy(pszBuff, m_pRecvBuff, dwBuffSize);

				DWORD		dwTempLength = m_dwRecvLength - dwBuffSize;
				char		*pTemp = (char*)malloc(dwTempLength + 1);
				if (pTemp != NULL)
				{
					memcpy(pTemp, &m_pRecvBuff[dwBuffSize], dwTempLength);
					pTemp[dwTempLength] = '\0';

					free(m_pRecvBuff);
					m_pRecvBuff = pTemp;
					m_dwRecvLength = dwTempLength;
				}
				else
				{
					eRet = SERIAL_RET_ERROR_SYSTEM;
				}
			}
		}

		// �~���[�e�b�N�X���
		ReleaseMutex(m_hMutex);
		break;

	case WAIT_TIMEOUT:					// �^�C���A�E�g
		eRet = SERIAL_RET_ERROR_MUTEX_TIMEOUT;
		break;

	case WAIT_FAILED:
	default:
		m_dwError = GetLastError();
		eRet = SERIAL_RET_ERROR_SYSTEM;
		break;
	}

	return eRet;
}



//-----------------------------------------------------------------------------
// ��M�X���b�h
//-----------------------------------------------------------------------------
unsigned CSerial::ThreadFunc(void* pUserData)
{
	CSerial				*pcThread = (CSerial*)pUserData;
	HANDLE				Events[] = { m_hEndReqEvent };
	DWORD				dwEventCount = sizeof(Events) / sizeof(HANDLE);
	BOOL				bLoop = TRUE;
	DWORD				dwRet = 0;
	BOOL				bRet = FALSE;
	char				szRecvBuff[RECV_BUFF_SIZE + 1];
	DWORD				dwRecvSize = 0;


	// �X���b�h�J�n�C�x���g�𑗐M
	m_eThreadRet = SERIAL_RET_SUCCESS;
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

				// ������������������������������������������������������������
				dwRet = WaitForSingleObject(m_hMutex, 100);

				switch (dwRet) {
				case (WAIT_OBJECT_0 + 0):		// �~���[�e�b�N�X�l��

					// ��M�f�[�^���擾
					dwRecvSize = 0;
					memset(szRecvBuff, 0x00, sizeof(szRecvBuff));
					bRet = ReadFile(m_hCom, szRecvBuff, RECV_BUFF_SIZE, &dwRecvSize, NULL);
					if (bRet == TRUE)
					{
						// ��M�f�[�^������ꍇ
						if (dwRecvSize > 0)
						{
							// �V�����̈���쐬
							char* pTemp = (char*)malloc(m_dwRecvLength + dwRecvSize + 1);
							if (pTemp == NULL)
							{
								// �ُ�I���ɂ���
								m_eThreadRet = SERIAL_RET_ERROR_SYSTEM;
								bLoop = FALSE;
							}
							else
							{
								// �ێ����Ă�����M�f�[�^������ꍇ
								if ((m_dwRecvLength != 0) && (m_pRecvBuff != NULL))
								{
									// �ێ����Ă�����M�f�[�^���R�s�[
									memcpy(pTemp, m_pRecvBuff, m_dwRecvLength);
								}

								// �����āA�����M�����f�[�^���R�s�[
								memcpy(&pTemp[m_dwRecvLength], szRecvBuff, dwRecvSize);
								pTemp[(m_dwRecvLength + dwRecvSize)] = '\0';

								// �ێ����Ă����̈�����
								if (m_pRecvBuff != NULL)
								{
									free(m_pRecvBuff);
								}

								m_pRecvBuff = pTemp;
								m_dwRecvLength += dwRecvSize;
							}
						}
					}

					// �~���[�e�b�N�X���
					ReleaseMutex(m_hMutex);
					break;

				case WAIT_TIMEOUT:					// �^�C���A�E�g
					// �����s��Ȃ�
					break;

				case WAIT_FAILED:
				default:
					m_eThreadRet = SERIAL_RET_ERROR_SYSTEM;
					bLoop = FALSE;
					break;
				}
				// ������������������������������������������������������������
				break;

			case WAIT_FAILED:
			default:
				bLoop = FALSE;
				break;
			}
		}
		ResetEvent(m_hEndReqEvent);
	}
	else
	{
		m_eThreadRet = SERIAL_RET_ERROR_SYSTEM;
	}

	return 0;
}
