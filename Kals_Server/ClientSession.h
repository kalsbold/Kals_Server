
//클라이언트 객체용 클래스
/*
연결용 함수
연결 확인 함수
연결 해제 함수.
송신, 수신함수.
*/
#pragma once

class ClientSession;
class SessionManager;

typedef enum PTYPE
{
	CHAT_NONE = -1,
	CHAT_REGISTER,  //회원가입.
	CHAT_SECEDE,	//탈퇴.
	CHAT_LOGIN,		//로그인 시도.
	CHAT_LOGOUT,	//로그아웃 시도.
	CHAT_DATA		//데이터 전송.
}Ptype;

//입출력 타입.
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
	DWORD Length;			//길이
	Ptype Type;				//타입
	char ID[64];			//ID
	char PW[64];			//PW
	char Data[BUFSIZE];		//데이터
	PACKET() {	//패킷 초기화
		Type = CHAT_NONE;
		Length = sizeof(*this);
		memset(ID, 0, sizeof(ID));
		memset(PW, 0, sizeof(PW));
		memset(Data, 0, sizeof(BUFSIZE));
	}

}Packet;
#pragma pack(pop)

//overlapped 용 구조체.
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

	//연결용 함수.
	bool	OnConnect(SOCKADDR_IN* addr);
	//연결 확인 함수.
	bool	IsConnected() const { return mConnected; }

	//수신함수.
	bool Recv();
	//송신함수.
	bool Send(Packet* pack);

	void Disconnect();

	//패킷을 분석하는 함수.
	bool PacketParsing(Packet * pack);
	//회원가입 패킷 구동함수
	void OnCHAT_REGISTER(Packet * pack);
	//탈퇴 패킷 구동함수.
	void OnCHAT_SECEDE(Packet * pack);
	//로그인패킷 구동함수.
	void OnCHAT_LOGIN(Packet * pack);
	//로그아웃패킷 구동함수.
	void OnCHAT_LOGOUT(Packet * pack);
	//데이터패킷 구동함수
	bool OnCHAT_DATA(Packet * pack);


private:
	bool			mConnected;
	SOCKET			mSocket;

	SOCKADDR_IN		mClientAddr;

	char mBuf[BUFSIZE*4];


	friend class SessionManager;
};
