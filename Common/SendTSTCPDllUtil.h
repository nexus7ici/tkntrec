#ifndef INCLUDE_SEND_TS_TCP_DLL_UTIL_H
#define INCLUDE_SEND_TS_TCP_DLL_UTIL_H

#include "ErrDef.h"

class CSendTSTCPDllUtil
{
public:
	CSendTSTCPDllUtil(void);
	~CSendTSTCPDllUtil(void);

	//DLL�̏�����
	//�߂�l�F�G���[�R�[�h
	DWORD Initialize(
		);

	//DLL�̊J��
	//�߂�l�F�Ȃ�
	void UnInitialize(
		);

	//���M���ǉ�
	//�߂�l�F�G���[�R�[�h
	DWORD AddSendAddr(
		LPCWSTR lpcwszIP,
		DWORD dwPort
		);

	//���M��N���A
	//�߂�l�F�G���[�R�[�h
	DWORD ClearSendAddr(
		);

	//�f�[�^���M���J�n
	//�߂�l�F�G���[�R�[�h
	DWORD StartSend(
		);

	//�f�[�^���M���~
	//�߂�l�F�G���[�R�[�h
	DWORD StopSend(
		);

	//�f�[�^���M���J�n
	//�߂�l�F�G���[�R�[�h
	DWORD AddSendData(
		BYTE* pbData,
		DWORD dwSize
		);

	//���M�o�b�t�@���N���A
	//�߂�l�F�G���[�R�[�h
	DWORD ClearSendBuff(
		);
		
protected:
	typedef int (WINAPI *InitializeDLL)();
	typedef DWORD (WINAPI *UnInitializeDLL)(int iID);
	typedef DWORD (WINAPI *AddSendAddrDLL)(int iID, LPCWSTR lpcwszIP, DWORD dwPort);
	typedef DWORD (WINAPI *ClearSendAddrDLL)(int iID);
	typedef DWORD (WINAPI *StartSendDLL)(int iID);
	typedef DWORD (WINAPI *StopSendDLL)(int iID);
	typedef DWORD (WINAPI *AddSendDataDLL)(int iID, BYTE* pbData, DWORD dwSize);
	typedef DWORD (WINAPI *ClearSendBuffDLL)(int iID);

	HMODULE m_hModule;
	int m_iID;

	InitializeDLL pfnInitializeDLL;
	UnInitializeDLL pfnUnInitializeDLL;
	AddSendAddrDLL pfnAddSendAddrDLL;
	ClearSendAddrDLL pfnClearSendAddrDLL;
	StartSendDLL pfnStartSendDLL;
	StopSendDLL pfnStopSendDLL;
	AddSendDataDLL pfnAddSendDataDLL;
	ClearSendBuffDLL pfnClearSendBuffDLL;

protected:
	BOOL LoadDll(void);
	void UnLoadDll(void);
};

#endif
