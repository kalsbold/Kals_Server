#include "stdafx.h"
#include "ClientSession.h"
#include "SessionManager.h"


SessionManager* GSessionManager = nullptr;

SessionManager::~SessionManager()
{
}


/*
�Լ��� : CreateClient()
���ڰ� : SOCKET sock
��� : clientsession�� ����� ���� ��ü�� �����.
*/
ClientSession * SessionManager::CreateClient(SOCKET sock)
{
	ClientSession* client = new ClientSession(sock);

	cout << "create ClientSession.." << endl;
	clientlist.insert(ClientList::value_type(sock,client));
	return client;

}

/*
�Լ��� : DeleteClient()
���ڰ� : ClientSession * Client
��� : ������ ����� clientsession�� ������.
*/
void SessionManager::DeleteClient(ClientSession * client)
{
	clientlist.erase(client->mSocket);

	delete client;

	cout << "client delete" << endl;
}

/*
�Լ��� : IncreaseClientCount()
���ڰ� : 
��� : ���ӵ� clientsession�� ����� ��ü Ŭ���̾�Ʈ ������ ������Ŵ. 
	   ���� �ٸ� ��ü�鳢�� ����ȭ��Ű�� ���� CRITICAL_SECTION�� ���.
*/
int SessionManager::IncreaseClientCount()
{
	EnterCriticalSection(&CS);
	mClientCount++;
	cout << "client count increase.." << endl;
	LeaveCriticalSection(&CS);

	return mClientCount;
}

/*
�Լ��� : DecreaseClientCount()
���ڰ� :
��� : ������ ����� clientsession�� ����� ��ü Ŭ���̾�Ʈ ������ ���ҽ�Ŵ.
	   ���� �ٸ� ��ü�鳢�� ����ȭ��Ű�� ���� CRITICAL_SECTION�� ���.
*/
int SessionManager::DecreaseClientCount()
{
	EnterCriticalSection(&CS);
	mClientCount--;
	cout << "client count decrease.." << endl;
	LeaveCriticalSection(&CS);

	return mClientCount;
}