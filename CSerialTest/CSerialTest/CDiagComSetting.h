#pragma once
#include "CSerial.h"

// CDiagComSetting ダイアログ

class CDiagComSetting : public CDialogEx
{
	DECLARE_DYNAMIC(CDiagComSetting)

public:
	CDiagComSetting(CWnd* pParent = nullptr);   // 標準コンストラクター
	virtual ~CDiagComSetting();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIAG_COM_SETTING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	CSerial								*m_pcSerial;
	std::list<COM_PORT_INFO_TABLE>		m_tComPortInfoList;


	CComboBox m_CombPort;
	virtual BOOL OnInitDialog();
	CComboBox m_CombBaudrate;
};
