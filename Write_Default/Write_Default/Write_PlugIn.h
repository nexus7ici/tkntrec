#ifndef INCLUDE_WRITE_PLUGIN_H
#define INCLUDE_WRITE_PLUGIN_H

#include <Windows.h>

//PlugIn�̖��O���擾����
//name��NULL���͕K�v�ȃT�C�Y��nameSize�ŕԂ�
//�ʏ�nameSize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// name						[OUT]����
// nameSize					[IN/OUT]name�̃T�C�Y(WCHAR�P��)
__declspec(dllexport)
BOOL WINAPI GetPlugInName(
	WCHAR* name,
	DWORD* nameSize
	);

//PlugIn�Őݒ肪�K�v�ȏꍇ�A�ݒ�p�̃_�C�A���O�Ȃǂ�\������
//�����F
// parentWnd				[IN]�e�E�C���h�E
__declspec(dllexport)
void WINAPI Setting(
	HWND parentWnd
	);

//////////////////////////////////////////////////////////
//��{�I�ȕۑ�����API�̌Ă΂��
//CreateCtrl
//��
//StartSave
//��
//GetSaveFilePath
//��
//AddTSBuff�i���[�v�j
//���i�^�掞�ԏI������j
//StopSave
//��
//DeleteCtrl
//
//AddTSBuff��FALSE���Ԃ��Ă����ꍇ�i�󂫗e�ʂȂ��Ȃ����Ȃǁj
//AddTSBuff
//���iFALSE�j
//StopSave
//��
//StartSave
//��
//GetSaveFilePath
//��
//AddTSBuff�i���[�v�j
//���i�^�掞�ԏI������j
//StopSave
//��
//DeleteCtrl


//�����ۑ��Ή��̂��߃C���X�^���X��V�K�ɍ쐬����
//�����Ή��ł��Ȃ��ꍇ�͂��̎��_�ŃG���[�Ƃ���
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// id				[OUT]����ID
__declspec(dllexport)
BOOL WINAPI CreateCtrl(
	DWORD* id
	);

//�C���X�^���X���폜����
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// id				[IN]����ID
__declspec(dllexport)
BOOL WINAPI DeleteCtrl(
	DWORD id
	);

//�t�@�C���ۑ����J�n����
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// id					[IN]����ID
// fileName				[IN]�ۑ��t�@�C���t���p�X�i�K�v�ɉ����Ċg���q�ς�����ȂǍs���j
// overWriteFlag		[IN]����t�@�C�������ݎ��ɏ㏑�����邩�ǂ����iTRUE�F����AFALSE�F���Ȃ��j
// createSize			[IN]���͗\�z�e�ʁi188�o�C�gTS�ł̗e�ʁB�����^�掞�ȂǑ����Ԗ���̏ꍇ��0�B�����Ȃǂ̉\��������̂Ŗڈ����x�j
__declspec(dllexport)
BOOL WINAPI StartSave(
	DWORD id,
	LPCWSTR fileName,
	BOOL overWriteFlag,
	ULONGLONG createSize
	);

//�t�@�C���ۑ����I������
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// id					[IN]����ID
__declspec(dllexport)
BOOL WINAPI StopSave(
	DWORD id
	);

//���ۂɕۑ����Ă���t�@�C���p�X���擾����i�Đ���o�b�`�����ɗ��p�����j
//filePath��NULL���͕K�v�ȃT�C�Y��filePathSize�ŕԂ�
//�ʏ�filePathSize=512�ŌĂяo��
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// id					[IN]����ID
// filePath				[OUT]�ۑ��t�@�C���t���p�X
// filePathSize			[IN/OUT]filePath�̃T�C�Y(WCHAR�P��)
__declspec(dllexport)
BOOL WINAPI GetSaveFilePath(
	DWORD id,
	WCHAR* filePath,
	DWORD* filePathSize
	);

//�ۑ��pTS�f�[�^�𑗂�
//�󂫗e�ʕs���Ȃǂŏ����o�����s�����ꍇ�AwriteSize�̒l������
//�ēx�ۑ���������Ƃ��̑��M�J�n�n�_�����߂�
//�߂�l�F
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// id					[IN]����ID
// data					[IN]TS�f�[�^
// size					[IN]data�̃T�C�Y
// writeSize			[OUT]�ۑ��ɗ��p�����T�C�Y
__declspec(dllexport)
BOOL WINAPI AddTSBuff(
	DWORD id,
	BYTE* data,
	DWORD size,
	DWORD* writeSize
	);

#endif
