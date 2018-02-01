#pragma once

#include <Windows.h>
#include "EpgDataCap3Def.h"

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


//���͂��ꂽ�\��������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
//�ʏ�recNamesize=256�ŌĂяo��
//�߂�l
// TRUE�i�����j�AFALSE�i���s�j
//�����F
// info						[IN]�\����
// recName					[OUT]����
// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
typedef BOOL (WINAPI* ConvertRecNameRNP)(
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
typedef BOOL (WINAPI* ConvertRecName2RNP)(
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
typedef BOOL (WINAPI* ConvertRecName3RNP)(
	PLUGIN_RESERVE_INFO* info,
	const WCHAR* pattern,
	WCHAR* recName,
	DWORD* recNamesize
	);

class CReNamePlugInUtil
{
public:
	//���͂��ꂽ�\����ƕϊ��p�^�[�������ɁA�^�掞�̃t�@�C�������쐬����i�g���q�܂ށj
	//recName��NULL���͕K�v�ȃT�C�Y��recNamesize�ŕԂ�
	//�ʏ�recNamesize=256�ŌĂяo��
	//�v���O�C���̃o�[�W�����ɉ�����ConvertRecName3��2��1�̏��Ɍ݊��Ăяo�����s��
	//�X���b�h�Z�[�t�ł͂Ȃ�
	//�߂�l
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// info						[IN]�\����
	// dllPattern				[IN]�v���O�C��DLL���A����эŏ���'?'�ɑ����ĕϊ��p�^�[��
	// dllFolder				[IN]�v���O�C��DLL�t�H���_�p�X(dllPattern�����̂܂ܘA�������)
	// recName					[OUT]����
	// recNamesize				[IN/OUT]name�̃T�C�Y(WCHAR�P��)
	static BOOL ConvertRecName3(
		PLUGIN_RESERVE_INFO* info,
		const WCHAR* dllPattern,
		const WCHAR* dllFolder,
		WCHAR* recName,
		DWORD* recNamesize
		);
};

