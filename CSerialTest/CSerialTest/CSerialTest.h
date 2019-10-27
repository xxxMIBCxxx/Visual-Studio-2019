
// CSerialTest.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'pch.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル


// CCSerialTestApp:
// このクラスの実装については、CSerialTest.cpp を参照してください
//

class CCSerialTestApp : public CWinApp
{
public:
	CCSerialTestApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CCSerialTestApp theApp;
