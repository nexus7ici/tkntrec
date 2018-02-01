#pragma once

#include "../Common/StructDef.h"
#include "../Common/ErrDef.h"
#include "../Common/StringUtil.h"
#include "../Common/ThreadUtil.h"
#include "../Common/WritePlugInUtil.h"
#include <list>

class CWriteTSFile
{
public:
	CWriteTSFile(void);
	~CWriteTSFile(void);

	//�t�@�C���ۑ����J�n����
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// fileName				[IN]�ۑ��t�@�C���p�X
	// overWriteFlag		[IN]����t�@�C�������ݎ��ɏ㏑�����邩�ǂ����iTRUE�F����AFALSE�F���Ȃ��j
	// createSize			[IN]�t�@�C���쐬���Ƀf�B�X�N�ɗ\�񂷂�e��
	// saveFolder			[IN]�g�p����t�H���_�ꗗ
	// saveFolderSub		[IN]HDD�̋󂫂��Ȃ��Ȃ����ꍇ�Ɉꎞ�I�Ɏg�p����t�H���_
	BOOL StartSave(
		const wstring& fileName,
		BOOL overWriteFlag_,
		ULONGLONG createSize_,
		const vector<REC_FILE_SET_INFO>& saveFolder,
		const vector<wstring>& saveFolderSub_,
		int maxBuffCount_
	);

	//�t�@�C���ۑ����I������
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	BOOL EndSave();

	//�o�͗pTS�f�[�^�𑗂�
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// data		[IN]TS�f�[�^
	// size		[IN]data�̃T�C�Y
	BOOL AddTSBuff(
		BYTE* data,
		DWORD size
		);

	//�^�撆�̃t�@�C���̃t�@�C���p�X���擾����
	//�����F
	// filePath			[OUT]�ۑ��t�@�C����
	// subRecFlag		[OUT]�T�u�^�悪�����������ǂ���
	void GetSaveFilePath(
		wstring* filePath,
		BOOL* subRecFlag_
		);

	//�^�撆�̃t�@�C���̏o�̓T�C�Y���擾����
	//�����F
	// writeSize			[OUT]�ۑ��t�@�C����
	void GetRecWriteSize(
		__int64* writeSize
		);

protected:
	//�ۑ��T�u�t�H���_����󂫂̂���t�H���_�p�X���擾
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// needFreeSize			[IN]�Œ�K�v�ȋ󂫃T�C�Y
	// freeFolderPath		[OUT]���������t�H���_
	BOOL GetFreeFolder(
		ULONGLONG needFreeSize,
		wstring& freeFolderPath
	);

	//�w��t�H���_�̋󂫂����邩�`�F�b�N
	//�߂�l�F
	// TRUE�i�����j�AFALSE�i���s�j
	//�����F
	// needFreeSize			[IN]�Œ�K�v�ȋ󂫃T�C�Y
	// chkFolderPath		[IN]�w��t�H���_
	BOOL ChkFreeFolder(
		ULONGLONG needFreeSize,
		const wstring& chkFolderPath
	);

	static void OutThread(CWriteTSFile* sys);

protected:
	recursive_mutex_ outThreadLock;
	std::list<vector<BYTE>> tsBuffList;

	thread_ outThread;
	BOOL outStopFlag;
	BOOL outStartFlag;

	struct SAVE_INFO {
		CWritePlugInUtil writeUtil;
		BOOL freeChk;
		wstring writePlugIn;
		wstring recFolder;
		wstring recFileName;
	};
	vector<std::unique_ptr<SAVE_INFO>> fileList;

	BOOL overWriteFlag;
	ULONGLONG createSize;
	vector<wstring> saveFolderSub;

	BOOL subRecFlag;
	__int64 writeTotalSize;
	wstring mainSaveFilePath;

	int maxBuffCount;
};

