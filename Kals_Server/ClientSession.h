
//클라이언트 객체용 클래스
/*
연결용 함수
연결 확인 함수
연결 해제 함수.
송신, 수신함수.
*/
#pragma once
#include "FastSpinLock.h"

class ClientSession;
class SessionManager;

//입출력 타입.
enum IOType
{
	IO_NONE, //초기값
	IO_SEND, //전송
	IO_RECV,	//수신
	IO_RECV_ZERO, //수신량 없음.
	IO_ACCEPT	//접속.
};

//연결 종료 이유
enum DisconnectReason
{
	DR_NONE,	//초기값
	DR_RECV_ZERO, // 수신량 없음
	DR_ACTIVE, //실행
	DR_ONCONNECT_ERROR, //연결 오류
	DR_COMPLETION_ERROR, //컴플리션 오류
};

//overlapped 용 구조체.
struct OverlappedIOSock
{
	OverlappedIOSock(const ClientSession* owner, IOType ioType) : m_SessionObject(owner), m_IoType(ioType)
	{
		memset(&m_Overlapped, 0, sizeof(OVERLAPPED));
		memset(&m_WsaBuf, 0, sizeof(WSABUF));
		memset(m_Buffer, 0, BUFSIZE);
	}

	OVERLAPPED				m_Overlapped;
	const ClientSession*	m_SessionObject;
	IOType			m_IoType;
	WSABUF			m_WsaBuf;
	char			m_Buffer[BUFSIZE];
};

class ClientSession
{
public:
	ClientSession(SOCKET sock) : m_Socket(sock), m_Connected(false)
	{
		memset(&m_ClientAddr, 0, sizeof(SOCKADDR_IN));
	}
	~ClientSession() {}

	//연결용 함수.
	bool	OnConnect(SOCKADDR_IN* addr);
	//연결 확인 함수.
	bool	IsConnected() const { return m_Connected; }

	//수신함수.
	bool PostRecv() const;
	//송신함수.
	bool PostSend(const char* buf, int len) const;

	void Disconnect(DisconnectReason dr);



private:
	bool			m_Connected;
	SOCKET			m_Socket;

	SOCKADDR_IN		m_ClientAddr;

	FastSpinLock m_Lock;
	
	friend class SessionManager;
};
