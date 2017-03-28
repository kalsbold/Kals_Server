/*
클라이언트 관리자.
클라이언트 증감이나 생성, 파괴등을 맡음.
접근 제한을 위해 CRITICAL_SECTION을 사용함.
추후 변경 가능.
클라이언트 관리를 위한 stl list 사용
추후 stl map으로 변경 예정.
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
	ClientList clientlist;

	//클라이언트 수.
	int mClientCount;

	CRITICAL_SECTION CS;
};


//전역으로 사용하기 위해 extern 변수로 정의.
extern SessionManager* GSessionManager;