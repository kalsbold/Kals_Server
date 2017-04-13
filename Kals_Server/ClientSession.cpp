#include "stdafx.h"
#include "Exception.h"
#include "Kals_Server.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "SessionManager.h"


/*
함수명 : OnConnect()
인자값 : SOCKADDR_IN * addr
기능 : 전달받은 주소를 저장하고
	   전달받은 주소에 대한 클라이언트가 존재하는지 파악함.
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
	//Completion Port에 클라이언트 소켓을 연결.
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
함수명 : Disconnect()
인자값 :
기능 : 접속 종료된 socket을 지움.
*/
void ClientSession::Disconnect(DisconnectReason dr)
{
	FastSpinLockGuard	lock(m_Lock);

	if (!IsConnected())
		return;

	//안정된 종료를 위한 LINGER 옵션.
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

	//세션 매니저에서 수 감소
	closesocket(m_Socket);

	m_Connected = false;
}



/*
함수명 : PostRecv()
인자값 : 
기능 : client가 전송한 data를 수신함.
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
함수명 : Send()
인자값 : const char * buf, int len
기능 : 전달받은 buf를 client에 전송함.
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