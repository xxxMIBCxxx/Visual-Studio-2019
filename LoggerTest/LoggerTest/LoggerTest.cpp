// LoggerTest.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "CLogger.h"

int main()
{
	BOOL				bRet = FALSE;
	CLogger				cLog;


	bRet = cLog.Open();
	if (bRet == TRUE)
	{
		for (DWORD i = 0; i < 100; i++)
		{
			cLog.Output(LOG_OUTPUT_ERROR, "%04d - 0123456789ABCDEF",i);
			cLog.Output(LOG_OUTPUT_ERROR, "%04d - ABCDEFGHIJK",i);
			cLog.Output(LOG_OUTPUT_ERROR, "%04d - LMNOPQRSTUV",i);
			cLog.Output(LOG_OUTPUT_ERROR, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", i);
			cLog.Output(LOG_OUTPUT_ERROR, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB", i);

			Sleep(100);
		}

		cLog.Close();
	}

	return 0;
}

	