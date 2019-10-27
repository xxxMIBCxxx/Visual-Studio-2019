#pragma once
//=============================================================================
// CLoggerクラス　ヘッダーファイル
//=============================================================================
#include <Windows.h>
#include "CThread.h"
#include "list"


#define LOG_MAX_COL				( 1024 )		// ログファイル１行の最大文字数


// ログ種別
typedef enum
{
	LOG_OUTPUT_ERROR		= 0x80000000,		// エラーログ
	LOG_OUTPUT_ERROR_02		= 0x40000000,		// エラーログ02（予備）
	LOG_OUTPUT_ERROR_03		= 0x20000000,		// エラーログ03（予備）
	LOG_OUTPUT_ERROR_04		= 0x10000000,		// エラーログ04（予備）
	LOG_OUTPUT_WARNING		= 0x08000000,		// 警告ログ
	LOG_OUTPUT_WARNING_02	= 0x04000000,		// 警告ログ02（予備）
	LOG_OUTPUT_WARNING_03	= 0x02000000,		// 警告ログ03（予備）
	LOG_OUTPUT_WARNING_04	= 0x01000000,		// 警告ログ04（予備）
	LOG_OUTPUT_TRACE		= 0x00800000,		// トレース
	LOG_OUTPUT_TRACE_02		= 0x00400000,		// トレース02
	LOG_OUTPUT_TRACE_03		= 0x00200000,		// トレース03
	LOG_OUTPUT_TRACE_04		= 0x00100000,		// トレース04
	LOG_OUTPUT_DEBUG		= 0x00008000,		// デバッグ
	LOG_OUTPUT_DEBUG_02		= 0x00004000,		// デバッグ02
	LOG_OUTPUT_DEBUG_03		= 0x00002000,		// デバッグ03
	LOG_OUTPUT_DEBUG_04		= 0x00001000,		// デバッグ04
	LOG_OUTPUT_DEBUG_05		= 0x00000800,		// デバッグ05
	LOG_OUTPUT_DEBUG_06		= 0x00000400,		// デバッグ06
	LOG_OUTPUT_DEBUG_07		= 0x00000200,		// デバッグ07
	LOG_OUTPUT_DEBUG_08		= 0x00000100,		// デバッグ08
	LOG_OUTPUT_DEBUG_09		= 0x00000080,		// デバッグ09
	LOG_OUTPUT_DEBUG_10		= 0x00000040,		// デバッグ10
	LOG_OUTPUT_DEBUG_11		= 0x00000020,		// デバッグ11
	LOG_OUTPUT_DEBUG_12		= 0x00000010,		// デバッグ12
	LOG_OUTPUT_DEBUG_13		= 0x00000008,		// デバッグ13
	LOG_OUTPUT_DEBUG_14		= 0x00000002,		// デバッグ14
	LOG_OUTPUT_DEBUG_15		= 0x00000001,		// デバッグ15
} LOG_OUTPUT_ENUM;




// ログ設定情報構造体
typedef struct
{
	DWORD					dwLogFileSize;				// ログファイルのサイズ
	DWORD					dwLogOutputBit;				// ログ出力判定用ビット
	char					szFileName[MAX_PATH + 1];	// ログファイル名
} LOG_SETTING_INFO_TABLE;


// ログ出力情報テーブル
typedef struct
{
	size_t					Length;						// ログの長さ
	char					*pszLog;					// ログ
} LOG_OUTPUT_INFO_TABLE;





class CLogger : public CThread
{
public:
	BOOL										m_bOpenFlag;

	
	LOG_SETTING_INFO_TABLE						m_tLogSettingInfo;			// 設定情報

	HANDLE										m_hMutex;					// ミューテックスハンドル
	DWORD										m_dwError;
	std::list<LOG_OUTPUT_INFO_TABLE>			m_tLogOutputList;			// ログ出力リスト
	char										m_szBuff[LOG_MAX_COL + 1];	// ログ出力作業用バッファ
	char										m_szIniFile[MAX_PATH + 1];	// INIファイルパス


	CLogger();
	~CLogger();
	BOOL Open( void );
	BOOL Close( void );
	void Output(LOG_OUTPUT_ENUM eLog, const char* format, ...);
	static void GetModulePath(char* pszBuff, DWORD dwBuffSize);
	static void GetIniFilePath(char* pszBuff, DWORD dwBuffSize);
	static void GetLogFilePath(char* pszBuff, DWORD dwBuffSize);

private:
	unsigned ThreadFunc(void* pUserData);
	void LogWrite(BOOL bEnd);
	void GetIniFileInfo(LOG_SETTING_INFO_TABLE& tLogSettingInfo);

};












