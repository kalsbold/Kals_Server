
//Ŭ���̾�Ʈ ��ü�� Ŭ����
/*
����� �Լ�
���� Ȯ�� �Լ�
���� ���� �Լ�.
�۽�, �����Լ�.
*/
#pragma once
#include "FastSpinLock.h"

class ClientSession;
class SessionManager;

//����� Ÿ��.
enum IOType
{
	IO_NONE, //�ʱⰪ
	IO_SEND, //����
	IO_RECV,	//����
	IO_RECV_ZERO, //���ŷ� ����.
	IO_ACCEPT	//����.
};

//���� ���� ����
enum DisconnectReason
{
	DR_NONE,	//�ʱⰪ
	DR_RECV_ZERO, // ���ŷ� ����
	DR_ACTIVE, //����
	DR_ONCONNECT_ERROR, //���� ����
	DR_COMPLETION_ERROR, //���ø��� ����
};

//overlapped �� ����ü.
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

	//����� �Լ�.
	bool	OnConnect(SOCKADDR_IN* addr);
	//���� Ȯ�� �Լ�.
	bool	IsConnected() const { return m_Connected; }

	//�����Լ�.
	bool PostRecv() const;
	//�۽��Լ�.
	bool PostSend(const char* buf, int len) const;

	void Disconnect(DisconnectReason dr);



private:
	bool			m_Connected;
	SOCKET			m_Socket;

	SOCKADDR_IN		m_ClientAddr;

	FastSpinLock m_Lock;
	
	friend class SessionManager;
};
