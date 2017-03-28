#pragma once
class DBSession
{
public:
	DBSession();
	~DBSession();
	bool InitDB();

	//��ġ üũ ����
	bool CheckQuery(string ID, string PW);
	//�Է� ����
	bool InsertQuery(string ID, string PW);
	//���� ����
	bool DeleteQuery(string ID);

private:
	const char * DB_HOST; //DB ȣ��Ʈ
	const char * DB_NAME; //DB �̸�.
	const char * DB_SOCK; //DB ����
	const char * DB_USER; //DB �����
	const char * DB_PASS; //DB ��й�ȣ

	enum {
	DB_PORT = 1000,	//DB ��Ʈ
	DB_OPT = 0		//DB �ɼ�
	};


	MYSQL * conn;		//DB�� �����ϴ� ��ü
	MYSQL_RES *res;	//���� ���
	MYSQL_ROW row;	//������� ��Ÿ�� ���ڿ�
};

extern DBSession * GDBSession;