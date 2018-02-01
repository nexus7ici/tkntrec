// SendTSTCP.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"

#include "../../Common/ErrDef.h"
#include "SendTSTCPMain.h"
#include "../../Common/InstanceManager.h"

CInstanceManager<CSendTSTCPMain> g_instMng;


//DLL�̏�����
//�߂�l�F����ID�i-1�ŃG���[�j
int WINAPI InitializeDLL(
	)
{
	int iID = -1;

	try{
		iID = (int)g_instMng.push(std::make_shared<CSendTSTCPMain>());
	}catch( std::bad_alloc& ){
	}

	return iID;
}


//DLL�̊J��
//�߂�l�F�G���[�R�[�h
DWORD WINAPI UnInitializeDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	)
{
	DWORD err = ERR_NOT_INIT;
	{
		std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.pop(iID);
		if( ptr != NULL ){
			err = NO_ERR;
		}
	}
	return err;
}


//���M���ǉ�
//�߂�l�F�G���[�R�[�h
DWORD WINAPI AddSendAddrDLL(
	int iID, //[IN] InitializeDLL�̖߂�l
	LPCWSTR lpcwszIP,
	DWORD dwPort
	)
{
	std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.find(iID);
	if( ptr == NULL ){
		return ERR_NOT_INIT;
	}
	DWORD dwRet = NO_ERR;
	dwRet = ptr->AddSendAddr(lpcwszIP, dwPort);
	return dwRet;
}

//���M��N���A
//�߂�l�F�G���[�R�[�h
DWORD WINAPI ClearSendAddrDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	)
{
	std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.find(iID);
	if( ptr == NULL ){
		return ERR_NOT_INIT;
	}
	DWORD dwRet = NO_ERR;
	dwRet = ptr->ClearSendAddr();
	return dwRet;
}

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
DWORD WINAPI StartSendDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	)
{
	std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.find(iID);
	if( ptr == NULL ){
		return ERR_NOT_INIT;
	}
	DWORD dwRet = NO_ERR;
	dwRet = ptr->StartSend();
	return dwRet;
}

//�f�[�^���M���~
//�߂�l�F�G���[�R�[�h
DWORD WINAPI StopSendDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	)
{
	std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.find(iID);
	if( ptr == NULL ){
		return ERR_NOT_INIT;
	}
	DWORD dwRet = NO_ERR;
	dwRet = ptr->StopSend();
	return dwRet;
}

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
DWORD WINAPI AddSendDataDLL(
	int iID, //[IN] InitializeDLL�̖߂�l
	BYTE* pbData,
	DWORD dwSize
	)
{
	std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.find(iID);
	if( ptr == NULL ){
		return ERR_NOT_INIT;
	}
	DWORD dwRet = NO_ERR;
	dwRet = ptr->AddSendData(pbData, dwSize);
	return dwRet;
}

//���M�o�b�t�@���N���A
//�߂�l�F�G���[�R�[�h
DWORD WINAPI ClearSendBuffDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	)
{
	std::shared_ptr<CSendTSTCPMain> ptr = g_instMng.find(iID);
	if( ptr == NULL ){
		return ERR_NOT_INIT;
	}
	DWORD dwRet = NO_ERR;
	dwRet = ptr->ClearSendBuff();
	return dwRet;
}
