// CThreadTest.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//


#include "CThread.h"
#include <stdio.h>
#include <time.h>

class CThreadEx : public CThread
{
	//-----------------------------------------------------------------------------
	// スレッド処理
	//-----------------------------------------------------------------------------
	unsigned ThreadFunc(void* pUserData)
	{
		CThread* pcThread = (CThread*)pUserData;
		HANDLE				Events[] = { m_hEndReqEvent };
		DWORD				dwEventCount = sizeof(Events) / sizeof(HANDLE);
		BOOL				bLoop = TRUE;
		DWORD				dwRet = 0;
		BOOL				bRet = FALSE;
		time_t				t;
		struct tm			tTm;


		// スレッド開始イベントを送信
		bRet = SetEvent(m_hStartEvent);
		if (bRet == TRUE)
		{
			// スレッド終了要求イベントが来るまでループ
			while (bLoop)
			{
				dwRet = WaitForMultipleObjects(dwEventCount, Events, FALSE, 500);
				switch (dwRet) {
				case (WAIT_OBJECT_0 + 0):			// スレッド終了要求イベント
					bLoop = FALSE;
					break;

				case WAIT_TIMEOUT:
					time(&t);
					localtime_s(&tTm, &t);
					printf("[%04d/%02d/%02d %02d:%02d:%02d] Execute ThreadEx Sample... \n", 
											(tTm.tm_year + 1900), tTm.tm_mon, tTm.tm_mday, tTm.tm_hour, tTm.tm_min, tTm.tm_sec);
					break;

				case WAIT_FAILED:
				default:
					bLoop = FALSE;
					break;
				}
			}
			ResetEvent(m_hEndReqEvent);
		}

		return 0;
	}
};



int main()
{

	CThread			a;
	CThreadEx		c;
	DWORD			i = 0;
	BOOL			bRet = FALSE;

	a.Start();
	bRet = c.Start();
	if (bRet == TRUE)
	{
		for (i = 0; i < 10; i++)
		{
			Sleep(1000);
		}

		bRet = c.End();
	}
	a.End();

	return 0;
   
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
