#include "stdafx.h"
#include "ReNamePlugInUtil.h"

BOOL CReNamePlugInUtil::ConvertRecName3(
	PLUGIN_RESERVE_INFO* info,
	const WCHAR* dllPattern,
	const WCHAR* dllFolder,
	WCHAR* recName,
	DWORD* recNamesize
	)
{
	wstring pattern = dllPattern;
	HMODULE hModule = LoadLibrary((dllFolder + pattern.substr(0, pattern.find('?'))).c_str());
	if( hModule == NULL ){
		OutputDebugString(L"dl�̃��[�h�Ɏ��s���܂���\r\n");
		return FALSE;
	}
	pattern.erase(0, pattern.find('?'));
	pattern.erase(0, 1);
	ConvertRecNameRNP pfnConvertRecName = (ConvertRecNameRNP)GetProcAddress(hModule, "ConvertRecName");
	ConvertRecName2RNP pfnConvertRecName2 = (ConvertRecName2RNP)GetProcAddress(hModule, "ConvertRecName2");
	ConvertRecName3RNP pfnConvertRecName3 = (ConvertRecName3RNP)GetProcAddress(hModule, "ConvertRecName3");
	BOOL ret;
	if( pfnConvertRecName3 ){
		ret = pfnConvertRecName3(info, pattern.empty() ? NULL : pattern.c_str(), recName, recNamesize);
	}else if( pfnConvertRecName2 ){
		ret = pfnConvertRecName2(info, info->epgInfo, recName, recNamesize);
	}else if( pfnConvertRecName ){
		ret = pfnConvertRecName(info, recName, recNamesize);
	}else{
		OutputDebugString(L"ConvertRecName�� GetProcAddress �Ɏ��s\r\n");
		ret = FALSE;
	}
	 FreeLibrary(hModule);
	return ret;
}

