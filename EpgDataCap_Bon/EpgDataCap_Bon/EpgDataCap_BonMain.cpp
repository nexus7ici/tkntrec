#include "stdafx.h"
#include "EpgDataCap_BonMain.h"

#include "EpgDataCap_BonDef.h"
#include "../../Common/CommonDef.h"
#include "../../Common/CtrlCmdDef.h"
#include "../../Common/CtrlCmdUtil.h"

CEpgDataCap_BonMain::CEpgDataCap_BonMain(void)
{
	this->msgWnd = NULL;
	this->nwCtrlID = 0;

	this->overWriteFlag = FALSE;
	this->enableScrambleFlag = TRUE;
	this->enableEMMFlag = FALSE;

	this->allService = FALSE;
	this->needCaption = TRUE;
	this->needData = FALSE;

	this->viewPath = L"";
	this->viewOpt = L"";

	this->lastONID = 0xFFFF;
	this->lastTSID = 0xFFFF;
	this->lastSID = 0xFFFF;

	this->recCtrlID = 0;

	this->outCtrlID = -1;

	this->cmdCapture = NULL;
	this->resCapture = NULL;

	this->openWait = 200;

	if( CPipeServer::GrantServerAccessToKernelObject(GetCurrentProcess(), SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_SET_INFORMATION) ){
		OutputDebugString(L"Granted SYNCHRONIZE|PROCESS_TERMINATE|PROCESS_SET_INFORMATION to " SERVICE_NAME L"\r\n");
	}
}


CEpgDataCap_BonMain::~CEpgDataCap_BonMain(void)
{
	this->pipeServer.StopServer();
}

void CEpgDataCap_BonMain::SetHwnd(HWND wnd)
{
	this->msgWnd = wnd;
}

//�ݒ���s��
void CEpgDataCap_BonMain::ReloadSetting()
{
	fs_path commonIniPath = GetCommonIniPath();

	fs_path appIniPath = GetModuleIniPath();

	this->bonCtrl.SetBonDriverFolder(GetModulePath().replace_filename(BON_DLL_FOLDER).c_str());

	this->recFolderList.clear();
	for( int i = 0; ; i++ ){
		this->recFolderList.push_back(GetRecFolderPath(i).native());
		if( this->recFolderList.back().empty() ){
			this->recFolderList.pop_back();
			break;
		}
	}

	this->enableScrambleFlag = GetPrivateProfileInt( L"SET", L"Scramble", 1, appIniPath.c_str() );
	this->enableEMMFlag = GetPrivateProfileInt( L"SET", L"EMM", 0, appIniPath.c_str() );
	this->allService = GetPrivateProfileInt( L"SET", L"AllService", 0, appIniPath.c_str() );
	this->needCaption = GetPrivateProfileInt( L"SET", L"Caption", 1, appIniPath.c_str() );
	this->needData = GetPrivateProfileInt( L"SET", L"Data", 0, appIniPath.c_str() );

	this->recFileName = GetPrivateProfileToString( L"SET", L"RecFileName", L"$DYYYY$$DMM$$DDD$-$THH$$TMM$$TSS$-$ServiceName$.ts", appIniPath.c_str() );
	this->overWriteFlag = GetPrivateProfileInt( L"SET", L"OverWrite", 0, appIniPath.c_str() );

	this->viewPath = GetPrivateProfileToString( L"SET", L"ViewPath", L"", appIniPath.c_str() );
	this->viewOpt = GetPrivateProfileToString( L"SET", L"ViewOption", L"", appIniPath.c_str() );

	this->setUdpSendList.clear();
	this->setTcpSendList.clear();
	for( int tcp = 0; tcp < 2; tcp++ ){
		int count = GetPrivateProfileInt(tcp ? L"SET_TCP" : L"SET_UDP", L"Count", 0, appIniPath.c_str());
		for( int i = 0; i < count; i++ ){
			NW_SEND_INFO item;
			WCHAR key[64];
			swprintf_s(key, L"IP%d", i);
			item.ipString = GetPrivateProfileToString(tcp ? L"SET_TCP" : L"SET_UDP", key, L"2130706433", appIniPath.c_str());
			if( item.ipString.size() >= 2 && item.ipString[0] == L'[' ){
				item.ipString.erase(0, 1).pop_back();
			}else{
				UINT ip = _wtoi(item.ipString.c_str());
				Format(item.ipString, L"%d.%d.%d.%d", ip >> 24, ip >> 16 & 0xFF, ip >> 8 & 0xFF, ip & 0xFF);
			}
			swprintf_s(key, L"Port%d", i);
			item.port = GetPrivateProfileInt(tcp ? L"SET_TCP" : L"SET_UDP", key, tcp ? 2230 : 1234, appIniPath.c_str());
			swprintf_s(key, L"BroadCast%d", i);
			item.broadcastFlag = tcp ? 0 : GetPrivateProfileInt(L"SET_UDP", key, 0, appIniPath.c_str());
			(tcp ? this->setTcpSendList : this->setUdpSendList).push_back(item);
		}
	}

	if( this->nwCtrlID != 0 ){
		if( this->allService == TRUE ){
			this->bonCtrl.SetServiceID(this->nwCtrlID, 0xFFFF);
		}else{
			this->bonCtrl.SetServiceID(this->nwCtrlID, this->lastSID);
		}
	}

	BOOL epgCapLive = (BOOL)GetPrivateProfileInt( L"SET", L"EpgCapLive", 1, appIniPath.c_str() );
	BOOL epgCapRec = (BOOL)GetPrivateProfileInt( L"SET", L"EpgCapRec", 1, appIniPath.c_str() );
	BOOL epgCapBackBSBasic = GetPrivateProfileInt( L"SET", L"EpgCapBackBSBasicOnly", 1, appIniPath.c_str() );
	BOOL epgCapBackCS1Basic = GetPrivateProfileInt( L"SET", L"EpgCapBackCS1BasicOnly", 1, appIniPath.c_str() );
	BOOL epgCapBackCS2Basic = GetPrivateProfileInt( L"SET", L"EpgCapBackCS2BasicOnly", 1, appIniPath.c_str() );
	BOOL epgCapBackCS3Basic = GetPrivateProfileInt( L"SET", L"EpgCapBackCS3BasicOnly", 0, appIniPath.c_str() );
	DWORD epgCapBackStartWaitSec = (DWORD)GetPrivateProfileInt( L"SET", L"EpgCapBackStartWaitSec", 30, appIniPath.c_str() );

	this->bonCtrl.SetBackGroundEpgCap(epgCapLive, epgCapRec, epgCapBackBSBasic, epgCapBackCS1Basic, epgCapBackCS2Basic, epgCapBackCS3Basic, epgCapBackStartWaitSec);
	if( this->udpSendList.empty() && this->tcpSendList.empty() ){
		this->bonCtrl.SetScramble(this->nwCtrlID, this->enableScrambleFlag);
	}
	this->bonCtrl.SetEMMMode(this->enableEMMFlag);
	this->bonCtrl.SetNoLogScramble(GetPrivateProfileInt( L"SET", L"NoLogScramble", 0, appIniPath.c_str() ) != 0);

	DWORD tsBuffMaxCount = (DWORD)GetPrivateProfileInt( L"SET", L"TsBuffMaxCount", 5000, appIniPath.c_str() );
	int writeBuffMaxCount = GetPrivateProfileInt( L"SET", L"WriteBuffMaxCount", -1, appIniPath.c_str() );
	this->bonCtrl.SetTsBuffMaxCount(tsBuffMaxCount, writeBuffMaxCount);

	this->openWait = (DWORD)GetPrivateProfileInt( L"SET", L"OpenWait", 200, appIniPath.c_str() );
}

//BonDriver�t�H���_��BonDriver_*.dll���
//�߂�l�F
// �����ł���BonDriver�ꗗ
vector<wstring> CEpgDataCap_BonMain::EnumBonDriver()
{
	return this->bonCtrl.EnumBonDriver();
}

//BonDriver�����[�h���ă`�����l�����Ȃǂ��擾�i�t�@�C�����Ŏw��j
//�߂�l�F
// �G���[�R�[�h
//�����F
// bonDriverFile	[IN]EnumBonDriver�Ŏ擾���ꂽBonDriver�̃t�@�C����
DWORD CEpgDataCap_BonMain::OpenBonDriver(
	LPCWSTR bonDriverFile
)
{
	DWORD ret = this->bonCtrl.OpenBonDriver(bonDriverFile, this->openWait);
	if( ret == NO_ERR ){
		this->lastONID = 0xFFFF;
		this->lastTSID = 0xFFFF;
		this->lastSID = 0xFFFF;
		if( this->nwCtrlID == 0 ){
			if( this->bonCtrl.CreateServiceCtrl(&this->nwCtrlID) == TRUE ){
				this->bonCtrl.SetScramble(this->nwCtrlID, this->enableScrambleFlag);
				this->bonCtrl.SetServiceMode(this->nwCtrlID, this->needCaption, this->needData);
			}
		}else{
			this->bonCtrl.ClearErrCount(this->nwCtrlID);
		}
	}
	return ret;
}

//���[�h���Ă���BonDriver�̊J��
void CEpgDataCap_BonMain::CloseBonDriver()
{
	this->bonCtrl.CloseBonDriver();
}

//�T�[�r�X�ꗗ���擾����
//�߂�l�F
// �G���[�R�[�h
//�����F
// serviceList				[OUT]�T�[�r�X���̃��X�g
DWORD CEpgDataCap_BonMain::GetServiceList(
	vector<CH_DATA4>* serviceList
	)
{
	return this->bonCtrl.GetServiceList(serviceList);
}

//���[�h����BonDriver�̃t�@�C�������擾����i���[�h�������Ă��邩�̔���j
//�߂�l�F
// TRUE�i�����j�FFALSE�iOpen�Ɏ��s���Ă���j
//�����F
// bonDriverFile		[OUT]BonDriver�̃t�@�C����(NULL��)
BOOL CEpgDataCap_BonMain::GetOpenBonDriver(
	wstring* bonDriverFile
	)
{
	return this->bonCtrl.GetOpenBonDriver(bonDriverFile);
}

//�`�����l���ύX
//�߂�l�F
// �G���[�R�[�h
//�����F
// ONID			[IN]�ύX�`�����l����orignal_network_id
// TSID			[IN]�ύX�`�����l���̕���transport_stream_id
// SID			[IN]�ύX�`�����l���̕���service_id
DWORD CEpgDataCap_BonMain::SetCh(
	WORD ONID,
	WORD TSID,
	WORD SID
	)
{
	DWORD err = ERR_FALSE;
	if( this->bonCtrl.IsRec() == FALSE ){
		this->bonCtrl.StopEpgCap();
		err = this->bonCtrl.SetCh(ONID, TSID, SID);
		if( err == NO_ERR ){
			this->lastONID = ONID;
			this->lastTSID = TSID;
			this->lastSID = SID;

			if( this->nwCtrlID != 0 ){
				if( this->allService == TRUE ){
					this->bonCtrl.SetServiceID(this->nwCtrlID, 0xFFFF);
				}else{
					this->bonCtrl.SetServiceID(this->nwCtrlID, this->lastSID);
				}
			}
		}
	}
	return err;
}

//�`�����l���ύX
//�߂�l�F
// �G���[�R�[�h
//�����F
// SID			[IN]�ύX�`�����l����service_id
// SID			[IN]�ύX�`�����l����space
// SID			[IN]�ύX�`�����l����ch
DWORD CEpgDataCap_BonMain::SetCh(
	WORD ONID,
	WORD TSID,
	WORD SID,
	DWORD space,
	DWORD ch
	)
{
	DWORD err = ERR_FALSE;
	if( this->bonCtrl.IsRec() == FALSE ){
		this->bonCtrl.StopEpgCap();
		err = this->bonCtrl.SetCh(space, ch);
		if( err == NO_ERR ){
			this->lastONID = ONID;
			this->lastTSID = TSID;
			this->lastSID = SID;

			if( this->nwCtrlID != 0 ){
				if( this->allService == TRUE ){
					this->bonCtrl.SetServiceID(this->nwCtrlID, 0xFFFF);
				}else{
					this->bonCtrl.SetServiceID(this->nwCtrlID, this->lastSID);
				}
			}
		}
	}
	return err;
}

//���݂̃T�[�r�X�擾
//�߂�l�F
// �G���[�R�[�h
//�����F
// ONID			[IN]���݂�orignal_network_id
// TSID			[IN]���݂�transport_stream_id
// SID			[IN]���݂�service_id
void CEpgDataCap_BonMain::GetCh(
	WORD* ONID,
	WORD* TSID,
	WORD* SID
	)
{
	*ONID = this->lastONID;
	*TSID = this->lastTSID;
	*SID = this->lastSID;
}

//�`�����l���ύX�����ǂ���
//�߂�l�F
// TRUE�i�ύX���j�AFALSE�i�����j
BOOL CEpgDataCap_BonMain::IsChChanging(BOOL* chChgErr)
{
	return this->bonCtrl.IsChChanging(chChgErr);
}

void CEpgDataCap_BonMain::SetSID(
	WORD SID
	)
{
	this->lastSID = SID;
}

//UDP�ő��M���s��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// enableFlag		[IN]TRUE�i�J�n�j�AFALSE�i��~�j
BOOL CEpgDataCap_BonMain::SendUDP(
	BOOL enableFlag
	)
{
	this->udpSendList.clear();
	if( enableFlag == TRUE ){
		this->udpSendList = this->setUdpSendList;
	}

	BOOL ret = TRUE;
	if( this->udpSendList.empty() ){
		if( this->nwCtrlID != 0 ){
			this->bonCtrl.SendUdp(this->nwCtrlID,NULL);
		}
	}else{
		if( this->nwCtrlID == 0 ){
			if( this->bonCtrl.CreateServiceCtrl(&this->nwCtrlID) == FALSE ){
				this->udpSendList.clear();
				return FALSE;
			}
		}
		if( this->nwCtrlID != 0 ){
			if( this->allService == TRUE ){
				this->bonCtrl.SetServiceID(this->nwCtrlID, 0xFFFF);
			}else{
				this->bonCtrl.SetServiceID(this->nwCtrlID, this->lastSID);
			}
		}
		this->bonCtrl.SetScramble(this->nwCtrlID, this->enableScrambleFlag);
		this->bonCtrl.SetServiceMode(this->nwCtrlID, this->needCaption, this->needData);
		ret = this->bonCtrl.SendUdp(this->nwCtrlID,&this->udpSendList);
	}
	return ret;
}

//TCP�ő��M���s��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// enableFlag		[IN]TRUE�i�J�n�j�AFALSE�i��~�j
BOOL CEpgDataCap_BonMain::SendTCP(
	BOOL enableFlag
	)
{
	this->tcpSendList.clear();
	if( enableFlag == TRUE ){
		this->tcpSendList = this->setTcpSendList;
	}

	BOOL ret = TRUE;
	if( this->tcpSendList.empty() ){
		if( this->nwCtrlID != 0 ){
			this->bonCtrl.SendTcp(this->nwCtrlID,NULL);
		}
	}else{
		if( this->nwCtrlID == 0 ){
			if( this->bonCtrl.CreateServiceCtrl(&this->nwCtrlID) == FALSE ){
				this->tcpSendList.clear();
				return FALSE;
			}
		}
		if( this->nwCtrlID != 0 ){
			if( this->allService == TRUE ){
				this->bonCtrl.SetServiceID(this->nwCtrlID, 0xFFFF);
			}else{
				this->bonCtrl.SetServiceID(this->nwCtrlID, this->lastSID);
			}
		}
		this->bonCtrl.SetScramble(this->nwCtrlID, this->enableScrambleFlag);
		this->bonCtrl.SetServiceMode(this->nwCtrlID, this->needCaption, this->needData);
		ret = this->bonCtrl.SendTcp(this->nwCtrlID,&this->tcpSendList);
	}
	return ret;
}

//�w��T�[�r�X�̌���or����EPG�����擾����
//�߂�l�F
// �G���[�R�[�h
//�����F
// serviceID				[IN]�擾�Ώۂ�ServiceID
// nextFlag					[IN]TRUE�i���̔ԑg�j�AFALSE�i���݂̔ԑg�j
// epgInfo					[OUT]EPG���iDLL���Ŏ����I��delete����B���Ɏ擾���s���܂ŗL���j
DWORD CEpgDataCap_BonMain::GetEpgInfo(
	BOOL nextFlag,
	wstring* epgInfo
	)
{
	WORD ONID = 0;
	WORD TSID = 0;
	if( this->bonCtrl.IsChChanging(NULL) == TRUE ){
		return FALSE;
	}
	if( this->bonCtrl.GetStreamID(&ONID, &TSID) == FALSE ){
		return FALSE;
	}
	EPGDB_EVENT_INFO info;

	DWORD ret = this->bonCtrl.GetEpgInfo(ONID, TSID, this->lastSID, nextFlag, &info);

	if(epgInfo != NULL ){
		*epgInfo = L"";
		if( ret == NO_ERR ){
			*epgInfo = ConvertEpgInfoText(&info);
		}
	}

	return ret;
}

//�V�O�i�����x���̎擾
//�߂�l�F
// �V�O�i�����x��
float CEpgDataCap_BonMain::GetSignalLevel()
{
	return this->bonCtrl.GetSignalLevel();
}

//�G���[�J�E���g���N���A����
void CEpgDataCap_BonMain::ClearErrCount(
	)
{
	if( this->nwCtrlID != 0 ){
		this->bonCtrl.ClearErrCount(this->nwCtrlID);
	}
}

//�h���b�v�ƃX�N�����u���̃J�E���g���擾����
//�����F
// drop				[OUT]�h���b�v��
// scramble			[OUT]�X�N�����u����
void CEpgDataCap_BonMain::GetErrCount(
	ULONGLONG* drop,
	ULONGLONG* scramble
	)
{
	if( this->nwCtrlID != 0 ){
		this->bonCtrl.GetErrCount(this->nwCtrlID, drop, scramble);
	}

}

//�^����J�n����
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
BOOL CEpgDataCap_BonMain::StartRec(
	)
{
	if( this->bonCtrl.IsRec() == TRUE || this->recCtrlID != 0 ){
		return FALSE;
	}

	if( this->bonCtrl.CreateServiceCtrl(&this->recCtrlID) == FALSE ){
		return FALSE;
	}
	if( this->allService == TRUE ){
		this->bonCtrl.SetServiceID(this->recCtrlID, 0xFFFF);
	}else{
		this->bonCtrl.SetServiceID(this->recCtrlID, this->lastSID);
	}
	this->bonCtrl.SetScramble(this->recCtrlID, this->enableScrambleFlag);
	this->bonCtrl.SetServiceMode(this->recCtrlID, this->needCaption, this->needData);

	vector<CH_DATA4> serviceList;
	this->bonCtrl.GetServiceList(&serviceList);
	wstring serviceName = L"";
	Format(serviceName, L"%04X", this->lastSID);
	for( size_t i=0; i<serviceList.size(); i++){
		if( serviceList[i].originalNetworkID == this->lastONID &&
			serviceList[i].transportStreamID == this->lastTSID &&
			serviceList[i].serviceID == this->lastSID ){
				serviceName = serviceList[i].serviceName;
		}
	}

	wstring fileName = this->recFileName;
	SYSTEMTIME now;
	ConvertSystemTime(GetNowI64Time(), &now);
	for( int i = 0; GetTimeMacroName(i); i++ ){
		wstring name;
		UTF8toW(GetTimeMacroName(i), name);
		Replace(fileName, L'$' + name + L'$', GetTimeMacroValue(i, now));
	}
	Replace(fileName, L"$ServiceName$", serviceName);
	CheckFileName(fileName);

	vector<REC_FILE_SET_INFO> saveFolder(1);
	saveFolder.back().recFolder = this->recFolderList[0];
	saveFolder.back().recFileName = fileName;

	this->bonCtrl.StartSave(this->recCtrlID, L"padding.ts", this->overWriteFlag, FALSE, 0,0,0,0, 0, saveFolder, this->recFolderList);

	return TRUE;
}

//�^����~����
// TRUE�i�����j�AFALSE�i���s�j
BOOL CEpgDataCap_BonMain::StopRec()
{
	if( this->recCtrlID != 0 ){
		this->bonCtrl.EndSave(this->recCtrlID);
		this->bonCtrl.DeleteServiceCtrl(this->recCtrlID);
		this->recCtrlID = 0;
	}
	return TRUE;
}

//�^�撆���ǂ������擾����
// TRUE�i�^�撆�j�AFALSE�i�^�悵�Ă��Ȃ��j
BOOL CEpgDataCap_BonMain::IsRec()
{
	return this->bonCtrl.IsRec();
}

//�\��^����~����
void CEpgDataCap_BonMain::StopReserveRec()
{
	while( this->cmdCtrlList.empty() == false ){
		this->bonCtrl.DeleteServiceCtrl(this->cmdCtrlList.back());
		this->cmdCtrlList.pop_back();
	}
}

//�`�����l���X�L�����̏�Ԃ��擾����
//�߂�l�F
// �X�e�[�^�X
//�����F
// space		[OUT]�X�L�������̕���CH��space
// ch			[OUT]�X�L�������̕���CH��ch
// chName		[OUT]�X�L�������̕���CH�̖��O
// chkNum		[OUT]�`�F�b�N�ς݂̐�
// totalNum		[OUT]�`�F�b�N�Ώۂ̑���
CBonCtrl::JOB_STATUS CEpgDataCap_BonMain::GetChScanStatus(
	DWORD* space,
	DWORD* ch,
	wstring* chName,
	DWORD* chkNum,
	DWORD* totalNum
	)
{
	return this->bonCtrl.GetChScanStatus(space, ch, chName, chkNum, totalNum);
}

//EPG�擾�̃X�e�[�^�X���擾����
//�߂�l�F
// �X�e�[�^�X
//�����F
// info			[OUT]�擾���̃T�[�r�X
CBonCtrl::JOB_STATUS CEpgDataCap_BonMain::GetEpgCapStatus(
	EPGCAP_SERVICE_INFO* info
	)
{
	return this->bonCtrl.GetEpgCapStatus(info);
}

//View�A�v���̋N�����s��
void CEpgDataCap_BonMain::ViewAppOpen()
{
	if( this->viewPath.size() > 0 ){
		ShellExecute(NULL, NULL, this->viewPath.c_str(), this->viewOpt.c_str(), NULL, SW_SHOWNORMAL);
	}
}

void CEpgDataCap_BonMain::StartServer()
{
	wstring pipeName = L"";
	wstring eventName = L"";

	Format(pipeName, L"%s%d", CMD2_VIEW_CTRL_PIPE, GetCurrentProcessId());
	Format(eventName, L"%s%d", CMD2_VIEW_CTRL_WAIT_CONNECT, GetCurrentProcessId());

	OutputDebugString(pipeName.c_str());
	OutputDebugString(eventName.c_str());
	this->pipeServer.StartServer(eventName.c_str(), pipeName.c_str(), [this](CMD_STREAM* cmdParam, CMD_STREAM* resParam) {
		resParam->param = CMD_ERR;
		//�����Ăяo�����s�v�ȃR�}���h�͂����ŏ�������
		switch( cmdParam->param ){
		case CMD2_VIEW_APP_GET_BONDRIVER:
			{
				wstring bonFile;
				if( this->bonCtrl.GetOpenBonDriver(&bonFile) ){
					resParam->data = NewWriteVALUE(bonFile, resParam->dataSize);
					resParam->param = CMD_SUCCESS;
				}
			}
			return;
		case CMD2_VIEW_APP_GET_DELAY:
			resParam->data = NewWriteVALUE(this->bonCtrl.GetTimeDelay(), resParam->dataSize);
			resParam->param = CMD_SUCCESS;
			return;
		case CMD2_VIEW_APP_GET_STATUS:
			{
				DWORD val = VIEW_APP_ST_NORMAL;
				BOOL chChgErr;
				if( this->bonCtrl.GetOpenBonDriver(NULL) == FALSE ){
					val = VIEW_APP_ST_ERR_BON;
				}else if( this->bonCtrl.IsRec() ){
					val = VIEW_APP_ST_REC;
				}else if( this->bonCtrl.GetEpgCapStatus(NULL) == CBonCtrl::ST_WORKING ){
					val = VIEW_APP_ST_GET_EPG;
				}else if( this->IsChChanging(&chChgErr) == FALSE && chChgErr ){
					val = VIEW_APP_ST_ERR_CH_CHG;
				}
				resParam->data = NewWriteVALUE(val, resParam->dataSize);
				resParam->param = CMD_SUCCESS;
			}
			return;
		case CMD2_VIEW_APP_SET_ID:
			OutputDebugString(L"CMD2_VIEW_APP_SET_ID");
			if( ReadVALUE(&this->outCtrlID, cmdParam->data, cmdParam->dataSize, NULL) ){
				resParam->param = CMD_SUCCESS;
			}
			return;
		case CMD2_VIEW_APP_GET_ID:
			OutputDebugString(L"CMD2_VIEW_APP_GET_ID");
			resParam->data = NewWriteVALUE(this->outCtrlID, resParam->dataSize);
			resParam->param = CMD_SUCCESS;
			return;
		case CMD2_VIEW_APP_SET_STANDBY_REC:
			OutputDebugString(L"CMD2_VIEW_APP_SET_STANDBY_REC");
			{
				DWORD val;
				if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL) ){
					PostMessage(this->msgWnd, WM_RESERVE_REC_STANDBY, val, 0);
					resParam->param = CMD_SUCCESS;
				}
			}
			return;
		case CMD2_VIEW_APP_REC_FILE_PATH:
			OutputDebugString(L"CMD2_VIEW_APP_REC_FILE_PATH");
			{
				DWORD id;
				if( ReadVALUE(&id, cmdParam->data, cmdParam->dataSize, NULL) ){
					wstring saveFile;
					BOOL subRec = FALSE;
					this->bonCtrl.GetSaveFilePath(id, &saveFile, &subRec);
					if( saveFile.size() > 0 ){
						resParam->data = NewWriteVALUE(saveFile, resParam->dataSize);
						resParam->param = CMD_SUCCESS;
					}
				}
			}
			return;
		case CMD2_VIEW_APP_SEARCH_EVENT:
			{
				SEARCH_EPG_INFO_PARAM key;
				EPGDB_EVENT_INFO epgInfo;
				if( ReadVALUE(&key, cmdParam->data, cmdParam->dataSize, NULL) &&
				    this->bonCtrl.SearchEpgInfo(key.ONID, key.TSID, key.SID, key.eventID, key.pfOnlyFlag, &epgInfo) == NO_ERR ){
					resParam->data = NewWriteVALUE(epgInfo, resParam->dataSize);
					resParam->param = CMD_SUCCESS;
				}
			}
			return;
		case CMD2_VIEW_APP_GET_EVENT_PF:
			{
				GET_EPG_PF_INFO_PARAM key;
				EPGDB_EVENT_INFO epgInfo;
				if( ReadVALUE(&key, cmdParam->data, cmdParam->dataSize, NULL) &&
				    this->bonCtrl.GetEpgInfo(key.ONID, key.TSID, key.SID, key.pfNextFlag, &epgInfo) == NO_ERR ){
					resParam->data = NewWriteVALUE(epgInfo, resParam->dataSize);
					resParam->param = CMD_SUCCESS;
				}
			}
			return;
		case CMD2_VIEW_APP_EXEC_VIEW_APP:
			//����͓����I
			PostMessage(this->msgWnd, WM_VIEW_APP_OPEN, 0, 0);
			resParam->param = CMD_SUCCESS;
			return;
		}
		//CtrlCmdCallbackInvoked()�����C���X���b�h�ŌĂ�
		//����: CPipeServer���A�N�e�B�u�ȊԁA�E�B���h�E�͊m���ɑ��݂��Ȃ���΂Ȃ�Ȃ�
		this->cmdCapture = cmdParam;
		this->resCapture = resParam;
		SendMessage(this->msgWnd, WM_INVOKE_CTRL_CMD, 0, 0);
		this->cmdCapture = NULL;
		this->resCapture = NULL;
	});
}

BOOL CEpgDataCap_BonMain::StopServer(BOOL checkOnlyFlag)
{
	return this->pipeServer.StopServer(checkOnlyFlag);
}

BOOL CEpgDataCap_BonMain::GetViewStatusInfo(
	float* signal,
	DWORD* space,
	DWORD* ch,
	ULONGLONG* drop,
	ULONGLONG* scramble
	)
{
	return this->bonCtrl.GetViewStatusInfo(this->nwCtrlID, signal, space, ch, drop, scramble);
}

void CEpgDataCap_BonMain::CtrlCmdCallbackInvoked()
{
	CMD_STREAM* cmdParam = this->cmdCapture;
	CMD_STREAM* resParam = this->resCapture;
	CEpgDataCap_BonMain* sys = this;

	switch( cmdParam->param ){
	case CMD2_VIEW_APP_SET_BONDRIVER:
		OutputDebugString(L"CMD2_VIEW_APP_SET_BONDRIVER");
		{
			wstring val;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				sys->CloseBonDriver();
				if( sys->OpenBonDriver(val.c_str()) == NO_ERR ){
					resParam->param = CMD_SUCCESS;
					PostMessage(sys->msgWnd, WM_CHG_TUNER, 0, 0);
				}
			}
		}
		break;
	case CMD2_VIEW_APP_SET_CH:
		OutputDebugString(L"CMD2_VIEW_APP_SET_CH");
		{
			{
				SET_CH_INFO val;
				if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
					if( sys->bonCtrl.IsRec() == FALSE ){
						sys->bonCtrl.StopEpgCap();
						if( val.useSID == TRUE ){
							if(sys->SetCh(val.ONID, val.TSID, val.SID) == TRUE){
								sys->lastONID = val.ONID;
								sys->lastTSID = val.TSID;
								sys->lastSID = val.SID;

								resParam->param = CMD_SUCCESS;
								PostMessage(sys->msgWnd, WM_CHG_CH, 0, 0);
							}
						}else if( val.useBonCh == TRUE ){
							if(sys->bonCtrl.SetCh(val.space, val.ch) == TRUE){

								resParam->param = CMD_SUCCESS;
								PostMessage(sys->msgWnd, WM_CHG_CH, 0, 0);
							}
						}
					}
				}
			}
		}
		break;
	case CMD2_VIEW_APP_CLOSE:
		OutputDebugString(L"CMD2_VIEW_APP_CLOSE");
		{
			sys->StopReserveRec();
			if( sys->recCtrlID != 0 ){
				sys->bonCtrl.DeleteServiceCtrl(sys->recCtrlID);
				sys->recCtrlID = 0;
			}
			if( sys->nwCtrlID != 0 ){
				sys->bonCtrl.DeleteServiceCtrl(sys->nwCtrlID);
				sys->nwCtrlID = 0;
			}
			resParam->param = CMD_SUCCESS;
			PostMessage(sys->msgWnd, WM_CLOSE, 0, 0);
		}
		break;
	case CMD2_VIEW_APP_CREATE_CTRL:
		OutputDebugString(L"CMD2_VIEW_APP_CREATE_CTRL");
		{
			DWORD val = 0;
			if( sys->bonCtrl.CreateServiceCtrl(&val) == TRUE ){
				sys->cmdCtrlList.push_back(val);
				resParam->data = NewWriteVALUE(val, resParam->dataSize);
				resParam->param = CMD_SUCCESS;
			}
		}
		break;
	case CMD2_VIEW_APP_DELETE_CTRL:
		OutputDebugString(L"CMD2_VIEW_APP_DELETE_CTRL");
		{
			DWORD val = 0;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				if( sys->bonCtrl.DeleteServiceCtrl(val) == TRUE ){
					auto itr = std::find(sys->cmdCtrlList.begin(), sys->cmdCtrlList.end(), val);
					if( itr != sys->cmdCtrlList.end() ){
						sys->cmdCtrlList.erase(itr);
					}
					resParam->param = CMD_SUCCESS;

					if( sys->cmdCtrlList.empty() == false ){
						WORD sid = 0xFFFF;
						sys->bonCtrl.GetServiceID(sys->cmdCtrlList.front(), &sid);
						sys->bonCtrl.SetServiceID(sys->nwCtrlID, sid);
						if( sid != 0xFFFF ){
							sys->lastSID = sid;
							PostMessage(sys->msgWnd, WM_CHG_CH, 0, 0);
						}
					}
				}
			}
		}
		break;
	case CMD2_VIEW_APP_SET_CTRLMODE:
		OutputDebugString(L"CMD2_VIEW_APP_SET_CTRLMODE");
		{
			SET_CTRL_MODE val;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				if(val.enableScramble == 0 && sys->nwCtrlID != 0){
					sys->bonCtrl.SetScramble(sys->nwCtrlID, FALSE);
				}
				sys->bonCtrl.SetScramble(val.ctrlID, val.enableScramble);
				sys->bonCtrl.SetServiceMode(val.ctrlID, val.enableCaption, val.enableData);
				sys->bonCtrl.SetServiceID(val.ctrlID, val.SID);
				
				resParam->param = CMD_SUCCESS;
			}
		}
		break;
	case CMD2_VIEW_APP_REC_START_CTRL:
		OutputDebugString(L"CMD2_VIEW_APP_REC_START_CTRL");
		{
			SET_CTRL_REC_PARAM val;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				BOOL overWrite = sys->overWriteFlag;
				if( val.overWriteFlag != 2 ){
					overWrite = val.overWriteFlag;
				}
				sys->bonCtrl.ClearErrCount(val.ctrlID);
				if(sys->bonCtrl.StartSave(val.ctrlID, val.fileName, overWrite, val.pittariFlag, val.pittariONID, val.pittariTSID, val.pittariSID, val.pittariEventID, val.createSize, val.saveFolder, sys->recFolderList) ){
					resParam->param = CMD_SUCCESS;
					PostMessage(sys->msgWnd, WM_RESERVE_REC_START, 0, 0);
				}
			}
		}
		break;
	case CMD2_VIEW_APP_REC_STOP_CTRL:
		OutputDebugString(L"CMD2_VIEW_APP_REC_STOP_CTRL");
		{
			SET_CTRL_REC_STOP_PARAM val;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				wstring saveFile = L"";
				BOOL subRec = FALSE;
				sys->bonCtrl.GetSaveFilePath(val.ctrlID, &saveFile, &subRec);
				if( saveFile.size() > 0 && val.saveErrLog == 1 ){
					fs_path infoPath = GetPrivateProfileToFolderPath(L"SET", L"RecInfoFolder", GetCommonIniPath().c_str());

					if( infoPath.empty() == false ){
						infoPath.append(fs_path(saveFile).filename().concat(L".err").native());
						sys->bonCtrl.SaveErrCount(val.ctrlID, infoPath.native());
					}else{
						wstring saveFileErr = saveFile;
						saveFileErr += L".err";
						sys->bonCtrl.SaveErrCount(val.ctrlID, saveFileErr);
					}
				}
				SET_CTRL_REC_STOP_RES_PARAM resVal;
				resVal.recFilePath = saveFile;
				resVal.drop = 0;
				resVal.scramble = 0;
				resVal.subRecFlag = (BYTE)subRec;
				sys->bonCtrl.GetErrCount(val.ctrlID, &resVal.drop, &resVal.scramble);
				if(sys->bonCtrl.EndSave(val.ctrlID) == TRUE){
					resParam->data = NewWriteVALUE(resVal, resParam->dataSize);
					resParam->param = CMD_SUCCESS;
					if( sys->cmdCtrlList.size() == 1 ){
						PostMessage(sys->msgWnd, WM_RESERVE_REC_STOP, 0, 0);
					}
				}
			}
		}
		break;
	case CMD2_VIEW_APP_EPGCAP_START:
		OutputDebugString(L"CMD2_VIEW_APP_EPGCAP_START");
		{
			vector<SET_CH_INFO> val;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				vector<EPGCAP_SERVICE_INFO> chList;
				for( size_t i=0; i<val.size(); i++ ){
					EPGCAP_SERVICE_INFO item;
					item.ONID = val[i].ONID;
					item.TSID = val[i].TSID;
					item.SID = val[i].SID;
					chList.push_back(item);
				}
				if( sys->bonCtrl.StartEpgCap(&chList) ){
					PostMessage(sys->msgWnd, WM_RESERVE_EPGCAP_START, 0, 0);
					
					resParam->param = CMD_SUCCESS;
				}
			}
		}
		break;
	case CMD2_VIEW_APP_EPGCAP_STOP:
		OutputDebugString(L"CMD2_VIEW_APP_EPGCAP_STOP");
		{
			sys->bonCtrl.StopEpgCap();
			resParam->param = CMD_SUCCESS;
			PostMessage(sys->msgWnd, WM_RESERVE_EPGCAP_STOP, 0, 0);
		}
		break;
	case CMD2_VIEW_APP_REC_STOP_ALL:
		OutputDebugString(L"CMD2_VIEW_APP_REC_STOP_ALL");
		{
			DWORD ret = CMD_SUCCESS;
			if( sys->recCtrlID != 0 ){
				if(sys->bonCtrl.EndSave(sys->recCtrlID) == FALSE ){
					ret = CMD_ERR;
				}
				sys->bonCtrl.DeleteServiceCtrl(sys->recCtrlID);
				sys->recCtrlID = 0;
			}
			sys->StopReserveRec();

			resParam->param = ret;
			PostMessage(sys->msgWnd, WM_RESERVE_REC_STOP, 0, 0);
		}
		break;
	case CMD2_VIEW_APP_REC_WRITE_SIZE:
		{
			DWORD val = 0;
			if( ReadVALUE(&val, cmdParam->data, cmdParam->dataSize, NULL ) == TRUE ){
				__int64 writeSize = -1;
				sys->bonCtrl.GetRecWriteSize(val, &writeSize);
				resParam->data = NewWriteVALUE(writeSize, resParam->dataSize);
				resParam->param = CMD_SUCCESS;
			}
		}
		break;
	default:
		_OutputDebugString(L"err default cmd %d\r\n", cmdParam->param);
		resParam->param = CMD_NON_SUPPORT;
		break;
	}
}
