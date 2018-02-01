#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <list>

#include "../../Common/StringUtil.h"
#include "../../Common/ThreadUtil.h"
#pragma comment(lib, "Ws2_32.lib")

class CSendTSTCPMain
{
public:
	CSendTSTCPMain(void);
	~CSendTSTCPMain(void);

	//���M���ǉ�
	//�߂�l�F�G���[�R�[�h
	DWORD AddSendAddr(
		LPCWSTR lpcwszIP,
		DWORD dwPort
		);

	//���M��N���A
	//�߂�l�F�G���[�R�[�h
	DWORD ClearSendAddr(
		);

	//�f�[�^���M���J�n
	//�߂�l�F�G���[�R�[�h
	DWORD StartSend(
		);

	//�f�[�^���M���~
	//�߂�l�F�G���[�R�[�h
	DWORD StopSend(
		);

	//�f�[�^���M���J�n
	//�߂�l�F�G���[�R�[�h
	DWORD AddSendData(
		BYTE* pbData,
		DWORD dwSize
		);

	//���M�o�b�t�@���N���A
	//�߂�l�F�G���[�R�[�h
	DWORD ClearSendBuff(
		);


protected:
	CAutoResetEvent m_stopSendEvent;
	thread_ m_sendThread;

	recursive_mutex_ m_sendLock;

	std::list<vector<BYTE>> m_TSBuff;

	struct SEND_INFO {
		string strIP;
		DWORD dwPort;
		SOCKET sock;
		BOOL bConnect;
	};
	vector<SEND_INFO> m_SendList;

protected:
	static void SendThread(CSendTSTCPMain* pSys);

};
