
//Ŭ���̾�Ʈ ��ü�� Ŭ����
/*
����� �Լ�
���� Ȯ�� �Լ�
���� ���� �Լ�.
�۽�, �����Լ�.
*/
#pragma once

class ClientSession;
class SessionManager;

typedef enum PTYPE
{
	CHAT_NONE = -1,
	CHAT_REGISTER,  //ȸ������.
	CHAT_SECEDE,	//Ż��.
	CHAT_LOGIN,		//�α��� �õ�.
	CHAT_LOGOUT,	//�α׾ƿ� �õ�.
	CHAT_DATA		//������ ����.
}Ptype;

//����� Ÿ��.
enum IOType
{
	IO_NONE,
	IO_SEND,
	IO_RECV,
	IO_RECV_ZERO,
	IO_ACCEPT
};


#pragma pack(push,1)
typedef struct PACKET
{
	DWORD Length;			//����
	Ptype Type;				//Ÿ��
	char ID[64];			//ID
	char PW[64];			//PW
	char Data[BUFSIZE];		//������
	PACKET() {	//��Ŷ �ʱ�ȭ
		Type = CHAT_NONE;
		Length = sizeof(*this);
		memset(ID, 0, sizeof(ID));
		memset(PW, 0, sizeof(PW));
		memset(Data, 0, sizeof(BUFSIZE));
	}

}Packet;
#pragma pack(pop)

//overlapped �� ����ü.
struct OverlappedSock
{
	OverlappedSock(const ClientSession* owner, IOType ioType) : mSessionObject(owner), mIoType(ioType)
	{
		memset(&mOverlapped, 0, sizeof(OVERLAPPED));
		memset(&mWsaBuf, 0, sizeof(WSABUF));
		memset(mBuffer, 0, BUFSIZE*4);
	}

	OVERLAPPED				mOverlapped;
	const ClientSession*	mSessionObject;
	IOType			mIoType;
	WSABUF			mWsaBuf;
	char			mBuffer[BUFSIZE*4];
};

class ClientSession
{
public:
	ClientSession(SOCKET sock) : mSocket(sock), mConnected(false)
	{
		memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));
	}
	~ClientSession() {}

	//����� �Լ�.
	bool	OnConnect(SOCKADDR_IN* addr);
	//���� Ȯ�� �Լ�.
	bool	IsConnected() const { return mConnected; }

	//�����Լ�.
	bool Recv();
	//�۽��Լ�.
	bool Send(Packet* pack);

	void Disconnect();

	//��Ŷ�� �м��ϴ� �Լ�.
	bool PacketParsing(Packet * pack);
	//ȸ������ ��Ŷ �����Լ�
	void OnCHAT_REGISTER(Packet * pack);
	//Ż�� ��Ŷ �����Լ�.
	void OnCHAT_SECEDE(Packet * pack);
	//�α�����Ŷ �����Լ�.
	void OnCHAT_LOGIN(Packet * pack);
	//�α׾ƿ���Ŷ �����Լ�.
	void OnCHAT_LOGOUT(Packet * pack);
	//��������Ŷ �����Լ�
	bool OnCHAT_DATA(Packet * pack);


private:
	bool			mConnected;
	SOCKET			mSocket;

	SOCKADDR_IN		mClientAddr;

	char mBuf[BUFSIZE*4];


	friend class SessionManager;
};
