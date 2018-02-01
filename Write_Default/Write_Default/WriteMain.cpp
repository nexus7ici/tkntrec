#include "stdafx.h"
#include "WriteMain.h"
#include <process.h>
#include "../../Common/BlockLock.h"

extern HINSTANCE g_instance;

CWriteMain::CWriteMain(void)
{
	this->file = INVALID_HANDLE_VALUE;
	this->writeBuffSize = 0;
	this->teeFile = INVALID_HANDLE_VALUE;
	this->teeThread = NULL;

	WCHAR dllPath[MAX_PATH];
	DWORD ret = GetModuleFileName(g_instance, dllPath, MAX_PATH);
	if( ret && ret < MAX_PATH ){
		wstring iniPath = wstring(dllPath) + L".ini";
		this->writeBuffSize = GetPrivateProfileInt(L"SET", L"Size", 770048, iniPath.c_str());
		this->writeBuff.reserve(this->writeBuffSize);
		this->teeCmd = GetPrivateProfileToString(L"SET", L"TeeCmd", L"", iniPath.c_str());
		if( this->teeCmd.empty() == false ){
			this->teeBuff.resize(GetPrivateProfileInt(L"SET", L"TeeSize", 770048, iniPath.c_str()));
			this->teeBuff.resize(max<size_t>(this->teeBuff.size(), 1));
			this->teeDelay = GetPrivateProfileInt(L"SET", L"TeeDelay", 0, iniPath.c_str());
		}
	}
	InitializeCriticalSection(&this->wroteLock);
}


CWriteMain::~CWriteMain(void)
{
	Stop();
	DeleteCriticalSection(&this->wroteLock);
}

BOOL CWriteMain::Start(
	LPCWSTR fileName,
	BOOL overWriteFlag,
	ULONGLONG createSize
	)
{
	Stop();

	this->savePath = fileName;
	_OutputDebugString(L"��CWriteMain::Start CreateFile:%s\r\n", this->savePath.c_str());
	UtilCreateDirectories(fs_path(this->savePath).parent_path());
	this->file = CreateFile(this->savePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, overWriteFlag ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if( this->file == INVALID_HANDLE_VALUE ){
		_OutputDebugString(L"��CWriteMain::Start Err:0x%08X\r\n", GetLastError());
		fs_path pathWoExt = this->savePath;
		fs_path ext = pathWoExt.extension();
		pathWoExt.replace_extension();
		for( int i = 1; ; i++ ){
			Format(this->savePath, L"%s-(%d)%s", pathWoExt.c_str(), i, ext.c_str());
			this->file = CreateFile(this->savePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, overWriteFlag ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if( this->file != INVALID_HANDLE_VALUE || i >= 999 ){
				DWORD err = GetLastError();
				_OutputDebugString(L"��CWriteMain::Start CreateFile:%s\r\n", this->savePath.c_str());
				if( this->file != INVALID_HANDLE_VALUE ){
					break;
				}
				_OutputDebugString(L"��CWriteMain::Start Err:0x%08X\r\n", err);
				this->savePath = L"";
				return FALSE;
			}
		}
	}

	//�f�B�X�N�ɗe�ʂ��m��
	if( createSize > 0 ){
		LARGE_INTEGER stPos;
		stPos.QuadPart = createSize;
		SetFilePointerEx( this->file, stPos, NULL, FILE_BEGIN );
		SetEndOfFile( this->file );
		SetFilePointer( this->file, 0, NULL, FILE_BEGIN );
	}
	this->wrotePos = 0;

	//�R�}���h�ɕ���o��
	if( this->teeCmd.empty() == false ){
		this->teeFile = CreateFile(this->savePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if( this->teeFile != INVALID_HANDLE_VALUE ){
			this->teeThreadStopFlag = FALSE;
			this->teeThread = (HANDLE)_beginthreadex(NULL, 0, TeeThread, this, 0, NULL);
		}
	}

	return TRUE;
}

BOOL CWriteMain::Stop(
	)
{
	if( this->file != INVALID_HANDLE_VALUE ){
		if( this->writeBuff.empty() == false ){
			DWORD write;
			if( WriteFile(this->file, &this->writeBuff.front(), (DWORD)this->writeBuff.size(), &write, NULL) == FALSE ){
				_OutputDebugString(L"��WriteFile Err:0x%08X\r\n", GetLastError());
			}else{
				this->writeBuff.erase(this->writeBuff.begin(), this->writeBuff.begin() + write);
				CBlockLock lock(&this->wroteLock);
				this->wrotePos += write;
			}
			//���o�͂̃o�b�t�@�͍�Start()�ɔ����ČJ��z��
		}
		SetEndOfFile(this->file);
		CloseHandle(this->file);
		this->file = INVALID_HANDLE_VALUE;
	}
	if( this->teeThread != NULL ){
		this->teeThreadStopFlag = TRUE;
		if( WaitForSingleObject(this->teeThread, 8000) == WAIT_TIMEOUT ){
			TerminateThread(this->teeThread, 0xffffffff);
		}
		CloseHandle(this->teeThread);
		this->teeThread = NULL;
	}
	if( this->teeFile != INVALID_HANDLE_VALUE ){
		CloseHandle(this->teeFile);
		this->teeFile = INVALID_HANDLE_VALUE;
	}

	return TRUE;
}

wstring CWriteMain::GetSavePath(
	)
{
	return this->savePath;
}

BOOL CWriteMain::Write(
	BYTE* data,
	DWORD size,
	DWORD* writeSize
	)
{
	if( this->file != INVALID_HANDLE_VALUE && data != NULL && size > 0 ){
		*writeSize = 0;
		if( this->writeBuff.empty() == false ){
			//�ł��邾���o�b�t�@�ɃR�s�[�B�R�s�[�ς݃f�[�^�͌Ăяo�����ɂƂ��Ắu�ۑ��ς݁v�ƂȂ�
			*writeSize = min(size, this->writeBuffSize - (DWORD)this->writeBuff.size());
			this->writeBuff.insert(this->writeBuff.end(), data, data + *writeSize);
			data += *writeSize;
			size -= *writeSize;
			if( this->writeBuff.size() >= this->writeBuffSize ){
				//�o�b�t�@�����܂����̂ŏo��
				DWORD write;
				if( WriteFile(this->file, &this->writeBuff.front(), (DWORD)this->writeBuff.size(), &write, NULL) == FALSE ){
					_OutputDebugString(L"��WriteFile Err:0x%08X\r\n", GetLastError());
					SetEndOfFile(this->file);
					CloseHandle(this->file);
					this->file = INVALID_HANDLE_VALUE;
					return FALSE;
				}
				this->writeBuff.erase(this->writeBuff.begin(), this->writeBuff.begin() + write);
				CBlockLock lock(&this->wroteLock);
				this->wrotePos += write;
			}
			if( this->writeBuff.empty() == false || size == 0 ){
				return TRUE;
			}
		}
		if( size > this->writeBuffSize ){
			//�o�b�t�@�T�C�Y���傫���̂ł��̂܂܏o��
			DWORD write;
			if( WriteFile(this->file, data, size, &write, NULL) == FALSE ){
				_OutputDebugString(L"��WriteFile Err:0x%08X\r\n", GetLastError());
				SetEndOfFile(this->file);
				CloseHandle(this->file);
				this->file = INVALID_HANDLE_VALUE;
				return FALSE;
			}
			*writeSize += write;
			CBlockLock lock(&this->wroteLock);
			this->wrotePos += write;
		}else{
			//�o�b�t�@�ɃR�s�[
			*writeSize += size;
			this->writeBuff.insert(this->writeBuff.end(), data, data + size);
		}
		return TRUE;
	}
	return FALSE;
}

UINT WINAPI CWriteMain::TeeThread(LPVOID param)
{
	CWriteMain* sys = (CWriteMain*)param;
	wstring cmd = sys->teeCmd;
	Replace(cmd, L"$FilePath$", sys->savePath);
	vector<WCHAR> cmdBuff(cmd.c_str(), cmd.c_str() + cmd.size() + 1);

	{
		//�J�����g�͎��s�t�@�C���̂���t�H���_
		fs_path currentDir = GetModulePath().parent_path();

		//�W�����͂Ƀp�C�v�����v���Z�X���N������
		HANDLE tempPipe;
		HANDLE writePipe;
		if( CreatePipe(&tempPipe, &writePipe, NULL, 0) ){
			HANDLE readPipe;
			BOOL bRet = DuplicateHandle(GetCurrentProcess(), tempPipe, GetCurrentProcess(), &readPipe, 0, TRUE, DUPLICATE_SAME_ACCESS);
			CloseHandle(tempPipe);
			if( bRet ){
				SECURITY_ATTRIBUTES sa;
				sa.nLength = sizeof(sa);
				sa.lpSecurityDescriptor = NULL;
				sa.bInheritHandle = TRUE;
				STARTUPINFO si = {};
				si.cb = sizeof(si);
				si.dwFlags = STARTF_USESTDHANDLES;
				si.hStdInput = readPipe;
				//�W��(�G���[)�o�͂�nul�f�o�C�X�Ɏ̂Ă�
				si.hStdOutput = CreateFile(L"nul", GENERIC_WRITE, 0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				si.hStdError = CreateFile(L"nul", GENERIC_WRITE, 0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				PROCESS_INFORMATION pi;
				bRet = CreateProcess(NULL, &cmdBuff.front(), NULL, NULL, TRUE, BELOW_NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, currentDir.c_str(), &si, &pi);
				CloseHandle(readPipe);
				if( si.hStdOutput != INVALID_HANDLE_VALUE ){
					CloseHandle(si.hStdOutput);
				}
				if( si.hStdError != INVALID_HANDLE_VALUE ){
					CloseHandle(si.hStdError);
				}
				if( bRet ){
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
					while( sys->teeThreadStopFlag == FALSE ){
						__int64 readablePos;
						{
							CBlockLock lock(&sys->wroteLock);
							readablePos = sys->wrotePos - sys->teeDelay;
						}
						LARGE_INTEGER liPos = {};
						DWORD read;
						if( SetFilePointerEx(sys->teeFile, liPos, &liPos, FILE_CURRENT) &&
						    readablePos - liPos.QuadPart >= (__int64)sys->teeBuff.size() &&
						    ReadFile(sys->teeFile, &sys->teeBuff.front(), (DWORD)sys->teeBuff.size(), &read, NULL) && read > 0 ){
							DWORD write;
							if( WriteFile(writePipe, &sys->teeBuff.front(), read, &write, NULL) == FALSE ){
								break;
							}
						}else{
							Sleep(100);
						}
					}
					//�v���Z�X�͉�����Ȃ�(�W�����͂�����ꂽ��ɂǂ����邩�̓v���Z�X�̔��f�ɔC����)
				}
			}
			CloseHandle(writePipe);
		}
	}
	return 0;
}
