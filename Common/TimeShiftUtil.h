#pragma once
#include "../BonCtrl/PacketInit.h"
#include "../BonCtrl/SendUDP.h"
#include "../BonCtrl/SendTCP.h"
#include "../BonCtrl/CreatePATPacket.h"
#include "TSPacketUtil.h"
#include "ThreadUtil.h"

class CTimeShiftUtil
{
public:
	CTimeShiftUtil(void);
	~CTimeShiftUtil(void);

	//UDP/TCP���M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// val		[IN/OUT]���M����
	BOOL Send(
		NWPLAY_PLAY_INFO* val
		);

	//�^�C���V�t�g�p�t�@�C�����J��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// filePath		[IN]�^�C���V�t�g�p�o�b�t�@�t�@�C���̃p�X
	// fileMode		[IN]�^��ς݃t�@�C���Đ����[�h
	BOOL OpenTimeShift(
		LPCWSTR filePath_,
		BOOL fileMode_
		);

	//�^�C���V�t�g���M���J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StartTimeShift();

	//�^�C���V�t�g���M���~����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StopTimeShift();

	//���݂̑��M�ʒu�ƗL���ȃt�@�C���T�C�Y���擾����
	//�����F
	// filePos		[OUT]�t�@�C���ʒu
	// fileSize		[OUT]�t�@�C���T�C�Y
	void GetFilePos(__int64* filePos, __int64* fileSize);

	//���M�J�n�ʒu��ύX����
	//�����F
	// filePos		[IN]�t�@�C���ʒu
	void SetFilePos(__int64 filePos);

protected:
	recursive_mutex_ utilLock;
	recursive_mutex_ ioLock;
	CSendUDP sendUdp;
	CSendTCP sendTcp;
	wstring sendUdpIP;
	wstring sendTcpIP;
	DWORD sendUdpPort;
	DWORD sendTcpPort;
	HANDLE udpPortMutex;
	HANDLE tcpPortMutex;

	wstring filePath;
	WORD PCR_PID;

	BOOL fileMode;
	int seekJitter;
	__int64 currentFilePos;

	thread_ readThread;
	BOOL readStopFlag;
	HANDLE readFile;
	HANDLE seekFile;
protected:
	static void ReadThread(CTimeShiftUtil* sys);
	__int64 GetAvailableFileSize() const;
};

