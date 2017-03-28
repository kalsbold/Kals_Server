/*
Ŭ���̾�Ʈ ������.
Ŭ���̾�Ʈ �����̳� ����, �ı����� ����.
���� ������ ���� CRITICAL_SECTION�� �����.
���� ���� ����.
Ŭ���̾�Ʈ ������ ���� stl list ���
���� stl map���� ���� ����.
*/
#pragma once

class ClientSession;

class SessionManager
{
public:
	SessionManager() : mClientCount(0) {
		InitializeCriticalSection(&CS);
	}
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
	ClientList clientlist;

	//Ŭ���̾�Ʈ ��.
	int mClientCount;

	CRITICAL_SECTION CS;
};


//�������� ����ϱ� ���� extern ������ ����.
extern SessionManager* GSessionManager;