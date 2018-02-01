#include "stdafx.h"
#include "SendTSTCPMain.h"
#include "../../Common/BlockLock.h"

#include <process.h>

//SendTSTCP�v���g�R���̃w�b�_�̑��M��}���������̃|�[�g�͈�
#define SEND_TS_TCP_NOHEAD_PORT_MIN 22000
#define SEND_TS_TCP_NOHEAD_PORT_MAX 22999

CSendTSTCPMain::CSendTSTCPMain(void)
{
	m_hStopSendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSendThread = NULL;

	InitializeCriticalSection(&m_sendLock);
	InitializeCriticalSection(&m_buffLock);

	WSAData wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

CSendTSTCPMain::~CSendTSTCPMain(void)
{
	UnInitialize();

	CloseHandle(m_hStopSendEvent);

	DeleteCriticalSection(&m_buffLock);
	DeleteCriticalSection(&m_sendLock);

	WSACleanup();
}

//DLL�̏�����
//�߂�l�FTRUE:�����AFALSE:���s
BOOL CSendTSTCPMain::Initialize(
	)
{
	return TRUE;
}

//DLL�̊J��
//�߂�l�F�Ȃ�
void CSendTSTCPMain::UnInitialize(
	)
{
	StopSend();
	ClearSendAddr();
	ClearSendBuff();
}

//���M���ǉ�
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::AddSendAddr(
	LPCWSTR lpcwszIP,
	DWORD dwPort
	)
{
	if( lpcwszIP == NULL ){
		return FALSE;
	}
	SEND_INFO Item;
	WtoUTF8(lpcwszIP, Item.strIP);
	Item.dwPort = dwPort;
	if( SEND_TS_TCP_NOHEAD_PORT_MIN <= dwPort && dwPort <= SEND_TS_TCP_NOHEAD_PORT_MAX ){
		//��ʃ��[�h��1�̂Ƃ��̓w�b�_�̑��M���}�������
		Item.dwPort |= 0x10000;
	}
	Item.sock = INVALID_SOCKET;
	Item.bConnect = FALSE;
	wstring strKey=L"";
	Format(strKey, L"%s:%d", lpcwszIP, dwPort);

	CBlockLock lock(&m_sendLock);
	m_SendList.insert(pair<wstring, SEND_INFO>(strKey, Item));

	return TRUE;

}

//���M��N���A
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::ClearSendAddr(
	)
{
	CBlockLock lock(&m_sendLock);

	map<wstring, SEND_INFO>::iterator itr;
	for( itr = m_SendList.begin(); itr != m_SendList.end(); itr++){
		if( itr->second.sock != INVALID_SOCKET ){
			closesocket(itr->second.sock);
			itr->second.sock = INVALID_SOCKET;
			itr->second.bConnect = FALSE;
		}
	}
	m_SendList.clear();

	return TRUE;

}

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::StartSend(
	)
{
	if( m_hSendThread != NULL ){
		return FALSE;
	}

	ResetEvent(m_hStopSendEvent);
	m_hSendThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, (LPVOID)this, CREATE_SUSPENDED, NULL);
	ResumeThread(m_hSendThread);

	return TRUE;
}

//�f�[�^���M���~
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::StopSend(
	)
{
	if( m_hSendThread != NULL ){
		::SetEvent(m_hStopSendEvent);
		// �X���b�h�I���҂�
		if ( ::WaitForSingleObject(m_hSendThread, 5000) == WAIT_TIMEOUT ){
			::TerminateThread(m_hSendThread, 0xffffffff);
		}
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}

	CBlockLock lock(&m_sendLock);
	map<wstring, SEND_INFO>::iterator itr;
	for( itr = m_SendList.begin(); itr != m_SendList.end(); itr++){
		if( itr->second.sock != INVALID_SOCKET ){
			shutdown(itr->second.sock,SD_BOTH);
			closesocket(itr->second.sock);
			itr->second.sock = INVALID_SOCKET;
			itr->second.bConnect = FALSE;
		}
	}

	return TRUE;
}

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::AddSendData(
	BYTE* pbData,
	DWORD dwSize
	)
{

	if( m_hSendThread != NULL ){
		CBlockLock lock(&m_buffLock);
		m_TSBuff.push_back(vector<BYTE>());
		m_TSBuff.back().reserve(sizeof(DWORD) * 2 + dwSize);
		m_TSBuff.back().resize(sizeof(DWORD) * 2);
		m_TSBuff.back().insert(m_TSBuff.back().end(), pbData, pbData + dwSize);
		if( m_TSBuff.size() > 500 ){
			for( ; m_TSBuff.size() > 250; m_TSBuff.pop_front() );
		}
	}
	return TRUE;
}

//���M�o�b�t�@���N���A
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::ClearSendBuff(
	)
{
	CBlockLock lock(&m_buffLock);
	m_TSBuff.clear();

	return TRUE;
}

UINT WINAPI CSendTSTCPMain::SendThread(LPVOID pParam)
{
	CSendTSTCPMain* pSys = (CSendTSTCPMain*)pParam;
	DWORD dwWait = 0;
	DWORD dwCount = 0;
	DWORD dwCheckConnectTick = GetTickCount();
	while(1){
		if( ::WaitForSingleObject(pSys->m_hStopSendEvent, dwWait) != WAIT_TIMEOUT ){
			//�L�����Z�����ꂽ
			break;
		}

		DWORD tick = GetTickCount();
		if( tick - dwCheckConnectTick > 1000 )
		{
		dwCheckConnectTick = tick;
		CBlockLock lock(&pSys->m_sendLock);

		map<wstring, SEND_INFO>::iterator itr;
		for( itr = pSys->m_SendList.begin(); itr != pSys->m_SendList.end(); itr++){
			if( itr->second.bConnect == FALSE ){
				if( itr->second.sock != INVALID_SOCKET && itr->second.bConnect == FALSE ){
					fd_set rmask,wmask;
					FD_ZERO(&rmask);
					FD_SET(itr->second.sock,&rmask);
					wmask=rmask;
					struct timeval tv={0,0};
					int rc=select((int)itr->second.sock+1,&rmask, &wmask, NULL, &tv);
					if(rc==SOCKET_ERROR || rc == 0){
						closesocket(itr->second.sock);
						itr->second.sock = INVALID_SOCKET;
					}else{
						ULONG x = 0;
						ioctlsocket(itr->second.sock,FIONBIO, &x);
						itr->second.bConnect = TRUE;
					}
				}else{
					string strPort;
					Format(strPort, "%d", (WORD)itr->second.dwPort);
					struct addrinfo hints = {};
					hints.ai_flags = AI_NUMERICHOST;
					hints.ai_socktype = SOCK_STREAM;
					hints.ai_protocol = IPPROTO_TCP;
					struct addrinfo* result;
					if( getaddrinfo(itr->second.strIP.c_str(), strPort.c_str(), &hints, &result) != 0 ){
						continue;
					}
					itr->second.sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
					if( itr->second.sock == INVALID_SOCKET ){
						freeaddrinfo(result);
						continue;
					}
					ULONG x = 1;
					ioctlsocket(itr->second.sock,FIONBIO, &x);

					if( connect(itr->second.sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR ){
						if( WSAGetLastError() != WSAEWOULDBLOCK ){
							closesocket(itr->second.sock);
							itr->second.sock = INVALID_SOCKET;
						}
					}
					freeaddrinfo(result);
				}
			}
		}
		} //m_sendLock

		std::list<vector<BYTE>> item;
		{
			CBlockLock lock(&pSys->m_buffLock);

			if( pSys->m_TSBuff.empty() == false ){
				item.splice(item.end(), pSys->m_TSBuff, pSys->m_TSBuff.begin());
				DWORD dwCmd[2] = { dwCount, (DWORD)(item.back().size() - sizeof(DWORD) * 2) };
				memcpy(&item.back().front(), dwCmd, sizeof(dwCmd));
			}
			dwWait = pSys->m_TSBuff.empty() ? 100 : 0;
		} //m_buffLock

		if( item.empty() == false ){
			vector<BYTE>& buffSend = item.back();
			CBlockLock lock(&pSys->m_sendLock);

			map<wstring, SEND_INFO>::iterator itr;
			for( itr = pSys->m_SendList.begin(); itr != pSys->m_SendList.end(); itr++){
				if( itr->second.bConnect == TRUE ){
					size_t adjust = HIWORD(itr->second.dwPort) == 1 ? buffSend.size() - sizeof(DWORD)*2 : buffSend.size();
					if( adjust > 0 && send(itr->second.sock, 
						(char*)&buffSend.front() + (buffSend.size() - adjust),
						(int)adjust,
						0
						) == SOCKET_ERROR){
							closesocket(itr->second.sock);
							itr->second.sock = INVALID_SOCKET;
							itr->second.bConnect = FALSE;
					}
					dwCount++;
				}
			}
		}
	}
	return 0;
}
