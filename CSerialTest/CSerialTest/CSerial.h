#pragma once
//*****************************************************************************
// CSerial�N���X �w�b�_�[�t�@�C��
//*****************************************************************************
#include <Windows.h>
#include "CThread.h"
#include "list"

typedef enum
{
	SERIAL_RET_SUCCESS				= 0x00000000,				// ����I��
	SERIAL_RET_ERROR_PARAM			= 0xE0000001,				// �p�����[�^�G���[
	SERIAL_RET_ERROR_NOT_OPEN		= 0xE0000002,				// COM�|�[�g���I�[�v�����Ă��Ȃ�
	SERIAL_RET_ERROR_MUTEX			= 0xE0000003,				// �~���[�e�b�N�X�擾�G���[
	SERIAL_RET_ERROR_OPEN			= 0xE0000004,				// �V���A���I�[�v���G���[
	SERIAL_RET_ERROR_SETTING		= 0xE0000005,				// �V���A���ݒ�G���[
	SERIAL_RET_ERROR_RECV_THREAD	= 0xE0000006,				// ��M�X���b�h�G���[
	SERIAL_RET_ERROR_MUTEX_TIMEOUT	= 0xE0000007,				// �~���[�e�b�N�X�^�C���A�E�g
	SERIAL_RET_ERROR_SEND			= 0xE0000008,				// ���M�G���[

	SERIAL_RET_ERROR_SYSTEM			= 0xE9999999,				// �V�X�e���G���[
} SERIAL_RET_ENUM;


// �{�[���[�g���
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


// �X�g�b�v�r�b�g���
typedef enum
{
	STOP_BITS_10 = 0,
	STOP_BITS_15 = 1,
	STOP_BITS_20 = 2,
} STOP_BITS_ENUM;


// �p���e�B�`�F�b�N���
typedef enum
{
	PARITY_CHECK_NOPARITY = 0,			// �Ȃ�
	PARITY_CHECK_ODDPARITY = 1,			// �
	PARITY_CHECK_EVENPARITY = 2,		// ����
	PARITY_CHECK_MARKPARITY = 3,		// ���1 
	PARITY_CHECK_SPACEPARITY = 4,		// ���0
} PARITY_CHECK_ENUM;


// COM�|�[�g���\����
typedef struct
{
	char									szComName[32 + 1];					// COM��
	char									szFriendlyName[MAX_PATH + 1];		// �t�����h���[��

} COM_PORT_INFO_TABLE;


// COM�|�[�g�ݒ���\����
typedef struct
{
	BAUD_RATE_ENUM							eBaudRate;				// �{�[���[�g
	STOP_BITS_ENUM							eStopBits;				// �X�g�b�v�r�b�g
	PARITY_CHECK_ENUM						eParityCheck;			// �p���e�B�`�F�b�N
	BYTE									ByteSize;				// �o�C�g�T�C�Y
} COM_PORT_SETTING_INFO_TABLE;



class CSerial : public CThread
{
//	std::list<COM_PORT_INFO_TABLE>			m_tComPortInfoList;




public:
	HANDLE									m_hCom;					// COM�|�[�g�n���h��
	COM_PORT_INFO_TABLE						m_tComPortInfo;			// COM�|�[�g���
	DCB										m_tDcb;					// COM�|�[�g������

	HANDLE									m_hMutex;				// �~���[�e�b�N�X�n���h��
	DWORD									m_dwRecvLength;			// ��M�f�[�^�̒���
	char									*m_pRecvBuff;			// ��M�f�[�^

	DWORD									m_dwError;				// �G���[���

	SERIAL_RET_ENUM							m_eThreadRet;			// �X���b�h�I�����R



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





