//*****************************************************************************
// CSerialクラス ソースファイル
//*****************************************************************************
#include "CSerial.h"
#include <setupapi.h>

#pragma comment( lib, "Setupapi.lib" )

#define	THREAD_LOOP_TIMEOUT						( 100 )						// 受信スレッドの待ち時間(ms)
#define RECV_BUFF_SIZE							( 1024 )					// 受信バッファサイズ


//-----------------------------------------------------------------------------
// コンストラクタ
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
// デストラクタ
//-----------------------------------------------------------------------------
CSerial::~CSerial()
{
	// COMポートが閉じられていない場合を考慮
	Close();
}


//-----------------------------------------------------------------------------
// シリアルポートオープン
//-----------------------------------------------------------------------------
SERIAL_RET_ENUM CSerial::Open(COM_PORT_INFO_TABLE& tComPortInfo, COM_PORT_SETTING_INFO_TABLE& tComPortSettingInfo)
{
	BOOL			bRet = FALSE;
	

	// 既にオープンしている場合
	if (m_hCom != INVALID_HANDLE_VALUE)
	{
		return SERIAL_RET_SUCCESS;
	}

	// ミューテックスハンドルを取得
	m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if (m_hMutex == NULL)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_MUTEX;
	}


	// シリアルポートをオープン
	m_hCom = CreateFile(tComPortInfo.szComName, (GENERIC_READ | GENERIC_WRITE), 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_OPEN;
	}
	memcpy(&m_tComPortInfo, &tComPortInfo, sizeof(COM_PORT_INFO_TABLE));

	// COMポートのデバイス制御ブロック情報を取得
	bRet = GetCommState(m_hCom, &m_tDcb);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_SETTING;
	}

	// COMポートのデバイス制御ブロック情報を設定
	DCB					tDcb;
	memcpy(&tDcb, &m_tDcb, sizeof(DCB));
	tDcb.BaudRate = (DWORD)tComPortSettingInfo.eBaudRate;		// ボー レート
	tDcb.fBinary = TRUE;										// バイナリ モード：有効(常にTRUE固定）
//	tDcb.fOutxCtsFlow = TRUE;									// CTS(送信クリア)信号の出力フロー監視監視
//	tDcb.fOutxDsrFlow = TRUE;									// DSR(データ・セット対応)信号は出力フロー制御監視
//	tDcb.fDtrControl = DTR_CONTROL_DISABLE;						// DTR(データ端末対応)フロー制御
//	tDcb.fDsrSensitivity = FALSE;								// DSR 信号の状態に敏感(DSR モデム入力行が高い場合を除き、ドライバーは受信したバイトを無視)
//	tDcb.fTXContinueOnXoff = TRUE;								// ドライバーがXoffChar文字を送信したあとの動作
//	tDcb.fOutX = FALSE;											// 伝送中の XON/XOFFフロー制御(TRUEの場合はXoffChar文字の受信時に送信が停止し、XonChar文字の受信時に再び開始される)
//	tDcb.fInX = FALSE;											// 受信中に XON/XOFFフロー制御(TRUEの場合は入力バッファーが完全なXoffLimバイト内に入るとXoffChar文字が送信され、入力バッファーがXonLimバイト内に入るとXonChar文字が送信されます)
//	tDcb.fNull = TRUE;											// null バイトは受信時に破棄される
//	tDcb.fRtsControl = RTS_CONTROL_DISABLE;						// RTS(送信要求) フロー制御
//	tDcb.fAbortOnError = FALSE;									// ドライバーはエラーが発生した場合にエラーステータスですべての読み取りおよび書き込み操作を終了するかの設定
//	tDcb.XonLim = 0;											// フロー制御がアクティブ化される前に入力バッファーで許可されている使用バイト数の最小数
//	tDcb.XoffLim = 0;											// 送信者を禁止するためにフロー制御がアクティブ化される前に、入力バッファーで許可される空きバイトの最小数
	tDcb.ByteSize = tComPortSettingInfo.ByteSize;				// 送受信されたバイト内のビット数
	tDcb.fParity = TRUE;										// パリティチェックの有無
	tDcb.Parity = (BYTE)tComPortSettingInfo.eParityCheck;		// パリティチェックの種別
	tDcb.StopBits = (BYTE)tComPortSettingInfo.eStopBits;		// ストップビット
//	tDcb.XonChar = '';											// 送信と受信の両方の XON 文字の値
//	tDcb.XoffChar = '';											// 送信と受信の両方の XOFF 文字の値
//	tDcb.fErrorChar = FALSE;									// パリティ エラーで受信したバイトを、ErrorCharメンバーで指定された文字に置き換えられるかどうかを示します
//	tDcb.ErrorChar = '';										// 受け取ったバイトをパリティ エラーで置き換えるために使用される文字の値
//	tDcb.EofChar = '';											// データの終わりを通知するために使用される文字の値
//  tDcb.EvtChar = '';											// イベントのシグナルを送信するために使用される文字の値
	bRet = SetCommState(m_hCom, &tDcb);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		Close();
		return SERIAL_RET_ERROR_SETTING;
	}
	memcpy(&m_tDcb, &tDcb, sizeof(DCB));
	
	// 受信スレッドを開始させる
	bRet = Start();
	if (bRet == FALSE)
	{
		Close();
		return SERIAL_RET_ERROR_RECV_THREAD;
	}

	return SERIAL_RET_SUCCESS;
}


//-----------------------------------------------------------------------------
// シリアルポートクローズ
//-----------------------------------------------------------------------------
void CSerial::Close(void)
{
	DWORD			dwRet = 0;

	// 既に閉じられている場合
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// ▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽
	dwRet = WaitForSingleObject(m_hMutex, (1000 * 5));

	// 受信スレッドを終了させる
	End();

	// 受信バッファクリア
	if (m_pRecvBuff != NULL)
	{
		free(m_pRecvBuff);
		m_pRecvBuff = NULL;
	}
	m_dwRecvLength = 0;

	// COMポートを閉じる
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
	// ▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△

	// ミューテックスハンドルを閉じる
	if (m_hMutex != NULL)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}


//-----------------------------------------------------------------------------
// シリアルポート情報を取得
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



	// リストを初期化
	tComPortInfoList.clear();


	// PORTSクラスのGUIDを取得
	bRet = SetupDiClassGuidsFromName("PORTS", &cGuid, 1, &dwRequiredSize);
	if (bRet == FALSE)
	{
		m_dwError = GetLastError();
		return FALSE;
	}

	// GUIDが示すクラスのデバイス情報ハンドルを取得
	hDevInfo = SetupDiGetClassDevs(&cGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_PROFILE));
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		m_dwError = GetLastError();
		return FALSE;
	}


	// デバイス情報が無くなるまでループ
	dwMemberIndex = 0;
	while (1)
	{
		// デバイス情報を取得
		memset(&tDevInfoData, 0x00, sizeof(tDevInfoData));
		memset(&tComPortInfo, 0x00, sizeof(tComPortInfo));
		tDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		bRet = SetupDiEnumDeviceInfo(hDevInfo, dwMemberIndex, &tDevInfoData);
		if (bRet == FALSE)
		{
			// エラー内容が「データはこれ以上ありません」の場合は、エラーとしない
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

		// デバイス情報のフレンドリー名を取得
		bRet = SetupDiGetDeviceRegistryProperty(hDevInfo, &tDevInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)tComPortInfo.szFriendlyName, MAX_PATH, NULL);
		if (bRet == FALSE)
		{
			m_dwError = GetLastError();
			bResult = FALSE;
			break;
		}

		// デバイスのレジストリキーオープン
		hKey = SetupDiOpenDevRegKey(hDevInfo, &tDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
		if (hKey != INVALID_HANDLE_VALUE)
		{
			// レジストリ情報に"PortName"がある？
			dwRequiredSize = 32;
			lRet = RegQueryValueEx(hKey, "PortName", 0, NULL, (PBYTE)tComPortInfo.szComName, &dwRequiredSize);
			if (lRet == ERROR_SUCCESS)
			{
				// シリアルポートなのか調べる
				if (_strnicmp(tComPortInfo.szComName, "COM", 3) == 0)
				{
					// リストに登録
					tComPortInfoList.push_back(tComPortInfo);
				}
			}

			// レジストリキーを閉じる
			RegCloseKey(hKey);
		}
		else
		{
			m_dwError = GetLastError();
			bResult = FALSE;
			break;
		}

		// 次のデバイス情報へ
		dwMemberIndex++;
	}

	// デバイス情報ハンドルを解放
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return bResult;
}


//-----------------------------------------------------------------------------
// データ送信
//-----------------------------------------------------------------------------
SERIAL_RET_ENUM CSerial::Send(const char *pszSendData, DWORD dwSendDataLength )
{
	BOOL				bRet = FALSE;
	DWORD				dwWriteLength = 0;


	// COMポートをオープンしていない場合
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return SERIAL_RET_ERROR_NOT_OPEN;
	}

	// パラメータチェック
	if ((pszSendData == NULL) || (dwSendDataLength == 0))
	{
		return SERIAL_RET_ERROR_PARAM;
	}
	
	// 送信データを全て送信するまでループ
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
// 受信データ有無チェック
//-----------------------------------------------------------------------------
BOOL CSerial::IsRecv(void)
{
	return ((m_dwRecvLength != 0) ? TRUE : FALSE);
}


//-----------------------------------------------------------------------------
// 受信データ取得
//-----------------------------------------------------------------------------
SERIAL_RET_ENUM CSerial::Recv(char* pszBuff, DWORD dwBuffSize)
{
	SERIAL_RET_ENUM			eRet = SERIAL_RET_SUCCESS;
	DWORD					dwRet = 0;


	// COMポートをオープンしていない場合
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		return SERIAL_RET_ERROR_NOT_OPEN;
	}

	// パラメータチェック
	if ((pszBuff == NULL) || (dwBuffSize == 0))
	{
		return SERIAL_RET_ERROR_PARAM;
	}

	// ▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽
	dwRet = WaitForSingleObject(m_hMutex, 5000);
	switch (dwRet) {
	case (WAIT_OBJECT_0 + 0):			// スレッド終了要求イベント
		
		// 受信データがない場合
		if (m_dwRecvLength == 0)
		{
		}
		else
		{
			// 保持している受信データをバッファサイズへコピー
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

		// ミューテックス解放
		ReleaseMutex(m_hMutex);
		break;

	case WAIT_TIMEOUT:					// タイムアウト
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
// 受信スレッド
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


	// スレッド開始イベントを送信
	m_eThreadRet = SERIAL_RET_SUCCESS;
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

				// ▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽▼▽
				dwRet = WaitForSingleObject(m_hMutex, 100);

				switch (dwRet) {
				case (WAIT_OBJECT_0 + 0):		// ミューテックス獲得

					// 受信データを取得
					dwRecvSize = 0;
					memset(szRecvBuff, 0x00, sizeof(szRecvBuff));
					bRet = ReadFile(m_hCom, szRecvBuff, RECV_BUFF_SIZE, &dwRecvSize, NULL);
					if (bRet == TRUE)
					{
						// 受信データがある場合
						if (dwRecvSize > 0)
						{
							// 新しい領域を作成
							char* pTemp = (char*)malloc(m_dwRecvLength + dwRecvSize + 1);
							if (pTemp == NULL)
							{
								// 異常終了にする
								m_eThreadRet = SERIAL_RET_ERROR_SYSTEM;
								bLoop = FALSE;
							}
							else
							{
								// 保持していた受信データがある場合
								if ((m_dwRecvLength != 0) && (m_pRecvBuff != NULL))
								{
									// 保持していた受信データをコピー
									memcpy(pTemp, m_pRecvBuff, m_dwRecvLength);
								}

								// 続けて、今回受信したデータをコピー
								memcpy(&pTemp[m_dwRecvLength], szRecvBuff, dwRecvSize);
								pTemp[(m_dwRecvLength + dwRecvSize)] = '\0';

								// 保持していた領域を解放
								if (m_pRecvBuff != NULL)
								{
									free(m_pRecvBuff);
								}

								m_pRecvBuff = pTemp;
								m_dwRecvLength += dwRecvSize;
							}
						}
					}

					// ミューテックス解放
					ReleaseMutex(m_hMutex);
					break;

				case WAIT_TIMEOUT:					// タイムアウト
					// 何も行わない
					break;

				case WAIT_FAILED:
				default:
					m_eThreadRet = SERIAL_RET_ERROR_SYSTEM;
					bLoop = FALSE;
					break;
				}
				// ▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△▲△
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
