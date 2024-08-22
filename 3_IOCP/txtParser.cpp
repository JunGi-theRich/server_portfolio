
#include <iostream>
#include <string>

#include "txtParser.h"
using namespace txtParser;

cTxtParser::cTxtParser()
{
	_pFileStart = NULL;

	_curError = NULL;
	_fileSize = NULL;
}
cTxtParser::~cTxtParser()
{
	//delete _pFileStart;
}

VOID cTxtParser::CloseFile()
{
	delete _pFileStart;
}

BOOL cTxtParser::LoadFile(LPCWSTR pFileName)
{
	HANDLE hFile;
	hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		if (ERROR_FILE_NOT_FOUND == GetLastError())
		{
			_curError = ErrorInfo::FILE_NOT_FOUND;
		}
		else
		{
			_curError = ErrorInfo::UNEXPECTED_ERROR;
		}
		return FALSE;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize))
	{
		_curError = ErrorInfo::UNEXPECTED_ERROR;
		return FALSE;
	}
	if (0 != fileSize.HighPart)
	{
		_curError = ErrorInfo::TOO_BIG_FILE_SIZE;
		return FALSE;
	}
	if (4 > fileSize.LowPart)
	{
		_curError = ErrorInfo::TOO_SMALL_FILE_SIZE;
		return FALSE;
	}

	if (!CopyToMemberBuffer(hFile, fileSize.LowPart))
	{
		return FALSE;
	}

	CloseHandle(hFile);
	return TRUE;
}
BOOL cTxtParser::LoadFile(LPCSTR pFileName)
{
	PWCHAR pWideFileName;
	INT bufferLen;

	// 1. WCHAR 문자열을 담는데에 필요한 공간 구하기
	bufferLen = MultiByteToWideChar(CP_ACP, 0, pFileName, -1, NULL, 0);

	// 2. 필요한 메모리 공간 할당 후에 WCHAR 문자열로 변경
	pWideFileName = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pFileName, -1, pWideFileName, bufferLen);

	// 3. WideChar 형태로 수행되는 함수 호출
	if (LoadFile(pWideFileName))
	{
		delete[] pWideFileName;
		return TRUE;
	}
	else
	{
		delete[] pWideFileName;
		return FALSE;
	}
}

BOOL cTxtParser::GetValue(LPCWSTR pItemName, bool* pValue)
{
	INT wBufLen = (INT)wcslen(pItemName);
	if (MAX_STRING <= wBufLen)
	{
		_curError = ErrorInfo::TOO_LONG_VAR_NAME;
		return FALSE;
	}

	PWCHAR pRead = _pFileStart;
	WCHAR item[MAX_STRING];
	INT wordLength;
	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			// 정해진 버퍼 크기 넘어서 읽어버린 상황
			_curError = ErrorInfo::NOT_MATCH_VAR_NAME;
			return FALSE;
		}
		wmemset(item, 0, MAX_STRING / 2);
		wmemcpy(item, pRead, wordLength);
		if (0 == wcscmp(pItemName, item))
		{
			pRead += wordLength;
			if (GetNextWordPtr(&pRead, &wordLength))
			{
				if (*pRead == L'=')
				{
					pRead += 1;
					if (GetNextWordPtr(&pRead, &wordLength))
					{
						wmemset(item, 0, MAX_STRING / 2);
						wmemcpy(item, pRead, wordLength);
						if (0 == _wtoi(item))
						{
							*pValue = FALSE;
						}
						else
						{
							*pValue = TRUE;
						}
						return TRUE;
					}
					_curError = ErrorInfo::NO_VAR_DETAIL;
					return FALSE;
				}
			}
			_curError = ErrorInfo::NO_EQUAL_SIGN;
			return FALSE;
		}
		++pRead;
	}
	return FALSE;
}
BOOL cTxtParser::GetValue(LPCSTR pItemName, bool* pValue)
{
	PWCHAR pWideBuffer;
	INT bufferLen;

	bufferLen = MultiByteToWideChar(CP_ACP, 0, pItemName, -1, NULL, 0);
	pWideBuffer = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pItemName, -1, pWideBuffer, bufferLen);

	if (GetValue(pWideBuffer, pValue))
	{
		delete[] pWideBuffer;
		return TRUE;
	}
	else
	{
		delete[] pWideBuffer;
		return FALSE;
	}
}

BOOL cTxtParser::GetValue(LPCWSTR pItemName, PSHORT pValue)
{
	INT wBufLen = (INT)wcslen(pItemName);
	if (MAX_STRING <= wBufLen)
	{
		_curError = ErrorInfo::TOO_LONG_VAR_NAME;
		return FALSE;
	}

	PWCHAR pRead = _pFileStart;
	WCHAR item[MAX_STRING];
	INT wordLength;
	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			// 정해진 버퍼 크기 넘어서 읽어버린 상황
			_curError = ErrorInfo::NOT_MATCH_VAR_NAME;
			return FALSE;
		}
		wmemset(item, 0, MAX_STRING / 2);
		wmemcpy(item, pRead, wordLength);
		if (0 == wcscmp(pItemName, item))
		{
			pRead += wordLength;
			if (GetNextWordPtr(&pRead, &wordLength))
			{
				if (*pRead == L'=')
				{
					pRead += 1;
					if (GetNextWordPtr(&pRead, &wordLength))
					{
						wmemset(item, 0, MAX_STRING / 2);
						wmemcpy(item, pRead, wordLength);
						*pValue = (SHORT)_wtoi(item);
						return TRUE;
					}
					_curError = ErrorInfo::NO_VAR_DETAIL;
					return FALSE;
				}
			}
			_curError = ErrorInfo::NO_EQUAL_SIGN;
			return FALSE;
		}
		++pRead;
	}
	return FALSE;
}
BOOL cTxtParser::GetValue(LPCSTR pItemName, PSHORT pValue)
{
	PWCHAR pWideBuffer;
	INT bufferLen;

	bufferLen = MultiByteToWideChar(CP_ACP, 0, pItemName, -1, NULL, 0);
	pWideBuffer = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pItemName, -1, pWideBuffer, bufferLen);

	if (GetValue(pWideBuffer, pValue))
	{
		delete[] pWideBuffer;
		return TRUE;
	}
	else
	{
		delete[] pWideBuffer;
		return FALSE;
	}
}

BOOL cTxtParser::GetValue(LPCWSTR pItemName, PINT pValue)
{
	INT wBufLen = (INT)wcslen(pItemName);
	if (MAX_STRING <= wBufLen)
	{
		_curError = ErrorInfo::TOO_LONG_VAR_NAME;
		return FALSE;
	}

	PWCHAR pRead = _pFileStart;
	WCHAR item[MAX_STRING];
	INT wordLength;
	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			// 정해진 버퍼 크기 넘어서 읽어버린 상황
			_curError = ErrorInfo::NOT_MATCH_VAR_NAME;
			return FALSE;
		}
		wmemset(item, 0, MAX_STRING / 2);
		wmemcpy(item, pRead, wordLength);
		if (0 == wcscmp(pItemName, item))
		{
			pRead += wordLength;
			if (GetNextWordPtr(&pRead, &wordLength))
			{
				if (*pRead == L'=')
				{
					pRead += 1;
					if (GetNextWordPtr(&pRead, &wordLength))
					{
						wmemset(item, 0, MAX_STRING / 2);
						wmemcpy(item, pRead, wordLength);
						*pValue = _wtoi(item);
						return TRUE;
					}
					_curError = ErrorInfo::NO_VAR_DETAIL;
					return FALSE;
				}
			}
			_curError = ErrorInfo::NO_EQUAL_SIGN;
			return FALSE;
		}
		++pRead;
	}
	return FALSE;
}
BOOL cTxtParser::GetValue(LPCSTR pItemName, PINT pValue)
{
	PWCHAR pWideBuffer;
	INT bufferLen;

	bufferLen = MultiByteToWideChar(CP_ACP, 0, pItemName, -1, NULL, 0);
	pWideBuffer = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pItemName, -1, pWideBuffer, bufferLen);

	if (GetValue(pWideBuffer, pValue))
	{
		delete[] pWideBuffer;
		return TRUE;
	}
	else
	{
		delete[] pWideBuffer;
		return FALSE;
	}
}

BOOL cTxtParser::GetValue(LPCWSTR pItemName, PINT64 pValue)
{
	INT wBufLen = (INT)wcslen(pItemName);
	if (MAX_STRING <= wBufLen)
	{
		_curError = ErrorInfo::TOO_LONG_VAR_NAME;
		return FALSE;
	}

	PWCHAR pRead = _pFileStart;
	WCHAR item[MAX_STRING];
	INT wordLength;
	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			// 정해진 버퍼 크기 넘어서 읽어버린 상황
			_curError = ErrorInfo::NOT_MATCH_VAR_NAME;
			return FALSE;
		}
		wmemset(item, 0, MAX_STRING / 2);
		wmemcpy(item, pRead, wordLength);
		if (0 == wcscmp(pItemName, item))
		{
			pRead += wordLength;
			if (GetNextWordPtr(&pRead, &wordLength))
			{
				if (*pRead == L'=')
				{
					pRead += 1;
					if (GetNextWordPtr(&pRead, &wordLength))
					{
						wmemset(item, 0, MAX_STRING / 2);
						wmemcpy(item, pRead, wordLength);
						*pValue = _wtoi64(item);
						return TRUE;
					}
					_curError = ErrorInfo::NO_VAR_DETAIL;
					return FALSE;
				}
			}
			_curError = ErrorInfo::NO_EQUAL_SIGN;
			return FALSE;
		}
		++pRead;
	}
	return FALSE;
}
BOOL cTxtParser::GetValue(LPCSTR pItemName, PINT64 pValue)
{
	PWCHAR pWideBuffer;
	INT bufferLen;

	bufferLen = MultiByteToWideChar(CP_ACP, 0, pItemName, -1, NULL, 0);
	pWideBuffer = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pItemName, -1, pWideBuffer, bufferLen);

	if (GetValue(pWideBuffer, pValue))
	{
		delete[] pWideBuffer;
		return TRUE;
	}
	else
	{
		delete[] pWideBuffer;
		return FALSE;
	}
}

BOOL cTxtParser::GetValue(LPCWSTR pItemName, PFLOAT pValue)
{
	INT wBufLen = (INT)wcslen(pItemName);
	if (MAX_STRING <= wBufLen)
	{
		_curError = ErrorInfo::TOO_LONG_VAR_NAME;
		return FALSE;
	}

	PWCHAR pRead = _pFileStart;
	WCHAR item[MAX_STRING];
	INT wordLength;
	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			// 정해진 버퍼 크기 넘어서 읽어버린 상황
			_curError = ErrorInfo::NOT_MATCH_VAR_NAME;
			return FALSE;
		}
		wmemset(item, 0, MAX_STRING / 2);
		wmemcpy(item, pRead, wordLength);
		if (0 == wcscmp(pItemName, item))
		{
			pRead += wordLength;
			if (GetNextWordPtr(&pRead, &wordLength))
			{
				if (*pRead == L'=')
				{
					pRead += 1;
					if (GetNextWordPtr(&pRead, &wordLength))
					{
						wmemset(item, 0, MAX_STRING / 2);
						wmemcpy(item, pRead, wordLength);
						*pValue = (FLOAT)_wtof(item);
						return TRUE;
					}
					_curError = ErrorInfo::NO_VAR_DETAIL;
					return FALSE;
				}
			}
			_curError = ErrorInfo::NO_EQUAL_SIGN;
			return FALSE;
		}
		++pRead;
	}
	return FALSE;
}
BOOL cTxtParser::GetValue(LPCSTR pItemName, PFLOAT pValue)
{
	PWCHAR pWideBuffer;
	INT bufferLen;

	bufferLen = MultiByteToWideChar(CP_ACP, 0, pItemName, -1, NULL, 0);
	pWideBuffer = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pItemName, -1, pWideBuffer, bufferLen);

	if (GetValue(pWideBuffer, pValue))
	{
		delete[] pWideBuffer;
		return TRUE;
	}
	else
	{
		delete[] pWideBuffer;
		return FALSE;
	}
}

BOOL cTxtParser::GetString(LPCWSTR pItemName, PWCHAR pBuffer, INT bufSize)
{
	INT wBufLen = (INT)wcslen(pItemName);
	if (MAX_STRING <= wBufLen)
	{
		_curError = ErrorInfo::TOO_LONG_VAR_NAME;
		return FALSE;
	}

	PWCHAR pRead = _pFileStart;
	WCHAR item[MAX_STRING];
	INT wordLength;
	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			_curError = ErrorInfo::NOT_MATCH_VAR_NAME;
			return FALSE;
		}
		wmemset(item, 0, MAX_STRING / 2);
		wmemcpy(item, pRead, wordLength);
		if (0 == wcscmp(pItemName, item))
		{
			pRead += wordLength;
			if (GetNextWordPtr(&pRead, &wordLength))
			{
				if (*pRead == L'=')
				{
					pRead += 1;
					if (GetNextWordPtr(&pRead, &wordLength))
					{
						if (bufSize <= wordLength)
						{
							_curError = ErrorInfo::SMALL_BUF_SIZE;
							return FALSE;
						}

						wmemcpy(pBuffer, pRead, wordLength);
						return TRUE;
					}
					_curError = ErrorInfo::NO_VAR_DETAIL;
					return FALSE;
				}
			}
			_curError = ErrorInfo::NO_EQUAL_SIGN;
			return FALSE;
		}
		++pRead;
	}
	return FALSE;
}
BOOL cTxtParser::GetString(LPCSTR pItemName, PCHAR pBuffer, INT bufSize)
{
	PWCHAR pWideBufferName;
	PWCHAR pWideBufferValue;
	int bufferLen;

	bufferLen = MultiByteToWideChar(CP_ACP, 0, pItemName, -1, NULL, 0);
	pWideBufferName = new WCHAR[bufferLen];
	MultiByteToWideChar(CP_ACP, 0, pItemName, -1, pWideBufferName, bufferLen);

	pWideBufferValue = new WCHAR[bufSize];
	wmemset(pWideBufferValue, 0, bufSize);

	if(GetString(pWideBufferName, pWideBufferValue, bufSize))
	{
		WideCharToMultiByte(CP_ACP, 0, pWideBufferValue, -1, pBuffer, bufSize, NULL, NULL);
		delete[] pWideBufferName;
		delete[] pWideBufferValue;
		return TRUE;
	}
	else
	{
		delete[] pWideBufferName;
		delete[] pWideBufferValue;
		return FALSE;
	}
}

BOOL cTxtParser::CopyToMemberBuffer(HANDLE hFile, DWORD fileSize)
{
	PCHAR pTmp = new CHAR[fileSize];
	DWORD readBytes;
	if (!ReadFile(hFile, pTmp, fileSize, &readBytes, NULL))
	{
		_curError = ErrorInfo::UNEXPECTED_ERROR;
		return FALSE;
	}

	UCHAR utf16_BE[] = { 0xFE, 0xFF };
	UCHAR utf16_LE[] = { 0xFF, 0xFE };
	UCHAR utf8_BOM[] = { 0xEF, 0xBB, 0xBF };

	if ((UCHAR)pTmp[0] == utf16_BE[0] && (UCHAR)pTmp[1] == utf16_BE[1])
	{
		_curError = ErrorInfo::INVALID_ENCODING;
		delete[] pTmp;
		return FALSE;
	}
	else if ((UCHAR)pTmp[0] == utf16_LE[0] && (UCHAR)pTmp[1] == utf16_LE[1])
	{
		fileSize -= 2;		// BOM 제외
		pTmp += 2;

		_pFileStart = new WCHAR[fileSize / 2];
		_fileSize = (DWORD)_msize(_pFileStart);
		memcpy(_pFileStart, pTmp, _fileSize);

		delete[] pTmp;
		return TRUE;
	}
	else if ((UCHAR)pTmp[0] == utf8_BOM[0] && (UCHAR)pTmp[1] == utf8_BOM[1] && (UCHAR)pTmp[2] == utf8_BOM[2])
	{
		fileSize -= 3;		// BOM 제외
		pTmp += 3;

		DWORD wBufLen = MultiByteToWideChar(CP_UTF8, 0, pTmp, fileSize, NULL, 0);
		_pFileStart = new WCHAR[wBufLen];
		_fileSize = (DWORD)_msize(_pFileStart);
		MultiByteToWideChar(CP_UTF8, 0, pTmp, fileSize, _pFileStart, wBufLen);

		delete[] pTmp;
		return TRUE;
	}
	else
	{
		if (IsUTF8(pTmp, fileSize))
		{
			DWORD wBufLen = MultiByteToWideChar(CP_UTF8, 0, pTmp, fileSize, NULL, 0);
			_pFileStart = new WCHAR[wBufLen];
			_fileSize = (DWORD)_msize(_pFileStart);
			MultiByteToWideChar(CP_UTF8, 0, pTmp, fileSize, _pFileStart, wBufLen);

			delete[] pTmp;
			return TRUE;
		}
		else
		{
			DWORD wBufLen = MultiByteToWideChar(CP_ACP, 0, pTmp, fileSize, NULL, 0);
			_pFileStart = new WCHAR[wBufLen];
			_fileSize = (DWORD)_msize(_pFileStart);
			MultiByteToWideChar(CP_ACP, 0, pTmp, fileSize, _pFileStart, wBufLen);

			delete[] pTmp;
			return TRUE;
		}
	}
}

BOOL cTxtParser::IsUTF8(PCHAR pBuf, INT bufSize)
{
	// 1. 현재 버퍼가 가리키는 포인터가 한글을 만날때까지 이동함
	// (값이 0x80보다 작다면 포인터는 계속 이동함)
	// 2. CP949 라면 첫 번째 바이트는 0x81~0xC6 사이를 할당받고
	// 두 번째 바이트는 0x41~0x5A, 0x61~0x7A, 0x81~0xFE를 할당받음
	// 3. UTF-8은 한글이 3바이트로 표현되며, 0xE3 84 B1 ~ 0xE3 85 A3 (자음과 모음)
	// 또는 0xEA B0 80 ~ 0xED 9E A3의 값을 할당 받음

	BOOL bUTF8 = FALSE;
	PCHAR pStart = pBuf;
	PCHAR pEnd = pBuf + bufSize;
	while (pStart < pEnd)
	{
		if (*(UCHAR*)pStart < 0x80)
		{
			++pStart;
		}
		else if (*(UCHAR*)pStart == 0xE3)
		{
			if (2 < pEnd - pStart)
			{
				DWORD low = (0xE3 << 16) + (0x84 << 8) + 0xB1;
				DWORD high = (0xE3 << 16) + (0x85 << 8) + 0xA3;
				DWORD cmp = ((UCHAR)pStart[0] << 16) + ((UCHAR)pStart[1] << 8) + (UCHAR)pStart[2];
				if (cmp >= low && cmp <= high)
				{
					bUTF8 = TRUE;
					break;
				}
			}
			pStart += 3;
		}
		else if ((*(UCHAR*)pStart >= 0xEA && *(UCHAR*)pStart <= 0xED))
		{
			if (2 < pEnd - pStart)
			{
				DWORD low = (0xEA << 16) + (0xB0 << 8) + 0x80;
				DWORD high = (0xED << 16) + (0x9E << 8) + 0xA3;
				DWORD cmp = ((UCHAR)pStart[0] << 16) + ((UCHAR)pStart[1] << 8) + (UCHAR)pStart[2];
				if (cmp >= low && cmp <= high)
				{
					bUTF8 = TRUE;
					break;
				}
			}
			pStart += 3;
		}
		else
		{
			break;
		}
	}
	return bUTF8;
}

BOOL cTxtParser::GetNextWordPtr(PWCHAR* ppReadPos, INT* pLength)
{
	while (1)
	{
		if (**ppReadPos == L',' || **ppReadPos == L'"' || **ppReadPos == L' ' ||
			**ppReadPos == 0x08 || **ppReadPos == 0x09 || 
			**ppReadPos == 0x0A || **ppReadPos == 0x0D)
		{
			++(*ppReadPos);
		}
		else if (**ppReadPos == L'/')
		{
			++*ppReadPos;
			if (**ppReadPos == L'/')
			{
				while (1)
				{
					if (**ppReadPos == 0x0D && *(*ppReadPos + 1) == 0x0A)
					{
						*ppReadPos += 2;
						break;
					}
					++(*ppReadPos);
				}
			}
			else if (**ppReadPos == L'*')
			{
				while (1)
				{
					if (**ppReadPos == L'*' && *(*ppReadPos + 1) == L'/')
					{
						*ppReadPos += 2;
						break;
					}
					++(*ppReadPos);
				}
			}
			else
			{
				_curError = ErrorInfo::WRONG_COMMENT_SYMBOL;
				return FALSE;
			}
		}
		else
		{
			// 단어를 찾은 상황으로 단어의 끝을 찾아 길이를 구함
			*pLength = IsValidCharacter(*ppReadPos);
			return TRUE;
		}
	}
}

INT cTxtParser::IsValidCharacter(PWCHAR pRead) const
{
	INT retVal = 0;
	while (1)
	{
		if ((*pRead >= 0x00 && *pRead <= 0x2D) || *pRead == 0x2F ||
			(*pRead >= 0x3A && *pRead <= 0x40) ||
			(*pRead >= 0x5B && *pRead <= 0x60) ||
			(*pRead >= 0x7B && *pRead <= 0x7F))
		{
			break;
		}
		else
		{
			++retVal;
			++pRead;
		}
	}
	return retVal;
}


// ====================
// ExcelTxtParser

cExcelTxtParser::cExcelTxtParser()
{
	_pFileStart = NULL;

	_curError = NULL;
	_fileSize = NULL;

	_rowMax = 1;
	_colMax = 1;

	_curRow = 0;
	_curCol = 0;
}
cExcelTxtParser::~cExcelTxtParser()
{
	delete _pFileStart;
}

BOOL cExcelTxtParser::LoadFile(LPCWSTR pFileName)
{
	HANDLE hFile;
	hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		if (ERROR_FILE_NOT_FOUND == GetLastError())
		{
			_curError = ErrorInfo::FILE_NOT_FOUND;
		}
		else if (ERROR_SHARING_VIOLATION == GetLastError())
		{
			_curError = ErrorInfo::FILE_SHARE_VIOLATION;
		}
		else
		{
			_curError = ErrorInfo::UNEXPECTED_ERROR;
		}
		return FALSE;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize))
	{
		_curError = ErrorInfo::UNEXPECTED_ERROR;
		return FALSE;
	}
	if (0 != fileSize.HighPart)
	{
		_curError = ErrorInfo::TOO_BIG_FILE_SIZE;
		return FALSE;
	}
	if (4 > fileSize.LowPart)
	{
		_curError = ErrorInfo::TOO_SMALL_FILE_SIZE;
		return FALSE;
	}

	if (!CopyToMemberBuffer(hFile, fileSize.LowPart))
	{
		return FALSE;
	}

	CountMaxRowAndColumn();
	PushWordToArray();

	CloseHandle(hFile);
	return TRUE;
}
BOOL cExcelTxtParser::LoadFile(LPCSTR pFileName)
{
	PWCHAR pWideFileName;
	int bufferLen;

	// 1. WCHAR 문자열을 담는데에 필요한 공간 구하기
	bufferLen = MultiByteToWideChar(CP_ACP, 0, pFileName, -1, NULL, 0);

	// 2. 필요한 메모리 공간 할당 후에 WCHAR 문자열로 변경
	pWideFileName = new WCHAR[bufferLen * sizeof(WCHAR)];
	MultiByteToWideChar(CP_ACP, 0, pFileName, -1, pWideFileName, bufferLen);

	// 3. WideChar 형태로 수행되는 함수 호출
	if (LoadFile(pWideFileName))
	{
		delete[] pWideFileName;
		return TRUE;
	}
	else
	{
		delete[] pWideFileName;
		return FALSE;
	}
}

BOOL cExcelTxtParser::SelectRow(DWORD destRow)
{
	while (1)
	{
		if (_rowMax <= _curRow)
		{
			_curError = ErrorInfo::END_OF_FILE;
			return FALSE;
		}
		else if (destRow > _curRow)
		{
			++_curRow;
		}
		else if (destRow < _curRow)
		{
			--_curRow;
		}
		else
		{
			return TRUE;
		}
	}
}
VOID cExcelTxtParser::PrevRow()
{
	if (0 < _curRow)
	{
		--_curRow;
	}
}
VOID cExcelTxtParser::cExcelTxtParser::NextRow()
{
	if (_rowMax - 1 > _curRow)
	{
		++_curRow;
	}
}

BOOL cExcelTxtParser::SelectColumn(DWORD destCol)
{
	while (1)
	{
		if (_colMax <= _curCol)
		{
			_curError = ErrorInfo::END_OF_FILE;
			return FALSE;
		}
		else if (destCol > _curCol)
		{
			++_curCol;
		}
		else if (destCol < _curCol)
		{
			--_curCol;
		}
		else
		{
			return TRUE;
		}
	}
}
VOID cExcelTxtParser::PrevColumn()
{
	if (0 < _curCol)
	{
		--_curCol;
	}
}
VOID cExcelTxtParser::NextColumn()
{
	if (_colMax - 1 > _curCol)
	{
		++_curCol;
	}
}

BOOL cExcelTxtParser::GetCurPosValue(PINT value)
{
	if (_curError == ErrorInfo::END_OF_FILE)
	{
		return FALSE;
	}

	DWORD idx = _colMax * _curRow + _curCol;
	if (idx >= _rowMax * _colMax)
	{
		_curError = ErrorInfo::UNEXPECTED_ERROR;
		return FALSE;
	}
	*value = _wtoi(_wordArr[idx].c_str());
	return TRUE;
}

BOOL cExcelTxtParser::CopyToMemberBuffer(HANDLE hFile, DWORD fileSize)
{
	// 엑셀을 csv 또는 txt로 저장했을 경우 ANSI 또는 UTF8 인코딩임을 기대함
	PCHAR pTmp = new CHAR[fileSize];
	DWORD readBytes;
	if (!ReadFile(hFile, pTmp, fileSize, &readBytes, NULL))
	{
		_curError = ErrorInfo::UNEXPECTED_ERROR;
		return FALSE;
	}

	if (IsUTF8(pTmp, fileSize))
	{
		DWORD wBufLen = MultiByteToWideChar(CP_UTF8, 0, pTmp, fileSize, NULL, 0);
		_pFileStart = new WCHAR[wBufLen];
		_fileSize = (DWORD)_msize(_pFileStart);
		MultiByteToWideChar(CP_UTF8, 0, pTmp, fileSize, _pFileStart, wBufLen);

		delete[] pTmp;
		return TRUE;
	}
	else
	{
		DWORD wBufLen = MultiByteToWideChar(CP_ACP, 0, pTmp, fileSize, NULL, 0);
		_pFileStart = new WCHAR[wBufLen];
		_fileSize = (DWORD)_msize(_pFileStart);
		MultiByteToWideChar(CP_ACP, 0, pTmp, fileSize, _pFileStart, wBufLen);

		delete[] pTmp;
		return TRUE;
	}
}

BOOL cExcelTxtParser::IsUTF8(PCHAR pBuf, INT bufSize)
{
	BOOL bUTF8 = FALSE;
	PCHAR pStart = pBuf;
	PCHAR pEnd = pBuf + bufSize;
	while (pStart < pEnd)
	{
		if (*(UCHAR*)pStart < 0x80)
		{
			++pStart;
		}
		else if (*(UCHAR*)pStart == 0xE3)
		{
			if (2 < pEnd - pStart)
			{
				DWORD low = (0xE3 << 16) + (0x84 << 8) + 0xB1;
				DWORD high = (0xE3 << 16) + (0x85 << 8) + 0xA3;
				DWORD cmp = ((UCHAR)pStart[0] << 16) + ((UCHAR)pStart[1] << 8) + (UCHAR)pStart[2];
				if (cmp >= low && cmp <= high)
				{
					bUTF8 = TRUE;
					break;
				}
			}
			pStart += 3;
		}
		else if ((*(UCHAR*)pStart >= 0xEA && *(UCHAR*)pStart <= 0xED))
		{
			if (2 < pEnd - pStart)
			{
				DWORD low = (0xEA << 16) + (0xB0 << 8) + 0x80;
				DWORD high = (0xED << 16) + (0x9E << 8) + 0xA3;
				DWORD cmp = ((UCHAR)pStart[0] << 16) + ((UCHAR)pStart[1] << 8) + (UCHAR)pStart[2];
				if (cmp >= low && cmp <= high)
				{
					bUTF8 = TRUE;
					break;
				}
			}
			pStart += 3;
		}
		else
		{
			break;
		}
	}
	return bUTF8;
}

VOID cExcelTxtParser::CountMaxRowAndColumn()
{
	PWCHAR pRead = _pFileStart;
	while (1)
	{
		if (_fileSize < pRead - _pFileStart)
		{
			break;
		}
		if (*pRead == 0x0D && *(pRead + 1) == 0x0A)
		{
			++_rowMax;
		}
		++pRead;
	}

	pRead = _pFileStart;
	while (1)
	{
		if (*pRead == 0x0D && *(pRead + 1) == 0x0A)
		{
			break;
		}
		else if (*pRead == L',' || *pRead == 0x09)
		{
			++_colMax;
		}
		++pRead;
	}

	// 엑셀로 저장한 파일의 가장 마지막은 0x0D 0x0A로 이루어짐
	// 따라서 _rowMax의 값을 하나 제거함
	--_rowMax;
}

BOOL cExcelTxtParser::PushWordToArray()
{
	PWCHAR pRead = _pFileStart;
	WCHAR inputStr[MAX_STRING];
	INT wordLength;

	while (GetNextWordPtr(&pRead, &wordLength))
	{
		if (pRead - _pFileStart > _fileSize)
		{
			return FALSE;
		}
		wmemset(inputStr, 0, MAX_STRING / 2);
		wmemcpy(inputStr, pRead, wordLength);
		if (inputStr[MAX_STRING / 2 - 1] != 0x00)
		{
			return FALSE;
		}
		pRead += wordLength;
		_wordArr.push_back(inputStr);
	}
	return TRUE;
}

BOOL cExcelTxtParser::GetNextWordPtr(PWCHAR* ppReadPos, INT* pLength)
{
	while (1)
	{
		if ((DWORD64)*ppReadPos - (DWORD64)_pFileStart >= _fileSize)
		{
			//_curError = ErrorInfo::END_OF_FILE;
			return FALSE;
		}
		else if (**ppReadPos == L',' || **ppReadPos == L'"' || **ppReadPos == L' ' ||
			**ppReadPos == 0x08 || **ppReadPos == 0x09 ||
			**ppReadPos == 0x0A || **ppReadPos == 0x0D)
		{
			++(*ppReadPos);
		}
		else if (**ppReadPos == L'/')
		{
			++*ppReadPos;
			if (**ppReadPos == L'/')
			{
				while (1)
				{
					if (**ppReadPos == 0x0D && *(*ppReadPos + 1) == 0x0A)
					{
						*ppReadPos += 2;
						break;
					}
					++(*ppReadPos);
				}
			}
			else if (**ppReadPos == L'*')
			{
				while (1)
				{
					if (**ppReadPos == L'*' && *(*ppReadPos + 1) == L'/')
					{
						*ppReadPos += 2;
						break;
					}
					++(*ppReadPos);
				}
			}
			else
			{
				_curError = ErrorInfo::WRONG_COMMENT_SYMBOL;
				return FALSE;
			}
		}
		else
		{
			// 단어를 찾은 상황으로 단어의 끝을 찾아 길이를 구함
			*pLength = IsValidCharacter(*ppReadPos);
			return TRUE;
		}
	}
}

INT cExcelTxtParser::IsValidCharacter(PWCHAR pRead) const
{
	INT retVal = 0;
	while (1)
	{
		if ((*pRead >= 0x00 && *pRead <= 0x2F) ||
			(*pRead >= 0x3A && *pRead <= 0x40) ||
			(*pRead >= 0x5B && *pRead <= 0x60) ||
			(*pRead >= 0x7B && *pRead <= 0x7F))
		{
			break;
		}
		else
		{
			++retVal;
			++pRead;
		}
	}
	return retVal;
}

