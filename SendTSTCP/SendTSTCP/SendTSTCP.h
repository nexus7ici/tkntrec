#ifndef INCLUDE_SEND_TS_TCP_H
#define INCLUDE_SEND_TS_TCP_H

//DLL�̏�����
//�߂�l�F����ID�i-1�ŃG���[�j
__declspec(dllexport)
int WINAPI InitializeDLL(
	);

//DLL�̊J��
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI UnInitializeDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	);

//���M���ǉ�
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI AddSendAddrDLL(
	int iID, //[IN] InitializeDLL�̖߂�l
	LPCWSTR lpcwszIP,
	DWORD dwPort
	);

//���M��N���A
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI ClearSendAddrDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	);

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI StartSendDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	);

//�f�[�^���M���~
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI StopSendDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	);

//�f�[�^���M���J�n
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI AddSendDataDLL(
	int iID, //[IN] InitializeDLL�̖߂�l
	BYTE* pbData,
	DWORD dwSize
	);

//���M�o�b�t�@���N���A
//�߂�l�F�G���[�R�[�h
__declspec(dllexport)
DWORD WINAPI ClearSendBuffDLL(
	int iID //[IN] InitializeDLL�̖߂�l
	);


#endif
