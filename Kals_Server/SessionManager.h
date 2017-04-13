/*
Ŭ���̾�Ʈ ������.
Ŭ���̾�Ʈ �����̳� ����, �ı����� ����.
���� ������ ���� FastSpinLock�� �����.
Ŭ���̾�Ʈ ������ ���� stl map ���
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

	//Ŭ���̾�Ʈ ����.
	ClientSession * CreateClient(SOCKET sock);
	//Ŭ���̾�Ʈ ����.
	void DeleteClient(ClientSession * client);
	//Ŭ���̾�Ʈ �� ����.
	int IncreaseClientCount();
	int DecreaseClientCount();

private:
	//Ŭ���̾�Ʈ ������ map
	typedef map<SOCKET, ClientSession*> ClientList;
	ClientList m_clientlist;

	FastSpinLock m_Lock;

	//Ŭ���̾�Ʈ ��.
	volatile long m_ClientCount;

};


//�������� ����ϱ� ���� extern ������ ����.
extern SessionManager* G_SessionManager;