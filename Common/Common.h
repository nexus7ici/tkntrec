#pragma once

// ���ׂẴv���W�F�N�g�ɓK�p�����ǉ��w�b�_����ђ�`

#include <string>
#include <utility>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include <tchar.h>
#include <stdarg.h>
#include <sal.h>

using std::min;
using std::max;
using std::string;
using std::wstring;
using std::pair;
using std::map;
using std::multimap;
using std::vector;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#pragma clang diagnostic ignored "-Wunused-parameter"
#else
// 'identifier': unreferenced formal parameter
#pragma warning(disable : 4100)

#if defined(_MSC_VER) && _MSC_VER < 1900
// 'class': assignment operator was implicitly defined as deleted
#pragma warning(disable : 4512)
#endif
#endif

// �K�؂łȂ�NULL�̌��o�p
//#undef NULL
//#define NULL nullptr

#define PRINTF_FORMAT_SZ _In_z_ _Printf_format_string_

inline void _OutputDebugString(PRINTF_FORMAT_SZ const TCHAR* format, ...)
{
	// TODO: ���̊֐����͗\�񖼈ᔽ�̏�ɕ���킵���̂ŕύX���ׂ�
	va_list params;
	va_start(params, format);
	int length = _vsctprintf(format, params);
	va_end(params);
	if( length >= 0 ){
		TCHAR* buff = new TCHAR[length + 1];
		va_start(params, format);
		_vstprintf_s(buff, length + 1, format, params);
		va_end(params);
		OutputDebugString(buff);
		delete[] buff;
	}
}
