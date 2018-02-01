#pragma once

#include "../Common/ErrDef.h"
#include "../Common/TSPacketUtil.h"
#include "../Common/StringUtil.h"

#include "BonCtrlDef.h"
#include "SendUDP.h"
#include "SendTCP.h"
#include "WriteTSFile.h"
#include "PMTUtil.h"
#include "CATUtil.h"
#include "CreatePMTPacket.h"
#include "CreatePATPacket.h"
#include "DropCount.h"
#include <functional>

class COneServiceUtil
{
public:
	COneServiceUtil(void);
	~COneServiceUtil(void);

	//�����Ώ�ServiceID��ݒ�
	//�����F
	// SID			[IN]ServiceID�B0xFFFF�őS�T�[�r�X�ΏہB
	void SetSID(
		WORD SID_
	);

	//�ݒ肳��Ă鏈���Ώۂ�ServiceID���擾
	//�߂�l�F
	// ServiceID
	WORD GetSID();

	//UDP�ő��M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// sendList		[IN/OUT]���M�惊�X�g�BNULL�Œ�~�BPort�͎��ۂɑ��M�Ɏg�p����Port���Ԃ�B
	BOOL SendUdp(
		vector<NW_SEND_INFO>* sendList
		);

	//TCP�ő��M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// sendList		[IN/OUT]���M�惊�X�g�BNULL�Œ�~�BPort�͎��ۂɑ��M�Ɏg�p����Port���Ԃ�B
	BOOL SendTcp(
		vector<NW_SEND_INFO>* sendList
		);

	//�o�͗pTS�f�[�^�𑗂�
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// data		[IN]TS�f�[�^
	// size		[IN]data�̃T�C�Y
	// funcGetPresent	[IN]EPG�̌��ݔԑgID�𒲂ׂ�֐�
	BOOL AddTSBuff(
		BYTE* data,
		DWORD size,
		const std::function<int(WORD, WORD, WORD)>& funcGetPresent
		);

	void SetPmtPID(
		WORD TSID,
		WORD pmtPID_
		);

	void SetEmmPID(
		const map<WORD,WORD>& PIDMap
		);

	//�t�@�C���ۑ����J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// fileName				[IN]�ۑ��t�@�C���p�X
	// overWriteFlag		[IN]����t�@�C�������ݎ��ɏ㏑�����邩�ǂ����iTRUE�F����AFALSE�F���Ȃ��j
	// pittariFlag			[IN]�҂����胂�[�h�iTRUE�F����AFALSE�F���Ȃ��j
	// pittariONID			[IN]�҂����胂�[�h�Ř^�悷��ONID
	// pittariTSID			[IN]�҂����胂�[�h�Ř^�悷��TSID
	// pittariSID			[IN]�҂����胂�[�h�Ř^�悷��SID
	// pittariEventID		[IN]�҂����胂�[�h�Ř^�悷��C�x���gID
	// createSize			[IN]�t�@�C���쐬���Ƀf�B�X�N�ɗ\�񂷂�e��
	// saveFolder			[IN]�g�p����t�H���_�ꗗ
	// saveFolderSub		[IN]HDD�̋󂫂��Ȃ��Ȃ����ꍇ�Ɉꎞ�I�Ɏg�p����t�H���_
	BOOL StartSave(
		const wstring& fileName,
		BOOL overWriteFlag,
		BOOL pittariFlag,
		WORD pittariONID_,
		WORD pittariTSID_,
		WORD pittariSID_,
		WORD pittariEventID_,
		ULONGLONG createSize,
		const vector<REC_FILE_SET_INFO>& saveFolder,
		const vector<wstring>& saveFolderSub,
		int maxBuffCount
	);

	//�t�@�C���ۑ����I������
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL EndSave();

	//�^�撆���ǂ���
	//�߂�l�F
	// TRUE�i�^�撆�j�AFALSE�i���Ă��Ȃ��j
	BOOL IsRec();

	//�X�N�����u�����������̓���ݒ�
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// enable		[IN] TRUE�i��������j�AFALSE�i�������Ȃ��j
	void SetScramble(
		BOOL enable
		);

	//�X�N�����u�������������s�����ǂ���
	//�߂�l�F
	// TRUE�i��������j�AFALSE�i�������Ȃ��j
	BOOL GetScramble(
		);

	//�����ƃf�[�^�����܂߂邩�ǂ���
	//�����F
	// enableCaption		[IN]������ TRUE�i�܂߂�j�AFALSE�i�܂߂Ȃ��j
	// enableData			[IN]�f�[�^������ TRUE�i�܂߂�j�AFALSE�i�܂߂Ȃ��j
	void SetServiceMode(
		BOOL enableCaption,
		BOOL enableData
		);

	//�G���[�J�E���g���N���A����
	void ClearErrCount();

	//�h���b�v�ƃX�N�����u���̃J�E���g���擾����
	//�����F
	// drop				[OUT]�h���b�v��
	// scramble			[OUT]�X�N�����u����
	void GetErrCount(ULONGLONG* drop, ULONGLONG* scramble);


	//�^�撆�̃t�@�C���̃t�@�C���p�X���擾����
	//�����F
	// filePath			[OUT]�ۑ��t�@�C����
	// subRecFlag		[OUT]�T�u�^�悪�����������ǂ���
	void GetSaveFilePath(
		wstring* filePath,
		BOOL* subRecFlag
		);

	//�h���b�v�ƃX�N�����u���̃J�E���g��ۑ�����
	//�����F
	// filePath			[IN]�ۑ��t�@�C����
	void SaveErrCount(
		const wstring& filePath
		);

	void SetSignalLevel(
		float signalLv
		);

	//�^�撆�̃t�@�C���̏o�̓T�C�Y���擾����
	//�����F
	// writeSize			[OUT]�o�̓T�C�Y
	void GetRecWriteSize(
		__int64* writeSize
		);

	void SetBonDriver(
		const wstring& bonDriver
		);
	void SetPIDName(
		const map<WORD, string>& pidName
		);
	void SetNoLogScramble(
		BOOL noLog
		);
protected:
	WORD SID;

	BOOL enableScramble;

	vector<HANDLE> udpPortMutex;
	vector<HANDLE> tcpPortMutex;

	std::unique_ptr<CSendUDP> sendUdp;
	std::unique_ptr<CSendTCP> sendTcp;
	std::unique_ptr<CWriteTSFile> writeFile;

	vector<BYTE> buff;

	CCreatePATPacket createPat;
	CCreatePMTPacket createPmt;

	WORD pmtPID;
	map<WORD,WORD> emmPIDMap;

	CDropCount dropCount;

	BOOL pittariStart;
	BOOL pittariEndChk;

	WORD lastPMTVer;

	wstring pittariFileName;
	BOOL pittariOverWriteFlag;
	ULONGLONG pittariCreateSize;
	vector<REC_FILE_SET_INFO> pittariSaveFolder;
	vector<wstring> pittariSaveFolderSub;
	WORD pittariONID;
	WORD pittariTSID;
	WORD pittariSID;
	WORD pittariEventID;
	wstring pittariRecFilePath;
	int pittariMaxBuffCount;

protected:
	BOOL WriteData(BYTE* data, DWORD size);

	void StratPittariRec();
	void StopPittariRec();
};

