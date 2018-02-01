#include "stdafx.h"
#include "TSBuffUtil.h"

CTSBuffUtil::CTSBuffUtil()
{
	this->sectionSize = 0;
	this->lastPID = 0xFFFF;
	this->lastCounter = 0xFF;

	this->duplicateFlag = FALSE;
}

void CTSBuffUtil::Clear()
{
	this->sectionSize = 0;
	this->sectionBuff.clear();
	this->carryPacket.clear();

	this->lastPID = 0xFFFF;
	this->lastCounter = 0xFF;
}

BOOL CTSBuffUtil::CheckCounter(CTSPacketUtil* tsPacket)
{
	if( tsPacket->PID == 0x1FFF ){
		//NULL�p�P�b�g���͈Ӗ��Ȃ�
		this->duplicateFlag = FALSE;
		return TRUE;
	}
	if( this->lastPID != 0xFFFF || this->lastCounter != 0xFF ){
		if( this->lastPID != tsPacket->PID ){
			//PID�ύX���ꂽ�̂ŕs�A��
			this->duplicateFlag = FALSE;
			return FALSE;
		}else{
			if( tsPacket->adaptation_field_control == 0x00 || tsPacket->adaptation_field_control == 0x02 ){
				//�y�C���[�h�����݂��Ȃ��ꍇ�͈Ӗ��Ȃ�
				this->duplicateFlag = FALSE;
				if( tsPacket->adaptation_field_control == 0x02 || tsPacket->adaptation_field_control == 0x03 ){
					if( tsPacket->transport_scrambling_control == 0 ){
						if(tsPacket->discontinuity_indicator == 1){
							//�s�A���̔��肪�K�v
							return FALSE;
						}else{
							return TRUE;
						}
					}else{
						return TRUE;
					}
				}else{
					return TRUE;
				}
			}
			if( this->lastCounter == tsPacket->continuity_counter ){
				if( tsPacket->adaptation_field_control == 0x01 || tsPacket->adaptation_field_control == 0x03 ){
					if( tsPacket->transport_scrambling_control == 0 ){
						if( this->duplicateFlag == FALSE ){
							//�d���H�ꉞ�A���Ɣ���
							this->duplicateFlag = TRUE;
							if( tsPacket->adaptation_field_control == 0x02 || tsPacket->adaptation_field_control == 0x03 ){
								if(tsPacket->discontinuity_indicator == 1){
									//�s�A���̔��肪�K�v
									return FALSE;
								}else{
									return TRUE;
								}
							}else{
								return TRUE;
							}
						}else{
							//�O��d���Ɣ��f���Ă�̂ŕs�A��
							this->duplicateFlag = FALSE;
							return FALSE;
						}
					}else{
						return TRUE;
					}
				}
			}
			if( this->lastCounter+1 != tsPacket->continuity_counter ){
				if( this->lastCounter != 0x0F && tsPacket->continuity_counter != 0x00 ){
					//�J�E���^�[����񂾂̂ŕs�A��
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

DWORD CTSBuffUtil::Add188TS(CTSPacketUtil* tsPacket)
{
	if( tsPacket == NULL ){
		return FALSE;
	}

	//�o�b�t�@�����ׂĎ󂯎��
	BYTE* sectionData;
	DWORD dataSize;
	while( GetSectionBuff(&sectionData, &dataSize) != FALSE );

	//�J�E���^�[�`�F�b�N
	if( CheckCounter(tsPacket) == FALSE ){
		Clear();
	}
	//�X�N�����u���̃`�F�b�N
	if( tsPacket->transport_scrambling_control != 0 ){
		//�X�N�����u���p�P�b�g�Ȃ̂ŉ�͂ł��Ȃ�
		Clear();
		return ERR_NOT_SUPPORT;
	}

	if( tsPacket->payload_unit_start_indicator == 1 ){
		if( tsPacket->data_byteSize < 3 ){
			//�T�C�Y������������
			return FALSE;
		}
		if(tsPacket->data_byte[0] == 0x00 && tsPacket->data_byte[1] == 0x00 && tsPacket->data_byte[2] == 0x01){
			//PES
			Clear();
			return ERR_NOT_SUPPORT;
		}else if( tsPacket->adaptation_field_length > 0 && tsPacket->random_access_indicator == 1 ){
			//PES
			Clear();
			return ERR_NOT_SUPPORT;
		}else if( tsPacket->adaptation_field_length > 0 && tsPacket->PCR_flag == 1 ){
			//PCR
			Clear();
			return ERR_NOT_SUPPORT;
		}else if( tsPacket->adaptation_field_length > 0 && tsPacket->OPCR_flag == 1 ){
			//OPCR
			Clear();
			return ERR_NOT_SUPPORT;
		}
	}

	if( this->lastPID == 0xFFFF && this->lastCounter == 0xFF ){
		//����
		if( tsPacket->payload_unit_start_indicator == 1 ){
			//PSI
			this->lastPID = tsPacket->PID;
			this->lastCounter = tsPacket->continuity_counter;
			return AddSectionBuff(tsPacket);
		}else{
			//�X�^�[�g�ʒu�ł͂Ȃ�
			return ERR_ADD_NEXT;
		}
	}else{
		this->lastPID = tsPacket->PID;
		this->lastCounter = tsPacket->continuity_counter;
		return AddSectionBuff(tsPacket);
	}

}

BOOL CTSBuffUtil::GetSectionBuff(BYTE** sectionData, DWORD* dataSize)
{
	if( sectionSize == 0 && carryPacket.empty() == false ){
		//�J��z���p�P�b�g������
		CTSPacketUtil tsPacket;
		tsPacket.payload_unit_start_indicator = 1;
		tsPacket.data_byteSize = (BYTE)carryPacket.size();
		tsPacket.data_byte = &carryPacket.front();
		if( AddSectionBuff(&tsPacket) != 2 ){
			carryPacket.clear();
		}
	}
	if( sectionSize == 0 || sectionSize != sectionBuff.size() ){
		//sectionBuff��Get�ς݂��쐬�r��
		carryPacket.clear();
		return FALSE;
	}

	*sectionData = &sectionBuff.front();
	*dataSize = sectionSize;
	//sectionBuff��Get�ς݂ł��邱�Ƃ�����
	sectionSize = 0;

	return TRUE;
}

DWORD CTSBuffUtil::AddSectionBuff(CTSPacketUtil* tsPacket)
{
	if( tsPacket->data_byteSize == 0 || tsPacket->data_byte == NULL ){
		return ERR_ADD_NEXT;
	}
	if( tsPacket->payload_unit_start_indicator != 1 && (sectionSize == 0 || sectionSize == sectionBuff.size()) ){
		return ERR_ADD_NEXT;
	}

	if( tsPacket->payload_unit_start_indicator == 1 ){
		BYTE pointer_field = tsPacket->data_byte[0];
		if( pointer_field + 1 > tsPacket->data_byteSize ){
			//�T�C�Y������������
			_OutputDebugString(L"��psi size err PID 0x%04X\r\n", tsPacket->PID);
			sectionSize = 0;
			return FALSE;
		}
		if( sectionSize != 0 && sectionSize != sectionBuff.size() ){
			if( sectionSize - sectionBuff.size() == pointer_field ){
				sectionBuff.insert(sectionBuff.end(), tsPacket->data_byte + 1, tsPacket->data_byte + 1 + pointer_field);
				//�c��̃y�C���[�h���J��z��
				carryPacket.assign(1, 0);
				carryPacket.insert(carryPacket.end(), tsPacket->data_byte + 1 + pointer_field, tsPacket->data_byte + tsPacket->data_byteSize);
				return TRUE;
			}else{
				//�T�C�Y�����������̂ŃN���A
				_OutputDebugString(L"��psi section size err PID 0x%04X\r\n", tsPacket->PID);
				sectionSize = 0;
			}
		}
		BYTE readSize = pointer_field + 1;

		//�}���`�Z�N�V�����`�F�b�N
		if( readSize + 2 >= tsPacket->data_byteSize ||
		    tsPacket->data_byte[readSize] == 0xFF &&
		    tsPacket->data_byte[readSize+1] == 0xFF &&
		    tsPacket->data_byte[readSize+2] == 0xFF ){
			//�c��̓X�^�b�t�B���O�o�C�g
			return ERR_ADD_NEXT;
		}

		sectionSize = (((DWORD)tsPacket->data_byte[readSize+1]&0x0F) << 8 | tsPacket->data_byte[readSize+2]) + 3;
		sectionBuff.assign(tsPacket->data_byte + readSize, tsPacket->data_byte + min((DWORD)tsPacket->data_byteSize, readSize + sectionSize));
		if( sectionSize == sectionBuff.size() ){
			//���̃p�P�b�g�����Ŋ����B�c��̃y�C���[�h���J��z��
			if( carryPacket.empty() == false && tsPacket->data_byte == &carryPacket.front() ){
				carryPacket.erase(carryPacket.begin() + readSize, carryPacket.begin() + readSize + sectionSize);
				//�}���`�Z�N�V�����ɂ��A���J��z���ł��邱�Ƃ��������ʂȖ߂�l
				return 2;
			}else{
				carryPacket.assign(1, 0);
				carryPacket.insert(carryPacket.end(), tsPacket->data_byte + readSize + sectionSize, tsPacket->data_byte + tsPacket->data_byteSize);
				return TRUE;
			}
		}else{
			//���̃p�P�b�g�K�v
			return ERR_ADD_NEXT;
		}
	}else{
		//�����p�P�b�g�ɂ܂������Ă���
		sectionBuff.insert(sectionBuff.end(), tsPacket->data_byte, tsPacket->data_byte + min((DWORD)tsPacket->data_byteSize, sectionSize - (DWORD)sectionBuff.size()));
		if( sectionSize == sectionBuff.size() ){
			return TRUE;
		}else{
			return ERR_ADD_NEXT;
		}
	}
}
