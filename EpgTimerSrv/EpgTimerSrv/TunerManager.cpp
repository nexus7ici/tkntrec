#include "stdafx.h"
#include "TunerManager.h"
#include "../../Common/PathUtil.h"
#include "../../Common/StringUtil.h"


//�`���[�i�[�ꗗ�̓ǂݍ��݂��s��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
BOOL CTunerManager::ReloadTuner()
{
	this->tunerMap.clear();

	const fs_path path = GetSettingPath();

	const fs_path srvIniPath = GetModuleIniPath();

	vector<pair<wstring, wstring>> nameList = CEpgTimerSrvSetting::EnumBonFileName(path.c_str());

	for( size_t i = 0; i < nameList.size(); i++ ){
		WORD count = (WORD)GetPrivateProfileInt(nameList[i].first.c_str(), L"Count", 0, srvIniPath.c_str());
		if( count != 0 ){
			//�J�E���g1�ȏ�̂��̂������p
			WORD priority = (WORD)GetPrivateProfileInt(nameList[i].first.c_str(), L"Priority", 0, srvIniPath.c_str());
			WORD epgCount = 0;
			if( GetPrivateProfileInt(nameList[i].first.c_str(), L"GetEpg", 1, srvIniPath.c_str()) != 0 ){
				epgCount = (WORD)GetPrivateProfileInt(nameList[i].first.c_str(), L"EPGCount", 0, srvIniPath.c_str());
				if( epgCount == 0 ){
					epgCount = count;
				}
			}
			if( this->tunerMap.find((DWORD)priority << 16 | 1) != this->tunerMap.end() ){
				OutputDebugString(L"CTunerManager::ReloadTuner(): Duplicate bonID\r\n");
				count = 0;
			}
			for( WORD j = 0; j < count; j++ ){
				TUNER_INFO& item = this->tunerMap.insert(std::make_pair((DWORD)priority << 16 | (j + 1), TUNER_INFO())).first->second;
				item.epgCapMaxOfThisBon = min(epgCount, count);
				item.bonFileName = nameList[i].first;
				CParseChText4 chUtil;
				chUtil.ParseText(fs_path(path).append(nameList[i].second).c_str());
				item.chList.reserve(chUtil.GetMap().size());
				for( map<DWORD, CH_DATA4>::const_iterator itr = chUtil.GetMap().begin(); itr != chUtil.GetMap().end(); itr++ ){
					item.chList.push_back(itr->second);
				}
			}
		}
	}

	return TRUE;
}


//�`���[�i�[��ID�ꗗ���擾����B
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// idList			[OUT]�`���[�i�[��ID�ꗗ
BOOL CTunerManager::GetEnumID(
	vector<DWORD>* idList
	) const
{
	if( idList == NULL ){
		return FALSE;
	}
	map<DWORD, TUNER_INFO>::const_iterator itr;
	for( itr = this->tunerMap.begin(); itr != this->tunerMap.end(); itr++ ){
		idList->push_back(itr->first);
	}
	return TRUE;
}

//�`���[�i�[�\�񐧌���擾����
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// ctrlMap			[OUT]�`���[�i�[�\�񐧌�̈ꗗ
// notifyManager	[IN]CTunerBankCtrl�ɓn������
// epgDBManager		[IN]CTunerBankCtrl�ɓn������
BOOL CTunerManager::GetEnumTunerBank(
	map<DWORD, std::unique_ptr<CTunerBankCtrl>>* ctrlMap,
	CNotifyManager& notifyManager,
	CEpgDBManager& epgDBManager
	) const
{
	if( ctrlMap == NULL ){
		return FALSE;
	}
	map<DWORD, TUNER_INFO>::const_iterator itr;
	for( itr = this->tunerMap.begin(); itr != this->tunerMap.end(); itr++ ){
		ctrlMap->insert(std::make_pair(itr->first,
			std::unique_ptr<CTunerBankCtrl>(new CTunerBankCtrl(itr->first, itr->second.bonFileName.c_str(), itr->second.chList, notifyManager, epgDBManager))));
	}
	return TRUE;
}

//�w��T�[�r�X���T�|�[�g���Ă��Ȃ��`���[�i�[�ꗗ���擾����
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// ONID				[IN]�m�F�������T�[�r�X��ONID
// TSID				[IN]�m�F�������T�[�r�X��TSID
// SID				[IN]�m�F�������T�[�r�X��SID
// idList			[OUT]�`���[�i�[��ID�ꗗ
BOOL CTunerManager::GetNotSupportServiceTuner(
	WORD ONID,
	WORD TSID,
	WORD SID,
	vector<DWORD>* idList
	) const
{
	if( idList == NULL ){
		return FALSE;
	}
	map<DWORD, TUNER_INFO>::const_iterator itr;
	for( itr = this->tunerMap.begin(); itr != this->tunerMap.end(); itr++ ){
		vector<CH_DATA4>::const_iterator itrCh;
		for( itrCh = itr->second.chList.begin(); itrCh != itr->second.chList.end(); itrCh++ ){
			if( itrCh->originalNetworkID == ONID && itrCh->transportStreamID == TSID && itrCh->serviceID == SID ){
				break;
			}
		}
		if( itrCh == itr->second.chList.end() ){
			idList->push_back(itr->first);
		}

	}
	return TRUE;
}

BOOL CTunerManager::GetSupportServiceTuner(
	WORD ONID,
	WORD TSID,
	WORD SID,
	vector<DWORD>* idList
	) const
{
	if( idList == NULL ){
		return FALSE;
	}
	map<DWORD, TUNER_INFO>::const_iterator itr;
	for( itr = this->tunerMap.begin(); itr != this->tunerMap.end(); itr++ ){
		vector<CH_DATA4>::const_iterator itrCh;
		for( itrCh = itr->second.chList.begin(); itrCh != itr->second.chList.end(); itrCh++ ){
			if( itrCh->originalNetworkID == ONID && itrCh->transportStreamID == TSID && itrCh->serviceID == SID ){
				idList->push_back(itr->first);
				break;
			}
		}

	}
	return TRUE;
}

BOOL CTunerManager::GetCh(
	DWORD tunerID,
	WORD ONID,
	WORD TSID,
	WORD SID,
	DWORD* space,
	DWORD* ch
	) const
{
	map<DWORD, TUNER_INFO>::const_iterator itr;
	for( itr = this->tunerMap.begin(); itr != this->tunerMap.end(); itr++ ){
		vector<CH_DATA4>::const_iterator itrCh;
		for( itrCh = itr->second.chList.begin(); itrCh != itr->second.chList.end(); itrCh++ ){
			if( itrCh->originalNetworkID == ONID && itrCh->transportStreamID == TSID && itrCh->serviceID == SID ){
				if( space != NULL ){
					*space = itrCh->space;
				}
				if( ch != NULL ){
					*ch = itrCh->ch;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

//�h���C�o���̃`���[�i�[�ꗗ��EPG�擾�Ɏg�p�ł���`���[�i�[���̃y�A���擾����
BOOL CTunerManager::GetEnumEpgCapTuner(
	vector<pair<vector<DWORD>, WORD>>* idList
	) const
{
	if( idList == NULL ){
		return FALSE;
	}
	map<DWORD, TUNER_INFO>::const_iterator itr;
	for( itr = this->tunerMap.begin(); itr != this->tunerMap.end(); itr++ ){
		if( idList->empty() || idList->back().first.back() >> 16 != itr->first >> 16 ){
			idList->push_back(pair<vector<DWORD>, WORD>(vector<DWORD>(), itr->second.epgCapMaxOfThisBon));
		}
		idList->back().first.push_back(itr->first);
	}
	return TRUE;
}

BOOL CTunerManager::IsSupportService(
	DWORD tunerID,
	WORD ONID,
	WORD TSID,
	WORD SID
	) const
{
	map<DWORD, TUNER_INFO>::const_iterator itr;
	itr = this->tunerMap.find(tunerID);
	if( itr == this->tunerMap.end() ){
		return FALSE;
	}

	vector<CH_DATA4>::const_iterator itrCh;
	for( itrCh = itr->second.chList.begin(); itrCh != itr->second.chList.end(); itrCh++ ){
		if( itrCh->originalNetworkID == ONID && itrCh->transportStreamID == TSID && itrCh->serviceID == SID ){
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CTunerManager::GetBonFileName(
	DWORD tunerID,
	wstring& bonFileName
	) const
{
	map<DWORD, TUNER_INFO>::const_iterator itr;
	itr = this->tunerMap.find(tunerID);
	if( itr == this->tunerMap.end() ){
		return FALSE;
	}
	bonFileName = itr->second.bonFileName;

	return TRUE;
}
