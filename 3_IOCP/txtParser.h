
#pragma once

#include <Windows.h>
#include <vector>


/*

	텍스트 파일 또는 엑셀 텍스트 파일 작성 시 주의 사항
	1. 인코딩은 UTF-16LE 또는 UTF-8(BOM)이 권장됨
	2. 각 변수 또는 값들의 문자 크기는 255자를 초과해서는 안됨
	(ANSI와 UTF-8은 비정상 작동 가능성이 존재함)
	3. 텍스트 파일은 '변수 = 값'의 형태로 작성해야 함
	4. 엑셀은 .txt 또는 .csv로 저장해야 하며, 내부에서 사용될 테이블은 m * n 형태여야 함

----------
텍스트 파일 작성 예시

	Version = 015
	ServerID = 0
	ServerIP = "0.0.0.0"
	ServerPort = 4200
	WorkerThread = 3		// 최대 스레드 수

	// 최대 유저 수
	MaxUser = 300


----------
코드 작성 예시

	cTxtParser prs;
	if (!prs.LoadFile("Setting.txt"))
	{
		printf("NO file Exist!\n");
		return 1;
	}

	int version;
	prs.GetValue("Version", &version);
	DWORD id;
	prs.GetValue("ServerID", &id);
	char ip[25] = { 0, };
	prs.GetString("ServerIP", ip, 25);


	cExcelTxtParser prs2;
	if(!prs2.LoadFile(L"table.csv"))
	{ ... }

	prs2.SelectRow(3);
	int value;
	for(int i = 0; i < 5; ++i)
	{
		prs2.NextColumn();
		prs2.GetCurPosValue(&value);
	}


*/


namespace txtParser
{
	constexpr int MAX_STRING = 256;
	enum ErrorInfo			// GetLastPrsError 참고
	{
		SUCCESS = 0,
		FILE_NOT_FOUND,
		FILE_SHARE_VIOLATION,
		TOO_BIG_FILE_SIZE,
		TOO_SMALL_FILE_SIZE,
		INVALID_ENCODING,

		TOO_LONG_VAR_NAME,
		NOT_MATCH_VAR_NAME,
		NO_EQUAL_SIGN,
		NO_VAR_DETAIL,

		SMALL_BUF_SIZE,
		WRONG_COMMENT_SYMBOL,

		END_OF_FILE,

		// 라이브러리에서 핸들링 할 수 없는 그 외 윈도우 에러 코드
		UNEXPECTED_ERROR = 255,
	};


	class cTxtParser
	{
	public:
		cTxtParser();
		~cTxtParser();

	public:
		BOOL LoadFile(LPCWSTR pFileName);
		BOOL LoadFile(LPCSTR pFileName);

		VOID CloseFile();

		BOOL GetValue(LPCWSTR pItemName, bool* pValue);
		BOOL GetValue(LPCSTR pItemName, bool* pValue);

		BOOL GetValue(LPCWSTR pItemName, PSHORT pValue);
		BOOL GetValue(LPCSTR pItemName, PSHORT pValue);

		BOOL GetValue(LPCWSTR pItemName, PINT pValue);
		BOOL GetValue(LPCSTR pItemName, PINT pValue);

		BOOL GetValue(LPCWSTR pItemName, PINT64 pValue);
		BOOL GetValue(LPCSTR pItemName, PINT64 pValue);

		BOOL GetValue(LPCWSTR pItemName, PFLOAT pValue);
		BOOL GetValue(LPCSTR pItemName, PFLOAT pValue);

		BOOL GetString(LPCWSTR pItemName, PWCHAR pBuffer, INT bufSize);
		BOOL GetString(LPCSTR pItemName, PCHAR pBuffer, INT bufSize);

	public:
		DWORD GetLastPrsError() const { return _curError; }

	private:
		// 파일의 인코딩에 맞춰 BOM 제거 및 WCHAR 형태로 복사함
		BOOL CopyToMemberBuffer(HANDLE hFile, DWORD fileSize);

		// 읽은 파일이 ANSI 형식인지, UTF-8 형식인지 구분함
		BOOL IsUTF8(PCHAR pBuf, INT bufSize);

		// 포인터를 단어의 시작 부분으로 옮기고, 그 단어의 길이를 알려줌
		BOOL GetNextWordPtr(PWCHAR* ppReadPos, INT* pLength);

		// 단어로 사용한 문자까지의 길이를 구함(숫자, 영어, 한글, '.')
		INT IsValidCharacter(PWCHAR pRead) const;

	private:
		PWCHAR _pFileStart;

		DWORD _curError;
		DWORD _fileSize;
	};


	// ====================
	// ExcelTxtParser

	class cExcelTxtParser
	{
	public:
		cExcelTxtParser();
		~cExcelTxtParser();

	public:
		BOOL LoadFile(LPCWSTR pFileName);
		BOOL LoadFile(LPCSTR pFileName);

		BOOL SelectRow(DWORD destRow);
		VOID PrevRow();
		VOID NextRow();

		BOOL SelectColumn(DWORD destCol);
		VOID PrevColumn();
		VOID NextColumn();

		BOOL GetCurPosValue(PINT value);

		DWORD GetLastPrsError() const { return _curError; }

	private:
		// 파일의 인코딩에 맞춰 BOM 제거 및 WCHAR 형태로 복사함
		BOOL CopyToMemberBuffer(HANDLE hFile, DWORD fileSize);

		// 읽은 파일이 ANSI 형식인지, UTF-8 형식인지 구분함
		BOOL IsUTF8(PCHAR pBuf, INT bufSize);

		// 읽어들인 파일의 행과 열의 개수를 구함
		VOID CountMaxRowAndColumn();

		// 읽어들인 파일 내용을 파싱하여 vector에 저장
		BOOL PushWordToArray();

		// 포인터를 단어의 시작 부분으로 옮기고, 그 단어의 길이를 알려줌
		BOOL GetNextWordPtr(PWCHAR* ppReadPos, INT* pLength);

		// 단어로 사용한 문자까지의 길이를 구함(숫자, 영어, 한글)
		INT IsValidCharacter(PWCHAR ppReadPos) const;

	private:
		PWCHAR _pFileStart;			// 이 멤버 없는 방식 고려
		std::vector<std::wstring> _wordArr;

		DWORD _curError;
		DWORD _fileSize;

		DWORD _rowMax;
		DWORD _colMax;

		DWORD _curRow;
		DWORD _curCol;
	};
}