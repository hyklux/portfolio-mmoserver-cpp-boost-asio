#pragma once
#include <sql.h>
#include <sqlext.h>
#include <stdlib.h>
#include <iostream>

enum
{
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000
};

class DBConn
{
public:
	bool			Connect(const WCHAR* connectionString);
	void			Clear();

	bool			Execute(const WCHAR* query);
	bool			Fetch();
	int32_t			GetRowCount();
	void			Unbind();

public:
	bool			BindParam(int32_t paramIndex, bool* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, float* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, double* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, int8_t* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, int16_t* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, int32_t* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, int64_t* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, const WCHAR* str, SQLLEN* index);
	bool			BindParam(int32_t paramIndex, const BYTE* bin, int32_t size, SQLLEN* index);

	bool			BindCol(int32_t columnIndex, bool* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, float* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, double* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, int8_t* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, int16_t* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, int32_t* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, int64_t* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, WCHAR* str, int32_t size, SQLLEN* index);
	bool			BindCol(int32_t columnIndex, BYTE* bin, int32_t size, SQLLEN* index);

	bool			BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	bool			BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);

private:
	void			HandleError(SQLRETURN ret);

private:
	SQLHENV			_environment = SQL_NULL_HANDLE;
	SQLHDBC			_connection = SQL_NULL_HANDLE;
	SQLHSTMT		_statement = SQL_NULL_HANDLE;
};

