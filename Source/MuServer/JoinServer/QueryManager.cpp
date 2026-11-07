#include "stdafx.h"
#include "QueryManager.h"
#include "Util.h"

#ifndef MYSQL

CQueryManager gQueryManager;

CQueryManager::CQueryManager()
{
	this->m_SQLEnvironment = SQL_NULL_HANDLE;

	this->m_SQLConnection = SQL_NULL_HANDLE;

	this->m_STMT = SQL_NULL_HANDLE;

	this->m_RowCount = -1;

	this->m_ColCount = -1;

	memset(this->m_SQLColName, 0, sizeof(this->m_SQLColName));

	memset(this->m_SQLData, 0, sizeof(this->m_SQLData));

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &this->m_SQLEnvironment);

	SQLSetEnvAttr(this->m_SQLEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
}

CQueryManager::~CQueryManager()
{
	this->Disconnect();
}

bool CQueryManager::Connect(char* odbc, char* user, char* pass)
{
	strcpy_s(this->m_odbc, odbc);

	strcpy_s(this->m_user, user);

	strcpy_s(this->m_pass, pass);

	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, this->m_SQLEnvironment, &this->m_SQLConnection)) == 0)
	{
		return false;
	}

	if (SQL_SUCCEEDED(SQLConnect(this->m_SQLConnection, (SQLCHAR*)this->m_odbc, SQL_NTS, (SQLCHAR*)this->m_user, SQL_NTS, (SQLCHAR*)this->m_pass, SQL_NTS)) == 0)
	{
		return false;
	}

	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, this->m_SQLConnection, &this->m_STMT)) == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CQueryManager::Disconnect()
{
	if (this->m_STMT != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, this->m_STMT);

		this->m_STMT = SQL_NULL_HANDLE;
	}

	if (this->m_SQLConnection != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, this->m_SQLConnection);

		this->m_SQLConnection = SQL_NULL_HANDLE;
	}

	if (this->m_SQLEnvironment != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, this->m_SQLEnvironment);

		this->m_SQLEnvironment = SQL_NULL_HANDLE;
	}
}

void CQueryManager::Diagnostic(char* query)
{
	LogAdd(LOG_BLACK, "%s", query);

	SQLINTEGER NativeError;

	SQLSMALLINT RecNumber = 1, BufferLength;

	SQLCHAR SqlState[6], MessageText[SQL_MAX_MESSAGE_LENGTH];

	while (SQLGetDiagRec(SQL_HANDLE_STMT, this->m_STMT, (RecNumber++), SqlState, &NativeError, MessageText, sizeof(MessageText), &BufferLength) != SQL_NO_DATA)
	{
		LogAdd(LOG_RED, "[QueryManager] State (%s), Diagnostic: %s", SqlState, MessageText);
	}

	if (strcmp((char*)SqlState, "08S01") == 0)
	{
		this->Connect(this->m_odbc, this->m_user, this->m_pass);
	}
}

bool CQueryManager::ExecQuery(char* query, ...)
{
	char buff[4096];

	va_list arg;

	va_start(arg, query);

	vsprintf_s(buff, query, arg);

	va_end(arg);

	SQLRETURN result;

	if (SQL_SUCCEEDED((result = SQLExecDirect(this->m_STMT, (SQLCHAR*)buff, SQL_NTS))) == 0 && result != SQL_NO_DATA)
	{
		this->Diagnostic(buff);

		return false;
	}

	SQLRowCount(this->m_STMT, &this->m_RowCount);

	if (this->m_RowCount == 0)
	{
		return true;
	}

	SQLNumResultCols(this->m_STMT, &this->m_ColCount);

	if (this->m_ColCount == 0)
	{
		return true;
	}

	if (this->m_ColCount > MAX_COLUMNS)
	{
		return false;
	}

	memset(this->m_SQLColName, 0, sizeof(this->m_SQLColName));

	memset(this->m_SQLData, 0, sizeof(this->m_SQLData));

	for (int n = 0; n < this->m_ColCount; n++)
	{
		SQLDescribeCol(this->m_STMT, (n + 1), this->m_SQLColName[n], sizeof(this->m_SQLColName[n]), 0, 0, 0, 0, 0);

		SQLBindCol(this->m_STMT, (n + 1), SQL_C_CHAR, this->m_SQLData[n], sizeof(this->m_SQLData[n]), &this->m_SQLDataLen[n]);
	}

	return true;
}

void CQueryManager::Close()
{
	SQLCloseCursor(this->m_STMT);

	SQLFreeStmt(this->m_STMT, SQL_UNBIND);
}

SQLRETURN CQueryManager::Fetch()
{
	return SQLFetch(this->m_STMT);
}

int CQueryManager::FindIndex(char* ColName)
{
	for (int n = 0; n < this->m_ColCount; n++)
	{
		if (_stricmp(ColName, (char*)this->m_SQLColName[n]) == 0)
		{
			return n;
		}
	}

	return -1;
}

int CQueryManager::GetResult(int index)
{
	return atoi(this->m_SQLData[index]);
}

int CQueryManager::GetAsInteger(char* ColName)
{
	int index = this->FindIndex(ColName);

	if (index == -1)
	{
		return index;
	}
	else
	{
		return atoi(this->m_SQLData[index]);
	}
}

float CQueryManager::GetAsFloat(char* ColName)
{
	int index = this->FindIndex(ColName);

	if (index == -1)
	{
		return (float)index;
	}
	else
	{
		return (float)atof(this->m_SQLData[index]);
	}
}

__int64 CQueryManager::GetAsInteger64(char* ColName)
{
	int index = this->FindIndex(ColName);

	if (index == -1)
	{
		return index;
	}
	else
	{
		return _atoi64(this->m_SQLData[index]);
	}
}

void CQueryManager::GetAsString(char* ColName, char* OutBuffer, int OutBufferSize)
{
	int index = this->FindIndex(ColName);

	if (index == -1)
	{
		memset(OutBuffer, 0, OutBufferSize);
	}
	else
	{
		strncpy_s(OutBuffer, OutBufferSize, this->m_SQLData[index], (OutBufferSize - 1));
	}
}

void CQueryManager::GetAsBinary(char* ColName, BYTE* OutBuffer, int OutBufferSize)
{
	int index = this->FindIndex(ColName);

	if (index == -1)
	{
		memset(OutBuffer, 0, OutBufferSize);
	}
	else
	{
		this->ConvertStringToBinary(this->m_SQLData[index], sizeof(this->m_SQLData[index]), OutBuffer, OutBufferSize);
	}
}

void CQueryManager::BindParameterAsString(int ParamNumber, void* InBuffer, int ColumnSize)
{
	this->m_SQLBindValue[(ParamNumber - 1)] = SQL_NTS;

	SQLBindParameter(this->m_STMT, ParamNumber, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, ColumnSize, 0, InBuffer, 0, &this->m_SQLBindValue[(ParamNumber - 1)]);
}

void CQueryManager::BindParameterAsBinary(int ParamNumber, void* InBuffer, int ColumnSize)
{
	this->m_SQLBindValue[(ParamNumber - 1)] = ColumnSize;

	SQLBindParameter(this->m_STMT, ParamNumber, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_VARBINARY, ColumnSize, 0, InBuffer, 0, &this->m_SQLBindValue[(ParamNumber - 1)]);
}

void CQueryManager::ConvertStringToBinary(char* InBuff, int InSize, BYTE* OutBuff, int OutSize)
{
	int size = 0;

	memset(OutBuff, 0, OutSize);

	for (int n = 0; n < InSize, size < OutSize; n++)
	{
		if (InBuff[n] == 0)
		{
			break;
		}

		if ((n % 2) == 0)
		{
			OutBuff[size] = ((InBuff[n] >= 'A') ? ((InBuff[n] - 'A') + 10) : (InBuff[n] - '0')) * 16;

			size = size + 0;
		}
		else
		{
			OutBuff[size] = OutBuff[size] | ((InBuff[n] >= 'A') ? ((InBuff[n] - 'A') + 10) : (InBuff[n] - '0'));

			size = size + 1;
		}
	}
}

void CQueryManager::ConvertBinaryToString(BYTE* InBuff, int InSize, char* OutBuff, int OutSize)
{
	int size = 0;

	memset(OutBuff, 0, OutSize);

	for (int n = 0; n < OutSize, size < InSize; n++)
	{
		if ((n % 2) == 0)
		{
			OutBuff[n] = (((InBuff[size] / 16) >= 10) ? ('A' + ((InBuff[size] / 16) - 10)) : ('0' + (InBuff[size] / 16)));

			size = size + 0;
		}
		else
		{
			OutBuff[n] = (((InBuff[size] % 16) >= 10) ? ('A' + ((InBuff[size] % 16) - 10)) : ('0' + (InBuff[size] % 16)));

			size = size + 1;
		}
	}
}

#else

#include "Log.h"

// mysql.h define el tipo `MYSQL`, que choca con la macro MYSQL de esta rama:
// se la saca mientras se incluye y en el código se usan los nombres de struct
// (st_mysql, st_mysql_res...), iguales en libmysqlclient y en MariaDB.
#pragma push_macro("MYSQL")
#undef MYSQL
#include <mysql.h>
#pragma pop_macro("MYSQL")

#include <cstdlib>
#include <cstring>

namespace
{
	enum ParamType { PARAM_INTEGER, PARAM_REAL, PARAM_STRING, PARAM_BINARY };
}

CQueryManager gQueryManager;

CQueryManager::CQueryManager()
{
	this->m_Connection = NULL;

	this->m_Result = NULL;

	this->m_Statement = NULL;

	this->m_Row = NULL;

	this->m_Lengths = NULL;

	this->m_HostPort = 0;
}

CQueryManager::~CQueryManager()
{
	this->Disconnect();
}

bool CQueryManager::Init(std::string HostName, WORD HostPort, std::string Username, std::string Password, std::string SchemaName)
{
	// Los .ini traen el host con el formato de URL de Connector/C++
	// ("tcp://127.0.0.1"); la API en C espera sólo el nombre.
	if (HostName.compare(0, 6, "tcp://") == 0)
	{
		HostName.erase(0, 6);
	}

	this->m_HostName = HostName;

	this->m_HostPort = HostPort;

	this->m_Username = Username;

	this->m_Password = Password;

	this->m_SchemaName = SchemaName;

	return this->Connect();
}

bool CQueryManager::Connect()
{
	this->Close();

	if (this->m_Connection)
	{
		mysql_close(this->m_Connection);

		this->m_Connection = NULL;
	}

	this->m_Connection = mysql_init(NULL);

	if (this->m_Connection == NULL)
	{
		LogAdd(LOG_RED, "[QueryManager] Connection to DataBase failed: mysql_init");

		return false;
	}

	// La base y el texto del server son UTF-8 (0.97.40).
	mysql_options(this->m_Connection, MYSQL_SET_CHARSET_NAME, "utf8mb4");

#if defined(MARIADB_PACKAGE_VERSION_ID)
	// MySQL 8.4 autentica con caching_sha2_password, que fuera del caché del
	// server exige TLS. La biblioteca de Oracle ya lo intenta por defecto
	// (PREFERRED); la de MariaDB no, así que se pide acá. El certificado que
	// genera MySQL es autofirmado: no se verifica (igual que PREFERRED).
	mysql_ssl_set(this->m_Connection, NULL, NULL, NULL, NULL, NULL);
	my_bool verify = 0;
	mysql_options(this->m_Connection, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify);
#endif

	// CLIENT_MULTI_RESULTS: los CALL a procedimientos devuelven más de un resultado.
	if (mysql_real_connect(this->m_Connection, this->m_HostName.c_str(), this->m_Username.c_str(), this->m_Password.c_str(), this->m_SchemaName.c_str(), this->m_HostPort, NULL, CLIENT_MULTI_RESULTS) == NULL)
	{
		LogAdd(LOG_RED, "[QueryManager] Connection to DataBase failed: %s", mysql_error(this->m_Connection));

		mysql_close(this->m_Connection);

		this->m_Connection = NULL;

		return false;
	}

	return true;
}

void CQueryManager::Disconnect()
{
	this->Close();

	if (this->m_Connection)
	{
		mysql_close(this->m_Connection);

		this->m_Connection = NULL;
	}
}

void CQueryManager::PrepareQuery(std::string query, ...)
{
	char buff[4096];

	va_list arg;

	va_start(arg, query);

	vsprintf_s(buff, query.c_str(), arg);

	va_end(arg);

	this->FreeResults();

	this->CloseStatement();

	this->m_Params.clear();

	if (this->m_Connection == NULL)
	{
		LogAdd(LOG_RED, "[PrepareQuery] No connection to DataBase");

		return;
	}

	this->m_Statement = mysql_stmt_init(this->m_Connection);

	if (this->m_Statement == NULL || mysql_stmt_prepare(this->m_Statement, buff, (unsigned long)strlen(buff)) != 0)
	{
		LogAdd(LOG_BLUE, "%s", buff);

		this->DiagnosticStatement();

		this->CloseStatement();

		return;
	}

	gLog.Output(LOG_QUERY, "[PrepareQuery] Query prepared: %s", buff);
}

bool CQueryManager::ExecPreparedUpdateQuery()
{
	if (this->m_Statement == NULL)
	{
		LogAdd(LOG_RED, "[ExecPreparedUpdateQuery] No prepared query");

		return false;
	}

	const unsigned long count = mysql_stmt_param_count(this->m_Statement);

	std::vector<MYSQL_BIND> binds(count);

	std::vector<unsigned long> lengths(count);

	memset(binds.data(), 0, sizeof(MYSQL_BIND) * count);

	for (unsigned long n = 0; n < count; n++)
	{
		std::map<int, Param>::iterator it = this->m_Params.find((int)n + 1);

		if (it == this->m_Params.end())
		{
			binds[n].buffer_type = MYSQL_TYPE_NULL;

			continue;
		}

		Param& param = it->second;

		switch (param.type)
		{
			case PARAM_INTEGER:
				binds[n].buffer_type = MYSQL_TYPE_LONGLONG;
				binds[n].buffer = &param.integer;
				break;
			case PARAM_REAL:
				binds[n].buffer_type = MYSQL_TYPE_DOUBLE;
				binds[n].buffer = &param.real;
				break;
			default:
				binds[n].buffer_type = (param.type == PARAM_STRING) ? MYSQL_TYPE_STRING : MYSQL_TYPE_BLOB;
				binds[n].buffer = (void*)param.bytes.data();
				binds[n].buffer_length = (unsigned long)param.bytes.size();
				lengths[n] = (unsigned long)param.bytes.size();
				binds[n].length = &lengths[n];
				break;
		}
	}

	if ((count > 0 && mysql_stmt_bind_param(this->m_Statement, binds.data()) != 0) || mysql_stmt_execute(this->m_Statement) != 0)
	{
		this->DiagnosticStatement();

		return false;
	}

	gLog.Output(LOG_QUERY, "[ExecPreparedUpdateQuery] %d rows updated", (int)mysql_stmt_affected_rows(this->m_Statement));

	return true;
}

bool CQueryManager::ExecPreparedResultQuery()
{
	// Ningún llamador lee filas de una consulta preparada; leerlas con la API
	// en C exige enlazar cada columna, así que no se implementa.
	LogAdd(LOG_RED, "[ExecPreparedResultQuery] Not supported");

	return false;
}

bool CQueryManager::ExecUpdateQuery(std::string query, ...)
{
	char buff[4096];

	va_list arg;

	va_start(arg, query);

	vsprintf_s(buff, query.c_str(), arg);

	va_end(arg);

	bool HasResult = false;

	if (this->RunQuery(buff, HasResult) == false)
	{
		return false;
	}

	gLog.Output(LOG_QUERY, "[ExecUpdateQuery] %d rows updated. Query: %s", (int)mysql_affected_rows(this->m_Connection), buff);

	return true;
}

bool CQueryManager::ExecResultQuery(std::string query, ...)
{
	char buff[4096];

	va_list arg;

	va_start(arg, query);

	vsprintf_s(buff, query.c_str(), arg);

	va_end(arg);

	bool HasResult = false;

	if (this->RunQuery(buff, HasResult) == false)
	{
		return false;
	}

	if (HasResult == false)
	{
		gLog.Output(LOG_QUERY, "[ExecResultQuery] No results available. Query: %s", buff);

		return false;
	}

	gLog.Output(LOG_QUERY, "[ExecResultQuery] Query with results. Query: %s", buff);

	return true;
}

bool CQueryManager::ExecQuery(std::string query, ...)
{
	char buff[4096];

	va_list arg;

	va_start(arg, query);

	vsprintf_s(buff, query.c_str(), arg);

	va_end(arg);

	bool HasResult = false;

	if (this->RunQuery(buff, HasResult) == false)
	{
		return false;
	}

	if (HasResult == false)
	{
		__int64 Quant = (__int64)mysql_affected_rows(this->m_Connection);

		gLog.Output(LOG_QUERY, "[ExecQuery] %d rows updated. Query: %s", (int)Quant, buff);

		return (Quant > 0);
	}

	gLog.Output(LOG_QUERY, "[ExecQuery] Query with results. Query: %s", buff);

	return true;
}

bool CQueryManager::RunQuery(const char* query, bool& HasResult)
{
	HasResult = false;

	// Lo que haya quedado de la consulta anterior (filas sin leer o los
	// resultados extra de un CALL) bloquea la conexión hasta descartarlo.
	this->FreeResults();

	if (this->m_Connection == NULL && this->Connect() == false)
	{
		return false;
	}

	if (mysql_real_query(this->m_Connection, query, (unsigned long)strlen(query)) != 0)
	{
		this->Diagnostic((char*)query);

		return false;
	}

	this->m_Result = mysql_store_result(this->m_Connection);

	if (this->m_Result == NULL)
	{
		if (mysql_field_count(this->m_Connection) != 0)
		{
			this->Diagnostic((char*)query);

			return false;
		}

		return true;
	}

	const unsigned int fields = mysql_num_fields(this->m_Result);

	MYSQL_FIELD* field = mysql_fetch_fields(this->m_Result);

	for (unsigned int n = 0; n < fields; n++)
	{
		this->m_Columns.push_back(field[n].name ? field[n].name : "");
	}

	HasResult = true;

	return true;
}

void CQueryManager::FreeResults()
{
	if (this->m_Result)
	{
		mysql_free_result(this->m_Result);

		this->m_Result = NULL;
	}

	this->m_Row = NULL;

	this->m_Lengths = NULL;

	this->m_Columns.clear();

	if (this->m_Connection == NULL)
	{
		return;
	}

	while (mysql_more_results(this->m_Connection) && mysql_next_result(this->m_Connection) == 0)
	{
		st_mysql_res* extra = mysql_store_result(this->m_Connection);

		if (extra)
		{
			mysql_free_result(extra);
		}
	}
}

void CQueryManager::CloseStatement()
{
	if (this->m_Statement)
	{
		mysql_stmt_close(this->m_Statement);

		this->m_Statement = NULL;
	}
}

void CQueryManager::Diagnostic(char* Query)
{
	if (Query)
	{
		LogAdd(LOG_BLUE, "%s", Query);
	}

	std::string SqlState = mysql_sqlstate(this->m_Connection);

	LogAdd(LOG_RED, "[QueryManager] State (%s), Diagnostic: %s", SqlState.c_str(), mysql_error(this->m_Connection));

	if (SqlState.compare("HY000") == 0)
	{
		this->Connect();
	}
}

void CQueryManager::DiagnosticStatement()
{
	if (this->m_Statement == NULL)
	{
		this->Diagnostic();

		return;
	}

	std::string SqlState = mysql_stmt_sqlstate(this->m_Statement);

	LogAdd(LOG_RED, "[QueryManager] State (%s), Diagnostic: %s", SqlState.c_str(), mysql_stmt_error(this->m_Statement));

	if (SqlState.compare("HY000") == 0)
	{
		this->Connect();
	}
}

void CQueryManager::Close()
{
	this->FreeResults();

	this->CloseStatement();

	this->m_Params.clear();
}

bool CQueryManager::Fetch()
{
	if (this->m_Result == NULL)
	{
		return false;
	}

	this->m_Row = mysql_fetch_row(this->m_Result);

	if (this->m_Row == NULL)
	{
		this->m_Lengths = NULL;

		return false;
	}

	this->m_Lengths = mysql_fetch_lengths(this->m_Result);

	return true;
}

void CQueryManager::SetAsInteger(int position, int value)
{
	Param& param = this->m_Params[position];

	param.type = PARAM_INTEGER;

	param.integer = value;
}

void CQueryManager::SetAsFloat(int position, float value)
{
	Param& param = this->m_Params[position];

	param.type = PARAM_REAL;

	param.real = (double)value;
}

void CQueryManager::SetAsInteger64(int position, __int64 value)
{
	Param& param = this->m_Params[position];

	param.type = PARAM_INTEGER;

	param.integer = value;
}

void CQueryManager::SetAsString(int position, char* InBuffer, int InBufferSize)
{
	Param& param = this->m_Params[position];

	param.type = PARAM_STRING;

	param.bytes.assign(InBuffer, TextField::Length(InBuffer, InBufferSize));
}

void CQueryManager::SetAsBinary(int position, unsigned char* InBuffer, int InBufferSize)
{
	Param& param = this->m_Params[position];

	param.type = PARAM_BINARY;

	param.bytes.assign((const char*)InBuffer, InBufferSize);
}

const char* CQueryManager::Column(const std::string& ColName, unsigned long* Length)
{
	if (Length)
	{
		*Length = 0;
	}

	if (this->m_Row == NULL)
	{
		return NULL;
	}

	// Connector/C++ buscaba la columna sin distinguir mayúsculas.
	for (size_t n = 0; n < this->m_Columns.size(); n++)
	{
		if (_stricmp(this->m_Columns[n].c_str(), ColName.c_str()) == 0)
		{
			if (Length && this->m_Lengths)
			{
				*Length = this->m_Lengths[n];
			}

			return this->m_Row[n];
		}
	}

	LogAdd(LOG_RED, "[QueryManager] Unknown column: %s", ColName.c_str());

	return NULL;
}

int CQueryManager::GetAsInteger(std::string ColName)
{
	const char* value = this->Column(ColName);

	return value ? (int)strtoll(value, NULL, 10) : 0;
}

float CQueryManager::GetAsFloat(std::string ColName)
{
	const char* value = this->Column(ColName);

	return value ? (float)strtod(value, NULL) : 0.0f;
}

__int64 CQueryManager::GetAsInteger64(std::string ColName)
{
	const char* value = this->Column(ColName);

	return value ? (__int64)strtoll(value, NULL, 10) : 0;
}

void CQueryManager::GetAsString(std::string ColName, char* OutBuffer, int OutBufferSize)
{
	unsigned long Length = 0;

	const char* value = this->Column(ColName, &Length);

	std::size_t length = value ? Length : 0;

	// Si no entra, se corta en un borde de codepoint para no dejar UTF-8 roto.
	if (length > (std::size_t)(OutBufferSize - 1))
	{
		length = OutBufferSize - 1;

		while (length > 0 && ((unsigned char)value[length] & 0xC0) == 0x80)
		{
			length--;
		}
	}

	if (length > 0)
	{
		memcpy(OutBuffer, value, length);
	}

	OutBuffer[length] = 0;
}

void CQueryManager::GetAsBinary(std::string ColName, unsigned char* OutBuffer, int OutBufferSize)
{
	unsigned long Length = 0;

	const char* value = this->Column(ColName, &Length);

	// Lo que falte (blob más corto o NULL) queda en cero; Connector/C++ dejaba
	// esos bytes sin inicializar.
	const unsigned long size = (value == NULL) ? 0 : (Length < (unsigned long)OutBufferSize ? Length : (unsigned long)OutBufferSize);

	memset(OutBuffer, 0, OutBufferSize);

	if (size > 0)
	{
		memcpy(OutBuffer, value, size);
	}
}

void CQueryManager::ConvertStringToBinary(char* InBuff, int InSize, unsigned char* OutBuff, int OutSize)
{
	int size = 0;

	memset(OutBuff, 0, OutSize);

	for (int n = 0; n < InSize, size < OutSize; n++)
	{
		if (InBuff[n] == 0)
		{
			break;
		}

		if ((n % 2) == 0)
		{
			OutBuff[size] = ((InBuff[n] >= 'A') ? ((InBuff[n] - 'A') + 10) : (InBuff[n] - '0')) * 16;

			size = size + 0;
		}
		else
		{
			OutBuff[size] = OutBuff[size] | ((InBuff[n] >= 'A') ? ((InBuff[n] - 'A') + 10) : (InBuff[n] - '0'));

			size = size + 1;
		}
	}
}

void CQueryManager::ConvertBinaryToString(unsigned char* InBuff, int InSize, char* OutBuff, int OutSize)
{
	int size = 0;

	memset(OutBuff, 0, OutSize);

	for (int n = 0; n < OutSize, size < InSize; n++)
	{
		if ((n % 2) == 0)
		{
			OutBuff[n] = (((InBuff[size] / 16) >= 10) ? ('A' + ((InBuff[size] / 16) - 10)) : ('0' + (InBuff[size] / 16)));

			size = size + 0;
		}
		else
		{
			OutBuff[n] = (((InBuff[size] % 16) >= 10) ? ('A' + ((InBuff[size] % 16) - 10)) : ('0' + (InBuff[size] % 16)));

			size = size + 1;
		}
	}
}

#endif
