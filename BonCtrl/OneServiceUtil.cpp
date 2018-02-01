#include "stdafx.h"
#include "OneServiceUtil.h"


COneServiceUtil::COneServiceUtil(void)
{
	this->SID = 0xFFFF;

	this->pmtPID = 0xFFFF;

	this->enableScramble = TRUE;

	this->pittariStart = FALSE;
	this->pittariEndChk = FALSE;
}


COneServiceUtil::~COneServiceUtil(void)
{
	SendUdp(NULL);
	SendTcp(NULL);
}

//�����Ώ�ServiceID��ݒ�
//�����F
// SID			[IN]ServiceID
void COneServiceUtil::SetSID(
	WORD SID_
)
{
	if( this->SID != SID_ ){
		this->pmtPID = 0xFFFF;
		this->emmPIDMap.clear();

		this->dropCount.Clear();
	}
	this->SID = SID_;
}

//�ݒ肳��Ă鏈���Ώۂ�ServiceID���擾
//�߂�l�F
// ServiceID
WORD COneServiceUtil::GetSID()
{
	return this->SID;
}

//UDP�ő��M���s��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// sendList		[IN/OUT]���M�惊�X�g�BNULL�Œ�~�BPort�͎��ۂɑ��M�Ɏg�p����Port���Ԃ�B
BOOL COneServiceUtil::SendUdp(
	vector<NW_SEND_INFO>* sendList
	)
{
	if( this->sendUdp != NULL ){
		this->sendUdp->CloseUpload();
	}

	if(udpPortMutex.size() != 0){
		for( int i=0; i<(int)udpPortMutex.size(); i++ ){
			::CloseHandle(udpPortMutex[i]);
		}
		udpPortMutex.clear();
	}

	if( sendList != NULL ){
		if( this->sendUdp == NULL ){
			this->sendUdp.reset(new CSendUDP);
		}
		for( size_t i=0; i<sendList->size(); i++ ){
			wstring key = L"";
			HANDLE portMutex;

			//�����ł��Ȃ��Ă��[���ł͂Ȃ��̂łقǂقǂɑł��؂�
			for( int j = 0; j < 100; j++ ){
				UINT u[4];
				if( swscanf_s((*sendList)[i].ipString.c_str(), L"%u.%u.%u.%u", &u[0], &u[1], &u[2], &u[3]) == 4 ){
					Format(key, L"%s%d_%d", MUTEX_UDP_PORT_NAME, u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3], (*sendList)[i].port);
				}else{
					Format(key, L"%s%s_%d", MUTEX_UDP_PORT_NAME, (*sendList)[i].ipString.c_str(), (*sendList)[i].port);
				}
				portMutex = CreateMutex(NULL, FALSE, key.c_str());
		
				if( portMutex == NULL ){
					(*sendList)[i].port++;
				}else if( GetLastError() == ERROR_ALREADY_EXISTS ){
					CloseHandle(portMutex);
					(*sendList)[i].port++;
				}else{
					_OutputDebugString(L"%s\r\n", key.c_str());
					this->udpPortMutex.push_back(portMutex);
					break;
				}
			}
		}

		this->sendUdp->StartUpload(sendList);
	}else{
		this->sendUdp.reset();
	}

	return TRUE;
}

//TCP�ő��M���s��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// sendList		[IN/OUT]���M�惊�X�g�BNULL�Œ�~�BPort�͎��ۂɑ��M�Ɏg�p����Port���Ԃ�B
BOOL COneServiceUtil::SendTcp(
	vector<NW_SEND_INFO>* sendList
	)
{
	if( this->sendTcp != NULL ){
		this->sendTcp->CloseUpload();
	}

	if(tcpPortMutex.size() != 0){
		for( int i=0; i<(int)tcpPortMutex.size(); i++ ){
			::CloseHandle(tcpPortMutex[i]);
		}
		tcpPortMutex.clear();
	}

	if( sendList != NULL ){
		if( this->sendTcp == NULL ){
			this->sendTcp.reset(new CSendTCP);
		}
		for( size_t i=0; i<sendList->size(); i++ ){
			wstring key = L"";
			HANDLE portMutex;

			//�����ł��Ȃ��Ă��[���ł͂Ȃ��̂łقǂقǂɑł��؂�
			for( int j = 0; j < 100; j++ ){
				UINT u[4];
				if( swscanf_s((*sendList)[i].ipString.c_str(), L"%u.%u.%u.%u", &u[0], &u[1], &u[2], &u[3]) == 4 ){
					Format(key, L"%s%d_%d", MUTEX_TCP_PORT_NAME, u[0] << 24 | u[1] << 16 | u[2] << 8 | u[3], (*sendList)[i].port);
				}else{
					Format(key, L"%s%s_%d", MUTEX_TCP_PORT_NAME, (*sendList)[i].ipString.c_str(), (*sendList)[i].port);
				}
				portMutex = CreateMutex(NULL, FALSE, key.c_str());
		
				if( portMutex == NULL ){
					(*sendList)[i].port++;
				}else if( GetLastError() == ERROR_ALREADY_EXISTS ){
					CloseHandle(portMutex);
					(*sendList)[i].port++;
				}else{
					_OutputDebugString(L"%s\r\n", key.c_str());
					this->tcpPortMutex.push_back(portMutex);
					break;
				}
			}
		}

		this->sendTcp->StartUpload(sendList);
	}else{
		this->sendTcp.reset();
	}

	return TRUE;
}

//�o�͗pTS�f�[�^�𑗂�
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// data		[IN]TS�f�[�^
// size		[IN]data�̃T�C�Y
// funcGetPresent	[IN]EPG�̌��ݔԑgID�𒲂ׂ�֐�
BOOL COneServiceUtil::AddTSBuff(
	BYTE* data,
	DWORD size,
	const std::function<int(WORD, WORD, WORD)>& funcGetPresent
	)
{
	BOOL ret = TRUE;
	if( this->SID == 0xFFFF || this->sendTcp != NULL || this->sendUdp != NULL){
		//�S�T�[�r�X����
		if( data != NULL ){
			ret = WriteData(data, size);
		}
		for( DWORD i=0; i<size; i+=188 ){
			CTSPacketUtil packet;
			if( packet.Set188TS(data + i, 188) == TRUE ){
				if( packet.PID == this->pmtPID ){
					createPmt.AddData(&packet);
				}
			}
		}
	}else{
		this->buff.clear();

		for( DWORD i=0; i<size; i+=188 ){
			CTSPacketUtil packet;
			if( packet.Set188TS(data + i, 188) == TRUE ){
				if( packet.PID == 0x0000 ){
					//PAT
					BYTE* patBuff = NULL;
					DWORD patBuffSize = 0;
					if( createPat.GetPacket(&patBuff, &patBuffSize) == TRUE ){
						if( packet.payload_unit_start_indicator == 1 ){
							this->buff.insert(this->buff.end(), patBuff, patBuff + patBuffSize);
						}
					}
				}else if( packet.PID == this->pmtPID ){
					//PMT
					DWORD err = createPmt.AddData(&packet);
					if( err == NO_ERR || err == CCreatePMTPacket::ERR_NO_CHAGE ){
						BYTE* pmtBuff = NULL;
						DWORD pmtBuffSize = 0;
						if( createPmt.GetPacket(&pmtBuff, &pmtBuffSize) == TRUE ){
							this->buff.insert(this->buff.end(), pmtBuff, pmtBuff + pmtBuffSize);
						}else{
							_OutputDebugString(L"createPmt.GetPacket Err");
							//���̂܂�
							this->buff.insert(this->buff.end(), data + i, data + i + 188);
						}
					}else if( err == FALSE ){
						_OutputDebugString(L"createPmt.AddData Err");
						//���̂܂�
						this->buff.insert(this->buff.end(), data + i, data + i + 188);
					}
				}else{
					//���̑�
					if( packet.PID < 0x0030 ){
						//���̂܂�
						this->buff.insert(this->buff.end(), data + i, data + i + 188);
					}else{
						if( createPmt.IsNeedPID(packet.PID) == TRUE ){
							//PMT�Œ�`����Ă�
							this->buff.insert(this->buff.end(), data + i, data + i + 188);
						}else{
							//EMM�Ȃ�K�v
							map<WORD,WORD>::iterator itr;
							itr = this->emmPIDMap.find(packet.PID);
							if( itr != this->emmPIDMap.end() ){
								this->buff.insert(this->buff.end(), data + i, data + i + 188);
							}
						}
					}
				}
			}
		}

		if( this->buff.empty() == false ){
			ret = WriteData(&this->buff.front(), (DWORD)this->buff.size());
		}
	}

	if( this->pittariStart == TRUE ){
		if( this->lastPMTVer == 0xFFFF ){
			this->lastPMTVer = createPmt.GetVersion();
		}else if(this->lastPMTVer != createPmt.GetVersion()){
			//�҂�����J�n
			StratPittariRec();
			this->lastPMTVer = createPmt.GetVersion();
		}
		if( funcGetPresent ){
			int eventID = funcGetPresent(this->pittariONID, this->pittariTSID, this->pittariSID);
			if( eventID >= 0 ){
				if( eventID == this->pittariEventID ){
					//�҂�����J�n
					StratPittariRec();
					this->pittariStart = FALSE;
					this->pittariEndChk = TRUE;
				}
			}
		}
	}
	if( this->pittariEndChk == TRUE ){
		if( funcGetPresent ){
			int eventID = funcGetPresent(this->pittariONID, this->pittariTSID, this->pittariSID);
			if( eventID >= 0 ){
				if( eventID != this->pittariEventID ){
					//�҂�����I��
					StopPittariRec();
					this->pittariEndChk = FALSE;
				}
			}
		}
	}

	return ret;
}

BOOL COneServiceUtil::WriteData(BYTE* data, DWORD size)
{
	if( this->sendUdp != NULL ){
		this->sendUdp->SendData(data, size);
	}
	if( this->sendTcp != NULL ){
		this->sendTcp->SendData(data, size);
	}
	if( this->writeFile != NULL ){
		this->writeFile->AddTSBuff(data, size);
	}

	dropCount.AddData(data, size);

	return TRUE;
}

void COneServiceUtil::SetPmtPID(
	WORD TSID,
	WORD pmtPID_
	)
{
	if( this->pmtPID != pmtPID_ && this->SID != 0xFFFF){
		_OutputDebugString(L"COneServiceUtil::SetPmtPID 0x%04x => 0x%04x", this->pmtPID, pmtPID_);
		map<WORD, CCreatePATPacket::PROGRAM_PID_INFO> PIDMap;

		CCreatePATPacket::PROGRAM_PID_INFO item;
		item.PMTPID = pmtPID_;
		item.SID = this->SID;
		PIDMap.insert(pair<WORD, CCreatePATPacket::PROGRAM_PID_INFO>(item.PMTPID,item));
		
		item.PMTPID = 0x0010;
		item.SID = 0x00;
		PIDMap.insert(pair<WORD, CCreatePATPacket::PROGRAM_PID_INFO>(item.PMTPID,item));
		
		createPat.SetParam(TSID, &PIDMap);

		this->pmtPID = pmtPID_;
	}
}

void COneServiceUtil::SetEmmPID(
	const map<WORD,WORD>& PIDMap
	)
{
	this->emmPIDMap = PIDMap;
}

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
BOOL COneServiceUtil::StartSave(
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
)
{
	if( pittariFlag == FALSE ){
		if( this->writeFile == NULL ){
			OutputDebugString(L"*:StartSave");
			this->pittariRecFilePath = L"";
			this->pittariStart = FALSE;
			this->pittariEndChk = FALSE;

			this->writeFile.reset(new CWriteTSFile);
			return this->writeFile->StartSave(fileName, overWriteFlag, createSize, saveFolder, saveFolderSub, maxBuffCount);
		}
	}else{
		if( this->writeFile == NULL ){
			OutputDebugString(L"*:StartSave pittariFlag");
			this->pittariRecFilePath = L"";
			this->pittariFileName = fileName;
			this->pittariOverWriteFlag = overWriteFlag;
			this->pittariCreateSize = createSize;
			this->pittariSaveFolder = saveFolder;
			this->pittariSaveFolderSub = saveFolderSub;
			this->pittariMaxBuffCount = maxBuffCount;
			this->pittariONID = pittariONID_;
			this->pittariTSID = pittariTSID_;
			this->pittariSID = pittariSID_;
			this->pittariEventID = pittariEventID_;

			this->lastPMTVer = 0xFFFF;

			this->pittariStart = TRUE;
			this->pittariEndChk = FALSE;

			return TRUE;
		}
	}

	return FALSE;
}

void COneServiceUtil::StratPittariRec()
{
	if( this->writeFile == NULL ){
		OutputDebugString(L"*:StratPittariRec");
		this->writeFile.reset(new CWriteTSFile);
		this->writeFile->StartSave(this->pittariFileName, this->pittariOverWriteFlag, this->pittariCreateSize,
		                           this->pittariSaveFolder, this->pittariSaveFolderSub, this->pittariMaxBuffCount);
	}
}

void COneServiceUtil::StopPittariRec()
{
	if( this->writeFile == NULL ){
		return ;
	}
	OutputDebugString(L"*:StopPittariRec");
	BOOL subRec;
	this->writeFile->GetSaveFilePath(&this->pittariRecFilePath, &subRec);
	this->writeFile->EndSave();
}

//�t�@�C���ۑ����I������
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
BOOL COneServiceUtil::EndSave()
{
	this->pittariRecFilePath = L"";
	this->pittariStart = FALSE;
	this->pittariEndChk = FALSE;

	if( this->writeFile == NULL ){
		return FALSE;
	}
	BOOL ret = this->writeFile->EndSave();
	this->writeFile.reset();
	OutputDebugString(L"*:EndSave");
	return ret;
}

//�^�撆���ǂ���
//�߂�l�F
// TRUE�i�^�撆�j�AFALSE�i���Ă��Ȃ��j
BOOL COneServiceUtil::IsRec()
{
	if( this->writeFile == NULL ){
		if( this->pittariStart == TRUE || this->pittariEndChk == TRUE ){
			return TRUE;
		}
		return FALSE;
	}else{
		return TRUE;
	}
}

//�X�N�����u�����������̓���ݒ�
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// enable		[IN] TRUE�i��������j�AFALSE�i�������Ȃ��j
void COneServiceUtil::SetScramble(
	BOOL enable
	)
{
	this->enableScramble = enable;
}

//�X�N�����u�������������s�����ǂ���
//�߂�l�F
// TRUE�i��������j�AFALSE�i�������Ȃ��j
BOOL COneServiceUtil::GetScramble(
	)
{
	return this->enableScramble;
}

//�����ƃf�[�^�����܂߂邩�ǂ���
//�����F
// enableCaption		[IN]������ TRUE�i�܂߂�j�AFALSE�i�܂߂Ȃ��j
// enableData			[IN]�f�[�^������ TRUE�i�܂߂�j�AFALSE�i�܂߂Ȃ��j
void COneServiceUtil::SetServiceMode(
	BOOL enableCaption,
	BOOL enableData
	)
{
	createPmt.SetCreateMode(enableCaption, enableData);
}

//�G���[�J�E���g���N���A����
void COneServiceUtil::ClearErrCount()
{
	this->dropCount.Clear();
}

//�h���b�v�ƃX�N�����u���̃J�E���g���擾����
//�����F
// drop				[OUT]�h���b�v��
// scramble			[OUT]�X�N�����u����
void COneServiceUtil::GetErrCount(ULONGLONG* drop, ULONGLONG* scramble)
{
	this->dropCount.GetCount(drop, scramble);
}

//�^�撆�̃t�@�C���̃t�@�C���p�X���擾����
//�����F
// filePath			[OUT]�ۑ��t�@�C����
// subRecFlag		[OUT]�T�u�^�悪�����������ǂ���
void COneServiceUtil::GetSaveFilePath(
	wstring* filePath,
	BOOL* subRecFlag
	)
{
	if( this->writeFile != NULL ){
		this->writeFile->GetSaveFilePath(filePath, subRecFlag);
		if( filePath->size() == 0 ){
			*filePath = this->pittariRecFilePath;
		}
	}
}

//�h���b�v�ƃX�N�����u���̃J�E���g��ۑ�����
//�����F
// filePath			[IN]�ۑ��t�@�C����
void COneServiceUtil::SaveErrCount(
	const wstring& filePath
	)
{
	this->dropCount.SaveLog(filePath);
}


void COneServiceUtil::SetSignalLevel(
	float signalLv
	)
{
	this->dropCount.SetSignal(signalLv);
}

//�^�撆�̃t�@�C���̏o�̓T�C�Y���擾����
//�����F
// writeSize			[OUT]�o�̓T�C�Y
void COneServiceUtil::GetRecWriteSize(
	__int64* writeSize
	)
{
	if( this->writeFile != NULL ){
		this->writeFile->GetRecWriteSize(writeSize);
	}
}

void COneServiceUtil::SetBonDriver(
	const wstring& bonDriver
	)
{
	this->dropCount.SetBonDriver(bonDriver);
}

void COneServiceUtil::SetPIDName(
	const map<WORD, string>& pidName
	)
{
	this->dropCount.SetPIDName(&pidName);
}

void COneServiceUtil::SetNoLogScramble(
	BOOL noLog
	)
{
	this->dropCount.SetNoLog(FALSE, noLog);
}
