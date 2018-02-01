// RecName_Macro.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"


#include "RecName_PlugIn.h"
#include "SettingDlg.h"
#include "ConvertMacro2.h"
#include "../../Common/PathUtil.h"

#define PLUGIN_NAME L"�}�N�� PlugIn"

extern HINSTANCE g_instance;

//PlugIn�̖��O���擾����
//name��NULL���͕K�v�ȃT�C�Y��nameSize�ŕԂ�
//�ʏ�nameSize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// name						[OUT]����
// nameSize					[IN/OUT]name�̃T�C�Y(WCHAR�P��)
BOOL WINAPI GetPlugInName(
	WCHAR* name,
	DWORD* nameSize
	)
{
	if( name == NULL ){
		if( nameSize == NULL ){
			return FALSE;
		}else{
			*nameSize = (DWORD)wcslen(PLUGIN_NAME)+1;
		}
	}else{
		if( nameSize == NULL ){
			return FALSE;
		}else{
			if( *nameSize < (DWORD)wcslen(PLUGIN_NAME)+1 ){
				*nameSize = (DWORD)wcslen(PLUGIN_NAME)+1;
				return FALSE;
			}else{
				wcscpy_s(name, *nameSize, PLUGIN_NAME);
			}
		}
	}
	return TRUE;
}

//PlugIn�Őݒ肪�K�v�ȏꍇ�A�ݒ�p�̃_�C�A���O�Ȃǂ�\������
//�����F
// parentWnd				[IN]�e�E�C���h�E
void WINAPI Setting(
	HWND parentWnd
	)
{
	WCHAR dllPath[512];
	DWORD dwRet = GetModuleFileName(g_instance, dllPath, 512);
	if( dwRet && dwRet < 512 ){
		wstring iniPath = wstring(dllPath) + L".ini";
		wstring macro = GetPrivateProfileToString(L"SET", L"Macro", L"$Title$.ts", iniPath.c_str());
		CSettingDlg dlg;
		if( dlg.CreateSettingDialog(g_instance, parentWnd, macro) == IDOK ){
			TouchFileAsUnicode(iniPath.c_str());
			WritePrivateProfileString(L"SET", L"Macro", macro.c_str(), iniPath.c_str());
		}
	}
}

//���͂��ꂽ�\��������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
//�ʏ�recNamesize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// info						[IN]�\����
// recName					[OUT]����
// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
BOOL WINAPI ConvertRecName(
	PLUGIN_RESERVE_INFO* info,
	WCHAR* recName,
	DWORD* recNamesize
	)
{
	return ConvertRecName2(info, NULL, recName, recNamesize);
}

//���͂��ꂽ�\��������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
//�ʏ�recNamesize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// info						[IN]�\����
// epgInfo					[IN]�ԑg���iEPG�\��Ŕԑg��񂪑��݂��鎞�A���݂��Ȃ��ꍇ��NULL�j
// recName					[OUT]����
// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
BOOL WINAPI ConvertRecName2(
	PLUGIN_RESERVE_INFO* info,
	EPG_EVENT_INFO* epgInfo,
	WCHAR* recName,
	DWORD* recNamesize
	)
{
	PLUGIN_RESERVE_INFO infoEx;
	memcpy(&infoEx, info, offsetof(PLUGIN_RESERVE_INFO, tunerID) + sizeof(infoEx.tunerID));
	infoEx.reserveID = 0;
	infoEx.epgInfo = epgInfo;
	infoEx.sizeOfStruct = 0;
	return ConvertRecName3(&infoEx, NULL, recName, recNamesize);
}

BOOL WINAPI ConvertRecName3(
	PLUGIN_RESERVE_INFO* info,
	const WCHAR* pattern,
	WCHAR* recName,
	DWORD* recNamesize
	)
{
	if( recNamesize == NULL ){
		return FALSE;
	}
	wstring buff;
	if( pattern == NULL ){
		buff = L"$Title$.ts";
		WCHAR dllPath[512];
		DWORD dwRet = GetModuleFileName(g_instance, dllPath, 512);
		if( dwRet && dwRet < 512 ){
			buff = GetPrivateProfileToString(L"SET", L"Macro", L"$Title$.ts", (wstring(dllPath) + L".ini").c_str());
		}
		pattern = buff.c_str();
	}

	wstring convert = CConvertMacro2::Convert(pattern, info);
	if( recName == NULL ){
		*recNamesize = (DWORD)convert.size()+1;
	}else{
		if( *recNamesize < (DWORD)convert.size()+1 ){
			*recNamesize = (DWORD)convert.size()+1;
			return FALSE;
		}else{
			wcscpy_s(recName, *recNamesize, convert.c_str());
		}
	}

	return TRUE;
}
