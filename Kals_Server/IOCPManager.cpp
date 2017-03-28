#include "stdafx.h"
#include "IOCPManager.h"
#include "ClientSession.h"
#include "SessionManager.h"

#define GQCS_TIMEOUT	200

IOCPManager* GIocpManager = nullptr;

IOCPManager::IOCPManager() : mCP(NULL), mThreadCount(2), mListenSocket(NULL)
{
}


IOCPManager::~IOCPManager()
{
}

bool IOCPManager::InitIOCPServer()
{
	//�ý��� ������ Ȯ��.
	SYSTEM_INFO sysinfo;
	SecureZeroMemory(&sysinfo, sizeof(sysinfo));
	GetSystemInfo(&sysinfo);

	//CPU �ھ� ������ 2��� Thread ���� ����.
	mThreadCount = sysinfo.dwNumberOfProcessors * 2;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	//I/O �� Completion Port�� ����.
	mCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,	0,	0	);

	if (mCP == NULL)
	{
		cout << "Completion Port Create Fail..." << endl;
		return false;
	}

	//�񵿱� �ɼǿ� ���� ����.
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (mListenSocket == NULL)
	{
		cout << "Socket Create Fail..." << endl;
		return false;
	}

	SOCKADDR_IN serveraddr;
	SecureZeroMemory(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);


	int opt = 1;
	//��Ʈ,IP ���� �ɼ� ����.
	setsockopt(mListenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));

	if (SOCKET_ERROR == bind(mListenSocket, (SOCKADDR*)&serveraddr, sizeof(serveraddr)))
	{
		cout << "bind() Fail..." << endl;
		return false;
	}
		

	cout << "IOCP Server Init Succese..." << endl;

	return true;
}
void IOCPManager::CloseIOCP()
{
	CloseHandle(mCP);

	/// winsock finalizing
	WSACleanup();
}

bool IOCPManager::StartIOThread()
{
	for (int i = 0; i < mThreadCount; ++i)
	{
		DWORD dwThreadId;
		//Thread ����.
		HANDLE threadHandle = (HANDLE)_beginthreadex( NULL,	0, WorkerThread,(LPVOID)i,	0, (unsigned int*)&dwThreadId);

		if (threadHandle == INVALID_HANDLE_VALUE)
		{
			cout << "Create Thread Fail... " << endl;
			return false;
		}
		CloseHandle(threadHandle);
	}

	return true;
}
bool IOCPManager::AcceptLoop()
{
	if (SOCKET_ERROR == listen(mListenSocket, SOMAXCONN))
	{
		cout << "listen() Fail..." << endl;
		return false;
	}
	while(true)
	{
		SOCKET acceptedSock = accept(mListenSocket, NULL, NULL);
		if (acceptedSock == INVALID_SOCKET)
		{
			printf_s("accept: invalid socket\n");
			continue;
		}

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(acceptedSock, (SOCKADDR*)&clientaddr, &addrlen);

		/// ���� ���� ����ü �Ҵ�� �ʱ�ȭ
		ClientSession* client = GSessionManager->CreateClient(acceptedSock);

		/// Ŭ���̾�Ʈ ���� ó��
		if (false == client->OnConnect(&clientaddr))
		{
			client->Disconnect();
			GSessionManager->DeleteClient(client);
		}
	}

	return true;
		
}

HANDLE IOCPManager::GetCPHandle()
{
	return mCP;
}

int IOCPManager::GetThreadCount()
{
	return mThreadCount;
}


unsigned int WINAPI IOCPManager::WorkerThread(LPVOID lpParam)
{
	HANDLE hCompletionPort = GIocpManager->GetCPHandle();

	while (true)
	{
		DWORD dwTransferred;
		OverlappedSock* OvSock = nullptr;
		ClientSession* CpClient = nullptr;
		//Completion Port�� ����� �Ϸ���¸� Ȯ��.
		int ret = GetQueuedCompletionStatus(hCompletionPort, &dwTransferred, (PULONG_PTR)&CpClient, 
			(LPOVERLAPPED*)&OvSock, -1);

		if (ret == 0 && GetLastError() == WAIT_TIMEOUT)
		{
			cout << "Thread Time Out" << endl;
			continue;
		}

		if (ret == 0 || dwTransferred == 0)
		{
			cout << "Recive Data is zero..." << endl;
			CpClient->Disconnect();
			GSessionManager->DeleteClient(CpClient);
			continue;
		}

		if (OvSock == nullptr)
		{
			cout << "Recive Data is zero...Overlapped error" << endl;
			CpClient->Disconnect();
			GSessionManager->DeleteClient(CpClient);
		}

		//����� Ÿ�Կ� ���� 
		bool completOK = true;
		switch (OvSock->mIoType)
		{
		case IO_SEND:
			completOK = SendCompletion(CpClient, OvSock, dwTransferred);
			break;

		case IO_RECV:
			completOK = ReceiveCompletion(CpClient, OvSock, dwTransferred);
			break;
		default:
			cout << "IO type Fail : " << OvSock->mIoType << endl;
			break;
		}

		if (!completOK)
		{
			cout << "Completion Error..." << endl;
			CpClient->Disconnect();
			GSessionManager->DeleteClient(CpClient);
		}
	}
	return 0;
}

//Completion Port���� ���� ����� ���� �۽��� �����ϴ� �Լ�.
bool IOCPManager::ReceiveCompletion(ClientSession* client, OverlappedSock* ovsock, DWORD dwTransferred)
{
	if (client == nullptr)
	{
		cout << "Recive Client is nullptr" << endl;
		return false;
	}
	//���ŵ� ���۸� ��Ŷ����ü�� ����ȯ�Ѵ�.
	Packet * pack = (Packet*)ovsock->mBuffer;
	//��ȯ�� ��Ŷ�� �м��ؼ� �´� ������ �Ѵ�.
	client->PacketParsing(pack);

	bool sendRe = client->Send(pack);

	delete ovsock;

	if (!sendRe)
	{
		cout << "send fail..." <<endl;
	}

	return client->Recv();
}
//Completion Port���� ���� ����� ���� �۽ſ��� Ȯ�� �Լ�.
bool IOCPManager::SendCompletion(ClientSession* client, OverlappedSock* ovsock, DWORD dwTransferred)
{
	if (client == nullptr)
	{
		printf_s("SendCompletion client returned nullptr! \n");
		return false;
	}

	/// ���� �� �Ǿ����� Ȯ���ϴ� �� ó��..
	if (ovsock->mWsaBuf.len != dwTransferred)
	{
		delete ovsock;
		return false;
	}

	delete ovsock;
	return true;
}