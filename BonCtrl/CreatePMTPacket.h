#pragma once

#include "../Common/ErrDef.h"
#include "../Common/EpgTimerUtil.h"
#include "../Common/TSPacketUtil.h"
#include "../Common/TSBuffUtil.h"

class CCreatePMTPacket
{
public:
	//����TS�p�P�b�g����Ȃ��Ɖ�͂ł��Ȃ�
	static const DWORD ERR_NEED_NEXT_PACKET = 20;
	//�o�[�W�����̕ύX�Ȃ����߉�͕s�v
	static const DWORD ERR_NO_CHAGE = 30;

	CCreatePMTPacket(void);

	//PMT�쐬���̃��[�h
	//�����F
	// needCaption			[IN]�����f�[�^���܂߂邩�ǂ����iTRUE:�܂߂�AFALSE�F�܂߂Ȃ��j
	// needData				[IN]�f�[�^�J���[�Z�����܂߂邩�ǂ����iTRUE:�܂߂�AFALSE�F�܂߂Ȃ��j
	void SetCreateMode(
		BOOL needCaption_,
		BOOL needData_
	);

	//�쐬���ƂȂ�PMT�̃p�P�b�g�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// packet			//[IN] PMT�̃p�P�b�g
	DWORD AddData(
		CTSPacketUtil* packet
	);

	//�K�v��PID�����m�F
	//�߂�l�F
	// TRUE�i�K�v�j�AFALSE�i�s�K�v�j
	//�����F
	// PID				//[IN]�m�F����PID
	BOOL IsNeedPID(
		WORD PID
	);

	//�쐬PMT�̃o�b�t�@�|�C���^���擾
	//�߂�l�F
	// �쐬PMT�̃o�b�t�@�|�C���^
	//�����F
	// buff					[OUT]�쐬����PMT�p�P�b�g�ւ̃|�C���^�i����Ăяo�����܂ŗL���j
	// size					[OUT]buff�̃T�C�Y
	// incrementFlag		[IN]TS�p�P�b�g��Counter���C���N�������g���邩�ǂ����iTRUE:����AFALSE�F���Ȃ��j
	BOOL GetPacket(
		BYTE** buff,
		DWORD* size,
		BOOL incrementFlag = TRUE
	);

	//���������N���A
	void Clear();

	BYTE GetVersion();

protected:
	DWORD DecodePMT(BYTE* data, DWORD dataSize);

	void CreatePMT();
	void CreatePacket();
	void IncrementCounter();
protected:
	CTSBuffUtil buffUtil;

	BOOL needCaption;
	BOOL needData;

	WORD lastPmtPID;
	WORD lastPcrPID;
	WORD lastPgNumber;
	BYTE lastVersion;

	vector<BYTE> firstDescBuff;

	struct SECOND_DESC_BUFF {
		BYTE stream_type;
		WORD elementary_PID;
		vector<BYTE> descBuff;
		WORD quality;
		WORD qualityPID;
	};
	vector<SECOND_DESC_BUFF> secondDescBuff;

	vector<WORD> emmPIDList;

	vector<WORD> needPIDList;

	vector<BYTE> createPSI;
	
	vector<BYTE> createPacket;

	BYTE createVer;
	BYTE createCounter;
};
