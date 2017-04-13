#include "stdafx.h"
#include "Exception.h"
#include "Kals_Server.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "SessionManager.h"


/*
�Լ��� : OnConnect()
���ڰ� : SOCKADDR_IN * addr
��� : ���޹��� �ּҸ� �����ϰ�
	   ���޹��� �ּҿ� ���� Ŭ���̾�Ʈ�� �����ϴ��� �ľ���.
*/
bool ClientSession::OnConnect(SOCKADDR_IN* addr)
{
	FastSpinLockGuard lock(m_Lock);

	CRASH_ASSERT(l_ThreadType == THREAD_MAIN_ACCEPT);

	u_long arg = 1;
	ioctlsocket(m_Socket, FIONBIO, &arg);

	int opt = 1;
	setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));

	opt = 0;
	if (SOCKET_ERROR == setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
	{
		cout << "SO_RCVBUF change error: " << GetLastError() << endl;
		return false;
	}
	//Completion Port�� Ŭ���̾�Ʈ ������ ����.
	HANDLE handle = CreateIoCompletionPort((HANDLE)m_Socket,G_IocpManager->GetCPHandle(),(ULONG_PTR)this,0	);

	if (handle != G_IocpManager->GetCPHandle())
	{
		cout << "CreateIoCompletionPort error: "<< GetLastError()<< endl;
		return false;
	}

	memcpy(&m_ClientAddr, addr, sizeof(SOCKADDR_IN));
	m_Connected = true;

	cout << " Client Connected: IP = "<< inet_ntoa(m_ClientAddr.sin_addr)<<" PORT = " <<  ntohs(m_ClientAddr.sin_port) << endl;

	G_SessionManager->IncreaseClientCount();

	return PostRecv();
}


/*
�Լ��� : Disconnect()
���ڰ� :
��� : ���� ����� socket�� ����.
*/
void ClientSession::Disconnect(DisconnectReason dr)
{
	FastSpinLockGuard	lock(m_Lock);

	if (!IsConnected())
		return;

	//������ ���Ḧ ���� LINGER �ɼ�.
	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)))
	{
		cout << "linger option error..." << endl;
	}

	cout << " Client Disconnected: IP = " << inet_ntoa(m_ClientAddr.sin_addr) << " PORT = " << ntohs(m_ClientAddr.sin_port) << endl;


	G_SessionManager->DecreaseClientCount();

	//���� �Ŵ������� �� ����
	closesocket(m_Socket);

	m_Connected = false;
}



/*
�Լ��� : PostRecv()
���ڰ� : 
��� : client�� ������ data�� ������.
*/
bool ClientSession::PostRecv() const
{
	if (!IsConnected())
		return false;

	cout << "recv in...." << endl;

	OverlappedIOSock * recvOV = new OverlappedIOSock(this, IO_RECV);
	
	DWORD flags = 0; 
	DWORD recvbytes = 0;
	recvOV->m_WsaBuf.buf = recvOV->m_Buffer;
	recvOV->m_WsaBuf.len = BUFSIZE;

	if (WSARecv(m_Socket, &recvOV->m_WsaBuf,1, &recvbytes, &flags,&(recvOV->m_Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "recv error.." << endl;
			delete recvOV;
			return false;
		}
	}
	cout << "Recv Message : "<< recvOV->m_WsaBuf.buf<< endl;

	return true;
}


/*
�Լ��� : Send()
���ڰ� : const char * buf, int len
��� : ���޹��� buf�� client�� ������.
*/
bool ClientSession::PostSend(const char* buf, int len) const
{
	if (!IsConnected())
		return false;

	cout << "send in...." << endl;

	OverlappedIOSock* sendOV = new OverlappedIOSock(this, IO_SEND);

	/// copy for echoing back..
	memcpy_s(sendOV->m_Buffer, BUFSIZE, buf, len);

	DWORD sendbytes = 0;
	DWORD flags = 0;

	sendOV->m_WsaBuf.buf = sendOV->m_Buffer;
	sendOV->m_WsaBuf.len =	len;

	if (WSASend(m_Socket,&sendOV->m_WsaBuf,1,&sendbytes,flags,(LPWSAOVERLAPPED)&sendOV->m_Overlapped,NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "send error..."<<endl;
			delete sendOV;
			return false;
		}
	}
	return true;
}