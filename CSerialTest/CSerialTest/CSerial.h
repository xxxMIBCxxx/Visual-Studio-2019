#pragma once
//*****************************************************************************
// CSerialクラス ヘッダーファイル
//*****************************************************************************
#include <Windows.h>
#include "CThread.h"
#include "list"

typedef enum
{
	SERIAL_RET_SUCCESS				= 0x00000000,				// 正常終了
	SERIAL_RET_ERROR_PARAM			= 0xE0000001,				// パラメータエラー
	SERIAL_RET_ERROR_NOT_OPEN		= 0xE0000002,				// COMポートをオープンしていない
	SERIAL_RET_ERROR_MUTEX			= 0xE0000003,				// ミューテックス取得エラー
	SERIAL_RET_ERROR_OPEN			= 0xE0000004,				// シリアルオープンエラー
	SERIAL_RET_ERROR_SETTING		= 0xE0000005,				// シリアル設定エラー
	SERIAL_RET_ERROR_RECV_THREAD	= 0xE0000006,				// 受信スレッドエラー
	SERIAL_RET_ERROR_MUTEX_TIMEOUT	= 0xE0000007,				// ミューテックスタイムアウト
	SERIAL_RET_ERROR_SEND			= 0xE0000008,				// 送信エラー

	SERIAL_RET_ERROR_SYSTEM			= 0xE9999999,				// システムエラー
} SERIAL_RET_ENUM;


// ボーレート種別
typedef enum
{
	BAUD_RATE_110	 = CBR_110,
	BAUD_RATE_300	 = CBR_300,
	BAUD_RATE_600	 = CBR_600,
	BAUD_RATE_1200	 = CBR_1200,
	BAUD_RATE_2400	 = CBR_2400,
	BAUD_RATE_4800	 = CBR_4800,
	BAUD_RATE_9600	 = CBR_9600,
	BAUD_RATE_14400  = CBR_14400,
	BAUD_RATE_19200  = CBR_19200,
	BAUD_RATE_38400  = CBR_38400,
	BAUD_RATE_57600  = CBR_57600,
	BAUD_RATE_115200 = CBR_115200,
	BAUD_RATE_128000 = CBR_128000,
	BAUD_RATE_256000 = CBR_256000,
} BAUD_RATE_ENUM;


// ストップビット種別
typedef enum
{
	STOP_BITS_10 = 0,
	STOP_BITS_15 = 1,
	STOP_BITS_20 = 2,
} STOP_BITS_ENUM;


// パリティチェック種別
typedef enum
{
	PARITY_CHECK_NOPARITY = 0,			// なし
	PARITY_CHECK_ODDPARITY = 1,			// 奇数
	PARITY_CHECK_EVENPARITY = 2,		// 偶数
	PARITY_CHECK_MARKPARITY = 3,		// 常に1 
	PARITY_CHECK_SPACEPARITY = 4,		// 常に0
} PARITY_CHECK_ENUM;


// COMポート情報構造体
typedef struct
{
	char									szComName[32 + 1];					// COM名
	char									szFriendlyName[MAX_PATH + 1];		// フレンドリー名

} COM_PORT_INFO_TABLE;


// COMポート設定情報構造体
typedef struct
{
	BAUD_RATE_ENUM							eBaudRate;				// ボーレート
	STOP_BITS_ENUM							eStopBits;				// ストップビット
	PARITY_CHECK_ENUM						eParityCheck;			// パリティチェック
	BYTE									ByteSize;				// バイトサイズ
} COM_PORT_SETTING_INFO_TABLE;



class CSerial : public CThread
{
//	std::list<COM_PORT_INFO_TABLE>			m_tComPortInfoList;




public:
	HANDLE									m_hCom;					// COMポートハンドル
	COM_PORT_INFO_TABLE						m_tComPortInfo;			// COMポート情報
	DCB										m_tDcb;					// COMポート制御情報

	HANDLE									m_hMutex;				// ミューテックスハンドル
	DWORD									m_dwRecvLength;			// 受信データの長さ
	char									*m_pRecvBuff;			// 受信データ

	DWORD									m_dwError;				// エラー情報

	SERIAL_RET_ENUM							m_eThreadRet;			// スレッド終了理由



	CSerial();
	~CSerial();
	SERIAL_RET_ENUM Open(COM_PORT_INFO_TABLE &tComPortInfo, COM_PORT_SETTING_INFO_TABLE &tComPortSettingInfo);
	void Close(void);
	SERIAL_RET_ENUM Send(const char* pszSendData, DWORD dwSendDataLength);


	BOOL IsRecv(void);
	SERIAL_RET_ENUM Recv(char* pszBuff, DWORD dwBuffSize);
	BOOL GetComPortInfo( std::list<COM_PORT_INFO_TABLE> &tComPortInfoList);


	unsigned ThreadFunc(void* pUserData);



};





