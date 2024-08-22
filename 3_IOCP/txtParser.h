
#pragma once

#include <Windows.h>
#include <vector>


/*

	�ؽ�Ʈ ���� �Ǵ� ���� �ؽ�Ʈ ���� �ۼ� �� ���� ����
	1. ���ڵ��� UTF-16LE �Ǵ� UTF-8(BOM)�� �����
	2. �� ���� �Ǵ� ������ ���� ũ��� 255�ڸ� �ʰ��ؼ��� �ȵ�
	(ANSI�� UTF-8�� ������ �۵� ���ɼ��� ������)
	3. �ؽ�Ʈ ������ '���� = ��'�� ���·� �ۼ��ؾ� ��
	4. ������ .txt �Ǵ� .csv�� �����ؾ� �ϸ�, ���ο��� ���� ���̺��� m * n ���¿��� ��

----------
�ؽ�Ʈ ���� �ۼ� ����

	Version = 015
	ServerID = 0
	ServerIP = "0.0.0.0"
	ServerPort = 4200
	WorkerThread = 3		// �ִ� ������ ��

	// �ִ� ���� ��
	MaxUser = 300


----------
�ڵ� �ۼ� ����

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
	enum ErrorInfo			// GetLastPrsError ����
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

		// ���̺귯������ �ڵ鸵 �� �� ���� �� �� ������ ���� �ڵ�
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
		// ������ ���ڵ��� ���� BOM ���� �� WCHAR ���·� ������
		BOOL CopyToMemberBuffer(HANDLE hFile, DWORD fileSize);

		// ���� ������ ANSI ��������, UTF-8 �������� ������
		BOOL IsUTF8(PCHAR pBuf, INT bufSize);

		// �����͸� �ܾ��� ���� �κ����� �ű��, �� �ܾ��� ���̸� �˷���
		BOOL GetNextWordPtr(PWCHAR* ppReadPos, INT* pLength);

		// �ܾ�� ����� ���ڱ����� ���̸� ����(����, ����, �ѱ�, '.')
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
		// ������ ���ڵ��� ���� BOM ���� �� WCHAR ���·� ������
		BOOL CopyToMemberBuffer(HANDLE hFile, DWORD fileSize);

		// ���� ������ ANSI ��������, UTF-8 �������� ������
		BOOL IsUTF8(PCHAR pBuf, INT bufSize);

		// �о���� ������ ��� ���� ������ ����
		VOID CountMaxRowAndColumn();

		// �о���� ���� ������ �Ľ��Ͽ� vector�� ����
		BOOL PushWordToArray();

		// �����͸� �ܾ��� ���� �κ����� �ű��, �� �ܾ��� ���̸� �˷���
		BOOL GetNextWordPtr(PWCHAR* ppReadPos, INT* pLength);

		// �ܾ�� ����� ���ڱ����� ���̸� ����(����, ����, �ѱ�)
		INT IsValidCharacter(PWCHAR ppReadPos) const;

	private:
		PWCHAR _pFileStart;			// �� ��� ���� ��� ���
		std::vector<std::wstring> _wordArr;

		DWORD _curError;
		DWORD _fileSize;

		DWORD _rowMax;
		DWORD _colMax;

		DWORD _curRow;
		DWORD _curCol;
	};
}