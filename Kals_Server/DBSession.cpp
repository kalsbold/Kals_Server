#include "stdafx.h"
#include "DBSession.h"

DBSession * GDBSession = nullptr;

DBSession::DBSession()
{
	DB_HOST = "localhost";
	DB_NAME = "testdb";
	DB_SOCK = NULL;
	DB_USER = "root";
	DB_PASS = "03090502";
}


DBSession::~DBSession()
{
	mysql_free_result(res);

	mysql_close(conn);
}


bool DBSession::InitDB()
{
	conn = mysql_init(NULL);

	//DB�� ����.
	if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, DB_SOCK, DB_OPT))
	{
		cout << "DB Connect Error : " << mysql_error(conn) << endl;
		return false;
	}
	//DB�� Table Ȯ��.
	if (mysql_query(conn, "SHOW TABLES"))
	{
		cout << "DB Query Error : " << mysql_error(conn) << endl;
		return false;
	}
	//������� ����.
	res = mysql_use_result(conn);

	cout << "**DB Show Tables in " << DB_NAME << " **" << endl;
	//���� ��� Ȯ��.
	while ((row = mysql_fetch_row(res)) != NULL)
	{
		cout << row[0] << endl;
	}

	//////������ üũ.
	//if (mysql_query(conn, "SELECT * FROM USER_INFO") != NULL)
	//{
	//	cout << "Query Error : " << mysql_error(conn) << endl;
	//	return false;
	//}

	//res = mysql_store_result(conn);

	//while ((row = mysql_fetch_row(res)) != NULL)
	//{
	//	cout << "ID : " << row[0] << " PW :" << row[1] << endl;
	//}

	//mysql_free_result(res);

	return true;
}

//DB�� ��ġ�ϴ� ���Ұ� �ִ��� üũ.
bool DBSession::CheckQuery(string ID, string PW)
{
	//����� ������ ���ڿ��� �ռ�.
	string query = "SELECT * FROM LOGIN_DB WHERE ID='";
	query += ID + "' AND PW='";
	query += PW + "'";
	char q[256];
	memset(q,0,sizeof(q));
	memcpy(q,query.c_str(),sizeof(query));

	//���� ����.
	if (mysql_query(conn, q) != NULL)
	{
		cout << "Query Error : " << mysql_error(conn) << endl;
		return false;
	}
	//��� ����.
	res = mysql_store_result(conn);
	//���� ��� Ȯ��
	while ((row = mysql_fetch_row(res)) != NULL){
		if (row[0] == ID){	//ID�� ��ġ??
			if (row[1] == PW)	//PW�� ��ġ??
				break;
			else
				return false;
		}
		else
			return false;

		if (row == NULL)
			return false;
	} 
	if (res->row_count == 0){
		return false;
	}

	cout << "Query Success..." << endl;

	return true;
}
//DB�� �����ϴ� ����.
bool DBSession::InsertQuery(string ID, string PW)
{
	//����� ������ ���ڿ��� �ռ�.
	string query = "INSERT INTO LOGIN_DB VALUE ('";

	query += ID;
	query += "','";
	query += PW + "')";

	//���� ����.
	if (mysql_query(conn, query.c_str()) != NULL){
		cout << "Query Error : " << mysql_error(conn) << endl;
		return false;
	}
	
	cout << "Query Success..." << endl;

	return true;
}

//DB���� ����.
bool DBSession::DeleteQuery(string ID)
{
	//����� ������ ���ڿ��� �ռ�.
	string query = "DELETE FROM LOGIN_DB WHERE id = '";
	query += ID + "'";

	if (mysql_query(conn, query.c_str()) != NULL){
		cout << "Query Error : " << mysql_error(conn) << endl;
		return false;
	}

	cout << "Query Success..." << endl;

	return true;
}