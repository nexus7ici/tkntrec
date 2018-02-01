#ifndef INCLUDE_REC_NAME_PLUGIN_H
#define INCLUDE_REC_NAME_PLUGIN_H

#include <Windows.h>
#include "../../Common/EpgDataCap3Def.h"

typedef struct {
	SYSTEMTIME startTime;		//�J�n����
	DWORD durationSec;			//�����ԁi�P�ʕb�j
	WCHAR eventName[512];		//�ԑg��
	WORD ONID;					//originai_network_id
	WORD TSID;					//transport_stream_id
	WORD SID;					//service_id
	WORD EventID;				//evend_id�i�v���O�����\�񈵂����A0xFFFF�j
	WCHAR serviceName[256];		//�T�[�r�X��
	WCHAR bonDriverName[256];	//�g�pBonDriver�t�@�C����
	DWORD bonDriverID;			//EpgTimerSrv�����ł�BonDriver����ID
	DWORD tunerID;				//EpgTimerSrv�����ł̃`���[�i�[����ID
	DWORD reserveID;			//�\��ID�iConvertRecName3�ŕK�{�j
	EPG_EVENT_INFO* epgInfo;	//�ԑg���i���݂��Ȃ��Ƃ�NULL�j�iConvertRecName3�ŕK�{�j
	DWORD sizeOfStruct;			//���g�p�i0�܂��͍\���̃T�C�Y�ŏ������j�iConvertRecName3�ŕK�{�j
}PLUGIN_RESERVE_INFO;

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

//���͂��ꂽ�\��������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
//�ʏ�recNamesize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// info						[IN]�\����
// recName					[OUT]����
// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
__declspec(dllexport)
BOOL WINAPI ConvertRecName(
	PLUGIN_RESERVE_INFO* info,
	WCHAR* recName,
	DWORD* recNamesize
	);

//���͂��ꂽ�\��������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
//�ʏ�recNamesize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// info						[IN]�\����
// epgInfo					[IN]�ԑg���iEPG�\��Ŕԑg��񂪑��݂��鎞�A���݂��Ȃ��ꍇ��NULL�j
// recName					[OUT]����
// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
__declspec(dllexport)
BOOL WINAPI ConvertRecName2(
	PLUGIN_RESERVE_INFO* info,
	EPG_EVENT_INFO* epgInfo,
	WCHAR* recName,
	DWORD* recNamesize
	);

//���͂��ꂽ�\����ƕϊ��p�^�[�������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
//�ʏ�recNamesize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// info						[IN]�\����
// pattern					[IN]�ϊ��p�^�[���i�f�t�H���g�̂Ƃ�NULL�j
// recName					[OUT]����
// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
__declspec(dllexport)
BOOL WINAPI ConvertRecName3(
	PLUGIN_RESERVE_INFO* info,
	const WCHAR* pattern,
	WCHAR* recName,
	DWORD* recNamesize
	);

#endif
