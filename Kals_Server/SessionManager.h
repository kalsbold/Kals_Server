/*
클라이언트 관리자.
클라이언트 증감이나 생성, 파괴등을 맡음.
접근 제한을 위해 FastSpinLock을 사용함.
클라이언트 관리를 위한 stl map 사용
*/
#pragma once
#include <map>
#include <WinSock2.h>
#include "FastSpinlock.h"

class ClientSession;

class SessionManager
{
public:
	SessionManager() : m_ClientCount(0) {}
	~SessionManager();

	//클라이언트 생성.
	ClientSession * CreateClient(SOCKET sock);
	//클라이언트 삭제.
	void DeleteClient(ClientSession * client);
	//클라이언트 수 증감.
	int IncreaseClientCount();
	int DecreaseClientCount();

private:
	//클라이언트 관리용 map
	typedef map<SOCKET, ClientSession*> ClientList;
	ClientList m_clientlist;

	FastSpinLock m_Lock;

	//클라이언트 수.
	volatile long m_ClientCount;

};


//전역으로 사용하기 위해 extern 변수로 정의.
extern SessionManager* G_SessionManager;