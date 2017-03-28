#pragma once

class ClientSession;
struct OverlappedSock;

class IOCPManager
{
public:
	IOCPManager();
	~IOCPManager();

	//서버 초기화
	bool InitIOCPServer();
	//서버 종료
	void CloseIOCP();

	//Thread 시작.
	bool StartIOThread();
	//클라이언트 접속 과정.
	bool AcceptLoop();

	//Completion Port용 함수.
	HANDLE GetCPHandle();
	//Thread 개수용 함수.
	int GetThreadCount();

private:
	//Thread 함수.
	static unsigned int WINAPI WorkerThread(LPVOID lpParam);
	
	//송수신 용 함수.
	static bool ReceiveCompletion(ClientSession* client, OverlappedSock* ovsock, DWORD dwTransferred);
	static bool SendCompletion(ClientSession* client, OverlappedSock* ovsock, DWORD dwTransferred);

	//Completion Port 용 함수.
	HANDLE mCP;
	//Thread 개수.
	int mThreadCount;

	SOCKET mListenSocket;

};

extern IOCPManager * GIocpManager;