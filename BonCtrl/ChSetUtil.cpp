#include "stdafx.h"
#include "ChSetUtil.h"

#include "../Common/EpgTimerUtil.h"

CChSetUtil::CChSetUtil(void)
{
}

//�`�����l���ݒ�t�@�C����ǂݍ���
BOOL CChSetUtil::LoadChSet(
	const wstring& chSet4FilePath,
	const wstring& chSet5FilePath
	)
{
	BOOL ret = TRUE;
	if( this->chText4.ParseText(chSet4FilePath.c_str()) == FALSE ){
		ret = FALSE;
	}
	if( this->chText5.ParseText(chSet5FilePath.c_str()) == FALSE ){
		ret = FALSE;
	}
	return ret;
}

//�`�����l���ݒ�t�@�C����ۑ�����
BOOL CChSetUtil::SaveChSet(
	const wstring& chSet4FilePath,
	const wstring& chSet5FilePath
	)
{
	//�ڑ��҂�
	HANDLE waitEvent = CreateEvent(NULL, FALSE, TRUE, CHSET_SAVE_EVENT_WAIT);
	if( waitEvent == NULL ){
		return FALSE;
	}
	if(WaitForSingleObject(waitEvent, 10000) == WAIT_TIMEOUT){
		CloseHandle(waitEvent);
		return FALSE;
	}

	BOOL ret = TRUE;
	this->chText4.SetFilePath(chSet4FilePath.c_str());
	if( this->chText4.SaveText() == false ){
		ret = FALSE;
	}

	//���ōX�V����Ă�\������̂ōēǂݍ���
	CParseChText5 mergeChText5;
	mergeChText5.ParseText(chSet5FilePath.c_str());
	//���ݕێ����Ă������ǉ�
	map<LONGLONG, CH_DATA5>::const_iterator itr;
	for( itr = this->chText5.GetMap().begin(); itr != this->chText5.GetMap().end(); itr++ ){
		mergeChText5.AddCh(itr->second);
	}
	//�ۑ�
	if( mergeChText5.SaveText() == false ){
		ret = FALSE;
	}
	//�ŐV�ł��ēǂݍ���
	this->chText5.ParseText(chSet5FilePath.c_str());

	SetEvent(waitEvent);
	CloseHandle(waitEvent);

	return ret;
}

//�`�����l���X�L�����p�ɃN���A����
BOOL CChSetUtil::Clear()
{
	this->chText4.ParseText(L"");
	this->chText5.ParseText(L"");
	return TRUE;
}

//�`�����l������ǉ�����
BOOL CChSetUtil::AddServiceInfo(
	DWORD space,
	DWORD ch,
	const wstring& chName,
	SERVICE_INFO* serviceInfo
	)
{
	CH_DATA4 item4;

	item4.space = space;
	item4.ch = ch;
	item4.originalNetworkID = serviceInfo->original_network_id;
	item4.transportStreamID = serviceInfo->transport_stream_id;
	item4.serviceID = serviceInfo->service_id;
	if( serviceInfo->extInfo != NULL ){
		item4.serviceType = serviceInfo->extInfo->service_type;
		item4.partialFlag = serviceInfo->extInfo->partialReceptionFlag;
		if( IsVideoServiceType(item4.serviceType) ){
			item4.useViewFlag = TRUE;
		}else{
			item4.useViewFlag = FALSE;
		}
		item4.serviceName = serviceInfo->extInfo->service_name;
		item4.chName = chName;
		if( serviceInfo->extInfo->ts_name != NULL ){
			item4.networkName = serviceInfo->extInfo->ts_name;
		}else if( serviceInfo->extInfo->network_name != NULL){
			item4.networkName = serviceInfo->extInfo->network_name;
		}
		item4.remoconID = serviceInfo->extInfo->remote_control_key_id;
	}

	map<DWORD, CH_DATA4>::const_iterator itr;
	for( itr = this->chText4.GetMap().begin(); itr != this->chText4.GetMap().end(); itr++ ){
		if( itr->second.originalNetworkID == item4.originalNetworkID &&
		    itr->second.transportStreamID == item4.transportStreamID &&
		    itr->second.serviceID == item4.serviceID &&
		    itr->second.space == item4.space &&
		    itr->second.ch == item4.ch ){
			break;
		}
	}
	if( itr == this->chText4.GetMap().end() ){
		this->chText4.AddCh(item4);
	}

	CH_DATA5 item5;

	item5.originalNetworkID = serviceInfo->original_network_id;
	item5.transportStreamID = serviceInfo->transport_stream_id;
	item5.serviceID = serviceInfo->service_id;
	if( serviceInfo->extInfo != NULL ){
		item5.serviceType = serviceInfo->extInfo->service_type;
		item5.partialFlag = serviceInfo->extInfo->partialReceptionFlag;
		item5.serviceName = serviceInfo->extInfo->service_name;
		if( serviceInfo->extInfo->ts_name != NULL ){
			item5.networkName = serviceInfo->extInfo->ts_name;
		}else if( serviceInfo->extInfo->network_name != NULL){
			item5.networkName = serviceInfo->extInfo->network_name;
		}
		if( IsVideoServiceType(item4.serviceType) ){
			item5.epgCapFlag = TRUE;
			item5.searchFlag = TRUE;
		}else{
			item5.epgCapFlag = FALSE;
			item5.searchFlag = FALSE;
		}
	}

	this->chText5.AddCh(item5);

	return TRUE;
}


//�T�[�r�X�ꗗ���擾����
BOOL CChSetUtil::GetEnumService(
	vector<CH_DATA4>* serviceList
	)
{
	if( this->chText4.GetMap().size() == 0 ){
		return FALSE;
	}
	map<DWORD, CH_DATA4>::const_iterator itr;
	for( itr = this->chText4.GetMap().begin(); itr != this->chText4.GetMap().end(); itr++ ){
		serviceList->push_back(itr->second);
	}
	return TRUE;
}

//ID���畨���`�����l������������
BOOL CChSetUtil::GetCh(
	WORD ONID,
	WORD TSID,
	WORD SID,
	DWORD& space,
	DWORD& ch
	)
{
	BOOL ret = FALSE;
	map<DWORD, CH_DATA4>::const_iterator itr;
	for( itr = this->chText4.GetMap().begin(); itr != this->chText4.GetMap().end(); itr++ ){
		if( itr->second.originalNetworkID == ONID && itr->second.transportStreamID == TSID ){
			if( ret == FALSE || itr->second.serviceID == SID ){
				ret = TRUE;
				space = itr->second.space;
				ch = itr->second.ch;
				//SID���������̂�D�悷��
				if( itr->second.serviceID == SID ){
					break;
				}
			}
		}
	}
	return ret;
}

//EPG�擾�Ώۂ̃T�[�r�X�ꗗ���擾����
//�߂�l�F
// �G���[�R�[�h
//�����F
// chList		[OUT]EPG�擾����`�����l���ꗗ
void CChSetUtil::GetEpgCapService(
	vector<EPGCAP_SERVICE_INFO>* chList
	)
{
	map<ULONGLONG, ULONGLONG> addMap;
	map<DWORD, CH_DATA4>::const_iterator itrCh4;
	for( itrCh4 = this->chText4.GetMap().begin(); itrCh4 != this->chText4.GetMap().end(); itrCh4++ ){
		LONGLONG key = Create64Key(itrCh4->second.originalNetworkID, itrCh4->second.transportStreamID, itrCh4->second.serviceID);
		map<LONGLONG, CH_DATA5>::const_iterator itrCh5;
		itrCh5 = this->chText5.GetMap().find(key);

		if( itrCh5 != this->chText5.GetMap().end() ){
			ULONGLONG addKey = ((ULONGLONG)itrCh4->second.space) << 32 | itrCh4->second.ch;
			map<ULONGLONG, ULONGLONG>::iterator itrAdd;
			itrAdd = addMap.find(addKey);
			if( itrAdd == addMap.end() ){
				if( itrCh5->second.epgCapFlag == TRUE ){
					EPGCAP_SERVICE_INFO item;
					item.ONID = itrCh5->second.originalNetworkID;
					item.TSID = itrCh5->second.transportStreamID;
					item.SID = itrCh5->second.serviceID;
					chList->push_back(item);

					addMap.insert(pair<ULONGLONG, ULONGLONG>(addKey,addKey));
				}
			}
		}
	}
}

vector<EPGCAP_SERVICE_INFO> CChSetUtil::GetEpgCapServiceAll(
	int ONID,
	int TSID
	)
{
	vector<EPGCAP_SERVICE_INFO> ret;
	map<LONGLONG, CH_DATA5>::const_iterator itrCh5;
	for( itrCh5 = this->chText5.GetMap().begin(); itrCh5 != this->chText5.GetMap().end(); itrCh5++ ){
		if( (ONID < 0 || itrCh5->second.originalNetworkID == ONID) &&
			(TSID < 0 || itrCh5->second.transportStreamID == TSID) &&
			itrCh5->second.epgCapFlag == TRUE
			){
			ret.push_back(EPGCAP_SERVICE_INFO());
			ret.back().ONID = itrCh5->second.originalNetworkID;
			ret.back().TSID = itrCh5->second.transportStreamID;
			ret.back().SID = itrCh5->second.serviceID;
		}
	}
	return ret;
}

BOOL CChSetUtil::IsPartial(
	WORD ONID,
	WORD TSID,
	WORD SID
	)
{
	LONGLONG key = Create64Key(ONID, TSID, SID);
	map<LONGLONG, CH_DATA5>::const_iterator itr;
	itr = this->chText5.GetMap().find(key);
	if( itr != this->chText5.GetMap().end() ){
		if( itr->second.partialFlag == 1 ){
			return TRUE;
		}
	}
	return FALSE;
}

