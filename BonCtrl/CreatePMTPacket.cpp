#include "stdafx.h"
#include "CreatePMTPacket.h"

CCreatePMTPacket::CCreatePMTPacket(void)
{
	this->needCaption = TRUE;
	this->needData = TRUE;

	this->lastPmtPID = 0xFFFF;
	this->lastPcrPID = 0xFFFF;
	this->lastPgNumber = 0xFFFF;
	this->lastVersion = 0xFF;

	this->createVer = 0;
	this->createCounter = 0;
}

//PMT�쐬���̃��[�h
//�����F
// needCaption			[IN]�����f�[�^���܂߂邩�ǂ����iTRUE:�܂߂�AFALSE�F�܂߂Ȃ��j
// needData				[IN]�f�[�^�J���[�Z�����܂߂邩�ǂ����iTRUE:�܂߂�AFALSE�F�܂߂Ȃ��j
void CCreatePMTPacket::SetCreateMode(
	BOOL needCaption_,
	BOOL needData_
)
{
	if( this->needCaption != needCaption_ || this->needData != needData_ ){
		this->needCaption = needCaption_;
		this->needData = needData_;
		CreatePMT();
	}
}

//�쐬���ƂȂ�PMT�̃p�P�b�g�����
//�߂�l�F
// �G���[�R�[�h
//�����F
// packet			//[IN] PMT�̃p�P�b�g
DWORD CCreatePMTPacket::AddData(
	CTSPacketUtil* packet
)
{
	if( packet == NULL ){
		return ERR_FALSE;
	}
	DWORD ret = ERR_NEED_NEXT_PACKET;
	ret = buffUtil.Add188TS(packet);
	if( ret == TRUE ){
		this->lastPmtPID = packet->PID;

		BYTE* section = NULL;
		DWORD sectionSize = 0;
		while( buffUtil.GetSectionBuff( &section, &sectionSize ) == TRUE ){
			ret = DecodePMT(section, sectionSize);
		}
	}
	return ret;
}

DWORD CCreatePMTPacket::DecodePMT(BYTE* data, DWORD dataSize)
{
	if( data == NULL ){
		return ERR_FALSE;
	}

	if( dataSize < 7 ){
		return ERR_FALSE;
	}

	BYTE table_id;
	BYTE section_syntax_indicator;
	WORD section_length;
	WORD program_number;
	BYTE version_number;
	BYTE current_next_indicator;
	BYTE section_number;
	BYTE last_section_number;
	WORD PCR_PID;
	WORD program_info_length;


	DWORD readSize = 0;
	//////////////////////////////////////////////////////
	//��͏���
	table_id = data[0];
	section_syntax_indicator = (data[1]&0x80)>>7;
	section_length = ((WORD)data[1]&0x0F)<<8 | data[2];
	readSize+=3;

	if( section_syntax_indicator != 1 ){
		//�Œ�l����������
		_OutputDebugString(L"CCreatePMTPacket::section_syntax_indicator Err");
		return ERR_FALSE;
	}
	if( table_id != 0x02 ){
		//table_id����������
		_OutputDebugString(L"CCreatePMTPacket::table_id Err");
		return ERR_FALSE;
	}
	if( readSize+section_length > dataSize || section_length < 4){
		//�T�C�Y�ُ�
		_OutputDebugString(L"CCreatePMTPacket::section_length Err");
		return ERR_FALSE;
	}
	//CRC�`�F�b�N
	if( CalcCrc32(3+section_length, data) != 0 ){
		_OutputDebugString(L"CCreatePMTPacket::CRC Err");
		return ERR_FALSE;
	}

	if( section_length - 4 < 9 ){
		_OutputDebugString(L"CCreatePMTPacket::section_length %d Err2", section_length);
		return ERR_FALSE;
	}

	program_number = ((WORD)data[readSize])<<8 | data[readSize+1];
	version_number = (data[readSize+2]&0x3E)>>1;
	current_next_indicator = data[readSize+2]&0x01;
	section_number = data[readSize+3];
	last_section_number = data[readSize+4];
	PCR_PID = ((WORD)data[readSize+5]&0x1F)<<8 | data[readSize+6];
	program_info_length = ((WORD)data[readSize+7]&0x0F)<<8 | data[readSize+8];
	readSize += 9;

	if( readSize + program_info_length > (DWORD)section_length+3-4 ){
		_OutputDebugString(L"CCreatePMTPacket::program_info_length %d Err", program_info_length);
		return ERR_FALSE;
	}

	if( this->lastPcrPID == PCR_PID && this->lastPgNumber == program_number && this->lastVersion == version_number ){
		//�o�[�W���������Ȃ̂ł���ȏ�K�v�Ȃ�
		return ERR_NO_CHAGE;
	}

	this->lastPcrPID = PCR_PID;
	this->lastPgNumber = program_number;
	this->lastVersion = version_number;

	//�ĉ��
	this->emmPIDList.clear();

	//descriptor1
	//�o�C�i�������R�s�[
	this->firstDescBuff.assign(data, data + readSize + program_info_length);

	//EMM���邩�����`�F�b�N
	WORD infoRead = 0;
	while(infoRead+1 < program_info_length){
		BYTE descriptor_tag = data[readSize];
		BYTE descriptor_length = data[readSize+1];
		readSize+=2;

		if( descriptor_tag == 0x09 && descriptor_length >= 4 && infoRead+2+3 < program_info_length){
			//CA
			WORD CA_PID = ((WORD)data[readSize+2]&0x1F)<<8 | (WORD)data[readSize+3];
			if (CA_PID != 0x1fff) {
				this->emmPIDList.push_back(CA_PID);
			}
		}
		readSize += descriptor_length;

		infoRead+= 2+descriptor_length;
	}

	//descriptor2
	size_t descCount = 0;
	while( readSize+4 < (DWORD)section_length+3-4 ){
		WORD ES_info_length = ((WORD)data[readSize+3]&0x0F)<<8 | data[readSize+4];
		if( readSize+ES_info_length+5 > (DWORD)section_length+3-4 ){
			break;
		}
		if( this->secondDescBuff.size() <= descCount ){
			this->secondDescBuff.resize(descCount + 1);
		}
		SECOND_DESC_BUFF* item = &this->secondDescBuff[descCount++];
		item->stream_type = data[readSize];
		item->elementary_PID = ((WORD)data[readSize+1]&0x1F)<<8 | data[readSize+2];
		item->quality = 0;
		item->qualityPID = 0;
		item->descBuff.assign(data + readSize, data + readSize + ES_info_length + 5);
		readSize += ES_info_length + 5;

		//descriptor
		infoRead = 5;
		while(infoRead+1 < (int)item->descBuff.size()){
			BYTE descriptor_tag = item->descBuff[infoRead];
			BYTE descriptor_length = item->descBuff[infoRead+1];

			if( descriptor_tag == 0x09 && descriptor_length >= 4 && infoRead+5 < (int)item->descBuff.size()){
				//CA
				WORD CA_PID = ((WORD)item->descBuff[2+infoRead+2]&0x1F)<<8 | (WORD)item->descBuff[2+infoRead+3];
				if (CA_PID != 0x1fff) {
					this->emmPIDList.push_back(CA_PID);
				}
			}else if( descriptor_tag == 0xC0 && descriptor_length >= 3 && infoRead+4 < (int)item->descBuff.size() ){
				//�K�w�`���L�q�q
				item->quality = item->descBuff[2+infoRead]&0x01;
				item->qualityPID = ((WORD)item->descBuff[2+infoRead+1]&0x1F)<<8 | item->descBuff[2+infoRead+2];
			}
			infoRead += 2+descriptor_length;
		}
	}
	this->secondDescBuff.resize(descCount);

	CreatePMT();

	return TRUE;
}

void CCreatePMTPacket::CreatePMT()
{
	if( this->firstDescBuff.empty() ){
		return;
	}
	this->needPIDList.clear();

	BOOL findVHighQ = FALSE;
	BOOL findAHighQ = FALSE;
	BOOL findMPEG2V = FALSE;
	BOOL findAAC = FALSE;

	this->createVer++;
	if( this->createVer > 31 ){
		this->createVer = 0;
	}

	//�f�[�^�ꗗ�`�F�b�N
	for( size_t i=0; i<secondDescBuff.size(); i++ ){
		if( secondDescBuff[i].quality == 1 ){
			//���K�w����
			if( secondDescBuff[i].stream_type == 0x02 ){
				findVHighQ = TRUE;
			}else if( secondDescBuff[i].stream_type == 0x0F ){
				findAHighQ = TRUE;
			}
		}
		if( secondDescBuff[i].stream_type == 0x02 ){
			findMPEG2V = TRUE;
		}
		else if( secondDescBuff[i].stream_type == 0x0F ){
			findAAC = TRUE;
		}
	}

	//pointer_field
	this->createPSI.assign(1, 0);
	//�ŏ���Descriptor���[�v�܂ŃR�s�[
	this->createPSI.insert(this->createPSI.end(), this->firstDescBuff.begin(), this->firstDescBuff.end());

	for( size_t i=0; i<secondDescBuff.size(); i++ ){
		BOOL matched = FALSE;
		switch(secondDescBuff[i].stream_type){
			case 0x02:
				//MPEG2 VIDEO
				if( findVHighQ == TRUE ){
					if( secondDescBuff[i].quality == 1 ){
						matched = TRUE;
					}
				}else{
					matched = TRUE;
				}
				break;
			case 0x0F:
				//MPEG2 AAC
				if( findAHighQ == TRUE ){
					if( secondDescBuff[i].quality == 1 ){
						matched = TRUE;
					}
				}else{
					matched = TRUE;
				}
				break;
			case 0x1B:
				//MPEG4 VIDEO
				if( findMPEG2V == FALSE ){
					matched = TRUE;
				}
				break;
			case 0x04:
				//MPEG2 AUDIO
				if( findAAC == FALSE ){
					matched = TRUE;
				}
				break;
			case 0x24:
				//HEVC VIDEO
				matched = TRUE;
				break;
			case 0x06:
				//����
				if( this->needCaption == TRUE ){
					matched = TRUE;
				}
				break;
			case 0x0D:
				//�f�[�^�J���[�Z��
				if( this->needData == TRUE ){
					matched = TRUE;
				}
				break;
			default:
				break;
		}
		if( matched != FALSE ){
			this->createPSI.insert(this->createPSI.end(), this->secondDescBuff[i].descBuff.begin(), this->secondDescBuff[i].descBuff.end());
			this->needPIDList.push_back(this->secondDescBuff[i].elementary_PID);
		}
	}

	//SectionLength
	this->createPSI[2] = (this->createPSI.size()+4-4)>>8&0x0F;
	this->createPSI[2] |= 0xB0; 
	this->createPSI[3] = (this->createPSI.size()+4-4)&0xFF;
	//�o�[�W����
	this->createPSI[6] = this->createVer<<1;
	this->createPSI[6] |= 0xC1;

	DWORD ulCrc = CalcCrc32((int)this->createPSI.size()-1, &this->createPSI[1]);
	this->createPSI.push_back(ulCrc>>24&0xFF);
	this->createPSI.push_back(ulCrc>>16&0xFF);
	this->createPSI.push_back(ulCrc>>8&0xFF);
	this->createPSI.push_back(ulCrc&0xFF);

	CreatePacket();
}

void CCreatePMTPacket::CreatePacket()
{
	this->createPacket.clear();

	//TS�p�P�b�g���쐬
	for( size_t i = 0 ; i<this->createPSI.size(); i+=184 ){
		this->createPacket.push_back(0x47);
		this->createPacket.push_back((this->lastPmtPID >> 8 & 0x1F) | (i==0 ? 0x40 : 0x00));
		this->createPacket.push_back(this->lastPmtPID & 0xFF);
		this->createPacket.push_back(0x10);
		this->createPacket.insert(this->createPacket.end(), this->createPSI.begin() + i, this->createPSI.begin() + min(i + 184, this->createPSI.size()));
		this->createPacket.resize(((this->createPacket.size() - 1) / 188 + 1) * 188, 0xFF);
	}
}

//�K�v��PID�����m�F
//�߂�l�F
// TRUE�i�K�v�j�AFALSE�i�s�K�v�j
//�����F
// PID				//[IN]�m�F����PID
BOOL CCreatePMTPacket::IsNeedPID(
	WORD PID
)
{
	if( this->lastPmtPID == PID || this->lastPcrPID == PID){
		return TRUE;
	}else{
		if( std::find(this->needPIDList.begin(), this->needPIDList.end(), PID) != this->needPIDList.end() ){
			return TRUE;
		}
		if( std::find(this->emmPIDList.begin(), this->emmPIDList.end(), PID) != this->emmPIDList.end() ){
			return TRUE;
		}
	}
	return FALSE;
}

//�쐬PMT�̃o�b�t�@�|�C���^���擾
//�߂�l�F
// �쐬PMT�̃o�b�t�@�|�C���^
//�����F
// buff					[OUT]�쐬����PMT�p�P�b�g�ւ̃|�C���^�i����Ăяo�����܂ŗL���j
// size					[OUT]buff�̃T�C�Y
// incrementFlag		[IN]TS�p�P�b�g��Counter���C���N�������g���邩�ǂ����iTRUE:����AFALSE�F���Ȃ��j
BOOL CCreatePMTPacket::GetPacket(
	BYTE** buff,
	DWORD* size,
	BOOL incrementFlag
)
{
	if( incrementFlag == TRUE ){
		IncrementCounter();
	}
	if( this->createPacket.empty() == false ){
		*buff = &this->createPacket[0];
		*size = (DWORD)this->createPacket.size();
	}else{
		return FALSE;
	}
	return TRUE;
}

//���������N���A
void CCreatePMTPacket::Clear()
{
	this->lastPmtPID = 0xFFFF;
	this->lastPcrPID = 0xFFFF;
	this->lastPgNumber = 0xFFFF;
	this->lastVersion = 0xFF;

	this->needPIDList.clear();
	this->emmPIDList.clear();
	this->firstDescBuff.clear();
	this->secondDescBuff.clear();

	this->createPSI.clear();
	this->createPacket.clear();
}

void CCreatePMTPacket::IncrementCounter()
{
	for( size_t i = 0 ; i+3<this->createPacket.size(); i+=188 ){
		this->createPacket[i+3] = (BYTE)(this->createCounter | 0x10);
		this->createCounter++;
		if( this->createCounter >= 16 ){
			this->createCounter = 0;
		}
	}
}

BYTE CCreatePMTPacket::GetVersion()
{
	return this->lastVersion;
}
