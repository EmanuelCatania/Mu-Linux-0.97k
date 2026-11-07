#pragma once

#ifndef MYSQL

#define MAX_COLUMNS 100

class CQueryManager
{
public:

	CQueryManager();

	~CQueryManager();

	bool Connect(char* odbc, char* user, char* pass);

	void Disconnect();

	void Diagnostic(char* query);

	bool ExecQuery(char* query, ...);

	void Close();

	SQLRETURN Fetch();

	int FindIndex(char* ColName);

	int GetResult(int index);

	int GetAsInteger(char* ColName);

	float GetAsFloat(char* ColName);

	__int64 GetAsInteger64(char* ColName);

	void GetAsString(char* ColName, char* OutBuffer, int OutBufferSize);

	void GetAsBinary(char* ColName, BYTE* OutBuffer, int OutBufferSize);

	void BindParameterAsString(int ParamNumber, void* InBuffer, int ColumnSize);

	void BindParameterAsBinary(int ParamNumber, void* InBuffer, int ColumnSize);

	void ConvertStringToBinary(char* InBuff, int InSize, BYTE* OutBuff, int OutSize);

	void ConvertBinaryToString(BYTE* InBuff, int InSize, char* OutBuff, int OutSize);

private:

	SQLHANDLE m_SQLEnvironment;

	SQLHANDLE m_SQLConnection;

	SQLHANDLE m_STMT;

	char m_odbc[32];

	char m_user[32];

	char m_pass[32];

	SQLINTEGER m_RowCount;

	SQLSMALLINT m_ColCount;

	SQLCHAR m_SQLColName[MAX_COLUMNS][30];

	char m_SQLData[MAX_COLUMNS][8192];

	SQLINTEGER m_SQLDataLen[MAX_COLUMNS];

	SQLINTEGER m_SQLBindValue[MAX_COLUMNS];
};

extern CQueryManager gQueryManager;

#else

#include <map>
#include <string>
#include <vector>


class CQueryManager
{
public:

	CQueryManager();

	virtual ~CQueryManager();

	bool Init(std::string HostName, WORD HostPort, std::string Username, std::string Password, std::string SchemaName);

	bool Connect();

	void Disconnect();

	void PrepareQuery(std::string query, ...);

	bool ExecPreparedUpdateQuery();

	bool ExecPreparedResultQuery();

	bool ExecUpdateQuery(std::string query, ...);

	bool ExecResultQuery(std::string query, ...);

	bool ExecQuery(std::string query, ...);

	void Close();

	bool Fetch();

	void SetAsInteger(int position, int value);

	void SetAsFloat(int position, float value);

	void SetAsInteger64(int position, __int64 value);

	void SetAsString(int position, char* InBuffer, int InBufferSize);

	void SetAsBinary(int position, unsigned char* InBuffer, int InBufferSize);

	int GetAsInteger(std::string ColName);

	float GetAsFloat(std::string ColName);

	__int64 GetAsInteger64(std::string ColName);

	void GetAsString(std::string ColName, char* OutBuffer, int OutBufferSize);

	void GetAsBinary(std::string ColName, unsigned char* OutBuffer, int OutBufferSize);

private:

	struct Param
	{
		int type;
		__int64 integer;
		double real;
		std::string bytes;
	};

	bool RunQuery(const char* query, bool& HasResult);

	void FreeResults();

	void CloseStatement();

	const char* Column(const std::string& ColName, unsigned long* Length = NULL);

	void Diagnostic(char* Query = NULL);

	void DiagnosticStatement();

	void ConvertStringToBinary(char* InBuff, int InSize, unsigned char* OutBuff, int OutSize);

	void ConvertBinaryToString(unsigned char* InBuff, int InSize, char* OutBuff, int OutSize);

private:

	// API en C de MySQL (libmysqlclient o MariaDB Connector/C). Los punteros
	// son opacos: mysql.h define un tipo `MYSQL` que choca con la macro MYSQL
	// que elige esta rama, y los nombres de struct no son los mismos en las dos
	// bibliotecas (st_mysql en MariaDB, MYSQL en la de Oracle). Sólo
	// QueryManager.cpp incluye mysql.h y les da el tipo.
	void* m_Connection;  // MYSQL*
	void* m_Result;      // MYSQL_RES*
	void* m_Statement;   // MYSQL_STMT*

	char** m_Row;
	unsigned long* m_Lengths;
	std::vector<std::string> m_Columns;

	std::map<int, Param> m_Params;

	std::string m_HostName;
	WORD m_HostPort;
	std::string m_Username;
	std::string m_Password;
	std::string m_SchemaName;
};

extern CQueryManager gQueryManager;

#endif
