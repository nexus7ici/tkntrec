#include "stdafx.h"
#include "SendUDP.h"
#include "../Common/PathUtil.h"

static const int SNDBUF_SIZE = 3 * 1024 * 1024;

CSendUDP::CSendUDP(void)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}

CSendUDP::~CSendUDP(void)
{
	CloseUpload();

	WSACleanup();

}

BOOL CSendUDP::StartUpload( vector<NW_SEND_INFO>* List )
{
	if( SockList.size() > 0 ){
		return FALSE;
	}

	for( int i=0; i<(int)List->size(); i++ ){
		SOCKET_DATA Item;
		string ipA, strPort;
		WtoUTF8((*List)[i].ipString, ipA);
		Format(strPort, "%d", (WORD)(*List)[i].port);
		struct addrinfo hints = {};
		hints.ai_flags = AI_NUMERICHOST;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		struct addrinfo* result;
		if( getaddrinfo(ipA.c_str(), strPort.c_str(), &hints, &result) != 0 ){
			continue;
		}
		Item.addrlen = min(result->ai_addrlen, sizeof(Item.addr));
		memcpy(&Item.addr, result->ai_addr, Item.addrlen);
		Item.sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		freeaddrinfo(result);
		if( Item.sock == INVALID_SOCKET ){
			CloseUpload();
			return FALSE;
		}
		//�m���u���b�L���O���[�h��
		ULONG x = 1;
		if( ioctlsocket(Item.sock,FIONBIO, &x) == SOCKET_ERROR ||
		    setsockopt(Item.sock, SOL_SOCKET, SO_SNDBUF, (const char *)&SNDBUF_SIZE, sizeof(SNDBUF_SIZE)) == SOCKET_ERROR ){
			closesocket(Item.sock);
			CloseUpload();
			return FALSE;
		}

		if( (*List)[i].broadcastFlag ){
			BOOL b=1;
			setsockopt(Item.sock,SOL_SOCKET, SO_BROADCAST, (char *)&b, sizeof(b));
		}
		SockList.push_back(Item);
	}
	m_uiSendSize = GetPrivateProfileInt(L"SET", L"UDPPacket", 128, GetModuleIniPath().c_str()) * 188;

	return TRUE;
}

BOOL CSendUDP::CloseUpload()
{
	for( int i=0; i<(int)SockList.size(); i++ ){
		closesocket(SockList[i].sock);
	}
	SockList.clear();

	return TRUE;
}

void CSendUDP::SendData(BYTE* pbBuff, DWORD dwSize)
{
	for( DWORD dwRead=0; dwRead<dwSize; ){
		//�y�C���[�h�����BBonDriver_UDP�ɑ���ꍇ�͎�M�T�C�Y48128�ȉ��łȂ���΂Ȃ�Ȃ�
		int iSendSize = min(max((int)m_uiSendSize, 188), (int)(dwSize - dwRead));
		for( size_t i=0; i<SockList.size(); i++ ){
			int iRet = sendto(SockList[i].sock, (char*)(pbBuff + dwRead), iSendSize, 0, (struct sockaddr *)&SockList[i].addr, (int)SockList[i].addrlen);
			if( iRet == SOCKET_ERROR ){
				if( WSAGetLastError() == WSAEWOULDBLOCK ){
					//���M�������ǂ�����SNDBUF_SIZE�Ŏw�肵���o�b�t�@���s���Ă��܂���
					//�ш悪����Ȃ��Ƃ��͂ǂ����~���Ă��h���b�v���邵���Ȃ��̂ŁASleep()�ɂ��t���[����͂��Ȃ�
					OutputDebugString(L"Dropped\r\n");
				}
			}
		}
		dwRead += iSendSize;
	}
}