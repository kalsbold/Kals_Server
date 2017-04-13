#include "stdafx.h"
#include "ClientSession.h"
#include "SessionManager.h"


SessionManager* G_SessionManager = nullptr;

SessionManager::~SessionManager()
{
}


/*
함수명 : CreateClient()
인자값 : SOCKET sock
기능 : clientsession을 만들고 관리 객체에 등록함.
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
함수명 : DeleteClient()
인자값 : ClientSession * Client
기능 : 접속이 끊기는 clientsession을 삭제함.
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
함수명 : IncreaseClientCount()
인자값 : 
기능 : 접속된 clientsession이 생기면 전체 클라이언트 갯수를 증가시킴. 
	   서로 다른 객체들끼리 동기화시키기 위해 CRITICAL_SECTION을 사용.
*/
int SessionManager::IncreaseClientCount()
{
	return InterlockedIncrement(&m_ClientCount);
}

/*
함수명 : DecreaseClientCount()
인자값 :
기능 : 접속이 끊기는 clientsession이 생기면 전체 클라이언트 갯수를 감소시킴.
	   서로 다른 객체들끼리 동기화시키기 위해 CRITICAL_SECTION을 사용.
*/
int SessionManager::DecreaseClientCount()
{
	return InterlockedDecrement(&m_ClientCount);
}