#pragma once

#include "../Common/StructDef.h"
#include "../Common/EpgDataCap3Def.h"
#include "BonCtrlDef.h"

#include "../Common/ParseTextInstances.h"

class CChSetUtil
{
public:
	CChSetUtil(void);

	//�`�����l���ݒ�t�@�C����ǂݍ���
	BOOL LoadChSet(
		const wstring& chSet4FilePath,
		const wstring& chSet5FilePath
		);

	//�`�����l���ݒ�t�@�C����ۑ�����
	BOOL SaveChSet(
		const wstring& chSet4FilePath,
		const wstring& chSet5FilePath
		);

	//�`�����l���X�L�����p�ɃN���A����
	BOOL Clear();

	//�`�����l������ǉ�����
	BOOL AddServiceInfo(
		DWORD space,
		DWORD ch,
		const wstring& chName,
		SERVICE_INFO* serviceInfo
		);

	//�T�[�r�X�ꗗ���擾����
	BOOL GetEnumService(
		vector<CH_DATA4>* serviceList
		);

	//ID���畨���`�����l������������
	BOOL GetCh(
		WORD ONID,
		WORD TSID,
		WORD SID,
		DWORD& space,
		DWORD& ch
		);

	//EPG�擾�Ώۂ̃T�[�r�X�ꗗ���擾����
	//�����F
	// chList		[OUT]EPG�擾����`�����l���ꗗ
	void GetEpgCapService(
		vector<EPGCAP_SERVICE_INFO>* chList
		);

	//���݂̃`���[�i�Ɍ��肳��Ȃ�EPG�擾�Ώۂ̃T�[�r�X�ꗗ���擾����
	vector<EPGCAP_SERVICE_INFO> GetEpgCapServiceAll(
		int ONID = -1,
		int TSID = -1
		);

	//������M�T�[�r�X���ǂ���
	BOOL IsPartial(
		WORD ONID,
		WORD TSID,
		WORD SID
		);

	//�T�[�r�X�^�C�v���f���T�[�r�X���ǂ���
	static BOOL IsVideoServiceType(
		WORD serviceType
		){
		return serviceType == 0x01 //�f�W�^��TV
			|| serviceType == 0xA5 //�v�����[�V�����f��
			|| serviceType == 0xAD //�������דx4K��pTV
			;
	}

protected:
	CParseChText4 chText4;
	CParseChText5 chText5;
};

