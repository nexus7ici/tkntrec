#pragma once

#include "../Common/ThreadUtil.h"

class IBonDriver2;

class CBonDriverUtil
{
public:
	CBonDriverUtil(void);
	~CBonDriverUtil(void);

	//BonDriver�t�H���_���w��
	//�����F
	// bonDriverFolderPath		[IN]BonDriver�t�H���_�p�X
	void SetBonDriverFolder(
		LPCWSTR bonDriverFolderPath
		);

	//BonDriver�t�H���_��BonDriver_*.dll���
	//�߂�l�F
	// �����ł���BonDriver�ꗗ
	vector<wstring> EnumBonDriver();

	//BonDriver�����[�h���ă`�����l�����Ȃǂ��擾�i�t�@�C�����Ŏw��j
	//�����F
	// bonDriverFile	[IN]EnumBonDriver�Ŏ擾���ꂽBonDriver�̃t�@�C����
	// recvFunc_		[IN]�X�g���[����M���̃R�[���o�b�N�֐�
	bool OpenBonDriver(
		LPCWSTR bonDriverFile,
		void (*recvFunc_)(void*, BYTE*, DWORD, DWORD),
		void* recvParam_,
		int openWait = 200
		);

	//���[�h���Ă���BonDriver�̊J��
	void CloseBonDriver();

	//���[�h����BonDriver�̏��擾
	//Space��Ch�̈ꗗ���擾����
	//�߂�l�F
	// Space��Ch�̈ꗗ�i���X�g�̓Y���������̂܂܃`���[�i�[��Ԃ�`�����l���̔ԍ��ɂȂ�j
	// ������̓`�����l��������̂��̂��X�L�b�v����d�l�Ȃ̂ŁA���p���͂���ɏ]�����ق����ǂ���������Ȃ�
	vector<pair<wstring, vector<wstring>>> GetOriginalChList();

	//BonDriver�̃`���[�i�[�����擾
	//�߂�l�F
	// �`���[�i�[��
	wstring GetTunerName();

	//�`�����l���ύX
	//�����F
	// space			[IN]�ύX�`�����l����Space
	// ch				[IN]�ύX�`�����l���̕���Ch
	bool SetCh(
		DWORD space,
		DWORD ch
		);

	//���݂̃`�����l���擾
	//�����F
	// space			[IN]���݂̃`�����l����Space
	// ch				[IN]���݂̃`�����l���̕���Ch
	bool GetNowCh(
		DWORD* space,
		DWORD* ch
		);

	//�V�O�i�����x���̎擾
	//�߂�l�F
	// �V�O�i�����x��
	float GetSignalLevel();

	//Open����BonDriver�̃t�@�C�������擾
	//�߂�l�F
	// BonDriver�̃t�@�C�����i�g���q�܂ށj�iempty�Ŗ�Open�j
	wstring GetOpenBonDriverFileName();

private:
	//BonDriver�ɃA�N�Z�X���郏�[�J�[�X���b�h
	static void DriverThread(CBonDriverUtil* sys);
	//���[�J�[�X���b�h�̃��b�Z�[�W��p�E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK DriverWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static class CInit { public: CInit(); } s_init;
	recursive_mutex_ utilLock;
	wstring loadDllFolder;
	wstring loadDllFileName;
	wstring loadTunerName;
	vector<pair<wstring, vector<wstring>>> loadChList;
	bool initChSetFlag;
	void (*recvFunc)(void*, BYTE*, DWORD, DWORD);
	void* recvParam;
	IBonDriver2* bon2IF;
	thread_ driverThread;
	HWND hwndDriver;
};

