#include "stdafx.h"
#include "ClientSession.h"
#include "SessionManager.h"


SessionManager* G_SessionManager = nullptr;

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

	
	{
		FastSpinLockGuard lock(m_Lock);
		cout << "create ClientSession.." << endl;
		m_clientlist.insert(ClientList::value_type(sock, client));
	}
	
	return client;

}

/*
�Լ��� : DeleteClient()
���ڰ� : ClientSession * Client
��� : ������ ����� clientsession�� ������.
*/
void SessionManager::DeleteClient(ClientSession * client)
{

	
	{
		FastSpinLockGuard lock(m_Lock);
		cout << "client delete" << endl;
		m_clientlist.erase(client->m_Socket);
	}
	delete client;

	
}

/*
�Լ��� : IncreaseClientCount()
���ڰ� : 
��� : ���ӵ� clientsession�� ����� ��ü Ŭ���̾�Ʈ ������ ������Ŵ. 
	   ���� �ٸ� ��ü�鳢�� ����ȭ��Ű�� ���� CRITICAL_SECTION�� ���.
*/
int SessionManager::IncreaseClientCount()
{
	return InterlockedIncrement(&m_ClientCount);
}

/*
�Լ��� : DecreaseClientCount()
���ڰ� :
��� : ������ ����� clientsession�� ����� ��ü Ŭ���̾�Ʈ ������ ���ҽ�Ŵ.
	   ���� �ٸ� ��ü�鳢�� ����ȭ��Ű�� ���� CRITICAL_SECTION�� ���.
*/
int SessionManager::DecreaseClientCount()
{
	return InterlockedDecrement(&m_ClientCount);
}