#include "stdafx.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "SessionManager.h"
#include "DBSession.h"

/*
함수명 : OnConnect()
인자값 : SOCKADDR_IN * addr
기능 : 전달받은 주소를 저장하고
	   전달받은 주소에 대한 클라이언트가 존재하는지 파악함.
*/
bool ClientSession::OnConnect(SOCKADDR_IN* addr)
{

	u_long arg = 1;
	ioctlsocket(mSocket, FIONBIO, &arg);

	int opt = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));

	opt = 0;
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
	{
		cout << "SO_RCVBUF change error: " << GetLastError() << endl;
		return false;
	}
	//Completion Port에 클라이언트 소켓을 연결.
	HANDLE handle = CreateIoCompletionPort(
		(HANDLE)mSocket,
		GIocpManager->GetCPHandle(),
		(ULONG_PTR)this,
		0
	);

	if (handle != GIocpManager->GetCPHandle())
	{
		cout << "CreateIoCompletionPort error: "<< GetLastError()<< endl;
		return false;
	}

	memcpy(&mClientAddr, addr, sizeof(SOCKADDR_IN));
	mConnected = true;

	cout << " Client Connected: IP = "<< inet_ntoa(mClientAddr.sin_addr)<<" PORT = " <<  ntohs(mClientAddr.sin_port) << endl;

	GSessionManager->IncreaseClientCount();

	return Recv();
}


/*
함수명 : Recv()
인자값 : 
기능 : client가 전송한 data를 수신함.
*/
bool ClientSession::Recv()
{
	if (!IsConnected())
		return false;

	cout << "recv in...." << endl;

	OverlappedSock * recvOV = new OverlappedSock(this, IO_RECV);
	
	DWORD flags = 0; 
	DWORD recvbytes = 0;
	recvOV->mWsaBuf.buf = recvOV->mBuffer;
	recvOV->mWsaBuf.len = BUFSIZE;

	if (WSARecv(mSocket, &recvOV->mWsaBuf,1, &recvbytes, &flags,(LPWSAOVERLAPPED)recvOV, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "recv error.." << endl;
			return false;
		}
	}
	//cout << "Recv Message : "<< recvOV->mWsaBuf.buf<< endl;
	
	memcpy(mBuf, recvOV->mWsaBuf.buf,recvOV->mWsaBuf.len);

	return true;
}


/*
함수명 : Send()
인자값 : const char * buf, int len
기능 : 전달받은 buf를 client에 전송함.
*/
bool ClientSession::Send(Packet* pack)
{
	if (!IsConnected())
		return false;

	cout << "send in...." << endl;

	OverlappedSock* sendOV = new OverlappedSock(this, IO_SEND);

	//패킷을 들여와서 버퍼에 옮김.
	memcpy(mBuf, pack, sizeof(pack));
	/// copy for echoing back..
	memcpy(sendOV->mBuffer, mBuf, sizeof(mBuf));

	DWORD sendbytes = 0;
	DWORD flags = 0;

	sendOV->mWsaBuf.buf = sendOV->mBuffer;
	sendOV->mWsaBuf.len = sizeof(mBuf);

	if (WSASend(mSocket,&sendOV->mWsaBuf,1,&sendbytes,flags,(LPWSAOVERLAPPED)&sendOV->mOverlapped,NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "send error..."<<endl;
			return false;
		}
	}
	return true;
}


/*
함수명 : Disconnect()
인자값 :
기능 : 접속 종료된 socket을 지움.
*/
void ClientSession::Disconnect()
{
	if (!IsConnected())
		return;

	//안정된 종료를 위한 LINGER 옵션.
	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)))
	{
		cout << "linger option error..." << endl;
	}

	cout << " Client Disconnected: IP = " << inet_ntoa(mClientAddr.sin_addr) << " PORT = " << ntohs(mClientAddr.sin_port) << endl;


	GSessionManager->DecreaseClientCount();

	//세션 매니저에서 수 감소
	closesocket(mSocket);
	
	mConnected = false;
}

//패킷 타입에 따른 처리 함수 호출
bool ClientSession::PacketParsing(Packet * pack)
{
	Packet *packet = pack;

	//packet = (Packet*)mBuf;

	cout << "packet type : " << packet->Type << endl;

	switch (packet->Type)
	{
	case CHAT_REGISTER:	//등록 패킷
		OnCHAT_REGISTER(packet);
		break;
	case CHAT_SECEDE:		//가입해지 패킷
		OnCHAT_SECEDE(packet);
		break;
	case CHAT_LOGIN:	//접속 패킷이 오면.
		OnCHAT_LOGIN(packet);
		break;
	case CHAT_LOGOUT:	//접속 해제 패킷이 오면.
		OnCHAT_LOGOUT(packet);
		break;
	case CHAT_DATA:		//데이터 패킷이 오면.
		if (OnCHAT_DATA(packet) == false)
		{
			return false;
		}
		break;
	case CHAT_NONE:
		break;
	}

	if (!IsConnected())
		return false;

	return true;
}

//로그인 패킷에 대한 동작
void ClientSession::OnCHAT_LOGIN(Packet * pack)
{
	mConnected = true;
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	//패킷으로 전달받은 ID와 PW를 검색하는 쿼리실행.
	if (GDBSession->CheckQuery(pack->ID, pack->PW) == false){
		cout << "Wrong ID or password..." << endl;
		memcpy(pack->Data, "WrongInfo", sizeof("WrongInfo"));
		Send(pack);
	}else{
		cout << "Login Succese..." << endl;
		memcpy(pack->Data, "Login Succese...", sizeof("Login Succese..."));
		Send(pack);
	}
	
}
//로그아웃 패킷에 대한 동작
void ClientSession::OnCHAT_LOGOUT(Packet * pack)
{
	mConnected = false;
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
}
//데이터 패킷에 대한 동작
bool ClientSession::OnCHAT_DATA(Packet * pack)
{
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	if (Send(pack) == false)
		return false;

	return true;
}
//회원가입 패킷 구동함수
void ClientSession::OnCHAT_REGISTER(Packet * pack)
{
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	//패킷으로 전달받은 ID와 PW를 삽입하는 쿼리실행.
	if (GDBSession->InsertQuery(pack->ID, pack->PW) == false){
		cout << "Exist ID..." << endl;
		memcpy( pack->Data, "ExistID",sizeof("ExistID"));
		Send(pack);
	}
	cout << "Regist OK..." << endl;
	memcpy(pack->Data, "RegistOK", sizeof("RegistOK"));
	Send(pack);
}
//탈퇴 패킷 구동함수.
void ClientSession::OnCHAT_SECEDE(Packet * pack)
{
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	//패킷으로 전달받은 ID와 PW를 삭제하는 쿼리실행.
	if (GDBSession->DeleteQuery(pack->ID) == false){
		cout << "Wrong ID..." << endl;
		memcpy(pack->Data, "WrongID", sizeof("WrongID"));
		Send(pack);
	}
	cout << "Secede OK..." << endl;
	memcpy(pack->Data, "SecedeOK", sizeof("SecedeOK"));
	Send(pack);
}