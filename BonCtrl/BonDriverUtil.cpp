#include "stdafx.h"
#include "BonDriverUtil.h"
#include "../Common/PathUtil.h"
#include "../Common/StringUtil.h"
#include "IBonDriver2.h"

enum {
	WM_GET_TS_STREAM = WM_APP,
	WM_SET_CH,
	WM_GET_NOW_CH,
	WM_GET_SIGNAL_LEVEL,
};

CBonDriverUtil::CInit CBonDriverUtil::s_init;

CBonDriverUtil::CInit::CInit()
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = DriverWindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"BonDriverUtilWorker";
	RegisterClassEx(&wc);
}

CBonDriverUtil::CBonDriverUtil(void)
	: hwndDriver(NULL)
{
}

CBonDriverUtil::~CBonDriverUtil(void)
{
	CloseBonDriver();
}

void CBonDriverUtil::SetBonDriverFolder(LPCWSTR bonDriverFolderPath)
{
	CBlockLock lock(&this->utilLock);
	this->loadDllFolder = bonDriverFolderPath;
}

vector<wstring> CBonDriverUtil::EnumBonDriver()
{
	CBlockLock lock(&this->utilLock);
	vector<wstring> list;
	if( this->loadDllFolder.empty() == false ){
		//�w��t�H���_�̃t�@�C���ꗗ�擾
		WIN32_FIND_DATA findData;
		HANDLE hFind = FindFirstFile(fs_path(this->loadDllFolder).append(L"BonDriver*.dll").c_str(), &findData);
		if( hFind != INVALID_HANDLE_VALUE ){
			do{
				if( (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ){
					//��������DLL���ꗗ�ɒǉ�
					list.push_back(findData.cFileName);
				}
			}while( FindNextFile(hFind, &findData) );
			FindClose(hFind);
		}
	}
	return list;
}

bool CBonDriverUtil::OpenBonDriver(LPCWSTR bonDriverFile, void (*recvFunc_)(void*, BYTE*, DWORD, DWORD), void* recvParam_, int openWait)
{
	CBlockLock lock(&this->utilLock);
	CloseBonDriver();
	this->loadDllFileName = bonDriverFile;
	if( this->loadDllFolder.empty() == false && this->loadDllFileName.empty() == false ){
		this->recvFunc = recvFunc_;
		this->recvParam = recvParam_;
		this->driverThread = thread_(DriverThread, this);
		//Open��������������܂ő҂�
		while( WaitForSingleObject(this->driverThread.native_handle(), 10) == WAIT_TIMEOUT && this->hwndDriver == NULL );
		if( this->hwndDriver ){
			Sleep(openWait);
			return true;
		}
		this->driverThread.join();
	}
	return false;
}

void CBonDriverUtil::CloseBonDriver()
{
	CBlockLock lock(&this->utilLock);
	if( this->hwndDriver ){
		PostMessage(this->hwndDriver, WM_CLOSE, 0, 0);
		this->driverThread.join();
		this->hwndDriver = NULL;
	}
}

void CBonDriverUtil::DriverThread(CBonDriverUtil* sys)
{
	//BonDriver��COM�𗘗p���邩������Ȃ�����
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	IBonDriver* bonIF = NULL;
	sys->bon2IF = NULL;
	HMODULE hModule = LoadLibrary(fs_path(sys->loadDllFolder).append(sys->loadDllFileName).c_str());
	if( hModule == NULL ){
		OutputDebugString(L"��BonDriver�����[�h�ł��܂���\r\n");
	}else{
		IBonDriver* (*funcCreateBonDriver)() = (IBonDriver*(*)())GetProcAddress(hModule, "CreateBonDriver");
		if( funcCreateBonDriver == NULL ){
			OutputDebugString(L"��GetProcAddress�Ɏ��s���܂���\r\n");
		}else if( (bonIF = funcCreateBonDriver()) != NULL &&
		          (sys->bon2IF = dynamic_cast<IBonDriver2*>(bonIF)) != NULL ){
			if( sys->bon2IF->OpenTuner() == FALSE ){
				OutputDebugString(L"��OpenTuner�Ɏ��s���܂���\r\n");
			}else{
				sys->initChSetFlag = false;
				//�`���[�i�[���̎擾
				LPCWSTR tunerName = sys->bon2IF->GetTunerName();
				sys->loadTunerName = tunerName ? tunerName : L"";
				Replace(sys->loadTunerName, L"(",L"�i");
				Replace(sys->loadTunerName, L")",L"�j");
				//�`�����l���ꗗ�̎擾
				sys->loadChList.clear();
				for( DWORD countSpace = 0; ; countSpace++ ){
					LPCWSTR spaceName = sys->bon2IF->EnumTuningSpace(countSpace);
					if( spaceName == NULL ){
						break;
					}
					sys->loadChList.push_back(pair<wstring, vector<wstring>>(spaceName, vector<wstring>()));
					for( DWORD countCh = 0; ; countCh++ ){
						LPCWSTR chName = sys->bon2IF->EnumChannelName(countSpace, countCh);
						if( chName == NULL ){
							break;
						}
						sys->loadChList.back().second.push_back(chName);
					}
				}
				sys->hwndDriver = CreateWindow(L"BonDriverUtilWorker", NULL, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), sys);
				if( sys->hwndDriver == NULL ){
					sys->bon2IF->CloseTuner();
				}
			}
		}
	}
	if( sys->hwndDriver == NULL ){
		//Open�ł��Ȃ�����
		if( bonIF ){
			bonIF->Release();
		}
		if( hModule ){
			FreeLibrary(hModule);
		}
		CoUninitialize();
		return;
	}
	//���荞�ݒx���ւ̑ϐ���BonDriver�̃o�b�t�@�\�͂Ɉˑ�����̂ŁA���ΗD�揇�ʂ��グ�Ă���
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	//���b�Z�[�W���[�v
	MSG msg;
	while( GetMessage(&msg, NULL, 0, 0) > 0 ){
		DispatchMessage(&msg);
	}
	sys->bon2IF->CloseTuner();
	bonIF->Release();
	FreeLibrary(hModule);

	CoUninitialize();
}

LRESULT CALLBACK CBonDriverUtil::DriverWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CBonDriverUtil* sys = (CBonDriverUtil*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if( uMsg != WM_CREATE && sys == NULL ){
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	switch( uMsg ){
	case WM_CREATE:
		sys = (CBonDriverUtil*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)sys);
		SetTimer(hwnd, 1, 20, NULL);
		return 0;
	case WM_DESTROY:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		if( wParam == 1 ){
			SendMessage(hwnd, WM_GET_TS_STREAM, 0, 0);
			return 0;
		}
		break;
	case WM_GET_TS_STREAM:
		{
			//TS�X�g���[�����擾
			BYTE* data;
			DWORD size;
			DWORD remain;
			if( sys->bon2IF->GetTsStream(&data, &size, &remain) && data && size != 0 ){
				if( sys->recvFunc ){
					sys->recvFunc(sys->recvParam, data, size, 1);
				}
				PostMessage(hwnd, WM_GET_TS_STREAM, 1, 0);
			}else if( wParam ){
				//EDCB��(�`���I��)GetTsStream��remain�𗘗p���Ȃ��̂ŁA�󂯎����̂��Ȃ��Ȃ�����remain=0��m�点��
				if( sys->recvFunc ){
					sys->recvFunc(sys->recvParam, NULL, 0, 0);
				}
			}
		}
		return 0;
	case WM_SET_CH:
		if( sys->bon2IF->SetChannel((DWORD)wParam, (DWORD)lParam) == FALSE ){
			Sleep(500);
			if( sys->bon2IF->SetChannel((DWORD)wParam, (DWORD)lParam) == FALSE ){
				return FALSE;
			}
		}
		return TRUE;
	case WM_GET_NOW_CH:
		*(DWORD*)wParam = sys->bon2IF->GetCurSpace();
		*(DWORD*)lParam = sys->bon2IF->GetCurChannel();
		return 0;
	case WM_GET_SIGNAL_LEVEL:
		*(float*)lParam = sys->bon2IF->GetSignalLevel();
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

vector<pair<wstring, vector<wstring>>> CBonDriverUtil::GetOriginalChList()
{
	CBlockLock lock(&this->utilLock);
	if( this->hwndDriver ){
		return this->loadChList;
	}
	return vector<pair<wstring, vector<wstring>>>();
}

wstring CBonDriverUtil::GetTunerName()
{
	CBlockLock lock(&this->utilLock);
	if( this->hwndDriver ){
		return this->loadTunerName;
	}
	return L"";
}

bool CBonDriverUtil::SetCh(DWORD space, DWORD ch)
{
	CBlockLock lock(&this->utilLock);
	if( this->hwndDriver ){
		if( this->initChSetFlag ){
			//2��ڈȍ~�͕ω��̂���ꍇ�����`�����l���ݒ肷��
			DWORD nowSpace = 0;
			DWORD nowCh = 0;
			SendMessage(this->hwndDriver, WM_GET_NOW_CH, (WPARAM)&nowSpace, (LPARAM)&nowCh);
			if( nowSpace == space && nowCh == ch ){
				return true;
			}
		}
		if( SendMessage(this->hwndDriver, WM_SET_CH, (WPARAM)space, (LPARAM)ch) ){
			this->initChSetFlag = true;
			return true;
		}
	}
	return false;
}

bool CBonDriverUtil::GetNowCh(DWORD* space, DWORD* ch)
{
	CBlockLock lock(&this->utilLock);
	if( this->hwndDriver && this->initChSetFlag ){
		SendMessage(this->hwndDriver, WM_GET_NOW_CH, (WPARAM)space, (LPARAM)ch);
		return true;
	}
	return false;
}

float CBonDriverUtil::GetSignalLevel()
{
	CBlockLock lock(&this->utilLock);
	float fLevel = 0.0f;
	if( this->hwndDriver ){
		SendMessage(this->hwndDriver, WM_GET_SIGNAL_LEVEL, 0, (LPARAM)&fLevel);
	}
	return fLevel;
}

wstring CBonDriverUtil::GetOpenBonDriverFileName()
{
	CBlockLock lock(&this->utilLock);
	if( this->hwndDriver ){
		return this->loadDllFileName;
	}
	return L"";
}
