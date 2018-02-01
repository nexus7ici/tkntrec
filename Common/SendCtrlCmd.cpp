#include "stdafx.h"
#include "SendCtrlCmd.h"
#ifndef SEND_CTRL_CMD_NO_TCP
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif
#include "StringUtil.h"

CSendCtrlCmd::CSendCtrlCmd(void)
{
	this->tcpFlag = FALSE;
	this->connectTimeOut = CONNECT_TIMEOUT;

	this->pipeName = CMD2_EPG_SRV_PIPE;
	this->eventName = CMD2_EPG_SRV_EVENT_WAIT_CONNECT;

	this->sendIP = L"127.0.0.1";
	this->sendPort = 5678;

}


CSendCtrlCmd::~CSendCtrlCmd(void)
{
#ifndef SEND_CTRL_CMD_NO_TCP
	SetSendMode(FALSE);
#endif
}

#ifndef SEND_CTRL_CMD_NO_TCP

//�R�}���h���M���@�̐ݒ�
//�����F
// tcpFlag		[IN] TRUE�FTCP/IP���[�h�AFALSE�F���O�t���p�C�v���[�h
void CSendCtrlCmd::SetSendMode(
	BOOL tcpFlag_
	)
{
	if( this->tcpFlag == FALSE && tcpFlag_ ){
		WSAData wsaData;
		WSAStartup(MAKEWORD(2, 0), &wsaData);
		this->tcpFlag = TRUE;
	}else if( this->tcpFlag && tcpFlag_ == FALSE ){
		WSACleanup();
		this->tcpFlag = FALSE;
	}
}

#endif

//���O�t���p�C�v���[�h���̐ڑ����ݒ�
//EpgTimerSrv.exe�ɑ΂���R�}���h�͐ݒ肵�Ȃ��Ă��i�f�t�H���g�l�ɂȂ��Ă���j
//�����F
// eventName	[IN]�r������pEvent�̖��O
// pipeName		[IN]�ڑ��p�C�v�̖��O
void CSendCtrlCmd::SetPipeSetting(
	LPCWSTR eventName_,
	LPCWSTR pipeName_
	)
{
	this->eventName = eventName_;
	this->pipeName = pipeName_;
}

//���O�t���p�C�v���[�h���̐ڑ����ݒ�i�ڔ��Ƀv���Z�XID�𔺂��^�C�v�j
//�����F
// pid			[IN]�v���Z�XID
void CSendCtrlCmd::SetPipeSetting(
	LPCWSTR eventName_,
	LPCWSTR pipeName_,
	DWORD pid
	)
{
	Format(this->eventName, L"%s%d", eventName_, pid);
	Format(this->pipeName, L"%s%d", pipeName_, pid);
}

//TCP/IP���[�h���̐ڑ����ݒ�
//�����F
// ip			[IN]�ڑ���IP
// port			[IN]�ڑ���|�[�g
void CSendCtrlCmd::SetNWSetting(
	const wstring& ip,
	DWORD port
	)
{
	this->sendIP = ip;
	this->sendPort = port;
}

//�ڑ��������̃^�C���A�E�g�ݒ�
// timeOut		[IN]�^�C���A�E�g�l�i�P�ʁFms�j
void CSendCtrlCmd::SetConnectTimeOut(
	DWORD timeOut
	)
{
	this->connectTimeOut = timeOut;
}

static DWORD ReadFileAll(HANDLE hFile, BYTE* lpBuffer, DWORD dwToRead)
{
	DWORD dwRet = 0;
	for( DWORD dwRead; dwRet < dwToRead && ReadFile(hFile, lpBuffer + dwRet, dwToRead - dwRet, &dwRead, NULL); dwRet += dwRead );
	return dwRet;
}

DWORD CSendCtrlCmd::SendPipe(LPCWSTR pipeName_, LPCWSTR eventName_, DWORD timeOut, CMD_STREAM* send, CMD_STREAM* res)
{
	if( pipeName_ == NULL || eventName_ == NULL || send == NULL || res == NULL ){
		return CMD_ERR_INVALID_ARG;
	}

	//�ڑ��҂�
	//CreateEvent()���Ă͂����Ȃ��B�C�x���g���쐬����̂̓T�[�o�̎d���̂͂�
	//CreateEvent()���Ă��܂��ƃT�[�o���I��������͏�Ƀ^�C���A�E�g�܂ő҂�����邱�ƂɂȂ�
	HANDLE waitEvent = OpenEvent(SYNCHRONIZE, FALSE, eventName_);
	if( waitEvent == NULL ){
		return CMD_ERR_CONNECT;
	}
	DWORD dwRet = WaitForSingleObject(waitEvent, timeOut);
	CloseHandle(waitEvent);
	if( dwRet == WAIT_TIMEOUT ){
		return CMD_ERR_TIMEOUT;
	}else if( dwRet != WAIT_OBJECT_0 ){
		return CMD_ERR_CONNECT;
	}

	//�ڑ�
	HANDLE pipe = CreateFile( pipeName_, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( pipe == INVALID_HANDLE_VALUE ){
		_OutputDebugString(L"*+* ConnectPipe Err:%d\r\n", GetLastError());
		return CMD_ERR_CONNECT;
	}

	DWORD write = 0;

	//���M
	DWORD head[2];
	head[0] = send->param;
	head[1] = send->dataSize;
	if( WriteFile(pipe, head, sizeof(DWORD)*2, &write, NULL ) == FALSE ){
		CloseHandle(pipe);
		return CMD_ERR;
	}
	if( send->dataSize > 0 ){
		if( WriteFile(pipe, send->data.get(), send->dataSize, &write, NULL ) == FALSE ){
			CloseHandle(pipe);
			return CMD_ERR;
		}
	}

	//��M
	if( ReadFileAll(pipe, (BYTE*)head, sizeof(head)) != sizeof(head) ){
		CloseHandle(pipe);
		return CMD_ERR;
	}
	res->param = head[0];
	res->dataSize = head[1];
	if( res->dataSize > 0 ){
		res->data.reset(new BYTE[res->dataSize]);
		if( ReadFileAll(pipe, res->data.get(), res->dataSize) != res->dataSize ){
			CloseHandle(pipe);
			return CMD_ERR;
		}
	}
	CloseHandle(pipe);

	return res->param;
}

#ifndef SEND_CTRL_CMD_NO_TCP

static int RecvAll(SOCKET sock, char* buf, int len, int flags)
{
	int n = 0;
	while( n < len ){
		int ret = recv(sock, buf + n, len - n, flags);
		if( ret < 0 ){
			return ret;
		}else if( ret <= 0 ){
			break;
		}
		n += ret;
	}
	return n;
}

DWORD CSendCtrlCmd::SendTCP(const wstring& ip, DWORD port, DWORD timeOut, CMD_STREAM* sendCmd, CMD_STREAM* resCmd)
{
	if( sendCmd == NULL || resCmd == NULL ){
		return CMD_ERR_INVALID_ARG;
	}

	string ipA, strPort;
	WtoUTF8(ip, ipA);
	Format(strPort, "%d", port);

	struct addrinfo hints = {};
	hints.ai_flags = AI_NUMERICHOST;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* result;
	if( getaddrinfo(ipA.c_str(), strPort.c_str(), &hints, &result) != 0 ){
		return CMD_ERR_INVALID_ARG;
	}
	SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if( sock != INVALID_SOCKET &&
	    connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR ){
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
	freeaddrinfo(result);

	if( sock == INVALID_SOCKET ){
		return CMD_ERR_CONNECT;
	}

	//���M
	DWORD head[256];
	head[0] = sendCmd->param;
	head[1] = sendCmd->dataSize;
	DWORD extSize = 0;
	if( sendCmd->dataSize > 0 ){
		extSize = min(sendCmd->dataSize, (DWORD)(sizeof(head) - sizeof(DWORD)*2));
		memcpy(head + 2, sendCmd->data.get(), extSize);
	}
	if( send(sock, (char*)head, sizeof(DWORD)*2 + extSize, 0) == SOCKET_ERROR ||
	    sendCmd->dataSize > extSize && send(sock, (char*)sendCmd->data.get() + extSize, sendCmd->dataSize - extSize, 0) == SOCKET_ERROR ){
		closesocket(sock);
		return CMD_ERR;
	}
	//��M
	if( RecvAll(sock, (char*)head, sizeof(DWORD)*2, 0) != sizeof(DWORD)*2 ){
		closesocket(sock);
		return CMD_ERR;
	}
	resCmd->param = head[0];
	resCmd->dataSize = head[1];
	if( resCmd->dataSize > 0 ){
		resCmd->data.reset(new BYTE[resCmd->dataSize]);
		if( RecvAll(sock, (char*)resCmd->data.get(), resCmd->dataSize, 0) != (int)resCmd->dataSize ){
			closesocket(sock);
			return CMD_ERR;
		}
	}
	closesocket(sock);

	return resCmd->param;
}

#endif

DWORD CSendCtrlCmd::SendFileCopy(
	const wstring& val,
	BYTE** resVal,
	DWORD* resValSize
	)
{
	CMD_STREAM res;
	DWORD ret = SendCmdData(CMD2_EPG_SRV_FILE_COPY, val, &res);

	if( ret == CMD_SUCCESS ){
		if( res.dataSize == 0 ){
			return CMD_ERR;
		}
		*resValSize = res.dataSize;
		*resVal = res.data.release();
	}
	return ret;
}

DWORD CSendCtrlCmd::SendGetEpgFile2(
	const wstring& val,
	BYTE** resVal,
	DWORD* resValSize
	)
{
	CMD_STREAM res;
	DWORD ret = SendCmdData2(CMD2_EPG_SRV_GET_EPG_FILE2, val, &res);

	if( ret == CMD_SUCCESS ){
		WORD ver = 0;
		DWORD readSize = 0;
		if( ReadVALUE(&ver, res.data, res.dataSize, &readSize) == FALSE || res.dataSize <= readSize ){
			return CMD_ERR;
		}
		*resValSize = res.dataSize - readSize;
		*resVal = res.data.release();
		memmove(*resVal, *resVal + readSize, *resValSize);
	}
	return ret;
}

DWORD CSendCtrlCmd::SendCmdStream(CMD_STREAM* send, CMD_STREAM* res)
{
	DWORD ret = CMD_ERR;
	CMD_STREAM tmpRes;

	if( res == NULL ){
		res = &tmpRes;
	}
	if( this->tcpFlag == FALSE ){
		ret = SendPipe(this->pipeName.c_str(), this->eventName.c_str(), this->connectTimeOut, send, res);
	}
#ifndef SEND_CTRL_CMD_NO_TCP
	else{
		ret = SendTCP(this->sendIP, this->sendPort, this->connectTimeOut, send, res);
	}
#endif

	return ret;
}

