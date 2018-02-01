#pragma once

#include "../../Common/ThreadUtil.h"

//UPnP��UDP(Port1900)������S������T�[�o
//UPnPCtrl�t�H���_�ɂ���C����x�[�X(?)�̃R�[�h��C++�ōĎ�����������
//��UPnPCtrl�t�H���_�͕s�v�̂��ߍ폜�ς݁B�K�v�Ȃ�ȑO�̃R�~�b�g���Q��
//  UPnP(DLNA)��HTTP�����╶���񏈗��Ȃǂ��قڃX�^���h�A�����Ŏ�������Ă���
class CUpnpSsdpServer
{
public:
	static const int RECV_BUFF_SIZE = 2048;
	static const unsigned int NOTIFY_INTERVAL_SEC = 1000;
	static const unsigned int NOTIFY_FIRST_DELAY_SEC = 5;
	struct SSDP_TARGET_INFO {
		string target;
		string location;
		string usn;
		bool notifyFlag;
	};
	~CUpnpSsdpServer();
	bool Start(const vector<SSDP_TARGET_INFO>& targetList_);
	void Stop();
	static string GetUserAgent();
private:
	static vector<string> GetNICList();
	static void SsdpThread(CUpnpSsdpServer* sys);
	string GetMSearchReply(const char* header, const char* host) const;
	void SendNotifyAliveOrByebye(bool byebyeFlag, const vector<string>& nicList);
	thread_ ssdpThread;
	bool stopFlag;
	vector<SSDP_TARGET_INFO> targetList;
};
