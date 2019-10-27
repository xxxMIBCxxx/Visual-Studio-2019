#pragma once
//=============================================================================
// CLogger�N���X�@�w�b�_�[�t�@�C��
//=============================================================================
#include <Windows.h>
#include "CThread.h"
#include "list"


#define LOG_MAX_COL				( 1024 )		// ���O�t�@�C���P�s�̍ő啶����


// ���O���
typedef enum
{
	LOG_OUTPUT_ERROR		= 0x80000000,		// �G���[���O
	LOG_OUTPUT_ERROR_02		= 0x40000000,		// �G���[���O02�i�\���j
	LOG_OUTPUT_ERROR_03		= 0x20000000,		// �G���[���O03�i�\���j
	LOG_OUTPUT_ERROR_04		= 0x10000000,		// �G���[���O04�i�\���j
	LOG_OUTPUT_WARNING		= 0x08000000,		// �x�����O
	LOG_OUTPUT_WARNING_02	= 0x04000000,		// �x�����O02�i�\���j
	LOG_OUTPUT_WARNING_03	= 0x02000000,		// �x�����O03�i�\���j
	LOG_OUTPUT_WARNING_04	= 0x01000000,		// �x�����O04�i�\���j
	LOG_OUTPUT_TRACE		= 0x00800000,		// �g���[�X
	LOG_OUTPUT_TRACE_02		= 0x00400000,		// �g���[�X02
	LOG_OUTPUT_TRACE_03		= 0x00200000,		// �g���[�X03
	LOG_OUTPUT_TRACE_04		= 0x00100000,		// �g���[�X04
	LOG_OUTPUT_DEBUG		= 0x00008000,		// �f�o�b�O
	LOG_OUTPUT_DEBUG_02		= 0x00004000,		// �f�o�b�O02
	LOG_OUTPUT_DEBUG_03		= 0x00002000,		// �f�o�b�O03
	LOG_OUTPUT_DEBUG_04		= 0x00001000,		// �f�o�b�O04
	LOG_OUTPUT_DEBUG_05		= 0x00000800,		// �f�o�b�O05
	LOG_OUTPUT_DEBUG_06		= 0x00000400,		// �f�o�b�O06
	LOG_OUTPUT_DEBUG_07		= 0x00000200,		// �f�o�b�O07
	LOG_OUTPUT_DEBUG_08		= 0x00000100,		// �f�o�b�O08
	LOG_OUTPUT_DEBUG_09		= 0x00000080,		// �f�o�b�O09
	LOG_OUTPUT_DEBUG_10		= 0x00000040,		// �f�o�b�O10
	LOG_OUTPUT_DEBUG_11		= 0x00000020,		// �f�o�b�O11
	LOG_OUTPUT_DEBUG_12		= 0x00000010,		// �f�o�b�O12
	LOG_OUTPUT_DEBUG_13		= 0x00000008,		// �f�o�b�O13
	LOG_OUTPUT_DEBUG_14		= 0x00000002,		// �f�o�b�O14
	LOG_OUTPUT_DEBUG_15		= 0x00000001,		// �f�o�b�O15
} LOG_OUTPUT_ENUM;




// ���O�ݒ���\����
typedef struct
{
	DWORD					dwLogFileSize;				// ���O�t�@�C���̃T�C�Y
	DWORD					dwLogOutputBit;				// ���O�o�͔���p�r�b�g
	char					szFileName[MAX_PATH + 1];	// ���O�t�@�C����
} LOG_SETTING_INFO_TABLE;


// ���O�o�͏��e�[�u��
typedef struct
{
	size_t					Length;						// ���O�̒���
	char					*pszLog;					// ���O
} LOG_OUTPUT_INFO_TABLE;





class CLogger : public CThread
{
public:
	BOOL										m_bOpenFlag;

	
	LOG_SETTING_INFO_TABLE						m_tLogSettingInfo;			// �ݒ���

	HANDLE										m_hMutex;					// �~���[�e�b�N�X�n���h��
	DWORD										m_dwError;
	std::list<LOG_OUTPUT_INFO_TABLE>			m_tLogOutputList;			// ���O�o�̓��X�g
	char										m_szBuff[LOG_MAX_COL + 1];	// ���O�o�͍�Ɨp�o�b�t�@
	char										m_szIniFile[MAX_PATH + 1];	// INI�t�@�C���p�X


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












