#include "stdafx.h"
#include "DropCount.h"
#include "../Common/StringUtil.h"
#include "../Common/TimeUtil.h"
#include <stdio.h>


CDropCount::CDropCount(void)
{
	this->drop = 0;
	this->scramble = 0;
	this->lastLogTime = 0;

	this->lastLogDrop = 0;
	this->lastLogScramble = 0;

	this->signalLv = 0;
	this->bonFile = L"";
}

void CDropCount::AddData(const BYTE* data, DWORD size)
{
	if( data == NULL || size == 0 ){
		return ;
	}
	for( DWORD i=0; i<size; i+=188 ){
		BYTE sync_byte = data[i];
		BYTE transport_error_indicator = data[i + 1] & 0x80;
		if( sync_byte == 0x47 && transport_error_indicator == 0 ){
			WORD pid = (data[i + 1] << 8 | data[i + 2]) & 0x1FFF;
			map<WORD, DROP_INFO>::iterator itr;
			itr = this->infoMap.find(pid);
			if( itr == this->infoMap.end() ){
				BYTE continuity_counter = data[i + 3] & 0x0F;
				DROP_INFO item = {};
				item.lastCounter = (continuity_counter + 15) & 0x0F;
				itr = this->infoMap.insert(pair<WORD, DROP_INFO>(pid, item)).first;
			}
			itr->second.total++;
			if( pid != 0x1FFF ){
				CheckCounter(data + i, &(itr->second));
			}
		}
	}
	DWORD tick = GetTickCount();
	if( tick - this->lastLogTime > 5000 ){
		if( this->lastLogDrop < this->drop ||
		    this->lastLogScramble < this->scramble ){
			string logline;
			SYSTEMTIME now;
			ConvertSystemTime(GetNowI64Time(), &now);
			Format(logline, "%04d/%02d/%02d %02d:%02d:%02d Drop:%I64d Scramble:%I64d Signal: %.02f\r\n",
				now.wYear,
				now.wMonth,
				now.wDay,
				now.wHour,
				now.wMinute,
				now.wSecond,
				this->drop,
				this->scramble,
				this->signalLv
				);
			this->log += logline;
			this->lastLogDrop = max(this->drop, this->lastLogDrop);
			this->lastLogScramble = max(this->scramble, this->lastLogScramble);
		}
		this->lastLogTime = tick;
	}
}

void CDropCount::Clear()
{
	this->infoMap.clear();
	this->drop = 0;
	this->scramble = 0;
	this->log.clear();
	this->lastLogTime = 0;

	if( this->lastLogDrop != MAXULONGLONG ){
		this->lastLogDrop = 0;
	}
	if( this->lastLogScramble != MAXULONGLONG ){
		this->lastLogScramble = 0;
	}
	this->signalLv = 0;
}

void CDropCount::SetSignal(float level)
{
	this->signalLv = level;
}

void CDropCount::SetBonDriver(const wstring& bonDriver)
{
	this->bonFile = bonDriver;
}

void CDropCount::SetNoLog(BOOL noLogDrop, BOOL noLogScramble)
{
	this->lastLogDrop = noLogDrop ? MAXULONGLONG : this->lastLogDrop == MAXULONGLONG ? 0 : this->lastLogDrop;
	this->lastLogScramble = noLogScramble ? MAXULONGLONG : this->lastLogScramble == MAXULONGLONG ? 0 : this->lastLogScramble;
}

void CDropCount::GetCount(ULONGLONG* drop_, ULONGLONG* scramble_)
{
	if( drop_ != NULL ){
		*drop_ = this->drop;
	}
	if( scramble_ != NULL ){
		*scramble_ = this->scramble;
	}
}

ULONGLONG CDropCount::GetDropCount()
{
	return this->drop;
}

ULONGLONG CDropCount::GetScrambleCount()
{
	return this->scramble;
}

void CDropCount::CheckCounter(const BYTE* packet, DROP_INFO* info)
{
	BYTE transport_scrambling_control = packet[3] >> 6;
	BYTE adaptation_field_control = (packet[3] >> 4) & 0x03;
	BYTE continuity_counter = packet[3] & 0x0F;

	if( transport_scrambling_control != 0 ){
		info->scramble++;
		this->scramble++;
	}
	
	if( adaptation_field_control == 0x00 || adaptation_field_control == 0x02 ){
		//�y�C���[�h�����݂��Ȃ��ꍇ�͈Ӗ��Ȃ�
		info->duplicateFlag = FALSE;
	}else{
		BYTE adaptation_field_length = packet[4];
		BYTE discontinuity_indicator = packet[5] & 0x80;
		if( info->lastCounter == continuity_counter ){
			if( adaptation_field_control == 0x01 || adaptation_field_length == 0 || discontinuity_indicator == 0 ){
				//�������ɂ͏d������͑O�p�P�b�g�Ƃ̊��S��r�����ׂ�
				if( info->duplicateFlag == FALSE ){
					//�d���H�ꉞ�A���Ɣ���
					info->duplicateFlag = TRUE;
				}else{
					//�O��d���Ɣ��f���Ă�̂ŕs�A��
					info->drop++;
					this->drop++;
				}
			}else{
				//�s�A���̔��肾������
				info->duplicateFlag = FALSE;
			}
		}else{
			//������͂��Ԃ�lastCounter==15�܂���continuity_counter==0�̂Ƃ��̘A�����肪�o�O���Ă���
			if( ((info->lastCounter + 1) & 0x0F) != continuity_counter ){
				if( adaptation_field_control == 0x01 || adaptation_field_length == 0 || discontinuity_indicator == 0 ){
					//�J�E���^�[����񂾂̂ŕs�A��
					//������͂����ō��������Z����
					info->drop++;
					this->drop++;
				}
			}
			info->duplicateFlag = FALSE;
		}
	}

	info->lastCounter = continuity_counter;
}

void CDropCount::SaveLog(const wstring& filePath)
{
	//������ƈقȂ�f�B���N�g���̎��������͂��Ȃ�
	FILE* fp_;
	if( _wfopen_s(&fp_, filePath.c_str(), L"wbN") == 0 ){
		std::unique_ptr<FILE, decltype(&fclose)> fp(fp_, fclose);
		fprintf(fp.get(), "%s\r\n", this->log.c_str());

		map<WORD, DROP_INFO>::iterator itr;
		for( itr = this->infoMap.begin(); itr != this->infoMap.end(); itr++ ){
			string desc = "";
			map<WORD, string>::iterator itrPID;
			switch(itr->first){
			case 0x0000:
				desc = "PAT";
				break;
			case 0x0001:
				desc = "CAT";
				break;
			case 0x0010:
				desc = "NIT";
				break;
			case 0x0011:
				desc = "SDT/BAT";
				break;
			case 0x0012:
			case 0x0026:
			case 0x0027:
				desc = "EIT";
				break;
			case 0x0013:
				desc = "RST";
				break;
			case 0x0014:
				desc = "TDT/TOT";
				break;
			case 0x0017:
				desc = "DCT";
				break;
			case 0x001E:
				desc = "DIT";
				break;
			case 0x001F:
				desc = "SIT";
				break;
			case 0x0020:
				desc = "LIT";
				break;
			case 0x0021:
				desc = "ERT";
				break;
			case 0x0022:
				desc = "PCAT";
				break;
			case 0x0023:
			case 0x0028:
				desc = "SDTT";
				break;
			case 0x0024:
				desc = "BIT";
				break;
			case 0x0025:
				desc = "NBIT/LDT";
				break;
			case 0x0029:
				desc = "CDT";
				break;
			case 0x1FFF:
				desc = "NULL";
				break;
			default:
				{
					itrPID = pidName.find(itr->first);
					if(itrPID != pidName.end() ){
						desc = itrPID->second;
					}
				}
				break;
			}
			fprintf(fp.get(), "PID: 0x%04X  Total:%9I64d  Drop:%9I64d  Scramble: %9I64d  %s\r\n",
				itr->first, itr->second.total, itr->second.drop, itr->second.scramble, desc.c_str() );
		}

		string strA;
		WtoA(L"�g�pBonDriver : " + bonFile, strA);
		fprintf(fp.get(), "\r\n%s\r\n", strA.c_str());
	}
}

void CDropCount::SetPIDName(
	const map<WORD, string>* pidName_
	)
{
	map<WORD, string>::const_iterator itrIn;
	for(itrIn = pidName_->begin(); itrIn != pidName_->end(); itrIn++){
		this->pidName[itrIn->first] = itrIn->second;
	}
}
