
// EpgDataCap_Bon.cpp : �A�v���P�[�V�����̃N���X������`���܂��B
//

#include "stdafx.h"
#include "EpgDataCap_Bon.h"
#include "EpgDataCap_BonDlg.h"

#include "CmdLineUtil.h"
#include "../../Common/ThreadUtil.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>

#ifndef SUPPRESS_OUTPUT_STACK_TRACE
#include <tlhelp32.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace
{

FILE* g_debugLog;
recursive_mutex_ g_debugLogLock;
bool g_saveDebugLog;

void StartDebugLog()
{
	if( GetPrivateProfileInt(L"SET", L"SaveDebugLog", 0, GetModuleIniPath().c_str()) != 0 ){
		for( int i = 0; i < 100; i++ ){
			//�p�X�ɓY���������ď������݉\�ȍŏ��̂��̂ɋL�^����
			WCHAR logFileName[64];
			swprintf_s(logFileName, L"EpgDataCap_Bon_DebugLog-%d.txt", i);
			fs_path logPath = GetModulePath().replace_filename(logFileName);
			//��肽�����Ƃ�_wfsopen(L"abN",_SH_DENYWR)����_wfsopen�ɂ�"N"�I�v�V�������Ȃ������Ȃ̂Œᐅ���ŊJ��
			int fd;
			if( _wsopen_s(&fd, logPath.c_str(), _O_APPEND | _O_BINARY | _O_CREAT | _O_NOINHERIT | _O_WRONLY, _SH_DENYWR, _S_IWRITE) == 0 ){
				g_debugLog = _wfdopen(fd, L"ab");
				if( g_debugLog == NULL ){
					_close(fd);
				}
			}
			if( g_debugLog ){
				_fseeki64(g_debugLog, 0, SEEK_END);
				if( _ftelli64(g_debugLog) == 0 ){
					fputwc(L'\xFEFF', g_debugLog);
				}
				g_saveDebugLog = true;
				OutputDebugString(L"****** LOG START ******\r\n");
				break;
			}
		}
	}
}

void StopDebugLog()
{
	if( g_saveDebugLog ){
		OutputDebugString(L"****** LOG STOP ******\r\n");
		g_saveDebugLog = false;
		fclose(g_debugLog);
	}
}

#ifndef SUPPRESS_OUTPUT_STACK_TRACE
// ��O�ɂ���ăA�v���P�[�V�������I�����钼�O�ɃX�^�b�N�g���[�X��"���s�t�@�C����.exe.err"�ɏo�͂���
// �f�o�b�O���(.pdb�t�@�C��)�����݂���Ώo�͂͂��ڍׂɂȂ�

void OutputStackTrace(DWORD exceptionCode, const PVOID* addrOffsets)
{
	WCHAR path[MAX_PATH + 4];
	path[GetModuleFileName(NULL, path, MAX_PATH)] = L'\0';
	wcscat_s(path, L".err");
	HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile != INVALID_HANDLE_VALUE ){
		char buff[384];
		DWORD written;
		int len = sprintf_s(buff, "ExceptionCode = 0x%08X\r\n", exceptionCode);
		WriteFile(hFile, buff, len, &written, NULL);
		for( int i = 0; addrOffsets[i]; i++ ){
			SYMBOL_INFO symbol[1 + (256 + sizeof(SYMBOL_INFO)) / sizeof(SYMBOL_INFO)];
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol->MaxNameLen = 256;
			DWORD64 displacement;
			if( SymFromAddr(GetCurrentProcess(), (DWORD64)addrOffsets[i], &displacement, symbol) ){
				len = sprintf_s(buff, "Trace%02d 0x%p = 0x%p(%s) + 0x%X\r\n", i, addrOffsets[i], (PVOID)symbol->Address, symbol->Name, (DWORD)displacement);
			}else{
				len = sprintf_s(buff, "Trace%02d 0x%p = ?\r\n", i, addrOffsets[i]);
			}
			WriteFile(hFile, buff, len, &written, NULL);
		}
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
		if( hSnapshot != INVALID_HANDLE_VALUE ){
			MODULEENTRY32W modent;
			modent.dwSize = sizeof(modent);
			if( Module32FirstW(hSnapshot, &modent) ){
				do{
					char moduleA[256] = {};
					for( int i = 0; i == 0 || i < 255 && moduleA[i - 1]; i++ ){
						//�����������Ă��\��Ȃ�
						moduleA[i] = (char)modent.szModule[i];
					}
					len = sprintf_s(buff, "0x%p - 0x%p = %s\r\n", modent.modBaseAddr, modent.modBaseAddr + modent.modBaseSize - 1, moduleA);
					WriteFile(hFile, buff, len, &written, NULL);
				}while( Module32NextW(hSnapshot, &modent) );
			}
			CloseHandle(hSnapshot);
		}
		CloseHandle(hFile);
	}
}

LONG WINAPI TopLevelExceptionFilter(_EXCEPTION_POINTERS* exceptionInfo)
{
	static struct {
		LONG used;
		CONTEXT contextRecord;
		STACKFRAME64 stackFrame;
		PVOID addrOffsets[32];
	} work;

	if( InterlockedExchange(&work.used, 1) == 0 ){
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		if( SymInitialize(GetCurrentProcess(), NULL, TRUE) ){
			work.addrOffsets[0] = exceptionInfo->ExceptionRecord->ExceptionAddress;
			work.contextRecord = *exceptionInfo->ContextRecord;
			work.stackFrame.AddrPC.Mode = AddrModeFlat;
			work.stackFrame.AddrFrame.Mode = AddrModeFlat;
			work.stackFrame.AddrStack.Mode = AddrModeFlat;
#if defined(_M_IX86) || defined(_M_X64)
#ifdef _M_X64
			work.stackFrame.AddrPC.Offset = work.contextRecord.Rip;
			work.stackFrame.AddrFrame.Offset = work.contextRecord.Rbp;
			work.stackFrame.AddrStack.Offset = work.contextRecord.Rsp;
#else
			work.stackFrame.AddrPC.Offset = work.contextRecord.Eip;
			work.stackFrame.AddrFrame.Offset = work.contextRecord.Ebp;
			work.stackFrame.AddrStack.Offset = work.contextRecord.Esp;
#endif
			for( int i = 1; i < _countof(work.addrOffsets) - 1 && StackWalk64(
#ifdef _M_X64
				IMAGE_FILE_MACHINE_AMD64,
#else
				IMAGE_FILE_MACHINE_I386,
#endif
				GetCurrentProcess(), GetCurrentThread(), &work.stackFrame, &work.contextRecord,
				NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL); i++ ){
				work.addrOffsets[i] = (PVOID)work.stackFrame.AddrPC.Offset;
			}
#endif
			OutputStackTrace(exceptionInfo->ExceptionRecord->ExceptionCode, work.addrOffsets);
			SymCleanup(GetCurrentProcess());
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

#endif // SUPPRESS_OUTPUT_STACK_TRACE

// �B��� CEpgDataCap_BonApp �I�u�W�F�N�g�ł��B

CEpgDataCap_BonApp theApp;

}

// CEpgDataCap_BonApp �R���X�g���N�V����

CEpgDataCap_BonApp::CEpgDataCap_BonApp()
{
	// TODO: ���̈ʒu�ɍ\�z�p�R�[�h��ǉ����Ă��������B
	// ������ InitInstance ���̏d�v�ȏ��������������ׂċL�q���Ă��������B
}

// CEpgDataCap_BonApp ������

BOOL CEpgDataCap_BonApp::InitInstance()
{
	// �A�v���P�[�V���� �}�j�t�F�X�g�� visual �X�^�C����L���ɂ��邽�߂ɁA
	// ComCtl32.dll Version 6 �ȍ~�̎g�p���w�肷��ꍇ�́A
	// Windows XP �� InitCommonControlsEx() ���K�v�ł��B�����Ȃ���΁A�E�B���h�E�쐬�͂��ׂĎ��s���܂��B
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// �A�v���P�[�V�����Ŏg�p���邷�ׂẴR���� �R���g���[�� �N���X���܂߂�ɂ́A
	// �����ݒ肵�܂��B
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	SetProcessShutdownParameters(0x300, 0);

#ifndef SUPPRESS_OUTPUT_STACK_TRACE
	SetUnhandledExceptionFilter(TopLevelExceptionFilter);
#endif

	// �R�}���h�I�v�V���������
	CCmdLineUtil cCmdUtil;
	int argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if (argv != NULL) {
		for (int i = 1; i < argc; i++) {
			BOOL bFlag = argv[i][0] == L'-' || argv[i][0] == L'/' ? TRUE : FALSE;
			cCmdUtil.ParseParam(&argv[i][bFlag ? 1 : 0], bFlag, i == argc - 1 ? TRUE : FALSE);
		}
		LocalFree(argv);
	}

	CEpgDataCap_BonDlg dlg;

	map<wstring, wstring>::iterator itr;
	dlg.SetIniMin(FALSE);
	dlg.SetIniView(TRUE);
	dlg.SetIniNW(TRUE);
	for( itr = cCmdUtil.m_CmdList.begin(); itr != cCmdUtil.m_CmdList.end(); itr++ ){
		if( CompareNoCase(itr->first, L"d") == 0 ){
			dlg.SetInitBon(itr->second.c_str());
			OutputDebugString(itr->second.c_str());
		}else if( CompareNoCase(itr->first, L"min") == 0 ){
			dlg.SetIniMin(TRUE);
		}else if( CompareNoCase(itr->first, L"noview") == 0 ){
			dlg.SetIniView(FALSE);
		}else if( CompareNoCase(itr->first, L"nonw") == 0 ){
			dlg.SetIniNW(FALSE);
		}else if( CompareNoCase(itr->first, L"nwudp") == 0 ){
			dlg.SetIniNWUDP(TRUE);
		}else if( CompareNoCase(itr->first, L"nwtcp") == 0 ){
			dlg.SetIniNWTCP(TRUE);
		}
	}


	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �_�C�A���O�� <OK> �ŏ����ꂽ���̃R�[�h��
		//  �L�q���Ă��������B
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �_�C�A���O�� <�L�����Z��> �ŏ����ꂽ���̃R�[�h��
		//  �L�q���Ă��������B
	}

	// �_�C�A���O�͕����܂����B�A�v���P�[�V�����̃��b�Z�[�W �|���v���J�n���Ȃ���
	//  �A�v���P�[�V�������I�����邽�߂� FALSE ��Ԃ��Ă��������B
	return FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	SetDllDirectory(TEXT(""));
	StartDebugLog();
	//���C���X���b�h�ɑ΂���COM�̏�����
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	theApp.InitInstance();
	CoUninitialize();
	StopDebugLog();
	return 0;
}

void OutputDebugStringWrapper(LPCWSTR lpOutputString)
{
	if( g_saveDebugLog ){
		//�f�o�b�O�o�̓��O�ۑ�
		CBlockLock lock(&g_debugLogLock);
		SYSTEMTIME st;
		GetLocalTime(&st);
		fwprintf(g_debugLog, L"[%02d%02d%02d%02d%02d%02d.%03d] %s%s",
		         st.wYear % 100, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		         lpOutputString ? lpOutputString : L"",
		         lpOutputString && lpOutputString[0] && lpOutputString[wcslen(lpOutputString) - 1] == L'\n' ? L"" : L"<NOBR>\r\n");
		fflush(g_debugLog);
	}
	OutputDebugStringW(lpOutputString);
}
