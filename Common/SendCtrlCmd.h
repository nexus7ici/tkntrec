#pragma once

#include "StructDef.h"

#include "CtrlCmdDef.h"
#include "ErrDef.h"
#include "CtrlCmdUtil.h"

class CSendCtrlCmd
{
public:
	CSendCtrlCmd(void);
	~CSendCtrlCmd(void);

#ifndef SEND_CTRL_CMD_NO_TCP
	//�R�}���h���M���@�̐ݒ�
	//�����F
	// tcpFlag		[IN] TRUE�FTCP/IP���[�h�AFALSE�F���O�t���p�C�v���[�h
	void SetSendMode(
		BOOL tcpFlag_
		);
#endif

	//���O�t���p�C�v���[�h���̐ڑ����ݒ�
	//EpgTimerSrv.exe�ɑ΂���R�}���h�͐ݒ肵�Ȃ��Ă��i�f�t�H���g�l�ɂȂ��Ă���j
	//�����F
	// eventName	[IN]�r������pEvent�̖��O
	// pipeName		[IN]�ڑ��p�C�v�̖��O
	void SetPipeSetting(
		LPCWSTR eventName_,
		LPCWSTR pipeName_
		);

	//���O�t���p�C�v���[�h���̐ڑ����ݒ�i�ڔ��Ƀv���Z�XID�𔺂��^�C�v�j
	//�����F
	// pid			[IN]�v���Z�XID
	void SetPipeSetting(
		LPCWSTR eventName_,
		LPCWSTR pipeName_,
		DWORD pid
		);

	//TCP/IP���[�h���̐ڑ����ݒ�
	//�����F
	// ip			[IN]�ڑ���IP
	// port			[IN]�ڑ���|�[�g
	void SetNWSetting(
		const wstring& ip,
		DWORD port
		);

	//�ڑ��������̃^�C���A�E�g�ݒ�
	// timeOut		[IN]�^�C���A�E�g�l�i�P�ʁFms�j
	void SetConnectTimeOut(
		DWORD timeOut
		);

	//Program.txt��ǉ��ōēǂݍ��݂���
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendAddloadReserve(){
		return SendCmdWithoutData(CMD2_EPG_SRV_ADDLOAD_RESERVE);
	}

	//EPG�f�[�^���ēǂݍ��݂���
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendReloadEpg(){
		return SendCmdWithoutData(CMD2_EPG_SRV_RELOAD_EPG);
	}

	//�ݒ�����ēǂݍ��݂���
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendReloadSetting(){
		return SendCmdWithoutData(CMD2_EPG_SRV_RELOAD_SETTING);
	}

	//EpgTimerSrv.exe���I������
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendClose(){
		return SendCmdWithoutData(CMD2_EPG_SRV_CLOSE);
	}

	//EpgTimerSrv.exe�̃p�C�v�ڑ�GUI�Ƃ��ăv���Z�X��o�^����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// processID			[IN]�v���Z�XID
	DWORD SendRegistGUI(DWORD processID){
		return SendCmdData(CMD2_EPG_SRV_REGIST_GUI, processID);
	}

	//EpgTimerSrv.exe�̃p�C�v�ڑ�GUI�o�^����������
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// processID			[IN]�v���Z�XID
	DWORD SendUnRegistGUI(DWORD processID){
		return SendCmdData(CMD2_EPG_SRV_UNREGIST_GUI, processID);
	}
	
	//EpgTimerSrv.exe��TCP�ڑ�GUI�Ƃ��ăv���Z�X��o�^����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// port					[IN]�|�[�g
	DWORD SendRegistTCP(DWORD port){
		return SendCmdData(CMD2_EPG_SRV_REGIST_GUI_TCP, port);
	}

	//EpgTimerSrv.exe��TCP�ڑ�GUI�o�^����������
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// port					[IN]�|�[�g
	DWORD SendUnRegistTCP(DWORD port){
		return SendCmdData(CMD2_EPG_SRV_UNREGIST_GUI_TCP, port);
	}

	//�\��ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�\��ꗗ
	DWORD SendEnumReserve(
		vector<RESERVE_DATA>* val
		){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_RESERVE, val);
	}

	//�\������擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// reserveID		[IN]�擾������̗\��ID
	// val				[OUT]�\����
	DWORD SendGetReserve(DWORD reserveID, RESERVE_DATA* val){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_GET_RESERVE, reserveID, val);
	}

	//�\���ǉ�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�ǉ�����\��ꗗ
	DWORD SendAddReserve(const vector<RESERVE_DATA>& val){
		return SendCmdData(CMD2_EPG_SRV_ADD_RESERVE, val);
	}

	//�\����폜����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�폜����\��ID�ꗗ
	DWORD SendDelReserve(const vector<DWORD>& val){
		return SendCmdData(CMD2_EPG_SRV_DEL_RESERVE, val);
	}

	//�\���ύX����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�ύX����\��ꗗ
	DWORD SendChgReserve(const vector<RESERVE_DATA>& val){
		return SendCmdData(CMD2_EPG_SRV_CHG_RESERVE, val);
	}

	//�`���[�i�[���Ƃ̗\��ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[OUT]�\��ꗗ
	DWORD SendEnumTunerReserve(vector<TUNER_RESERVE_INFO>* val){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_TUNER_RESERVE, val);
	}

	//�^��ςݏ��ꗗ�擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�^��ςݏ��ꗗ
	DWORD SendEnumRecInfo(
		vector<REC_FILE_INFO>* val
		){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_RECINFO, val);
	}
	
	//�^��ςݏ����폜����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�폜����ID�ꗗ
	DWORD SendDelRecInfo(const vector<DWORD>& val){
		return SendCmdData(CMD2_EPG_SRV_DEL_RECINFO, val);
	}

	//�T�[�r�X�ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[OUT]�T�[�r�X�ꗗ
	DWORD SendEnumService(
		vector<EPGDB_SERVICE_INFO>* val
		){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_SERVICE, val);
	}

	//�T�[�r�X�w��Ŕԑg�����ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// service			[IN]ONID<<32 | TSID<<16 | SID�Ƃ����T�[�r�XID
	// val				[OUT]�ԑg���ꗗ
	DWORD SendEnumPgInfo(
		ULONGLONG service,
		vector<EPGDB_EVENT_INFO>* val
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_ENUM_PG_INFO, service, val);
	}

	//�w��C�x���g�̔ԑg�����擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// pgID				[IN]ONID<<48 | TSID<<32 | SID<<16 | EventID�Ƃ���ID
	// val				[OUT]�ԑg���
	DWORD SendGetPgInfo(
		ULONGLONG pgID,
		EPGDB_EVENT_INFO* val
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_GET_PG_INFO, pgID, val);
	}

	//�w��L�[���[�h�Ŕԑg������������
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// key				[IN]�����L�[�i�����w�莞�͂܂Ƃ߂Č������ʂ��Ԃ�j
	// val				[OUT]�ԑg���ꗗ
	DWORD SendSearchPg(
		const vector<EPGDB_SEARCH_KEY_INFO>& key,
		vector<EPGDB_EVENT_INFO>* val
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_SEARCH_PG, key, val);
	}

	//�ԑg���ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[OUT]�ԑg���ꗗ
	DWORD SendEnumPgAll(
		vector<EPGDB_SERVICE_EVENT_INFO>* val
		){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_PG_ALL, val);
	}

	//�����\��o�^�����ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�����ꗗ
	DWORD SendEnumEpgAutoAdd(
		vector<EPG_AUTO_ADD_DATA>* val
		){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_AUTO_ADD, val);
	}

	//�����\��o�^������ǉ�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendAddEpgAutoAdd(
		const vector<EPG_AUTO_ADD_DATA>& val
		){
		return SendCmdData(CMD2_EPG_SRV_ADD_AUTO_ADD, val);
	}

	//�����\��o�^�������폜����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendDelEpgAutoAdd(
		const vector<DWORD>& val
		){
		return SendCmdData(CMD2_EPG_SRV_DEL_AUTO_ADD, val);
	}

	//�����\��o�^������ύX����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendChgEpgAutoAdd(
		const vector<EPG_AUTO_ADD_DATA>& val
		){
		return SendCmdData(CMD2_EPG_SRV_CHG_AUTO_ADD, val);
	}

	//�����\��o�^�����ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�����ꗗ	
	DWORD SendEnumManualAdd(
		vector<MANUAL_AUTO_ADD_DATA>* val
		){
		return ReceiveCmdData(CMD2_EPG_SRV_ENUM_MANU_ADD, val);
	}

	//�����\��o�^������ǉ�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendAddManualAdd(
		const vector<MANUAL_AUTO_ADD_DATA>& val
		){
		return SendCmdData(CMD2_EPG_SRV_ADD_MANU_ADD, val);
	}

	//�v���O�����\�񎩓��o�^�̏����폜
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendDelManualAdd(
		const vector<DWORD>& val
		){
		return SendCmdData(CMD2_EPG_SRV_DEL_MANU_ADD, val);
	}

	//�v���O�����\�񎩓��o�^�̏����ύX
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendChgManualAdd(
		const vector<MANUAL_AUTO_ADD_DATA>& val
		){
		return SendCmdData(CMD2_EPG_SRV_CHG_MANU_ADD, val);
	}


	DWORD SendChkSuspend(){
		return SendCmdWithoutData(CMD2_EPG_SRV_CHK_SUSPEND);
	}

	DWORD SendSuspend(
		WORD val
		){
		return SendCmdData(CMD2_EPG_SRV_SUSPEND, val);
	}

	DWORD SendReboot(){
		return SendCmdWithoutData(CMD2_EPG_SRV_REBOOT);
	}

	DWORD SendEpgCapNow(){
		return SendCmdWithoutData(CMD2_EPG_SRV_EPG_CAP_NOW);
	}

	//�w��t�@�C����]������
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�t�@�C����
	// resVal		[OUT]�t�@�C���̃o�C�i���f�[�^
	// resValSize	[OUT]resVal�̃T�C�Y
	DWORD SendFileCopy(
		const wstring& val,
		BYTE** resVal,
		DWORD* resValSize
		);

	//PlugIn�t�@�C���̈ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]1:ReName�A2:Write
	// resVal		[OUT]�t�@�C�����ꗗ
	DWORD SendEnumPlugIn(
		WORD val,
		vector<wstring>* resVal
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_ENUM_PLUGIN, val, resVal);
	}

	//TVTest�̃`�����l���؂�ւ��p�̏����擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]ONID<<32 | TSID<<16 | SID�Ƃ����T�[�r�XID
	// resVal		[OUT]�`�����l�����
	DWORD SendGetChgChTVTest(
		ULONGLONG val,
		TVTEST_CH_CHG_INFO* resVal
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_GET_CHG_CH_TVTEST, val, resVal);
	}

	//�ݒ�t�@�C��(ini)�̍X�V��ʒm������
	//�߂�l�F
	//�����F
	// val			[IN]Sender
	// �G���[�R�[�h
	DWORD SendProfileUpdate(
		const wstring& val
		){
		return SendCmdData(CMD2_EPG_SRV_PROFILE_UPDATE, val);
	}

	//�l�b�g���[�N���[�h��EpgDataCap_Bon�̃`�����l����؂�ւ�
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// chInfo				[IN]�`�����l�����
	DWORD SendNwTVSetCh(
		const SET_CH_INFO& val
		){
		return SendCmdData(CMD2_EPG_SRV_NWTV_SET_CH, val);
	}

	//�l�b�g���[�N���[�h�ŋN������EpgDataCap_Bon���I��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendNwTVClose(
		){
		return SendCmdWithoutData(CMD2_EPG_SRV_NWTV_CLOSE);
	}

	//�l�b�g���[�N���[�h�ŋN������Ƃ��̃��[�h
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]���[�h�i1:UDP 2:TCP 3:UDP+TCP�j
	DWORD SendNwTVMode(
		DWORD val
		){
		return SendCmdData(CMD2_EPG_SRV_NWTV_MODE, val);
	}

	//�X�g���[���z�M�p�t�@�C�����J��
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�J���t�@�C���̃T�[�o�[���t�@�C���p�X
	// resVal			[OUT]����pCtrlID
	DWORD SendNwPlayOpen(
		const wstring& val,
		DWORD* resVal
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_NWPLAY_OPEN, val, resVal);
	}

	//�X�g���[���z�M�p�t�@�C�������
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]����pCtrlID
	DWORD SendNwPlayClose(
		DWORD val
		){
		return SendCmdData(CMD2_EPG_SRV_NWPLAY_CLOSE, val);
	}

	//�X�g���[���z�M�J�n
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]����pCtrlID
	DWORD SendNwPlayStart(
		DWORD val
		){
		return SendCmdData(CMD2_EPG_SRV_NWPLAY_PLAY, val);
	}

	//�X�g���[���z�M��~
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]����pCtrlID
	DWORD SendNwPlayStop(
		DWORD val
		){
		return SendCmdData(CMD2_EPG_SRV_NWPLAY_STOP, val);
	}

	//�X�g���[���z�M�Ō��݂̑��M�ʒu�Ƒ��t�@�C���T�C�Y���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN/OUT]�T�C�Y���
	DWORD SendNwPlayGetPos(
		NWPLAY_POS_CMD* val
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_NWPLAY_GET_POS, *val, val);
	}

	//�X�g���[���z�M�ő��M�ʒu���V�[�N����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�T�C�Y���
	DWORD SendNwPlaySetPos(
		const NWPLAY_POS_CMD* val
		){
		return SendCmdData(CMD2_EPG_SRV_NWPLAY_SET_POS, *val);
	}

	//�X�g���[���z�M�ő��M���ݒ肷��
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN/OUT]�T�C�Y���
	DWORD SendNwPlaySetIP(
		NWPLAY_PLAY_INFO* val
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_NWPLAY_SET_IP, *val, val);
	}

	//�X�g���[���z�M�p�t�@�C�����^�C���V�t�g���[�h�ŊJ��
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�\��ID
	// resVal			[OUT]�t�@�C���p�X��CtrlID
	DWORD SendNwTimeShiftOpen(
		DWORD val,
		NWPLAY_TIMESHIFT_INFO* resVal
		){
		return SendAndReceiveCmdData(CMD2_EPG_SRV_NWPLAY_TF_OPEN, val, resVal);
	}

//�R�}���h�o�[�W�����Ή���
	//�\��ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�\��ꗗ
	DWORD SendEnumReserve2(
		vector<RESERVE_DATA>* val
		){
		return ReceiveCmdData2(CMD2_EPG_SRV_ENUM_RESERVE2, val);
	}

	//�\������擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// reserveID		[IN]�擾������̗\��ID
	// val				[OUT]�\����
	DWORD SendGetReserve2(DWORD reserveID, RESERVE_DATA* val){
		return SendAndReceiveCmdData2(CMD2_EPG_SRV_GET_RESERVE2, reserveID, val);
	}

	//�\���ǉ�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�ǉ�����\��ꗗ
	DWORD SendAddReserve2(const vector<RESERVE_DATA>& val){
		return SendCmdData2(CMD2_EPG_SRV_ADD_RESERVE2, val);
	}

	//�\���ύX����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�ύX����\��ꗗ
	DWORD SendChgReserve2(const vector<RESERVE_DATA>& val){
		return SendCmdData2(CMD2_EPG_SRV_CHG_RESERVE2, val);
	}

	//�\��ǉ����\���m�F����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�\����
	// resVal			[OUT]�ǉ��\���̃X�e�[�^�X
	DWORD SendAddChkReserve2(const RESERVE_DATA& val, WORD* resVal){
		return SendAndReceiveCmdData2(CMD2_EPG_SRV_ADDCHK_RESERVE2, val, resVal);
	}


	//EPG�f�[�^�t�@�C���̃^�C���X�^���v�擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�擾�t�@�C����
	// resVal			[OUT]�^�C���X�^���v
	DWORD SendGetEpgFileTime2(const wstring& val, LONGLONG* resVal){
		return SendAndReceiveCmdData2(CMD2_EPG_SRV_GET_EPG_FILETIME2, val, resVal);
	}

	//EPG�f�[�^�t�@�C���擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�t�@�C����
	// resVal		[OUT]�t�@�C���̃o�C�i���f�[�^
	// resValSize	[OUT]resVal�̃T�C�Y
	DWORD SendGetEpgFile2(
		const wstring& val,
		BYTE** resVal,
		DWORD* resValSize
		);

	//�����\��o�^�����ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�����ꗗ
	DWORD SendEnumEpgAutoAdd2(
		vector<EPG_AUTO_ADD_DATA>* val
		){
		return ReceiveCmdData2(CMD2_EPG_SRV_ENUM_AUTO_ADD2, val);
	}

	//�����\��o�^������ǉ�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendAddEpgAutoAdd2(
		const vector<EPG_AUTO_ADD_DATA>& val
		){
		return SendCmdData2(CMD2_EPG_SRV_ADD_AUTO_ADD2, val);
	}

	//�����\��o�^������ύX����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendChgEpgAutoAdd2(
		const vector<EPG_AUTO_ADD_DATA>& val
		){
		return SendCmdData2(CMD2_EPG_SRV_CHG_AUTO_ADD2, val);
	}

	//�����\��o�^�����ꗗ���擾����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�����ꗗ	
	DWORD SendEnumManualAdd2(
		vector<MANUAL_AUTO_ADD_DATA>* val
		){
		return ReceiveCmdData2(CMD2_EPG_SRV_ENUM_MANU_ADD2, val);
	}

	//�����\��o�^������ǉ�����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendAddManualAdd2(
		const vector<MANUAL_AUTO_ADD_DATA>& val
		){
		return SendCmdData2(CMD2_EPG_SRV_ADD_MANU_ADD2, val);
	}

	//�v���O�����\�񎩓��o�^�̏����ύX
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�����ꗗ
	DWORD SendChgManualAdd2(
		const vector<MANUAL_AUTO_ADD_DATA>& val
		){
		return SendCmdData2(CMD2_EPG_SRV_CHG_MANU_ADD2, val);
	}

	//�^��ςݏ��ꗗ�擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[OUT]�^��ςݏ��ꗗ
	DWORD SendEnumRecInfo2(
		vector<REC_FILE_INFO>* val
		){
		return ReceiveCmdData2(CMD2_EPG_SRV_ENUM_RECINFO2, val);
	}

	//�^��ςݏ��̃v���e�N�g�ύX
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val			[IN]�^��ςݏ��ꗗ
	DWORD SendChgProtectRecInfo2(
		const vector<REC_FILE_INFO>& val
		){
		return SendCmdData2(CMD2_EPG_SRV_CHG_PROTECT_RECINFO2, val);
	}

//�^�C�}�[GUI�iEpgTimer_Bon.exe�j�p

	//�_�C�A���O��O�ʂɕ\��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendGUIShowDlg(
		){
		return SendCmdWithoutData(CMD2_TIMER_GUI_SHOW_DLG);
	}

	//�\��ꗗ�̏�񂪍X�V���ꂽ
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendGUIUpdateReserve(
		){
		return SendCmdWithoutData(CMD2_TIMER_GUI_UPDATE_RESERVE);
	}

	//EPG�f�[�^�̍ēǂݍ��݂���������
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendGUIUpdateEpgData(
		){
		return SendCmdWithoutData(CMD2_TIMER_GUI_UPDATE_EPGDATA);
	}

	//���X�V��ʒm����
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val				[IN]�ʒm���
	DWORD SendGUINotifyInfo2(const NOTIFY_SRV_INFO& val){
		return SendCmdData2(CMD2_TIMER_GUI_SRV_STATUS_NOTIFY2, val);
	}

//View�A�v���iEpgDataCap_Bon.exe�j���N��
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// exeCmd			[IN]�R�}���h���C��
	// PID				[OUT]�N������exe��PID
	DWORD SendGUIExecute(
		const wstring& exeCmd,
		DWORD* PID
		){
		return SendAndReceiveCmdData(CMD2_TIMER_GUI_VIEW_EXECUTE, exeCmd, PID);
	}

	//�X�^���o�C�A�x�~�A�V���b�g�_�E���ɓ����Ă������̊m�F�����[�U�[�ɍs��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendGUIQuerySuspend(
		BYTE rebootFlag,
		BYTE suspendMode
		){
		return SendCmdData(CMD2_TIMER_GUI_QUERY_SUSPEND, (WORD)(rebootFlag<<8|suspendMode));
	}

	//PC�ċN���ɓ����Ă������̊m�F�����[�U�[�ɍs��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendGUIQueryReboot(
		BYTE rebootFlag
		){
		return SendCmdData(CMD2_TIMER_GUI_QUERY_REBOOT, (WORD)(rebootFlag<<8));
	}

	//�T�[�o�[�̃X�e�[�^�X�ύX�ʒm
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// status			[IN]�X�e�[�^�X
	DWORD SendGUIStatusChg(
		WORD status
		){
		return SendCmdData(CMD2_TIMER_GUI_SRV_STATUS_CHG, status);
	}


//View�A�v���iEpgDataCap_Bon.exe�j�p

	//BonDriver�̐؂�ւ�
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// bonDriver			[IN]BonDriver�t�@�C����
	DWORD SendViewSetBonDrivere(
		const wstring& bonDriver
		){
		return SendCmdData(CMD2_VIEW_APP_SET_BONDRIVER, bonDriver);
	}

	//�g�p����BonDriver�̃t�@�C�������擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// bonDriver			[OUT]BonDriver�t�@�C����
	DWORD SendViewGetBonDrivere(
		wstring* bonDriver
		){
		return ReceiveCmdData(CMD2_VIEW_APP_GET_BONDRIVER, bonDriver);
	}

	//�`�����l���؂�ւ�
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// chInfo				[IN]�`�����l�����
	DWORD SendViewSetCh(
		const SET_CH_INFO& chInfo
		){
		return SendCmdData(CMD2_VIEW_APP_SET_CH, chInfo);
	}

	//�����g�̎��Ԃ�PC���Ԃ̌덷�擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// delaySec				[OUT]�덷�i�b�j
	DWORD SendViewGetDelay(
		int* delaySec
		){
		return ReceiveCmdData(CMD2_VIEW_APP_GET_DELAY, delaySec);
	}

	//���݂̏�Ԃ��擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// status				[OUT]���
	DWORD SendViewGetStatus(
		DWORD* status
		){
		return ReceiveCmdData(CMD2_VIEW_APP_GET_STATUS, status);
	}

	//�A�v���P�[�V�����̏I��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendViewAppClose(
		){
		return SendCmdWithoutData(CMD2_VIEW_APP_CLOSE);
	}

	//���ʗpID�̐ݒ�
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// id				[IN]ID
	DWORD SendViewSetID(
		int id
		){
		return SendCmdData(CMD2_VIEW_APP_SET_ID, id);
	}

	//���ʗpID�̎擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// id				[OUT]ID
	DWORD SendViewGetID(
		int* id
		){
		return ReceiveCmdData(CMD2_VIEW_APP_GET_ID, id);
	}

	//�\��^��p��GUI�L�[�v
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendViewSetStandbyRec(
		DWORD keepFlag
		){
		return SendCmdData(CMD2_VIEW_APP_SET_STANDBY_REC, keepFlag);
	}

	//�X�g���[������p�R���g���[���쐬
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// ctrlID				[OUT]����ID
	DWORD SendViewCreateCtrl(
		DWORD* ctrlID
		){
		return ReceiveCmdData(CMD2_VIEW_APP_CREATE_CTRL, ctrlID);
	}

	//�X�g���[������p�R���g���[���폜
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// ctrlID				[IN]����ID
	DWORD SendViewDeleteCtrl(
		DWORD ctrlID
		){
		return SendCmdData(CMD2_VIEW_APP_DELETE_CTRL, ctrlID);
	}

	//����R���g���[���̐ݒ�
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val					[IN]�ݒ�l
	DWORD SendViewSetCtrlMode(
		const SET_CTRL_MODE& val
		){
		return SendCmdData(CMD2_VIEW_APP_SET_CTRLMODE, val);
	}

	//�^�揈���J�n
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val					[IN]�ݒ�l
	DWORD SendViewStartRec(
		const SET_CTRL_REC_PARAM& val
		){
		return SendCmdData(CMD2_VIEW_APP_REC_START_CTRL, val);
	}

	//�^�揈����~
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val					[IN]�ݒ�l
	// resVal				[OUT]�h���b�v��
	DWORD SendViewStopRec(
		const SET_CTRL_REC_STOP_PARAM& val,
		SET_CTRL_REC_STOP_RES_PARAM* resVal
		){
		return SendAndReceiveCmdData(CMD2_VIEW_APP_REC_STOP_CTRL, val, resVal);
	}

	//�^�撆�̃t�@�C���p�X���擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val					[OUT]�t�@�C���p�X
	DWORD SendViewGetRecFilePath(
		DWORD ctrlID,
		wstring* resVal
		){
		return SendAndReceiveCmdData(CMD2_VIEW_APP_REC_FILE_PATH, ctrlID, resVal);
	}

	//�����^����~
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendViewStopRecAll(
		){
		return SendCmdWithoutData(CMD2_VIEW_APP_REC_STOP_ALL);
	}

	//�t�@�C���o�͂����T�C�Y���擾
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// resVal					[OUT]�t�@�C���o�͂����T�C�Y
	DWORD SendViewGetWriteSize(
		DWORD ctrlID,
		__int64* resVal
		){
		return SendAndReceiveCmdData(CMD2_VIEW_APP_REC_WRITE_SIZE, ctrlID, resVal);
	}

	//EPG�擾�J�n
	//�߂�l�F
	// �G���[�R�[�h
	//�����F
	// val					[IN]�擾�`�����l�����X�g
	DWORD SendViewEpgCapStart(
		const vector<SET_CH_INFO>& val
		){
		return SendCmdData(CMD2_VIEW_APP_EPGCAP_START, val);
	}

	//EPG�擾�L�����Z��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendViewEpgCapStop(
		){
		return SendCmdWithoutData(CMD2_VIEW_APP_EPGCAP_STOP);
	}

	//EPG�f�[�^�̌���
	//�߂�l�F
	// �G���[�R�[�h
	// val					[IN]�擾�ԑg
	// resVal				[OUT]�ԑg���
	DWORD SendViewSearchEvent(
		const SEARCH_EPG_INFO_PARAM& val,
		EPGDB_EVENT_INFO* resVal
		){
		return SendAndReceiveCmdData(CMD2_VIEW_APP_SEARCH_EVENT, val, resVal);
	}

	//����or���̔ԑg�����擾����
	//�߂�l�F
	// �G���[�R�[�h
	// val					[IN]�擾�ԑg
	// resVal				[OUT]�ԑg���
	DWORD SendViewGetEventPF(
		const GET_EPG_PF_INFO_PARAM& val,
		EPGDB_EVENT_INFO* resVal
		){
		return SendAndReceiveCmdData(CMD2_VIEW_APP_GET_EVENT_PF, val, resVal);
	}

	//View�{�^���o�^�A�v���N��
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendViewExecViewApp(
		){
		return SendCmdWithoutData(CMD2_VIEW_APP_EXEC_VIEW_APP);
	}

//TVTest�A�g�̃X�g���[�~���O�z�M��p
	//�X�g���[�~���O�z�M����ID�̐ݒ�
	//�߂�l�F
	// �G���[�R�[�h
	DWORD SendViewSetStreamingInfo(
		const TVTEST_STREAMING_INFO& val
		){
		return SendCmdData(CMD2_VIEW_APP_TT_SET_CTRL, val);
	}

protected:
	BOOL tcpFlag;
	DWORD connectTimeOut;
	wstring eventName;
	wstring pipeName;
	wstring sendIP;
	DWORD sendPort;

protected:
	DWORD SendPipe(LPCWSTR pipeName_, LPCWSTR eventName_, DWORD timeOut, CMD_STREAM* send, CMD_STREAM* res);
#ifndef SEND_CTRL_CMD_NO_TCP
	DWORD SendTCP(const wstring& ip, DWORD port, DWORD timeOut, CMD_STREAM* sendCmd, CMD_STREAM* resCmd);
#endif

	DWORD SendCmdStream(CMD_STREAM* send, CMD_STREAM* res);
	DWORD SendCmdWithoutData(DWORD param, CMD_STREAM* res = NULL);
	DWORD SendCmdWithoutData2(DWORD param, CMD_STREAM* res = NULL);
	template<class T> DWORD SendCmdData(DWORD param, const T& val, CMD_STREAM* res = NULL);
	template<class T> DWORD SendCmdData2(DWORD param, const T& val, CMD_STREAM* res = NULL);
	template<class T> DWORD ReceiveCmdData(DWORD param, T* resVal);
	template<class T> DWORD ReceiveCmdData2(DWORD param, T* resVal);
	template<class T, class U> DWORD SendAndReceiveCmdData(DWORD param, const T& val, U* resVal);
	template<class T, class U> DWORD SendAndReceiveCmdData2(DWORD param, const T& val, U* resVal);
};

#if 1 //�C�����C��/�e���v���[�g��`

inline DWORD CSendCtrlCmd::SendCmdWithoutData(DWORD param, CMD_STREAM* res)
{
	CMD_STREAM send;
	send.param = param;
	return SendCmdStream(&send, res);
}

inline DWORD CSendCtrlCmd::SendCmdWithoutData2(DWORD param, CMD_STREAM* res)
{
	return SendCmdData(param, (WORD)CMD_VER, res);
}

template<class T>
DWORD CSendCtrlCmd::SendCmdData(DWORD param, const T& val, CMD_STREAM* res)
{
	CMD_STREAM send;
	send.param = param;
	send.data = NewWriteVALUE(val, send.dataSize);
	if( send.data == NULL ){
		return CMD_ERR;
	}
	return SendCmdStream(&send, res);
}

template<class T>
DWORD CSendCtrlCmd::SendCmdData2(DWORD param, const T& val, CMD_STREAM* res)
{
	WORD ver = CMD_VER;
	CMD_STREAM send;
	send.param = param;
	send.data = NewWriteVALUE2WithVersion(ver, val, send.dataSize);
	if( send.data == NULL ){
		return CMD_ERR;
	}
	return SendCmdStream(&send, res);
}

template<class T>
DWORD CSendCtrlCmd::ReceiveCmdData(DWORD param, T* resVal)
{
	CMD_STREAM res;
	DWORD ret = SendCmdWithoutData(param, &res);

	if( ret == CMD_SUCCESS ){
		if( ReadVALUE(resVal, res.data, res.dataSize, NULL) == FALSE ){
			ret = CMD_ERR;
		}
	}
	return ret;
}

template<class T>
DWORD CSendCtrlCmd::ReceiveCmdData2(DWORD param, T* resVal)
{
	CMD_STREAM res;
	DWORD ret = SendCmdWithoutData2(param, &res);

	if( ret == CMD_SUCCESS ){
		WORD ver = 0;
		DWORD readSize = 0;
		if( ReadVALUE(&ver, res.data, res.dataSize, &readSize) == FALSE ||
			ReadVALUE2(ver, resVal, res.data.get() + readSize, res.dataSize - readSize, NULL) == FALSE ){
			ret = CMD_ERR;
		}
	}
	return ret;
}

template<class T, class U>
DWORD CSendCtrlCmd::SendAndReceiveCmdData(DWORD param, const T& val, U* resVal)
{
	CMD_STREAM res;
	DWORD ret = SendCmdData(param, val, &res);

	if( ret == CMD_SUCCESS ){
		if( ReadVALUE(resVal, res.data, res.dataSize, NULL) == FALSE ){
			ret = CMD_ERR;
		}
	}
	return ret;
}

template<class T, class U>
DWORD CSendCtrlCmd::SendAndReceiveCmdData2(DWORD param, const T& val, U* resVal)
{
	CMD_STREAM res;
	DWORD ret = SendCmdData2(param, val, &res);

	if( ret == CMD_SUCCESS ){
		WORD ver = 0;
		DWORD readSize = 0;
		if( ReadVALUE(&ver, res.data, res.dataSize, &readSize) == FALSE ||
			ReadVALUE2(ver, resVal, res.data.get() + readSize, res.dataSize - readSize, NULL) == FALSE ){
			ret = CMD_ERR;
		}
	}
	return ret;
}

#endif
