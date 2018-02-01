#pragma once

#include "../../Common/ParseTextInstances.h"

#include "TunerBankCtrl.h"

class CTunerManager
{
public:
	//�`���[�i�[�ꗗ�̓ǂݍ��݂��s��
	void ReloadTuner();

	//�`���[�i�[�\�񐧌���擾����
	//�����F
	// ctrlMap			[OUT]�`���[�i�[�\�񐧌�̈ꗗ
	// notifyManager	[IN]CTunerBankCtrl�ɓn������
	// epgDBManager		[IN]CTunerBankCtrl�ɓn������
	void GetEnumTunerBank(
		map<DWORD, std::unique_ptr<CTunerBankCtrl>>* ctrlMap,
		CNotifyManager& notifyManager,
		CEpgDBManager& epgDBManager
		) const;

	//�w��T�[�r�X���T�|�[�g����`���[�i�[�ꗗ���擾����
	//�߂�l�F
	// �`���[�i�[��ID�ꗗ
	//�����F
	// ONID				[IN]�m�F�������T�[�r�X��ONID
	// TSID				[IN]�m�F�������T�[�r�X��TSID
	// SID				[IN]�m�F�������T�[�r�X��SID
	vector<DWORD> GetSupportServiceTuner(
		WORD ONID,
		WORD TSID,
		WORD SID
		) const;

	bool GetCh(
		DWORD tunerID,
		WORD ONID,
		WORD TSID,
		WORD SID,
		DWORD* space,
		DWORD* ch
		) const;

	//�h���C�o���̃`���[�i�[�ꗗ��EPG�擾�Ɏg�p�ł���`���[�i�[���̃y�A���擾����
	vector<pair<vector<DWORD>, WORD>> GetEnumEpgCapTuner(
		) const;

	bool IsSupportService(
		DWORD tunerID,
		WORD ONID,
		WORD TSID,
		WORD SID
		) const;

	bool GetBonFileName(
		DWORD tunerID,
		wstring& bonFileName
		) const;

protected:
	struct TUNER_INFO {
		wstring bonFileName;
		WORD epgCapMaxOfThisBon;
		vector<CH_DATA4> chList;
	};

	map<DWORD, TUNER_INFO> tunerMap; //�L�[ bonID<<16 | tunerID
};

