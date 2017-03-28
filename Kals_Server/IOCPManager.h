#pragma once

class ClientSession;
struct OverlappedSock;

class IOCPManager
{
public:
	IOCPManager();
	~IOCPManager();

	//���� �ʱ�ȭ
	bool InitIOCPServer();
	//���� ����
	void CloseIOCP();

	//Thread ����.
	bool StartIOThread();
	//Ŭ���̾�Ʈ ���� ����.
	bool AcceptLoop();

	//Completion Port�� �Լ�.
	HANDLE GetCPHandle();
	//Thread ������ �Լ�.
	int GetThreadCount();

private:
	//Thread �Լ�.
	static unsigned int WINAPI WorkerThread(LPVOID lpParam);
	
	//�ۼ��� �� �Լ�.
	static bool ReceiveCompletion(ClientSession* client, OverlappedSock* ovsock, DWORD dwTransferred);
	static bool SendCompletion(ClientSession* client, OverlappedSock* ovsock, DWORD dwTransferred);

	//Completion Port �� �Լ�.
	HANDLE mCP;
	//Thread ����.
	int mThreadCount;

	SOCKET mListenSocket;

};

extern IOCPManager * GIocpManager;