#pragma once

#define TVTEST_PLUGIN_CLASS_IMPLEMENT	// �N���X�Ƃ��Ď���
#define TVTEST_PLUGIN_VERSION TVTEST_PLUGIN_VERSION_(0,0,13)
#include "../../Common/TVTestPlugin.h"

#include "../../Common/PipeServer.h"
#include "../../Common/ErrDef.h"
#include "../../Common/CtrlCmdDef.h"
#include "../../Common/CtrlCmdUtil.h"
#include "../../Common/SendCtrlCmd.h"

#include "StreamCtrlDlg.h"

class CEpgTimerPlugIn : public TVTest::CTVTestPlugin
{
private:
	CPipeServer pipeServer;

	BOOL nwMode;
	DWORD nwModeCurrentCtrlID;
	TVTEST_STREAMING_INFO nwModeInfo;
	CSendCtrlCmd cmd;
	BOOL fullScreen;
	BOOL showNormal;
	CStreamCtrlDlg ctrlDlg;
	CMD_STREAM cmdCapture;
	CMD_STREAM resCapture;
	CRITICAL_SECTION cmdLock;

private:
	static LRESULT CALLBACK EventCallback(UINT Event,LPARAM lParam1,LPARAM lParam2,void *pClientData);
	void CtrlCmdCallbackInvoked();
	static BOOL CALLBACK WindowMsgeCallback(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT *pResult,void *pUserData);
	static LRESULT CALLBACK StreamCtrlDlgCallback(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,void *pUserData);
	void ResetStreamingCtrlView();

public:
	CEpgTimerPlugIn();
	~CEpgTimerPlugIn();
	virtual bool GetPluginInfo(TVTest::PluginInfo *pInfo);
	virtual bool Initialize();
	virtual bool Finalize();

	void EnablePlugin(BOOL enable);
};
