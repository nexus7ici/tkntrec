// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
#define NOMINMAX
// Windows �w�b�_�[ �t�@�C��:
#include <windows.h>
#include <commctrl.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Ws2_32.lib")

// C �����^�C�� �w�b�_�[ �t�@�C��
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

static inline FILE* secure_wfopen(const wchar_t* name, const wchar_t* mode)
{
	FILE* fp;
	return _wfopen_s(&fp, name, mode) == 0 ? fp : NULL;
}

static inline FILE* shared_wfopen(const wchar_t* name, const wchar_t* mode)
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
	return _wfopen(name, mode);
#ifdef __clang__
#pragma clang diagnostic pop
#else
#pragma warning(pop)
#endif
}

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă��������B

#if defined(_UNICODE) && defined(OutputDebugString)
#undef OutputDebugString
#define OutputDebugString OutputDebugStringWrapper
// OutputDebugStringW�̃��b�p�[�֐�
// API�t�b�N�ɂ�鍂�x�Ȃ��̂łȂ��P�Ȃ�u���BOutputDebugStringA��DLL����̌Ăяo���̓��b�v����Ȃ�
void OutputDebugStringWrapper(LPCWSTR lpOutputString);
#endif

#include "../../Common/Common.h"
