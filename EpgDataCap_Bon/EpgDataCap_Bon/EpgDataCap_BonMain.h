#pragma once

#include "../../BonCtrl/BonCtrl.h"
#include "../../Common/PathUtil.h"
#include "../../Common/TimeUtil.h"
#include "../../Common/PipeServer.h"

class CEpgDataCap_BonMain
{
public:
	CEpgDataCap_BonMain(void);
	~CEpgDataCap_BonMain(void);

	//�ʒm�pHWND��ݒ�
	void SetHwnd(HWND wnd);

	//�ݒ���s��
	void ReloadSetting();
	
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
		LPCWSTR bonDriverFile
		);

	//���[�h���Ă���BonDriver�̊J��
	void CloseBonDriver();

	//�T�[�r�X�ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// serviceList				[OUT]�T�[�r�X���̃��X�g
	DWORD GetServiceList(
		vector<CH_DATA4>* serviceList
		);

	//���[�h����BonDriver�̃t�@�C�������擾����i���[�h�������Ă��邩�̔���j
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
	// ONID			[IN]�ύX�`�����l����orignal_network_id
	// TSID			[IN]�ύX�`�����l����transport_stream_id
	// SID			[IN]�ύX�`�����l����service_id
	DWORD SetCh(
		WORD ONID,
		WORD TSID,
		WORD SID
		);

	//�`�����l���ύX
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// ONID			[IN]�ύX�`�����l����orignal_network_id
	// TSID			[IN]�ύX�`�����l����transport_stream_id
	// SID			[IN]�ύX�`�����l����service_id
	// space		[IN]�ύX�`�����l����space
	// ch			[IN]�ύX�`�����l����ch
	DWORD SetCh(
		WORD ONID,
		WORD TSID,
		WORD SID,
		DWORD space,
		DWORD ch
		);

	//���݂̃T�[�r�X�擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// ONID			[IN]���݂�orignal_network_id
	// TSID			[IN]���݂�transport_stream_id
	// SID			[IN]���݂�service_id
	void GetCh(
		WORD* ONID,
		WORD* TSID,
		WORD* SID
		);

	//�`�����l���ύX�����ǂ���
	//�߂�l�F
	// TRUE�i�ύX���j�AFALSE�i�����j
	BOOL IsChChanging(BOOL* chChgErr);

	//�O������Ȃǂ�CH�ύX���ꂽ�ꍇ��SID�̂ݐݒ�
	void SetSID(
		WORD SID
		);

	//UDP�ő��M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// enableFlag		[IN]TRUE�i�J�n�j�AFALSE�i��~�j
	BOOL SendUDP(
		BOOL enableFlag
		);

	//TCP�ő��M���s��
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// enableFlag		[IN]TRUE�i�J�n�j�AFALSE�i��~�j
	BOOL SendTCP(
		BOOL enableFlag
		);

	//UDP���M�̐ݒ萔���擾
	size_t GetCountUDP() { return this->setUdpSendList.size(); }
	//TCP���M�̐ݒ萔���擾
	size_t GetCountTCP() { return this->setTcpSendList.size(); }
	//UDP�̑��M����ꗗ�擾
	vector<NW_SEND_INFO> GetSendUDPList() { return this->udpSendList; }
	//TCP�̑��M����ꗗ�擾
	vector<NW_SEND_INFO> GetSendTCPList() { return this->tcpSendList; }

	//�w��T�[�r�X�̌���or����EPG�����擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// nextFlag					[IN]TRUE�i���̔ԑg�j�AFALSE�i���݂̔ԑg�j
	// epgInfo					[OUT]EPG���
	DWORD GetEpgInfo(
		BOOL nextFlag,
		wstring* epgInfo
		);

	//�V�O�i�����x���̎擾
	//�߂�l�F
	// �V�O�i�����x��
	float GetSignalLevel();

	//�G���[�J�E���g���N���A����
	void ClearErrCount(
		);

	//�h���b�v�ƃX�N�����u���̃J�E���g���擾����
	//�����F
	// drop				[OUT]�h���b�v��
	// scramble			[OUT]�X�N�����u����
	void GetErrCount(
		ULONGLONG* drop,
		ULONGLONG* scramble
		);

	//�^����J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StartRec(
		);

	//�^����~����
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StopRec();

	//�^�撆���ǂ������擾����
	// TRUE�i�^�撆�j�AFALSE�i�^�悵�Ă��Ȃ��j
	BOOL IsRec();

	//�\��^����~����
	void StopReserveRec();

	//�`�����l���X�L�������J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StartChScan() { return this->bonCtrl.StartChScan(); }

	//�`�����l���X�L�������L�����Z������
	void StopChScan() { this->bonCtrl.StopChScan(); }

	//�`�����l���X�L�����̏�Ԃ��擾����
	//�߂�l�F
	// �X�e�[�^�X
	//�����F
	// space		[OUT]�X�L�������̕���CH��space
	// ch			[OUT]�X�L�������̕���CH��ch
	// chName		[OUT]�X�L�������̕���CH�̖��O
	// chkNum		[OUT]�`�F�b�N�ς݂̐�
	// totalNum		[OUT]�`�F�b�N�Ώۂ̑���
	CBonCtrl::JOB_STATUS GetChScanStatus(
		DWORD* space,
		DWORD* ch,
		wstring* chName,
		DWORD* chkNum,
		DWORD* totalNum
		);

	//EPG�擾���J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL StartEpgCap() { return this->bonCtrl.StartEpgCap(NULL); }

	//EPG�擾���~����
	void StopEpgCap() { this->bonCtrl.StopEpgCap(); }

	//EPG�擾�̃X�e�[�^�X���擾����
	//�߂�l�F
	// �X�e�[�^�X
	//�����F
	// info			[OUT]�擾���̃T�[�r�X
	CBonCtrl::JOB_STATUS GetEpgCapStatus(EPGCAP_SERVICE_INFO* info);

	//View�A�v���̋N�����s��
	void ViewAppOpen();

	void StartServer();
	BOOL StopServer(BOOL checkOnlyFlag = FALSE);

	BOOL GetViewStatusInfo(
		float* signal,
		DWORD* space,
		DWORD* ch,
		ULONGLONG* drop,
		ULONGLONG* scramble
		);

	void CtrlCmdCallbackInvoked();

protected:
	HWND msgWnd;

	CBonCtrl bonCtrl;
	CPipeServer pipeServer;

	vector<wstring> recFolderList;

	DWORD nwCtrlID;
	vector<NW_SEND_INFO> setUdpSendList;
	vector<NW_SEND_INFO> setTcpSendList;
	vector<NW_SEND_INFO> udpSendList;
	vector<NW_SEND_INFO> tcpSendList;

	wstring recFileName;
	BOOL overWriteFlag;

	BOOL enableScrambleFlag;
	BOOL enableEMMFlag;

	BOOL allService;
	BOOL needCaption;
	BOOL needData;

	int openWait;

	wstring viewPath;
	wstring viewOpt;

	WORD lastONID;
	WORD lastTSID;
	WORD lastSID;

	DWORD recCtrlID;

	int outCtrlID;
	vector<DWORD> cmdCtrlList;

	CMD_STREAM* cmdCapture;
	CMD_STREAM* resCapture;
};

