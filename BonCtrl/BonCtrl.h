#pragma once

#include <windows.h>

#include "../Common/StructDef.h"
#include "../Common/EpgTimerUtil.h"
#include "../Common/StringUtil.h"

#include "BonDriverUtil.h"
#include "PacketInit.h"
#include "TSOut.h"
#include "ChSetUtil.h"
#include <list>
#if !defined(_MSC_VER) || _MSC_VER >= 1900
#include <atomic>
#endif

class CBonCtrl
{
public:
	//�`�����l���X�L�����AEPG�擾�̃X�e�[�^�X�p
	enum JOB_STATUS {
		ST_STOP = -4,	//��~��
		ST_WORKING,		//���s��
		ST_COMPLETE,	//����
		ST_CANCEL,		//�L�����Z�����ꂽ
	};

	CBonCtrl(void);
	~CBonCtrl(void);

	//BonDriver�t�H���_���w��
	//�����F
	// bonDriverFolderPath		[IN]BonDriver�t�H���_�p�X
	void SetBonDriverFolder(
		LPCWSTR bonDriverFolderPath
		);

	void SetEMMMode(BOOL enable);

	void SetNoLogScramble(BOOL noLog);

	void SetTsBuffMaxCount(DWORD tsBuffMaxCount_, int writeBuffMaxCount_);

	//BonDriver�t�H���_��BonDriver_*.dll���
	//�߂�l�F
	// �����ł���BonDriver�ꗗ
	vector<wstring> EnumBonDriver();

	//BonDriver�����[�h���ă`�����l�����Ȃǂ��擾�i�t�@�C�����Ŏw��j
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// bonDriverFile	[IN]EnumBonDriver�Ŏ擾���ꂽBonDriver�̃t�@�C����
	DWORD OpenBonDriver(
		LPCWSTR bonDriverFile,
		int openWait = 200
		);

	//���[�h���Ă���BonDriver�̊J��
	void CloseBonDriver();

	//���[�h����BonDriver�̃t�@�C�������擾����i���[�h�������Ă��邩�̔���j
	//���X���b�h�Z�[�t
	//�߂�l�F
	// TRUE�i�����j�FFALSE�iOpen�Ɏ��s���Ă���j
	//�����F
	// bonDriverFile		[OUT]BonDriver�̃t�@�C����(NULL��)
	BOOL GetOpenBonDriver(
		wstring* bonDriverFile
		);

	//�`�����l���ύX
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// space			[IN]�ύX�`�����l����Space
	// ch				[IN]�ύX�`�����l���̕���Ch
	DWORD SetCh(
		DWORD space,
		DWORD ch
		);

	//�`�����l���ύX
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// ONID			[IN]�ύX�`�����l����orignal_network_id
	// TSID			[IN]�ύX�`�����l����transport_stream_id
	// SID			[IN]�ύX�`�����l����service_id
	DWORD SetCh(
		WORD ONID,
		WORD TSID,
		WORD SID
		);

	//�`�����l���ύX�����ǂ���
	//���X���b�h�Z�[�t
	//�߂�l�F
	// TRUE�i�ύX���j�AFALSE�i�����j
	BOOL IsChChanging(BOOL* chChgErr);

	//���݂̃X�g���[����ID���擾����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// ONID		[OUT]originalNetworkID
	// TSID		[OUT]transportStreamID
	BOOL GetStreamID(
		WORD* ONID,
		WORD* TSID
		);

	//�V�O�i�����x���̎擾
	//�߂�l�F
	// �V�O�i�����x��
	float GetSignalLevel();

	//�T�[�r�X�ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// serviceList				[OUT]�T�[�r�X���̃��X�g
	DWORD GetServiceList(
		vector<CH_DATA4>* serviceList
		);

	//TS�X�g���[������p�R���g���[�����쐬����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// id			[OUT]���䎯��ID
	BOOL CreateServiceCtrl(
		DWORD* id
		);

	//TS�X�g���[������p�R���g���[�����쐬����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// id			[IN]���䎯��ID
	BOOL DeleteServiceCtrl(
		DWORD id
		);

	//����Ώۂ̃T�[�r�X��ݒ肷��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s
	//�����F
	// id			[IN]���䎯��ID
	// serviceID	[IN]�ΏۃT�[�r�XID�A0xFFFF�őS�T�[�r�X�Ώ�
	BOOL SetServiceID(
		DWORD id,
		WORD serviceID
		);

	BOOL GetServiceID(
		DWORD id,
		WORD* serviceID
		);

	//UDP�ő��M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// id			[IN]���䎯��ID
	// sendList		[IN/OUT]���M�惊�X�g�BNULL�Œ�~�BPort�͎��ۂɑ��M�Ɏg�p����Port���Ԃ�B
	BOOL SendUdp(
		DWORD id,
		vector<NW_SEND_INFO>* sendList
		);

	//TCP�ő��M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// id			[IN]���䎯��ID
	// sendList		[IN/OUT]���M�惊�X�g�BNULL�Œ�~�BPort�͎��ۂɑ��M�Ɏg�p����Port���Ԃ�B
	BOOL SendTcp(
		DWORD id,
		vector<NW_SEND_INFO>* sendList
		);

	//�t�@�C���ۑ����J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// id					[IN]���䎯��ID
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
		DWORD id,
		const wstring& fileName,
		BOOL overWriteFlag,
		BOOL pittariFlag,
		WORD pittariONID,
		WORD pittariTSID,
		WORD pittariSID,
		WORD pittariEventID,
		ULONGLONG createSize,
		const vector<REC_FILE_SET_INFO>& saveFolder,
		const vector<wstring>& saveFolderSub
	);

	//�t�@�C���ۑ����I������
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// id			[IN]���䎯��ID
	BOOL EndSave(
		DWORD id
		);

	//�X�N�����u�����������̓���ݒ�
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// id			[IN]���䎯��ID
	// enable		[IN] TRUE�i��������j�AFALSE�i�������Ȃ��j
	BOOL SetScramble(
		DWORD id,
		BOOL enable
		);

	//�����ƃf�[�^�����܂߂邩�ǂ���
	//�����F
	// id					[IN]���䎯��ID
	// enableCaption		[IN]������ TRUE�i�܂߂�j�AFALSE�i�܂߂Ȃ��j
	// enableData			[IN]�f�[�^������ TRUE�i�܂߂�j�AFALSE�i�܂߂Ȃ��j
	void SetServiceMode(
		DWORD id,
		BOOL enableCaption,
		BOOL enableData
		);

	//�G���[�J�E���g���N���A����
	//�����F
	// id					[IN]���䎯��ID
	void ClearErrCount(
		DWORD id
		);

	//�h���b�v�ƃX�N�����u���̃J�E���g���擾����
	//�����F
	// id					[IN]���䎯��ID
	// drop					[OUT]�h���b�v��
	// scramble				[OUT]�X�N�����u����
	void GetErrCount(
		DWORD id,
		ULONGLONG* drop,
		ULONGLONG* scramble
		);

	//�^�撆�̃t�@�C���̃t�@�C���p�X���擾����
	//���X���b�h�Z�[�t
	//�����F
	// id					[IN]���䎯��ID
	// filePath				[OUT]�ۑ��t�@�C����
	// subRecFlag			[OUT]�T�u�^�悪�����������ǂ���
	void GetSaveFilePath(
		DWORD id,
		wstring* filePath,
		BOOL* subRecFlag
		) {
		this->tsOut.GetSaveFilePath(id, filePath, subRecFlag);
	}

	//�h���b�v�ƃX�N�����u���̃J�E���g��ۑ�����
	//�����F
	// id					[IN]���䎯��ID
	// filePath				[IN]�ۑ��t�@�C����
	void SaveErrCount(
		DWORD id,
		wstring filePath
		);

	//�^�撆�̃t�@�C���̏o�̓T�C�Y���擾����
	//�����F
	// id					[IN]���䎯��ID
	// writeSize			[OUT]�ۑ��t�@�C����
	void GetRecWriteSize(
		DWORD id,
		__int64* writeSize
		);

	//�w��T�[�r�X�̌���or����EPG�����擾����
	//���X���b�h�Z�[�t
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// originalNetworkID		[IN]�擾�Ώۂ�originalNetworkID
	// transportStreamID		[IN]�擾�Ώۂ�transportStreamID
	// serviceID				[IN]�擾�Ώۂ�ServiceID
	// nextFlag					[IN]TRUE�i���̔ԑg�j�AFALSE�i���݂̔ԑg�j
	// epgInfo					[OUT]EPG���
	DWORD GetEpgInfo(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		BOOL nextFlag,
		EPGDB_EVENT_INFO* epgInfo
		) {
		return this->tsOut.GetEpgInfo(originalNetworkID, transportStreamID, serviceID, nextFlag, epgInfo);
	}

	//�w��C�x���g��EPG�����擾����
	//���X���b�h�Z�[�t
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// originalNetworkID		[IN]�擾�Ώۂ�originalNetworkID
	// transportStreamID		[IN]�擾�Ώۂ�transportStreamID
	// serviceID				[IN]�擾�Ώۂ�ServiceID
	// eventID					[IN]�擾�Ώۂ�EventID
	// pfOnlyFlag				[IN]p/f����̂݌������邩�ǂ���
	// epgInfo					[OUT]EPG���
	DWORD SearchEpgInfo(
		WORD originalNetworkID,
		WORD transportStreamID,
		WORD serviceID,
		WORD eventID,
		BYTE pfOnlyFlag,
		EPGDB_EVENT_INFO* epgInfo
		) {
		return this->tsOut.SearchEpgInfo(originalNetworkID, transportStreamID, serviceID, eventID, pfOnlyFlag, epgInfo);
	}
	
	//PC���v�����Ƃ����X�g���[�����ԂƂ̍����擾����
	//���X���b�h�Z�[�t
	//�߂�l�F
	// ���̕b��
	int GetTimeDelay() { return this->tsOut.GetTimeDelay(); }

	//�^�撆���ǂ������擾����
	//���X���b�h�Z�[�t
	// TRUE�i�^�撆�j�AFALSE�i�^�悵�Ă��Ȃ��j
	BOOL IsRec() { return this->tsOut.IsRec(); }

	//�`�����l���X�L�������J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StartChScan();

	//�`�����l���X�L�������L�����Z������
	void StopChScan();

	//�`�����l���X�L�����̏�Ԃ��擾����
	//�߂�l�F
	// �X�e�[�^�X
	//�����F
	// space		[OUT]�X�L�������̕���CH��space
	// ch			[OUT]�X�L�������̕���CH��ch
	// chName		[OUT]�X�L�������̕���CH�̖��O
	// chkNum		[OUT]�`�F�b�N�ς݂̐�
	// totalNum		[OUT]�`�F�b�N�Ώۂ̑���
	JOB_STATUS GetChScanStatus(
		DWORD* space,
		DWORD* ch,
		wstring* chName,
		DWORD* chkNum,
		DWORD* totalNum
		);

	//EPG�擾���J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// chList		[IN]EPG�擾����`�����l���ꗗ(NULL��)
	BOOL StartEpgCap(
		vector<EPGCAP_SERVICE_INFO>* chList
		);

	//EPG�擾���~����
	void StopEpgCap(
		);

	//EPG�擾�̃X�e�[�^�X���擾����
	//��info==NULL�̏ꍇ�Ɍ���X���b�h�Z�[�t
	//�߂�l�F
	// �X�e�[�^�X
	//�����F
	// info			[OUT]�擾���̃T�[�r�X
	JOB_STATUS GetEpgCapStatus(
		EPGCAP_SERVICE_INFO* info
		);

	//�o�b�N�O���E���h�ł�EPG�擾�ݒ�
	//�����F
	// enableLive	[IN]�������Ɏ擾����
	// enableRec	[IN]�^�撆�Ɏ擾����
	// enableRec	[IN]EPG�擾����`�����l���ꗗ
	// *Basic		[IN]�P�`�����l�������{���̂ݎ擾���邩�ǂ���
	// backStartWaitSec	[IN]Ch�؂�ւ��A�^��J�n��A�o�b�N�O���E���h�ł�EPG�擾���J�n����܂ł̕b��
	void SetBackGroundEpgCap(
		BOOL enableLive,
		BOOL enableRec,
		BOOL BSBasic,
		BOOL CS1Basic,
		BOOL CS2Basic,
		BOOL CS3Basic,
		DWORD backStartWaitSec
		);

	BOOL GetViewStatusInfo(
		DWORD id,
		float* signal,
		DWORD* space,
		DWORD* ch,
		ULONGLONG* drop,
		ULONGLONG* scramble
		);

protected:
	CBonDriverUtil bonUtil;
	CPacketInit packetInit;
	CTSOut tsOut;
	CChSetUtil chUtil;

	CRITICAL_SECTION buffLock;
	std::list<vector<BYTE>> tsBuffList;

	HANDLE analyzeThread;
	HANDLE analyzeEvent;
	BOOL analyzeStopFlag;

	//�`�����l���X�L�����p
	HANDLE chScanThread;
	HANDLE chScanStopEvent;
	struct CHK_CH_INFO {
		DWORD space;
		DWORD ch;
		wstring spaceName;
		wstring chName;
	};
	//�X�L��������const����̂�
	vector<CHK_CH_INFO> chScanChkList;
#if defined(_MSC_VER) && _MSC_VER < 1900
	LONG chScanIndexOrStatus;
#else
	std::atomic<int> chScanIndexOrStatus;
#endif

	//EPG�擾�p
	HANDLE epgCapThread;
	HANDLE epgCapStopEvent;
	//�擾����const����̂�
	vector<EPGCAP_SERVICE_INFO> epgCapChList;
#if defined(_MSC_VER) && _MSC_VER < 1900
	LONG epgCapIndexOrStatus;
#else
	std::atomic<int> epgCapIndexOrStatus;
#endif

	HANDLE epgCapBackThread;
	HANDLE epgCapBackStopEvent;
	BOOL enableLiveEpgCap;
	BOOL enableRecEpgCap;

	BOOL epgCapBackBSBasic;
	BOOL epgCapBackCS1Basic;
	BOOL epgCapBackCS2Basic;
	BOOL epgCapBackCS3Basic;
	DWORD epgCapBackStartWaitSec;
	DWORD tsBuffMaxCount;
	int writeBuffMaxCount;
protected:
	DWORD ProcessSetCh(
		DWORD space,
		DWORD ch,
		BOOL chScan = FALSE
		);

	static void GetEpgDataFilePath(WORD ONID, WORD TSID, wstring& epgDataFilePath);

	static void RecvCallback(void* param, BYTE* data, DWORD size, DWORD remain);
	static UINT WINAPI AnalyzeThread(LPVOID param);

	static UINT WINAPI ChScanThread(LPVOID param);
	static UINT WINAPI EpgCapThread(LPVOID param);

	void StartBackgroundEpgCap();
	void StopBackgroundEpgCap();
	static UINT WINAPI EpgCapBackThread(LPVOID param);
};

