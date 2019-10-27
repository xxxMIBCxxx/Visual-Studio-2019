
// CSerialTestDlg.h : ヘッダー ファイル
//

#pragma once


#include "CSerial.h"
#include "CDiagComSetting.h"


// CCSerialTestDlg ダイアログ
class CCSerialTestDlg : public CDialogEx
{
// コンストラクション
public:
	CCSerialTestDlg(CWnd* pParent = nullptr);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CSERIALTEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	CDiagComSetting						m_cDiagComSetting;
	CSerial								m_cSerial;

	CString								m_strSendData;
	CString								m_strRecvData;
	UINT_PTR							m_TimerID;

	//CFont								m_cFont;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditRecv;
	CEdit m_EditSend;
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnDestroy();
	CButton m_BtnClear;
	CButton m_BtnSend;
	CStatic m_StaticRecv;
	CStatic m_StaticSend;
};
