#include "stdafx.h"
#include "PacketInit.h"

CPacketInit::CPacketInit(void)
{
	this->packetSize = 0;
}

void CPacketInit::ClearBuff()
{
	this->nextStartBuff.clear();
	this->packetSize = 0;
}

//���̓o�b�t�@��188�o�C�g�P�ʂ�TS�ɕϊ����A188�̔{���ɂȂ�悤�ɂ��낦��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// inData			[IN]����TS�f�[�^
// inSize			[IN]inData�̃T�C�Y�iBYTE�P�ʁj
// outData			[OUT]188�o�C�g�ɐ��񂵂��o�b�t�@�i����Ăяo���܂ŕێ��j
// outSize			[OUT]outData�̃T�C�Y�iBYTE�P�ʁj
BOOL CPacketInit::GetTSData(
	const BYTE* inData,
	DWORD inSize,
	BYTE** outData,
	DWORD* outSize
)
{
	if( inData == NULL || inSize == 0 || outData == NULL || outSize == NULL ){
		return FALSE;
	}

	if( this->packetSize != 0 ){
		//�����ς�
		for( size_t i = (this->packetSize - this->nextStartBuff.size()) % this->packetSize; i < inSize; i += this->packetSize ){
			if( inData[i] != 0x47 ){
				//�ē������K�v
				this->packetSize = 0;
				break;
			}
		}
		if( this->packetSize != 0 ){
			if( this->nextStartBuff.size() + inSize < this->packetSize ){
				this->outBuff.resize(1);
				*outSize = 0;
				//�J��z������
				this->nextStartBuff.insert(this->nextStartBuff.end(), inData, inData + inSize);
			}else{
				if( this->nextStartBuff.size() >= 188 ){
					this->outBuff.assign(this->nextStartBuff.begin(), this->nextStartBuff.begin() + 188);
				}else{
					this->outBuff.assign(this->nextStartBuff.begin(), this->nextStartBuff.end());
					this->outBuff.insert(this->outBuff.end(), inData, inData + (188 - this->nextStartBuff.size()));
				}
				size_t inPos = this->packetSize - this->nextStartBuff.size();
				for( ; inPos + this->packetSize <= inSize; inPos += this->packetSize ){
					this->outBuff.insert(this->outBuff.end(), inData + inPos, inData + inPos + 188);
				}
				*outSize = (DWORD)this->outBuff.size();
				//�J��z��
				this->nextStartBuff.assign(inData + inPos, inData + inSize);
			}
			*outData = &this->outBuff.front();
			return TRUE;
		}
	}

	DWORD nss = (DWORD)this->nextStartBuff.size();

	for( DWORD pos = 0; pos + 188 < nss + inSize; pos++ ){
		if( pos < nss && this->nextStartBuff[pos] == 0x47 || pos >= nss && inData[pos - nss] == 0x47 ){
			for( int i = 0; i < 3; i++ ){
				this->packetSize = (i == 0 ? 188 : i == 1 ? 192 : 204);
				BOOL syncOK = FALSE;
				for( DWORD j = pos + this->packetSize; j < nss + inSize; j += this->packetSize ){
					if( j < nss && this->nextStartBuff[j] != 0x47 || j >= nss && inData[j - nss] != 0x47 ){
						syncOK = FALSE;
						break;
					}
					syncOK = TRUE;
				}
				if( syncOK == FALSE ){
					this->packetSize = 0;
				}else if( pos < nss ){
					this->nextStartBuff.erase(this->nextStartBuff.begin(), this->nextStartBuff.begin() + pos);
					//�����ς݂̂Ƃ��̌J��z���T�C�Y�̓p�P�b�g�T�C�Y�����łȂ���΂Ȃ�Ȃ�
					if( this->nextStartBuff.size() >= this->packetSize ){
						this->nextStartBuff.erase(this->nextStartBuff.begin(), this->nextStartBuff.begin() + this->packetSize);
					}
					return GetTSData(inData, inSize, outData, outSize);
				}else{
					this->nextStartBuff.clear();
					return GetTSData(inData + (pos - nss), inSize - (pos - nss), outData, outSize);
				}
			}
		}
	}

	//�ē����Ɏ��s�B256�o�C�g�ȉ��ŉ\�Ȃ����J��z���Ă���
	if( inSize >= 256 ){
		this->nextStartBuff.assign(inData + inSize - 256, inData + inSize);
	}else{
		if( this->nextStartBuff.size() + inSize > 256 ){
			this->nextStartBuff.erase(this->nextStartBuff.begin(), this->nextStartBuff.end() - (256 - inSize));
		}
		this->nextStartBuff.insert(this->nextStartBuff.end(), inData, inData + inSize);
	}
	return FALSE;
}
