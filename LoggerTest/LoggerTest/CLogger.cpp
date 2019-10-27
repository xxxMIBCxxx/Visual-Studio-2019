//=============================================================================
// CLogger�N���X�@�\�[�X�t�@�C��
//=============================================================================
#include "CLogger.h"


#define DEFAULT_LOG_FILE_SIZE			( 1024 * 1024 * 4 )
#define MIN_LOG_FILE_SIZE				( 1024 * 1 )
#define MAX_LOG_FILE_SIZE				( 1024 * 1024 * 10 )
#define	DEFAULT_LOG_OUTPUT				( LOG_OUTPUT_ERROR )
#define LOG_OUTPUT_COUNT				( 25 )


//-----------------------------------------------------------------------------
// �R���X�g���N�^
//-----------------------------------------------------------------------------
CLogger::CLogger()
{
	m_bOpenFlag = FALSE;

	memset(&m_tLogSettingInfo, 0x00, sizeof(m_tLogSettingInfo));
	m_tLogSettingInfo.dwLogFileSize = DEFAULT_LOG_FILE_SIZE;
	m_tLogSettingInfo.dwLogOutputBit = (DWORD)DEFAULT_LOG_OUTPUT;

	GetModulePath(m_tLogSettingInfo.szFileName, MAX_PATH);
	strncat_s(m_tLogSettingInfo.szFileName, MAX_PATH, "aaaaaaaa.log", (MAX_PATH - 1));

	m_hMutex = NULL;


	m_dwError = 0;

	m_tLogOutputList.clear();

	// INI�t�@�C���p�X�擾
	memset(m_szIniFile, 0x00, sizeof(m_szIniFile));
	GetIniFilePath(m_szIniFile, MAX_PATH);

	// INI�t�@�C�����擾
	GetIniFileInfo(m_tLogSettingInfo);
}


//-----------------------------------------------------------------------------
// �f�X�g���N�^
//-----------------------------------------------------------------------------
CLogger::~CLogger()
{
	// �N���[�Y���Y��l��
	Close();
}


//-----------------------------------------------------------------------------
// �I�[�v��
//-----------------------------------------------------------------------------
BOOL CLogger::Open(void)
{
	BOOL					bRet = FALSE;
	FILE					*pFile = NULL;
	errno_t					error;


	// ���ɃI�[�v�����Ă���ꍇ
	if (m_bOpenFlag == TRUE)
	{
		return TRUE;
	}

	// �~���[�e�b�N�X�n���h�����擾
	m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutex == NULL)
	{
		m_dwError = GetLastError();
		return FALSE;
	}

	// �t�@�C�����m�F���邽�߁A�ꎞ�I�ɃI�[�v������
	error = fopen_s(&pFile, m_tLogSettingInfo.szFileName,"r+");
	if (error == ENOENT)
	{
		error = fopen_s(&pFile, m_tLogSettingInfo.szFileName, "w+");
	}

	if (error != 0)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return FALSE;
	}
	fclose(pFile);

	// ���O�����݃X���b�h���J�n����
	bRet = Start();
	if (bRet == FALSE)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return FALSE;
	}

	m_bOpenFlag = TRUE;

	return TRUE;
}



//-----------------------------------------------------------------------------
// �N���[�Y
//-----------------------------------------------------------------------------
BOOL CLogger::Close(void)
{
	DWORD			dwRet = 0;


	// ���O���I�[�v�����Ă��Ȃ��ꍇ
	if (m_bOpenFlag == FALSE)
	{
		return TRUE;
	}

	// ������������������������������������������������������������
	dwRet = WaitForSingleObject(m_hMutex, INFINITE);

	// ���O�����݃X���b�h���I��������
	End();

	if (dwRet == (WAIT_OBJECT_0 + 0))
	{
		ReleaseMutex(m_hMutex);
	}
	// ������������������������������������������������������������

	// �~���[�e�b�N�X�n���h�������
	CloseHandle(m_hMutex);
	m_hMutex = NULL;

	m_bOpenFlag = FALSE;

	return TRUE;
}


//-----------------------------------------------------------------------------
// ���O�o��
//-----------------------------------------------------------------------------
void CLogger::Output(LOG_OUTPUT_ENUM eLog, const char* format, ...)
{
	DWORD				dwRet = 0;
	char				szTimeBuff[30];
	size_t				Length;

	// �I�[�v�����Ă��Ȃ��ꍇ
	if (m_bOpenFlag == FALSE)
	{
		return;
	}

	// ���O�o�͔���
	if (!(m_tLogSettingInfo.dwLogOutputBit & (DWORD)eLog))
	{
		return;
	}


	// ���ݎ������擾
	memset(szTimeBuff, 0x00, sizeof(szTimeBuff));
	SYSTEMTIME			tSt;
	GetSystemTime(&tSt);
	sprintf_s(szTimeBuff, 30, "[%04d/%02d/%02d %02d:%02d:%02d.%03d] ",
		tSt.wYear, tSt.wMonth, tSt.wDay, (tSt.wHour + 9), tSt.wMinute, tSt.wSecond, tSt.wMilliseconds);

	// �ψ�����W�J
	memset(m_szBuff, 0x00, sizeof(m_szBuff));
	va_list			ap;
	va_start(ap, format);
	vsprintf_s(m_szBuff, LOG_MAX_COL, format, ap);
	va_end(ap);

	// ���O�o�̓��X�g�ɓo�^
	LOG_OUTPUT_INFO_TABLE		tLogOutput;
	tLogOutput.Length = strlen(m_szBuff) + strlen(szTimeBuff) + 1 + 1;			// CR + NULL
	tLogOutput.pszLog = (char*)malloc(tLogOutput.Length);
	if (tLogOutput.pszLog != NULL)
	{
		memset(tLogOutput.pszLog, 0x00, tLogOutput.Length);
		Length = strlen(szTimeBuff);
		strncpy_s(tLogOutput.pszLog, tLogOutput.Length, szTimeBuff, Length);
		strcat_s(&tLogOutput.pszLog[Length], (tLogOutput.Length - Length), m_szBuff);
		Length += strlen(m_szBuff);
		strcat_s(&tLogOutput.pszLog[Length], (tLogOutput.Length - Length), "\n");

		// ������������������������������������������������������������
		dwRet = WaitForSingleObject(m_hMutex, (1000 * 5));
		switch (dwRet) {
		case (WAIT_OBJECT_0 + 0):		// �~���[�e�b�N�X�l��
			m_tLogOutputList.push_back(tLogOutput);
			ReleaseMutex(m_hMutex);
			break;

		default:
			break;
		}
		// ������������������������������������������������������������
	}
}


//-----------------------------------------------------------------------------
// ���W���[���p�X���擾����
//-----------------------------------------------------------------------------
void CLogger::GetModulePath(char* pszBuff, DWORD dwBuffSize)
{
	DWORD		dwRet = 0;
	char		szPath[MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE + 1];
	char		szDir[_MAX_DIR + 1];


	// �p�����[�^�`�F�b�N
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return;
	}

	// ���W���[���t�@�C�������擾
	memset(szPath, 0x00, sizeof(szPath));
	dwRet = GetModuleFileName(NULL, szPath, MAX_PATH);

	// �p�X�𕪊�
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);

	// ���W���[���p�X�𐶐�
	memset(szPath, 0x00, sizeof(szPath));
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, NULL, NULL);

	strncpy_s(pszBuff, dwBuffSize, szPath, (dwBuffSize - 1));
}


//-----------------------------------------------------------------------------
// INI�t�@�C���p�X���擾����
//-----------------------------------------------------------------------------
void CLogger::GetIniFilePath(char* pszBuff, DWORD dwBuffSize)
{
	DWORD		dwRet = 0;
	char		szPath[MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE + 1];
	char		szDir[_MAX_DIR + 1];
	char		szFname[_MAX_FNAME + 1];


	// �p�����[�^�`�F�b�N
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return;
	}

	// ���W���[���t�@�C�������擾
	memset(szPath, 0x00, sizeof(szPath));
	dwRet = GetModuleFileName(NULL, szPath, MAX_PATH);

	// �p�X�𕪊�
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFname, _MAX_FNAME, NULL, 0);

	// INI�t�@�C���p�X�𐶐�
	memset(szPath, 0x00, sizeof(szPath));
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, szFname, ".ini");

	strncpy_s(pszBuff, dwBuffSize, szPath, (dwBuffSize - 1));
}


//-----------------------------------------------------------------------------
// LOG�t�@�C���p�X���擾����
//-----------------------------------------------------------------------------
void CLogger::GetLogFilePath(char* pszBuff, DWORD dwBuffSize)
{
	DWORD		dwRet = 0;
	char		szPath[MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE + 1];
	char		szDir[_MAX_DIR + 1];
	char		szFname[_MAX_FNAME + 1];


	// �p�����[�^�`�F�b�N
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return;
	}

	// ���W���[���t�@�C�������擾
	memset(szPath, 0x00, sizeof(szPath));
	dwRet = GetModuleFileName(NULL, szPath, MAX_PATH);

	// �p�X�𕪊�
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFname, _MAX_FNAME, NULL, 0);

	// LOG�t�@�C���p�X�𐶐�
	memset(szPath, 0x00, sizeof(szPath));
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, szFname, ".log");

	strncpy_s(pszBuff, dwBuffSize, szPath, (dwBuffSize - 1));
}


//-----------------------------------------------------------------------------
// ���O�o�̓X���b�h
//-----------------------------------------------------------------------------
unsigned CLogger::ThreadFunc(void* pUserData)
{
	CLogger				*pcThread = (CLogger*)pUserData;
	HANDLE				Events[] = { m_hEndReqEvent };
	DWORD				dwEventCount = sizeof(Events) / sizeof(HANDLE);
	BOOL				bLoop = TRUE;
	DWORD				dwRet = 0;
	BOOL				bRet = FALSE;


	// �X���b�h�J�n�C�x���g�𑗐M
	bRet = SetEvent(m_hStartEvent);
	if (bRet == TRUE)
	{
		// �X���b�h�I���v���C�x���g������܂Ń��[�v
		while (bLoop)
		{
			dwRet = WaitForMultipleObjects(dwEventCount, Events, FALSE, 1000);
			switch (dwRet) {
			case (WAIT_OBJECT_0 + 0):			// �X���b�h�I���v���C�x���g
				bLoop = FALSE;
				break;

			case WAIT_TIMEOUT:					// �^�C���A�E�g�i�����p���j

				// ������������������������������������������������������������
				dwRet = WaitForSingleObject(m_hMutex, 200);

				switch (dwRet) {
				case (WAIT_OBJECT_0 + 0):		// �~���[�e�b�N�X�l��

					// ���O�����ݏ���
					LogWrite( FALSE );

					ReleaseMutex(m_hMutex);
					break;

				default:
					// �������Ȃ�
					break;
				}
				break;
				// ������������������������������������������������������������

			case WAIT_FAILED:
			default:
				bLoop = FALSE;
				break;
			}
		}
		ResetEvent(m_hEndReqEvent);
	}

	// ���O�����ݏ����i���X�g�ɓo�^����Ă�����̂�S�ď����݁j
	LogWrite(TRUE);

	return 0;
}


//-----------------------------------------------------------------------------
// ���O�o�͏���
//-----------------------------------------------------------------------------
void CLogger::LogWrite( BOOL bEnd )
{
	char				szBuff[16];
	DWORD				dwPos = 0;
	DWORD				dwCount = 0;
	FILE				*pFile = NULL;
	errno_t				error;


	// ���O�o�̓f�[�^���Ȃ���Ώ������s��Ȃ�
	if (m_tLogOutputList.size() == 0)
	{
		return;
	}

	// �t�@�C�����I�[�v������
	error = fopen_s(&pFile, m_tLogSettingInfo.szFileName, "r+");
	if (error == ENOENT)
	{
		error = fopen_s(&pFile, m_tLogSettingInfo.szFileName, "w+");
	}
	if (error != 0)
	{
		return;
	}
	
	// ���O�o�̓f�[�^���Ȃ��Ȃ�܂Ń��[�v
	while (m_tLogOutputList.size() != 0)
	{
		// ���O�o�͂̐擪�f�[�^���擾
		std::list<LOG_OUTPUT_INFO_TABLE>::iterator			it = m_tLogOutputList.begin();

		// �t�@�C�������݈ʒu���擾
		memset(szBuff, 0x00, sizeof(szBuff));
		fseek(pFile, 0, SEEK_SET);
		fread(szBuff, 8, 1, pFile);
		dwPos = atol(szBuff);
		if (dwPos == 0)
		{
			sprintf_s(szBuff, 16, "%08d\n", dwPos);
			fseek(pFile, 0, SEEK_SET);
			fwrite(szBuff, 9, 1, pFile);
			dwPos = 10;
		}


		// ���O�������ލہA�t�@�C���ő�T�C�Y�𒴂��Ă��邩���ׂ�
		if ((dwPos + it->Length) > m_tLogSettingInfo.dwLogFileSize)
		{
			dwPos = 10;
		}

		// ���O������
		fseek(pFile, dwPos, SEEK_SET);
		fwrite(it->pszLog, (it->Length -1), 1, pFile);
		dwPos += (it->Length - 1);

		// �t�@�C���ʒu������
		memset(szBuff, 0x00, sizeof(szBuff));
		sprintf_s(szBuff, 16, "%08d\n", dwPos);
		fseek(pFile, 0, SEEK_SET);
		fwrite(szBuff, 9, 1, pFile);

		// ���O�o�̓��X�g�̐擪���O�����폜
		if (it->pszLog != NULL)
		{
			free(it->pszLog);
		}
		m_tLogOutputList.pop_front();

		// �I����������Ă΂�Ă��Ȃ��ꍇ
		if (bEnd == FALSE)
		{
			dwCount++;

			if (dwCount >= LOG_OUTPUT_COUNT)
			{
				break;
			}
		}
	}

	// �t�@�C�������
	fclose(pFile);
}


//-----------------------------------------------------------------------------
// INI�t�@�C�����擾
//-----------------------------------------------------------------------------
void CLogger::GetIniFileInfo(LOG_SETTING_INFO_TABLE& tLogSettingInfo)
{
	// ���O�o�̓p�X
	GetPrivateProfileString("LOG", "PATH", "", tLogSettingInfo.szFileName, MAX_PATH, m_szIniFile);
	if (strlen(tLogSettingInfo.szFileName) == 0)
	{
		GetLogFilePath(tLogSettingInfo.szFileName, MAX_PATH);
	}

	// ���O�t�@�C���T�C�Y
	tLogSettingInfo.dwLogFileSize = GetPrivateProfileInt("LOG", "SIZE", DEFAULT_LOG_FILE_SIZE, m_szIniFile);
	if (tLogSettingInfo.dwLogFileSize < MIN_LOG_FILE_SIZE)
	{
		tLogSettingInfo.dwLogFileSize = MIN_LOG_FILE_SIZE;
	}
	else if (tLogSettingInfo.dwLogFileSize > MAX_LOG_FILE_SIZE)
	{
		tLogSettingInfo.dwLogFileSize = MAX_LOG_FILE_SIZE;
	}

	// ���O�o�͎��
	tLogSettingInfo.dwLogOutputBit = GetPrivateProfileInt("LOG", "KIND", DEFAULT_LOG_OUTPUT, m_szIniFile);
	if (tLogSettingInfo.dwLogOutputBit == 0)
	{
		tLogSettingInfo.dwLogOutputBit = DEFAULT_LOG_OUTPUT;
	}
}
