#include "stdafx.h"
#include "SendTSTCPMain.h"

//SendTSTCP�v���g�R���̃w�b�_�̑��M��}���������̃|�[�g�͈�
#define SEND_TS_TCP_NOHEAD_PORT_MIN 22000
#define SEND_TS_TCP_NOHEAD_PORT_MAX 22999
//���M�o�b�t�@�̍ő吔(�T�C�Y��AddSendData()�̓��͂Ɉˑ�)
#define SEND_TS_TCP_BUFF_MAX 500
//���M��(�T�[�o)�ڑ��̂��߂̃|�[�����O�Ԋu
#define SEND_TS_TCP_CONNECT_INTERVAL_MSEC 2000

CSendTSTCPMain::CSendTSTCPMain(void)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

CSendTSTCPMain::~CSendTSTCPMain(void)
{
	StopSend();

	WSACleanup();
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

	CBlockLock lock(&m_sendLock);
	if( std::find_if(m_SendList.begin(), m_SendList.end(), [&Item](const SEND_INFO& a) {
	        return a.strIP == Item.strIP && (WORD)a.dwPort == (WORD)Item.dwPort; }) == m_SendList.end() ){
		m_SendList.push_back(Item);
	}

	return TRUE;
}

//���M��N���A
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::ClearSendAddr(
	)
{
	if( m_sendThread.joinable() ){
		StopSend();
		m_SendList.clear();
		StartSend();
	}else{
		m_SendList.clear();
	}

	return TRUE;
}

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::StartSend(
	)
{
	if( m_sendThread.joinable() ){
		return FALSE;
	}

	m_stopSendEvent.Reset();
	m_sendThread = thread_(SendThread, this);

	return TRUE;
}

//�f�[�^���M���~
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::StopSend(
	)
{
	if( m_sendThread.joinable() ){
		m_stopSendEvent.Set();
		m_sendThread.join();
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
	if( m_sendThread.joinable() ){
		CBlockLock lock(&m_sendLock);
		m_TSBuff.push_back(vector<BYTE>());
		m_TSBuff.back().reserve(sizeof(DWORD) * 2 + dwSize);
		m_TSBuff.back().resize(sizeof(DWORD) * 2);
		m_TSBuff.back().insert(m_TSBuff.back().end(), pbData, pbData + dwSize);
		if( m_TSBuff.size() > SEND_TS_TCP_BUFF_MAX ){
			for( ; m_TSBuff.size() > SEND_TS_TCP_BUFF_MAX / 2; m_TSBuff.pop_front() );
		}
	}
	return TRUE;
}

//���M�o�b�t�@���N���A
//�߂�l�F�G���[�R�[�h
DWORD CSendTSTCPMain::ClearSendBuff(
	)
{
	CBlockLock lock(&m_sendLock);
	m_TSBuff.clear();

	return TRUE;
}

void CSendTSTCPMain::SendThread(CSendTSTCPMain* pSys)
{
	DWORD dwCount = 0;
	DWORD dwCheckConnectTick = GetTickCount();
	for(;;){
		DWORD tick = GetTickCount();
		if( tick - dwCheckConnectTick > SEND_TS_TCP_CONNECT_INTERVAL_MSEC ){
			dwCheckConnectTick = tick;
			CBlockLock lock(&pSys->m_sendLock);

			for( auto itr = pSys->m_SendList.begin(); itr != pSys->m_SendList.end(); itr++ ){
				if( itr->bConnect == FALSE && itr->sock != INVALID_SOCKET ){
					fd_set wmask;
					FD_ZERO(&wmask);
					FD_SET(itr->sock, &wmask);
					struct timeval tv = {0, 0};
					if( select((int)itr->sock + 1, NULL, &wmask, NULL, &tv) == 1 ){
						itr->bConnect = TRUE;
					}else{
						closesocket(itr->sock);
						itr->sock = INVALID_SOCKET;
					}
				}
				if( itr->sock == INVALID_SOCKET ){
					string strPort;
					Format(strPort, "%d", (WORD)itr->dwPort);
					struct addrinfo hints = {};
					hints.ai_flags = AI_NUMERICHOST;
					hints.ai_socktype = SOCK_STREAM;
					hints.ai_protocol = IPPROTO_TCP;
					struct addrinfo* result;
					if( getaddrinfo(itr->strIP.c_str(), strPort.c_str(), &hints, &result) == 0 ){
						itr->sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
						if( itr->sock != INVALID_SOCKET ){
							//�m���u���b�L���O���[�h��
							unsigned long x = 1;
							if( ioctlsocket(itr->sock, FIONBIO, &x) == SOCKET_ERROR ){
								closesocket(itr->sock);
								itr->sock = INVALID_SOCKET;
							}else if( connect(itr->sock, result->ai_addr, (int)result->ai_addrlen) != SOCKET_ERROR ){
								itr->bConnect = TRUE;
							}else if( WSAGetLastError() != WSAEWOULDBLOCK ){
								closesocket(itr->sock);
								itr->sock = INVALID_SOCKET;
							}
						}
						freeaddrinfo(result);
					}
				}
			}
		} //m_sendLock

		std::list<vector<BYTE>> item;
		size_t sendListSizeOrStop;
		{
			CBlockLock lock(&pSys->m_sendLock);

			if( pSys->m_TSBuff.empty() == false ){
				item.splice(item.end(), pSys->m_TSBuff, pSys->m_TSBuff.begin());
				DWORD dwCmd[2] = { dwCount, (DWORD)(item.back().size() - sizeof(DWORD) * 2) };
				memcpy(&item.back().front(), dwCmd, sizeof(dwCmd));
			}
			//�r���Ō��邱�Ƃ͂Ȃ�
			sendListSizeOrStop = pSys->m_SendList.size();
		}

		if( item.empty() || sendListSizeOrStop == 0 ){
			if( WaitForSingleObject(pSys->m_stopSendEvent.Handle(), item.empty() ? 100 : 0) != WAIT_TIMEOUT ){
				//�L�����Z�����ꂽ
				break;
			}
		}else{
			for( size_t i = 0; i < sendListSizeOrStop; i++ ){
				SOCKET sock = INVALID_SOCKET;
				size_t adjust = item.back().size();
				{
					CBlockLock lock(&pSys->m_sendLock);
					if( pSys->m_SendList[i].bConnect ){
						sock = pSys->m_SendList[i].sock;
					}
					if( pSys->m_SendList[i].dwPort >> 16 == 1 ){
						adjust -= sizeof(DWORD) * 2;
					}
				}
				for(;;){
					if( WaitForSingleObject(pSys->m_stopSendEvent.Handle(), 0) != WAIT_TIMEOUT ){
						//�L�����Z�����ꂽ
						sendListSizeOrStop = 0;
						break;
					}
					if( sock == INVALID_SOCKET ){
						break;
					}
					if( adjust != 0 ){
						int ret = send(sock, (char*)(item.back().data() + item.back().size() - adjust), (int)adjust, 0);
						if( ret == 0 || (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) ){
							closesocket(sock);
							CBlockLock lock(&pSys->m_sendLock);
							pSys->m_SendList[i].sock = INVALID_SOCKET;
							pSys->m_SendList[i].bConnect = FALSE;
							break;
						}else if( ret != SOCKET_ERROR ){
							adjust -= ret;
						}
					}
					if( adjust == 0 ){
						dwCount++;
						break;
					}
					//�������҂�
					fd_set wmask;
					FD_ZERO(&wmask);
					FD_SET(sock, &wmask);
					struct timeval tv10msec = {0, 10000};
					select((int)sock + 1, NULL, &wmask, NULL, &tv10msec);
				}
			}
			if( sendListSizeOrStop == 0 ){
				break;
			}
		}
	}

	CBlockLock lock(&pSys->m_sendLock);
	for( auto itr = pSys->m_SendList.begin(); itr != pSys->m_SendList.end(); itr++ ){
		if( itr->sock != INVALID_SOCKET ){
			//�����M�f�[�^���̂Ă��Ă����Ȃ��̂�shutdown()�͏ȗ�
			closesocket(itr->sock);
			itr->sock = INVALID_SOCKET;
			itr->bConnect = FALSE;
		}
	}
}
