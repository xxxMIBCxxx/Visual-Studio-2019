// CDiagComSetting.cpp : 実装ファイル
//

#include "pch.h"
#include "CSerialTest.h"
#include "CDiagComSetting.h"
#include "afxdialogex.h"


// CDiagComSetting ダイアログ

IMPLEMENT_DYNAMIC(CDiagComSetting, CDialogEx)

CDiagComSetting::CDiagComSetting(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIAG_COM_SETTING, pParent)
{

}

CDiagComSetting::~CDiagComSetting()
{
}

void CDiagComSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMB_PORT, m_CombPort);
	DDX_Control(pDX, IDC_COMB_BAUDRATE, m_CombBaudrate);
}


BEGIN_MESSAGE_MAP(CDiagComSetting, CDialogEx)
END_MESSAGE_MAP()


// CDiagComSetting メッセージ ハンドラー


BOOL CDiagComSetting::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	BOOL		bRet = FALSE;
	CString		strTemp;


	// 接続されているCOMポートを取得
	bRet = m_pcSerial->GetComPortInfo(m_tComPortInfoList);
	if (bRet == FALSE)
	{
		OnCancel();
		return TRUE;
	}

	// ポート名を設定
	std::list<COM_PORT_INFO_TABLE>::iterator		it = m_tComPortInfoList.begin();
	while (it != m_tComPortInfoList.end())
	{
		strTemp.Format("%s : %s", it->szComName, it->szFriendlyName);
		m_CombPort.AddString(strTemp);
		it++;
	}
	m_CombPort.SetCurSel(0);

	// ボーレート設定
	m_CombBaudrate.AddString("110");
	m_CombBaudrate.AddString("300");
	m_CombBaudrate.AddString("600");
	m_CombBaudrate.AddString("1200");
	m_CombBaudrate.AddString("2400");
	m_CombBaudrate.AddString("4800");
	m_CombBaudrate.AddString("9600");
	m_CombBaudrate.AddString("14400");
	m_CombBaudrate.AddString("19200");
	m_CombBaudrate.AddString("38400");
	m_CombBaudrate.AddString("56000");
	m_CombBaudrate.AddString("1152000");
	m_CombBaudrate.AddString("1280000");
	m_CombBaudrate.AddString("2560000");

	m_CombBaudrate.SetCurSel(5);












	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}
