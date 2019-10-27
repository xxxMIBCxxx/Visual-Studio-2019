//=============================================================================
// CLoggerクラス　ソースファイル
//=============================================================================
#include "CLogger.h"


#define DEFAULT_LOG_FILE_SIZE			( 1024 * 1024 * 4 )
#define MIN_LOG_FILE_SIZE				( 1024 * 1 )
#define MAX_LOG_FILE_SIZE				( 1024 * 1024 * 10 )
#define	DEFAULT_LOG_OUTPUT				( LOG_OUTPUT_ERROR )
#define LOG_OUTPUT_COUNT				( 25 )


//-----------------------------------------------------------------------------
// コンストラクタ
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

	// INIファイルパス取得
	memset(m_szIniFile, 0x00, sizeof(m_szIniFile));
	GetIniFilePath(m_szIniFile, MAX_PATH);

	// INIファイル情報取得
	GetIniFileInfo(m_tLogSettingInfo);
}


//-----------------------------------------------------------------------------
// デストラクタ
//-----------------------------------------------------------------------------
CLogger::~CLogger()
{
	// クローズし忘れ考慮
	Close();
}


//-----------------------------------------------------------------------------
// オープン
//-----------------------------------------------------------------------------
BOOL CLogger::Open(void)
{
	BOOL					bRet = FALSE;
	FILE					*pFile = NULL;
	errno_t					error;


	// 既にオープンしている場合
	if (m_bOpenFlag == TRUE)
	{
		return TRUE;
	}

	// ミューテックスハンドルを取得
	m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutex == NULL)
	{
		m_dwError = GetLastError();
		return FALSE;
	}

	// ファイルを確認するため、一時的にオープンする
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

	// ログ書込みスレッドを開始する
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
// クローズ
//-----------------------------------------------------------------------------
BOOL CLogger::Close(void)
{
	DWORD			dwRet = 0;


	// ログをオープンしていない場合
	if (m_bOpenFlag == FALSE)
	{
		return TRUE;
	}

	// ▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽
	dwRet = WaitForSingleObject(m_hMutex, INFINITE);

	// ログ書込みスレッドを終了させる
	End();

	if (dwRet == (WAIT_OBJECT_0 + 0))
	{
		ReleaseMutex(m_hMutex);
	}
	// ▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△

	// ミューテックスハンドルを解放
	CloseHandle(m_hMutex);
	m_hMutex = NULL;

	m_bOpenFlag = FALSE;

	return TRUE;
}


//-----------------------------------------------------------------------------
// ログ出力
//-----------------------------------------------------------------------------
void CLogger::Output(LOG_OUTPUT_ENUM eLog, const char* format, ...)
{
	DWORD				dwRet = 0;
	char				szTimeBuff[30];
	size_t				Length;

	// オープンしていない場合
	if (m_bOpenFlag == FALSE)
	{
		return;
	}

	// ログ出力判定
	if (!(m_tLogSettingInfo.dwLogOutputBit & (DWORD)eLog))
	{
		return;
	}


	// 現在時刻を取得
	memset(szTimeBuff, 0x00, sizeof(szTimeBuff));
	SYSTEMTIME			tSt;
	GetSystemTime(&tSt);
	sprintf_s(szTimeBuff, 30, "[%04d/%02d/%02d %02d:%02d:%02d.%03d] ",
		tSt.wYear, tSt.wMonth, tSt.wDay, (tSt.wHour + 9), tSt.wMinute, tSt.wSecond, tSt.wMilliseconds);

	// 可変引数を展開
	memset(m_szBuff, 0x00, sizeof(m_szBuff));
	va_list			ap;
	va_start(ap, format);
	vsprintf_s(m_szBuff, LOG_MAX_COL, format, ap);
	va_end(ap);

	// ログ出力リストに登録
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

		// ▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽
		dwRet = WaitForSingleObject(m_hMutex, (1000 * 5));
		switch (dwRet) {
		case (WAIT_OBJECT_0 + 0):		// ミューテックス獲得
			m_tLogOutputList.push_back(tLogOutput);
			ReleaseMutex(m_hMutex);
			break;

		default:
			break;
		}
		// ▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△
	}
}


//-----------------------------------------------------------------------------
// モジュールパスを取得する
//-----------------------------------------------------------------------------
void CLogger::GetModulePath(char* pszBuff, DWORD dwBuffSize)
{
	DWORD		dwRet = 0;
	char		szPath[MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE + 1];
	char		szDir[_MAX_DIR + 1];


	// パラメータチェック
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return;
	}

	// モジュールファイル名を取得
	memset(szPath, 0x00, sizeof(szPath));
	dwRet = GetModuleFileName(NULL, szPath, MAX_PATH);

	// パスを分割
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);

	// モジュールパスを生成
	memset(szPath, 0x00, sizeof(szPath));
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, NULL, NULL);

	strncpy_s(pszBuff, dwBuffSize, szPath, (dwBuffSize - 1));
}


//-----------------------------------------------------------------------------
// INIファイルパスを取得する
//-----------------------------------------------------------------------------
void CLogger::GetIniFilePath(char* pszBuff, DWORD dwBuffSize)
{
	DWORD		dwRet = 0;
	char		szPath[MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE + 1];
	char		szDir[_MAX_DIR + 1];
	char		szFname[_MAX_FNAME + 1];


	// パラメータチェック
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return;
	}

	// モジュールファイル名を取得
	memset(szPath, 0x00, sizeof(szPath));
	dwRet = GetModuleFileName(NULL, szPath, MAX_PATH);

	// パスを分割
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFname, _MAX_FNAME, NULL, 0);

	// INIファイルパスを生成
	memset(szPath, 0x00, sizeof(szPath));
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, szFname, ".ini");

	strncpy_s(pszBuff, dwBuffSize, szPath, (dwBuffSize - 1));
}


//-----------------------------------------------------------------------------
// LOGファイルパスを取得する
//-----------------------------------------------------------------------------
void CLogger::GetLogFilePath(char* pszBuff, DWORD dwBuffSize)
{
	DWORD		dwRet = 0;
	char		szPath[MAX_PATH + 1];
	char		szDrive[_MAX_DRIVE + 1];
	char		szDir[_MAX_DIR + 1];
	char		szFname[_MAX_FNAME + 1];


	// パラメータチェック
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return;
	}

	// モジュールファイル名を取得
	memset(szPath, 0x00, sizeof(szPath));
	dwRet = GetModuleFileName(NULL, szPath, MAX_PATH);

	// パスを分割
	_splitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFname, _MAX_FNAME, NULL, 0);

	// LOGファイルパスを生成
	memset(szPath, 0x00, sizeof(szPath));
	_makepath_s(szPath, MAX_PATH, szDrive, szDir, szFname, ".log");

	strncpy_s(pszBuff, dwBuffSize, szPath, (dwBuffSize - 1));
}


//-----------------------------------------------------------------------------
// ログ出力スレッド
//-----------------------------------------------------------------------------
unsigned CLogger::ThreadFunc(void* pUserData)
{
	CLogger				*pcThread = (CLogger*)pUserData;
	HANDLE				Events[] = { m_hEndReqEvent };
	DWORD				dwEventCount = sizeof(Events) / sizeof(HANDLE);
	BOOL				bLoop = TRUE;
	DWORD				dwRet = 0;
	BOOL				bRet = FALSE;


	// スレッド開始イベントを送信
	bRet = SetEvent(m_hStartEvent);
	if (bRet == TRUE)
	{
		// スレッド終了要求イベントが来るまでループ
		while (bLoop)
		{
			dwRet = WaitForMultipleObjects(dwEventCount, Events, FALSE, 1000);
			switch (dwRet) {
			case (WAIT_OBJECT_0 + 0):			// スレッド終了要求イベント
				bLoop = FALSE;
				break;

			case WAIT_TIMEOUT:					// タイムアウト（処理継続）

				// ▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽
				dwRet = WaitForSingleObject(m_hMutex, 200);

				switch (dwRet) {
				case (WAIT_OBJECT_0 + 0):		// ミューテックス獲得

					// ログ書込み処理
					LogWrite( FALSE );

					ReleaseMutex(m_hMutex);
					break;

				default:
					// 何もしない
					break;
				}
				break;
				// ▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△

			case WAIT_FAILED:
			default:
				bLoop = FALSE;
				break;
			}
		}
		ResetEvent(m_hEndReqEvent);
	}

	// ログ書込み処理（リストに登録されているものを全て書込み）
	LogWrite(TRUE);

	return 0;
}


//-----------------------------------------------------------------------------
// ログ出力処理
//-----------------------------------------------------------------------------
void CLogger::LogWrite( BOOL bEnd )
{
	char				szBuff[16];
	DWORD				dwPos = 0;
	DWORD				dwCount = 0;
	FILE				*pFile = NULL;
	errno_t				error;


	// ログ出力データがなければ処理を行わない
	if (m_tLogOutputList.size() == 0)
	{
		return;
	}

	// ファイルをオープンする
	error = fopen_s(&pFile, m_tLogSettingInfo.szFileName, "r+");
	if (error == ENOENT)
	{
		error = fopen_s(&pFile, m_tLogSettingInfo.szFileName, "w+");
	}
	if (error != 0)
	{
		return;
	}
	
	// ログ出力データがなくなるまでループ
	while (m_tLogOutputList.size() != 0)
	{
		// ログ出力の先頭データを取得
		std::list<LOG_OUTPUT_INFO_TABLE>::iterator			it = m_tLogOutputList.begin();

		// ファイル書込み位置を取得
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


		// ログを書込む際、ファイル最大サイズを超えているか調べる
		if ((dwPos + it->Length) > m_tLogSettingInfo.dwLogFileSize)
		{
			dwPos = 10;
		}

		// ログ書込み
		fseek(pFile, dwPos, SEEK_SET);
		fwrite(it->pszLog, (it->Length -1), 1, pFile);
		dwPos += (it->Length - 1);

		// ファイル位置書込み
		memset(szBuff, 0x00, sizeof(szBuff));
		sprintf_s(szBuff, 16, "%08d\n", dwPos);
		fseek(pFile, 0, SEEK_SET);
		fwrite(szBuff, 9, 1, pFile);

		// ログ出力リストの先頭ログ情報を削除
		if (it->pszLog != NULL)
		{
			free(it->pszLog);
		}
		m_tLogOutputList.pop_front();

		// 終了処理から呼ばれていない場合
		if (bEnd == FALSE)
		{
			dwCount++;

			if (dwCount >= LOG_OUTPUT_COUNT)
			{
				break;
			}
		}
	}

	// ファイルを閉じる
	fclose(pFile);
}


//-----------------------------------------------------------------------------
// INIファイル情報取得
//-----------------------------------------------------------------------------
void CLogger::GetIniFileInfo(LOG_SETTING_INFO_TABLE& tLogSettingInfo)
{
	// ログ出力パス
	GetPrivateProfileString("LOG", "PATH", "", tLogSettingInfo.szFileName, MAX_PATH, m_szIniFile);
	if (strlen(tLogSettingInfo.szFileName) == 0)
	{
		GetLogFilePath(tLogSettingInfo.szFileName, MAX_PATH);
	}

	// ログファイルサイズ
	tLogSettingInfo.dwLogFileSize = GetPrivateProfileInt("LOG", "SIZE", DEFAULT_LOG_FILE_SIZE, m_szIniFile);
	if (tLogSettingInfo.dwLogFileSize < MIN_LOG_FILE_SIZE)
	{
		tLogSettingInfo.dwLogFileSize = MIN_LOG_FILE_SIZE;
	}
	else if (tLogSettingInfo.dwLogFileSize > MAX_LOG_FILE_SIZE)
	{
		tLogSettingInfo.dwLogFileSize = MAX_LOG_FILE_SIZE;
	}

	// ログ出力種別
	tLogSettingInfo.dwLogOutputBit = GetPrivateProfileInt("LOG", "KIND", DEFAULT_LOG_OUTPUT, m_szIniFile);
	if (tLogSettingInfo.dwLogOutputBit == 0)
	{
		tLogSettingInfo.dwLogOutputBit = DEFAULT_LOG_OUTPUT;
	}
}
