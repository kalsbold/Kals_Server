// Kals_Server.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Exception.h"
#include "Kals_Server.h"
#include "ClientSession.h"
#include "SessionManager.h"
#include "IOCPManager.h"


__declspec(thread) int l_ThreadType = -1;

int _tmain(int argc, _TCHAR* argv[])
{

	l_ThreadType = THREAD_MAIN_ACCEPT;
	SetUnhandledExceptionFilter(ExceptionFilter);

	G_SessionManager = new SessionManager;
	G_IocpManager = new IOCPManager;

	if (G_IocpManager->InitIOCPServer() == false)
	{
		return -1;
	}

	if (G_IocpManager->StartIOThread() == false)
	{
		return -1;
	}

	cout << "start IOCP Server" << endl;

	if (G_IocpManager->AcceptLoop() == false)
	{
		return -1;
	}

	G_IocpManager->CloseIOCP();

	cout << "End IOCP Server" << endl;

	delete G_IocpManager;
	delete G_SessionManager;

    return 0;
}

