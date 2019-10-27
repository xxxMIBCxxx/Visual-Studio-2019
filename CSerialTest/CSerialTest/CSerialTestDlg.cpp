
// CSerialTestDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "CSerialTest.h"
#include "CSerialTestDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCSerialTestDlg ダイアログ



CCSerialTestDlg::CCSerialTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CSERIALTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCSerialTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RECV, m_EditRecv);
	DDX_Control(pDX, IDC_EDIT_SEND, m_EditSend);
	DDX_Control(pDX, IDC_BTN_CLEAR, m_BtnClear);
	DDX_Control(pDX, IDC_BTN_SEND, m_BtnSend);
	DDX_Control(pDX, IDC_STATIC_RECV, m_StaticRecv);
	DDX_Control(pDX, IDC_STATIC_SEND, m_StaticSend);
}

BEGIN_MESSAGE_MAP(CCSerialTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SEND, &CCSerialTestDlg::OnBnClickedBtnSend)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CCSerialTestDlg::OnBnClickedBtnClear)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCSerialTestDlg メッセージ ハンドラー

BOOL CCSerialTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。


	BOOL								bRet = FALSE;
	COM_PORT_SETTING_INFO_TABLE			tComPortSettingInfo;



	



	//m_cFont.CreatePointFont(160, "MS ゴシック");
	//m_EditRecv.SetFont(&m_cFont);
	//m_EditSend.SetFont(&m_cFont);
	//m_BtnClear.SetFont(&m_cFont);
	//m_BtnSend.SetFont(&m_cFont);
	//m_StaticRecv.SetFont(&m_cFont);
	//m_StaticSend.SetFont(&m_cFont);


	m_cDiagComSetting.m_pcSerial = &m_cSerial;
	m_cDiagComSetting.DoModal();

	tComPortSettingInfo.ByteSize = 8;
	tComPortSettingInfo.eBaudRate = BAUD_RATE_38400;
	tComPortSettingInfo.eStopBits = STOP_BITS_10;
	tComPortSettingInfo.eParityCheck = PARITY_CHECK_NOPARITY;
//	bRet = m_cSerial.Open(tComPortInfoList.front(), tComPortSettingInfo);






	// タイマーを発行
	m_TimerID = SetTimer(1, 500, NULL);


	m_strSendData = "";
	m_strRecvData = "";


	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CCSerialTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CCSerialTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




//-----------------------------------------------------------------------------
// [送信]ボタン押下時の処理
//-----------------------------------------------------------------------------
void CCSerialTestDlg::OnBnClickedBtnSend()
{
	SERIAL_RET_ENUM			eRet = SERIAL_RET_SUCCESS;
	CString					strMsg;


	m_EditSend.GetWindowText(m_strSendData);


	// 送信
	eRet = m_cSerial.Send((const char*)m_strSendData, m_strSendData.GetLength());
	if (eRet != SERIAL_RET_SUCCESS)
	{
		strMsg.Format("送信に失敗しました[eRet：0x%08X]", eRet);
		MessageBox(strMsg, "エラー", (MB_OK | MB_ICONERROR));
	}
}


//-----------------------------------------------------------------------------
// タイマー
//-----------------------------------------------------------------------------
void CCSerialTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	SERIAL_RET_ENUM			eRet = SERIAL_RET_SUCCESS;
	BOOL					bRet = FALSE;
	char					szRecv[1024 + 1];


	// 受信データあり？
	bRet = m_cSerial.IsRecv();
	if (bRet == TRUE)
	{
		memset(szRecv, 0x00, sizeof(szRecv));
		//eRet = m_cSerial.Recv(szRecv, 1024);
		eRet = m_cSerial.Recv(szRecv, 2);
		if (eRet == SERIAL_RET_SUCCESS)
		{
			m_strRecvData += szRecv;
			m_EditRecv.SetWindowText(m_strRecvData);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


//-----------------------------------------------------------------------------
// [受信データクリア]ボタン押下時の処理
//-----------------------------------------------------------------------------
void CCSerialTestDlg::OnBnClickedBtnClear()
{
	m_strRecvData = "";
	m_EditRecv.SetWindowText(m_strRecvData);
}


void CCSerialTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();


	// タイマーを止める
	if (m_TimerID != 0)
	{
		KillTimer(m_TimerID);
		m_TimerID = 0;
	}

	// COMポートを閉じる
	m_cSerial.Close();
}
