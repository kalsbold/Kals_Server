#include "stdafx.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "SessionManager.h"
#include "DBSession.h"

/*
�Լ��� : OnConnect()
���ڰ� : SOCKADDR_IN * addr
��� : ���޹��� �ּҸ� �����ϰ�
	   ���޹��� �ּҿ� ���� Ŭ���̾�Ʈ�� �����ϴ��� �ľ���.
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
	//Completion Port�� Ŭ���̾�Ʈ ������ ����.
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
�Լ��� : Recv()
���ڰ� : 
��� : client�� ������ data�� ������.
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
�Լ��� : Send()
���ڰ� : const char * buf, int len
��� : ���޹��� buf�� client�� ������.
*/
bool ClientSession::Send(Packet* pack)
{
	if (!IsConnected())
		return false;

	cout << "send in...." << endl;

	OverlappedSock* sendOV = new OverlappedSock(this, IO_SEND);

	//��Ŷ�� �鿩�ͼ� ���ۿ� �ű�.
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
�Լ��� : Disconnect()
���ڰ� :
��� : ���� ����� socket�� ����.
*/
void ClientSession::Disconnect()
{
	if (!IsConnected())
		return;

	//������ ���Ḧ ���� LINGER �ɼ�.
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

	//���� �Ŵ������� �� ����
	closesocket(mSocket);
	
	mConnected = false;
}

//��Ŷ Ÿ�Կ� ���� ó�� �Լ� ȣ��
bool ClientSession::PacketParsing(Packet * pack)
{
	Packet *packet = pack;

	//packet = (Packet*)mBuf;

	cout << "packet type : " << packet->Type << endl;

	switch (packet->Type)
	{
	case CHAT_REGISTER:	//��� ��Ŷ
		OnCHAT_REGISTER(packet);
		break;
	case CHAT_SECEDE:		//�������� ��Ŷ
		OnCHAT_SECEDE(packet);
		break;
	case CHAT_LOGIN:	//���� ��Ŷ�� ����.
		OnCHAT_LOGIN(packet);
		break;
	case CHAT_LOGOUT:	//���� ���� ��Ŷ�� ����.
		OnCHAT_LOGOUT(packet);
		break;
	case CHAT_DATA:		//������ ��Ŷ�� ����.
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

//�α��� ��Ŷ�� ���� ����
void ClientSession::OnCHAT_LOGIN(Packet * pack)
{
	mConnected = true;
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	//��Ŷ���� ���޹��� ID�� PW�� �˻��ϴ� ��������.
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
//�α׾ƿ� ��Ŷ�� ���� ����
void ClientSession::OnCHAT_LOGOUT(Packet * pack)
{
	mConnected = false;
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
}
//������ ��Ŷ�� ���� ����
bool ClientSession::OnCHAT_DATA(Packet * pack)
{
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	if (Send(pack) == false)
		return false;

	return true;
}
//ȸ������ ��Ŷ �����Լ�
void ClientSession::OnCHAT_REGISTER(Packet * pack)
{
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	//��Ŷ���� ���޹��� ID�� PW�� �����ϴ� ��������.
	if (GDBSession->InsertQuery(pack->ID, pack->PW) == false){
		cout << "Exist ID..." << endl;
		memcpy( pack->Data, "ExistID",sizeof("ExistID"));
		Send(pack);
	}
	cout << "Regist OK..." << endl;
	memcpy(pack->Data, "RegistOK", sizeof("RegistOK"));
	Send(pack);
}
//Ż�� ��Ŷ �����Լ�.
void ClientSession::OnCHAT_SECEDE(Packet * pack)
{
	cout << "ID : " << pack->ID << endl << "  recv message : " << pack->Data << "  IP = " << inet_ntoa(mClientAddr.sin_addr) << ",  Port = " << ntohs(mClientAddr.sin_port) << endl;
	//��Ŷ���� ���޹��� ID�� PW�� �����ϴ� ��������.
	if (GDBSession->DeleteQuery(pack->ID) == false){
		cout << "Wrong ID..." << endl;
		memcpy(pack->Data, "WrongID", sizeof("WrongID"));
		Send(pack);
	}
	cout << "Secede OK..." << endl;
	memcpy(pack->Data, "SecedeOK", sizeof("SecedeOK"));
	Send(pack);
}