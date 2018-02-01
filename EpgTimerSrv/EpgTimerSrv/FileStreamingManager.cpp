#include "stdafx.h"
#include "FileStreamingManager.h"


void CFileStreamingManager::CloseAllFile(
	)
{
	this->utilMng.clear();
}

BOOL CFileStreamingManager::IsStreaming()
{
	return this->utilMng.empty() == false;
}

BOOL CFileStreamingManager::OpenTimeShift(
	LPCWSTR filePath,
	DWORD* ctrlID
	)
{
	std::shared_ptr<CTimeShiftUtil> util = std::make_shared<CTimeShiftUtil>();
	if( util->OpenTimeShift(filePath, FALSE) == FALSE ){
		return FALSE;
	}
	*ctrlID = this->utilMng.push(util);
	return TRUE;
}

BOOL CFileStreamingManager::OpenFile(
	LPCWSTR filePath,
	DWORD* ctrlID
	)
{
	std::shared_ptr<CTimeShiftUtil> util = std::make_shared<CTimeShiftUtil>();
	if( util->OpenTimeShift(filePath, TRUE) == FALSE ){
		return FALSE;
	}
	*ctrlID = this->utilMng.push(util);
	return TRUE;
}

BOOL CFileStreamingManager::CloseFile(
	DWORD ctrlID
	)
{
	std::shared_ptr<CTimeShiftUtil> util = this->utilMng.pop(ctrlID);
	return util != NULL;
}

BOOL CFileStreamingManager::StartSend(
	DWORD ctrlID
	)
{
	std::shared_ptr<CTimeShiftUtil> util = this->utilMng.find(ctrlID);
	if( util == NULL ){
		return FALSE;
	}
	return util->StartTimeShift();
}

BOOL CFileStreamingManager::StopSend(
	DWORD ctrlID
	)
{
	std::shared_ptr<CTimeShiftUtil> util = this->utilMng.find(ctrlID);
	if( util == NULL ){
		return FALSE;
	}
	return util->StopTimeShift();
}

//�X�g���[���z�M�Ō��݂̑��M�ʒu�Ƒ��t�@�C���T�C�Y���擾����
//�߂�l�F
// �G���[�R�[�h
//�����F
// val				[IN/OUT]�T�C�Y���
BOOL CFileStreamingManager::GetPos(
	NWPLAY_POS_CMD* val
	)
{
	std::shared_ptr<CTimeShiftUtil> util = this->utilMng.find(val->ctrlID);
	if( util == NULL ){
		return FALSE;
	}
	util->GetFilePos(&val->currentPos, &val->totalPos);
	return TRUE;
}

//�X�g���[���z�M�ő��M�ʒu���V�[�N����
//�߂�l�F
// �G���[�R�[�h
//�����F
// val				[IN]�T�C�Y���
BOOL CFileStreamingManager::SetPos(
	NWPLAY_POS_CMD* val
	)
{
	std::shared_ptr<CTimeShiftUtil> util = this->utilMng.find(val->ctrlID);
	if( util == NULL ){
		return FALSE;
	}
	util->SetFilePos(val->currentPos);
	return TRUE;
}

//�X�g���[���z�M�ő��M���ݒ肷��
//�߂�l�F
// �G���[�R�[�h
//�����F
// val				[IN]�T�C�Y���
BOOL CFileStreamingManager::SetIP(
	NWPLAY_PLAY_INFO* val
	)
{
	std::shared_ptr<CTimeShiftUtil> util = this->utilMng.find(val->ctrlID);
	if( util == NULL ){
		return FALSE;
	}
	return util->Send(val);
}
