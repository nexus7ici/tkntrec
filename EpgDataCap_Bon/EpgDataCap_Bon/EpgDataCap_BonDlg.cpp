
// EpgDataCap_BonDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "EpgDataCap_Bon.h"
#include "EpgDataCap_BonDlg.h"

#include "../../Common/TimeUtil.h"


// CEpgDataCap_BonDlg �_�C�A���O


UINT CEpgDataCap_BonDlg::taskbarCreated = 0;
BOOL CEpgDataCap_BonDlg::disableKeyboardHook = FALSE;

CEpgDataCap_BonDlg::CEpgDataCap_BonDlg()
	: m_hWnd(NULL)
	, m_hKeyboardHook(NULL)
{
	HMODULE hModule = GetModuleHandle(NULL);
	HRESULT (WINAPI* pfnLoadIconMetric)(HINSTANCE,PCWSTR,int,HICON*) =
		(HRESULT (WINAPI*)(HINSTANCE,PCWSTR,int,HICON*))GetProcAddress(GetModuleHandle(L"comctl32.dll"), "LoadIconMetric");
	if( pfnLoadIconMetric == NULL ||
	    pfnLoadIconMetric(hModule, MAKEINTRESOURCE(IDI_ICON_BLUE), LIM_SMALL, &m_hIcon) != S_OK ||
	    pfnLoadIconMetric(hModule, MAKEINTRESOURCE(IDI_ICON_BLUE), LIM_LARGE, &m_hIcon2) != S_OK ||
	    pfnLoadIconMetric(hModule, MAKEINTRESOURCE(IDI_ICON_RED), LIM_SMALL, &iconRed) != S_OK ||
	    pfnLoadIconMetric(hModule, MAKEINTRESOURCE(IDI_ICON_GREEN), LIM_SMALL, &iconGreen) != S_OK ||
	    pfnLoadIconMetric(hModule, MAKEINTRESOURCE(IDI_ICON_GRAY), LIM_SMALL, &iconGray) != S_OK ){
		m_hIcon = (HICON)LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON_BLUE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		m_hIcon2 = (HICON)LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON_BLUE), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
		iconRed = (HICON)LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON_RED), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		iconGreen = (HICON)LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON_GREEN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		iconGray = (HICON)LoadImage(hModule, MAKEINTRESOURCE(IDI_ICON_GRAY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	}
	iconBlue = m_hIcon;

	taskbarCreated = RegisterWindowMessage(L"TaskbarCreated");

	this->moduleIniPath = GetModuleIniPath().native();

	this->initONID = GetPrivateProfileInt( L"Set", L"LastONID", -1, this->moduleIniPath.c_str() );
	this->initTSID = GetPrivateProfileInt( L"Set", L"LastTSID", -1, this->moduleIniPath.c_str() );
	this->initSID = GetPrivateProfileInt( L"Set", L"LastSID", -1, this->moduleIniPath.c_str() );
	this->iniBonDriver = GetPrivateProfileToString( L"Set", L"LastBon", L"", this->moduleIniPath.c_str() );

	iniView = FALSE;
	iniNetwork = TRUE;
	iniMin = FALSE;
	this->iniUDP = FALSE;
	this->iniTCP = FALSE;
	
	this->minTask = GetPrivateProfileInt( L"Set", L"MinTask", 0, this->moduleIniPath.c_str() );
	this->openLastCh = GetPrivateProfileInt( L"Set", L"OpenLast", 1, this->moduleIniPath.c_str() );
	if( this->openLastCh == 0 ){
		if( GetPrivateProfileInt( L"Set", L"OpenFix", 0, this->moduleIniPath.c_str() ) == 1){
			this->initONID = GetPrivateProfileInt( L"Set", L"FixONID", -1, this->moduleIniPath.c_str() );
			this->initTSID = GetPrivateProfileInt( L"Set", L"FixTSID", -1, this->moduleIniPath.c_str() );
			this->initSID = GetPrivateProfileInt( L"Set", L"FixSID", -1, this->moduleIniPath.c_str() );
			this->iniBonDriver = GetPrivateProfileToString( L"Set", L"FixBon", L"", this->moduleIniPath.c_str() );
		}else{
			this->initONID = -1;
			this->initTSID = -1;
			this->initSID = -1;
			this->iniBonDriver = L"";
		}
	}
	this->initOpenWait = 0;
	this->initChgWait = 0;
}

INT_PTR CEpgDataCap_BonDlg::DoModal()
{
	return DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD), NULL, DlgProc, (LPARAM)this);
}


// CEpgDataCap_BonDlg ���b�Z�[�W �n���h���[
void CEpgDataCap_BonDlg::SetInitBon(LPCWSTR bonFile)
{
	iniBonDriver = bonFile;
	if( GetPrivateProfileInt( iniBonDriver.c_str(), L"OpenFix", 0, this->moduleIniPath.c_str() ) == 1){
		OutputDebugString(L"�����T�[�r�X�w�� �ݒ�l���[�h");
		this->initONID = GetPrivateProfileInt( iniBonDriver.c_str(), L"FixONID", -1, this->moduleIniPath.c_str() );
		this->initTSID = GetPrivateProfileInt( iniBonDriver.c_str(), L"FixTSID", -1, this->moduleIniPath.c_str() );
		this->initSID = GetPrivateProfileInt( iniBonDriver.c_str(), L"FixSID", -1, this->moduleIniPath.c_str() );
		this->initOpenWait = GetPrivateProfileInt( iniBonDriver.c_str(), L"OpenWait", 0, this->moduleIniPath.c_str() );
		this->initChgWait = GetPrivateProfileInt( iniBonDriver.c_str(), L"ChgWait", 0, this->moduleIniPath.c_str() );
		_OutputDebugString(L"%d,%d,%d,%d,%d",initONID,initTSID,initSID,initOpenWait,initChgWait );
	}
}

BOOL CEpgDataCap_BonDlg::OnInitDialog()
{
	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
	SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)m_hIcon2);	// �傫���A�C�R���̐ݒ�
	SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);	// �������A�C�R���̐ݒ�

	// TODO: �������������ɒǉ����܂��B
	this->main.ReloadSetting();

	for( int i=0; i<24; i++ ){
		WCHAR buff[32];
		swprintf_s(buff, L"%d", i);
		int index = ComboBox_AddString(GetDlgItem(IDC_COMBO_REC_H), buff);
		ComboBox_SetItemData(GetDlgItem(IDC_COMBO_REC_H), index, i);
	}
	ComboBox_SetCurSel(GetDlgItem(IDC_COMBO_REC_H), 0);

	for( int i=0; i<60; i++ ){
		WCHAR buff[32];
		swprintf_s(buff, L"%d", i);
		int index = ComboBox_AddString(GetDlgItem(IDC_COMBO_REC_M), buff);
		ComboBox_SetItemData(GetDlgItem(IDC_COMBO_REC_M), index, i);
	}
	ComboBox_SetCurSel(GetDlgItem(IDC_COMBO_REC_M), 0);

	//BonDriver�̈ꗗ�擾
	ReloadBonDriver();

	//BonDriver�̃I�[�v��
	DWORD err = NO_ERR;
	if( this->iniBonDriver.empty() == false ){
		err = SelectBonDriver(this->iniBonDriver.c_str(), TRUE);
		Sleep(this->initOpenWait);
	}else{
		if( this->bonList.empty() == false ){
			err = SelectBonDriver(this->bonList.front().c_str());
		}else{
			err = ERR_FALSE;
			WCHAR log[512 + 64] = L"";
			GetDlgItemText(m_hWnd, IDC_EDIT_LOG, log, 512);
			wcscat_s(log, L"BonDriver��������܂���ł���\r\n");
			SetDlgItemText(m_hWnd, IDC_EDIT_LOG, log);
		}
	}

	if( err == NO_ERR ){
		//�`�����l���ύX
		if( this->initONID != -1 && this->initTSID != -1 && this->initSID != -1 ){
			SelectService((WORD)this->initONID, (WORD)this->initTSID, (WORD)this->initSID);
			this->initONID = -1;
			this->initTSID = -1;
			this->initSID = -1;
			Sleep(this->initChgWait);
		}else{
			int sel = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_SERVICE));
			if( sel != CB_ERR ){
				DWORD index = (DWORD)ComboBox_GetItemData(GetDlgItem(IDC_COMBO_SERVICE), sel);
				SelectService(this->serviceList[index].originalNetworkID, this->serviceList[index].transportStreamID, this->serviceList[index].serviceID, this->serviceList[index].space, this->serviceList[index].ch );
			}
		}
	}

	//�E�C���h�E�̕���
	WINDOWPLACEMENT Pos;
	Pos.length = sizeof(WINDOWPLACEMENT);
	int left = GetPrivateProfileInt(L"SET_WINDOW", L"left", INT_MAX, this->moduleIniPath.c_str());
	int top = GetPrivateProfileInt(L"SET_WINDOW", L"top", INT_MAX, this->moduleIniPath.c_str());
	if( left != INT_MAX && top != INT_MAX && GetWindowPlacement(m_hWnd, &Pos) ){
		Pos.flags = 0;
		Pos.showCmd = this->iniMin ? SW_SHOWMINNOACTIVE : SW_SHOW;
		int width = GetPrivateProfileInt(L"SET_WINDOW", L"width", 0, this->moduleIniPath.c_str());
		int height = GetPrivateProfileInt(L"SET_WINDOW", L"height", 0, this->moduleIniPath.c_str());
		if( width > 0 && height > 0 ){
			Pos.rcNormalPosition.right = left + width;
			Pos.rcNormalPosition.bottom = top + height;
		}else{
			Pos.rcNormalPosition.right += left - Pos.rcNormalPosition.left;
			Pos.rcNormalPosition.bottom += top - Pos.rcNormalPosition.top;
		}
		Pos.rcNormalPosition.left = left;
		Pos.rcNormalPosition.top = top;
		SetWindowPlacement(m_hWnd, &Pos);
	}
	SetTimer(TIMER_STATUS_UPDATE, 1000, NULL);
	SetTimer(TIMER_INIT_DLG, 1, NULL);
	this->main.SetHwnd(GetSafeHwnd());

	if( this->iniNetwork == TRUE ){
		if( this->iniUDP == TRUE || this->iniTCP == TRUE ){
			if( this->iniUDP == TRUE ){
				Button_SetCheck(GetDlgItem(IDC_CHECK_UDP), BST_CHECKED);
			}
			if( this->iniTCP == TRUE ){
				Button_SetCheck(GetDlgItem(IDC_CHECK_TCP), BST_CHECKED);
			}
		}else{
			Button_SetCheck(GetDlgItem(IDC_CHECK_UDP), GetPrivateProfileInt(L"SET", L"ChkUDP", 0, this->moduleIniPath.c_str()));
			Button_SetCheck(GetDlgItem(IDC_CHECK_TCP), GetPrivateProfileInt(L"SET", L"ChkTCP", 0, this->moduleIniPath.c_str()));
		}
	}

	ReloadNWSet();

	this->main.StartServer();

	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}


void CEpgDataCap_BonDlg::OnSysCommand(UINT nID, LPARAM lParam, BOOL* pbProcessed)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h���[ �R�[�h��ǉ����邩�A����̏������Ăяo���܂��B
	if( nID == SC_CLOSE ){
		if( this->main.IsRec() == TRUE ){
			WCHAR caption[128] = L"";
			GetWindowText(m_hWnd, caption, 128);
			disableKeyboardHook = TRUE;
			int result = MessageBox( m_hWnd, L"�^�撆�ł����I�����܂����H", caption, MB_YESNO | MB_ICONQUESTION );
			disableKeyboardHook = FALSE;
			if( result == IDNO ){
				*pbProcessed = TRUE;
				return ;
			}
			this->main.StopReserveRec();
			this->main.StopRec();
		}
	}
}


void CEpgDataCap_BonDlg::OnDestroy()
{
	this->main.StopServer();
	this->main.CloseBonDriver();
	KillTimer(TIMER_STATUS_UPDATE);

	KillTimer(RETRY_ADD_TRAY);
	DeleteTaskBar(GetSafeHwnd(), TRAYICON_ID);

	WINDOWPLACEMENT Pos;
	Pos.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hWnd, &Pos);

	WritePrivateProfileInt(L"SET_WINDOW", L"top", Pos.rcNormalPosition.top, this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET_WINDOW", L"left", Pos.rcNormalPosition.left, this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET_WINDOW", L"bottom", Pos.rcNormalPosition.bottom, this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET_WINDOW", L"right", Pos.rcNormalPosition.right, this->moduleIniPath.c_str());

	int selONID = -1;
	int selTSID = -1;
	int selSID = -1;
	WCHAR bon[512] = L"";

	GetWindowText(GetDlgItem(IDC_COMBO_TUNER), bon, 512);
	int sel = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_SERVICE));
	if( sel != CB_ERR ){
		DWORD index = (DWORD)ComboBox_GetItemData(GetDlgItem(IDC_COMBO_SERVICE), sel);
		selONID = this->serviceList[index].originalNetworkID;
		selTSID = this->serviceList[index].transportStreamID;
		selSID = this->serviceList[index].serviceID;
	}

	WritePrivateProfileInt(L"SET", L"LastONID", selONID, this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET", L"LastTSID", selTSID, this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET", L"LastSID", selSID, this->moduleIniPath.c_str());
	WritePrivateProfileString(L"SET", L"LastBon", bon, this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET", L"ChkUDP", Button_GetCheck(GetDlgItem(IDC_CHECK_UDP)), this->moduleIniPath.c_str());
	WritePrivateProfileInt(L"SET", L"ChkTCP", Button_GetCheck(GetDlgItem(IDC_CHECK_TCP)), this->moduleIniPath.c_str());

	// TODO: �����Ƀ��b�Z�[�W �n���h���[ �R�[�h��ǉ����܂��B
}


void CEpgDataCap_BonDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h���[ �R�[�h��ǉ����邩�A����̏������Ăяo���܂��B
	switch(nIDEvent){
		case TIMER_INIT_DLG:
			{
				KillTimer( TIMER_INIT_DLG );
				if( this->iniMin == TRUE && this->minTask == TRUE){
				    ShowWindow(m_hWnd, SW_HIDE);
				}
			}
			break;
		case TIMER_STATUS_UPDATE:
			{
				SetThreadExecutionState(ES_SYSTEM_REQUIRED);

				int iLine = Edit_GetFirstVisibleLine(GetDlgItem(IDC_EDIT_STATUS));
				float signal = 0;
				DWORD space = 0;
				DWORD ch = 0;
				ULONGLONG drop = 0;
				ULONGLONG scramble = 0;
				vector<NW_SEND_INFO> udpSendList = this->main.GetSendUDPList();
				vector<NW_SEND_INFO> tcpSendList = this->main.GetSendTCPList();

				BOOL ret = this->main.GetViewStatusInfo(&signal, &space, &ch, &drop, &scramble);

				wstring statusLog = L"";
				if(ret==TRUE){
					Format(statusLog, L"Signal: %.02f Drop: %I64d Scramble: %I64d  space: %d ch: %d",signal, drop, scramble, space, ch);
				}else{
					Format(statusLog, L"Signal: %.02f Drop: %I64d Scramble: %I64d",signal, drop, scramble);
				}
				statusLog += L"\r\n";

				wstring udp = L"";
				if( udpSendList.size() > 0 ){
					udp = L"UDP���M�F";
					for( size_t i=0; i<udpSendList.size(); i++ ){
						wstring buff;
						Format(buff, L":%d%s ", udpSendList[i].port, udpSendList[i].broadcastFlag ? L"(Broadcast)" : L"");
						udp += udpSendList[i].ipString.find(L':') == wstring::npos ? udpSendList[i].ipString : L'[' + udpSendList[i].ipString + L']';
						udp += buff;
					}
					udp += L"\r\n";
				}
				statusLog += udp;

				wstring tcp = L"";
				if( tcpSendList.size() > 0 ){
					tcp = L"TCP���M�F";
					for( size_t i=0; i<tcpSendList.size(); i++ ){
						wstring buff;
						Format(buff, L":%d ", tcpSendList[i].port);
						tcp += tcpSendList[i].ipString.find(L':') == wstring::npos ? tcpSendList[i].ipString : L'[' + tcpSendList[i].ipString + L']';
						tcp += buff;
					}
					tcp += L"\r\n";
				}
				statusLog += tcp;

				SetDlgItemText(m_hWnd, IDC_EDIT_STATUS, statusLog.c_str());
				Edit_Scroll(GetDlgItem(IDC_EDIT_STATUS), iLine, 0);

				wstring info = L"";
				this->main.GetEpgInfo(Button_GetCheck(GetDlgItem(IDC_CHECK_NEXTPG)), &info);
				WCHAR pgInfo[512] = L"";
				GetDlgItemText(m_hWnd, IDC_EDIT_PG_INFO, pgInfo, 512);
				if( info.substr(0, 511).compare(pgInfo) != 0 ){
					SetDlgItemText(m_hWnd, IDC_EDIT_PG_INFO, info.c_str());
				}
			}
			break;
		case TIMER_CHSCAN_STATSU:
			{
				DWORD space = 0;
				DWORD ch = 0;
				wstring chName = L"";
				DWORD chkNum = 0;
				DWORD totalNum = 0;
				CBonCtrl::JOB_STATUS status = this->main.GetChScanStatus(&space, &ch, &chName, &chkNum, &totalNum);
				if( status == CBonCtrl::ST_WORKING ){
					wstring log;
					Format(log, L"%s (%d/%d �c��� %d �b)\r\n", chName.c_str(), chkNum, totalNum, (totalNum - chkNum)*10);
					SetDlgItemText(m_hWnd, IDC_EDIT_LOG, log.c_str());
				}else if( status == CBonCtrl::ST_CANCEL ){
					KillTimer(TIMER_CHSCAN_STATSU);
					SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�L�����Z������܂���\r\n");
				}else if( status == CBonCtrl::ST_COMPLETE ){
					KillTimer(TIMER_CHSCAN_STATSU);
					SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�I�����܂���\r\n");
					ReloadServiceList();
					int sel = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_SERVICE));
					if( sel != CB_ERR ){
						DWORD index = (DWORD)ComboBox_GetItemData(GetDlgItem(IDC_COMBO_SERVICE), sel);
						SelectService(this->serviceList[index].originalNetworkID, this->serviceList[index].transportStreamID, this->serviceList[index].serviceID, this->serviceList[index].space, this->serviceList[index].ch );
					}
					BtnUpdate(GUI_NORMAL);
					ChgIconStatus();

					//�����T�[�r�X���ʂ̕����`�����l���ɂ��邩�`�F�b�N
					wstring msg = L"";
					for( size_t i=0; i<this->serviceList.size(); i++ ){
						for( size_t j=i+1; j<this->serviceList.size(); j++ ){
							if( this->serviceList[i].originalNetworkID == this->serviceList[j].originalNetworkID &&
								this->serviceList[i].transportStreamID == this->serviceList[j].transportStreamID &&
								this->serviceList[i].serviceID == this->serviceList[j].serviceID ){
									wstring log = L"";
									Format(log, L"%s space:%d ch:%d <=> %s space:%d ch:%d\r\n",
										this->serviceList[i].serviceName.c_str(),
										this->serviceList[i].space,
										this->serviceList[i].ch,
										this->serviceList[j].serviceName.c_str(),
										this->serviceList[j].space,
										this->serviceList[j].ch);
									msg += log;
									break;
							}
						}
					}
					if( msg.size() > 0){
						wstring log = L"����T�[�r�X�������̕����`�����l���Ō��o����܂����B\r\n��M���̂悢�����`�����l���̃T�[�r�X�̂ݎc���悤�ɐݒ���s���Ă��������B\r\n����ɘ^��ł��Ȃ��\�����o�Ă��܂��B\r\n\r\n";
						log += msg;
						MessageBox(m_hWnd, log.c_str(), NULL, MB_OK);
					}
				}else{
					KillTimer(TIMER_CHSCAN_STATSU);
				}
			}
			break;
		case TIMER_EPGCAP_STATSU:
			{
				EPGCAP_SERVICE_INFO info;
				CBonCtrl::JOB_STATUS status = this->main.GetEpgCapStatus(&info);
				if( status == CBonCtrl::ST_WORKING ){
					int sel = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_SERVICE));
					if( sel != CB_ERR ){
						DWORD index = (DWORD)ComboBox_GetItemData(GetDlgItem(IDC_COMBO_SERVICE), sel);
						if( info.ONID != this->serviceList[index].originalNetworkID ||
							info.TSID != this->serviceList[index].transportStreamID ||
							info.SID != this->serviceList[index].serviceID ){
						}
						this->initONID = info.ONID;
						this->initTSID = info.TSID;
						this->initSID = info.SID;
						ReloadServiceList();
						this->main.SetSID(info.SID);
					}

					SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"EPG�擾��\r\n");
				}else if( status == CBonCtrl::ST_CANCEL ){
					KillTimer(TIMER_EPGCAP_STATSU);
					SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�L�����Z������܂���\r\n");
				}else if( status == CBonCtrl::ST_COMPLETE ){
					KillTimer(TIMER_EPGCAP_STATSU);
					SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�I�����܂���\r\n");
					BtnUpdate(GUI_NORMAL);
					ChgIconStatus();
				}else{
					KillTimer(TIMER_EPGCAP_STATSU);
				}
			}
			break;
		case TIMER_REC_END:
			{
				this->main.StopRec();
				KillTimer(TIMER_REC_END);
				SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�^���~���܂���\r\n");
				BtnUpdate(GUI_NORMAL);
				Button_SetCheck(GetDlgItem(IDC_CHECK_REC_SET), BST_UNCHECKED);
				ChgIconStatus();
			}
			break;
		case RETRY_ADD_TRAY:
			{
				KillTimer(RETRY_ADD_TRAY);
				wstring buff=L"";
				wstring bonFile = L"";
				this->main.GetOpenBonDriver(&bonFile);
				WCHAR szBuff2[256]=L"";
				GetWindowText(GetDlgItem(IDC_COMBO_SERVICE), szBuff2, 256);
				Format(buff, L"%s �F %s", bonFile.c_str(), szBuff2);

				HICON setIcon = this->iconBlue;
				if( this->main.IsRec() == TRUE ){
					setIcon = this->iconRed;
				}else if( this->main.GetEpgCapStatus(NULL) == CBonCtrl::ST_WORKING ){
					setIcon = this->iconGreen;
				}else if( this->main.GetOpenBonDriver(NULL) == FALSE ){
					setIcon = this->iconGray;
				}
		
				if( AddTaskBar( GetSafeHwnd(),
						WM_TRAY_PUSHICON,
						TRAYICON_ID,
						setIcon,
						buff ) == FALSE ){
							SetTimer(RETRY_ADD_TRAY, 5000, NULL);
				}
			}
			break;
		case TIMER_TRY_STOP_SERVER:
			if( this->main.StopServer(true) ){
				KillTimer(TIMER_TRY_STOP_SERVER);
				OutputDebugString(L"CmdServer stopped\r\n");
				EndDialog(m_hWnd, IDCANCEL);
			}
			break;
		default:
			break;
	}
}


void CEpgDataCap_BonDlg::OnSize(UINT nType, int cx, int cy)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h���[ �R�[�h��ǉ����܂��B
	if( nType == SIZE_MINIMIZED && this->minTask == TRUE){
		SetTimer(RETRY_ADD_TRAY, 0, NULL);
		if(!this->iniMin) ShowWindow(m_hWnd, SW_HIDE);
	}
}


LRESULT CEpgDataCap_BonDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: �����ɓ���ȃR�[�h��ǉ����邩�A�������͊�{�N���X���Ăяo���Ă��������B
	switch(message){
	case WM_RESERVE_REC_START:
		{
			BtnUpdate(GUI_OTHER_CTRL);
			WCHAR log[512 + 64] = L"";
			GetDlgItemText(m_hWnd, IDC_EDIT_LOG, log, 512);
			if( wstring(log).find(L"�\��^�撆\r\n") == wstring::npos ){
				wcscat_s(log, L"�\��^�撆\r\n");
				SetDlgItemText(m_hWnd, IDC_EDIT_LOG, log);
			}
			ChgIconStatus();
		}
		break;
	case WM_RESERVE_REC_STOP:
		{
			BtnUpdate(GUI_NORMAL);
			SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�\��^��I�����܂���\r\n");
			ChgIconStatus();
		}
		break;
	case WM_RESERVE_EPGCAP_START:
		{
			SetTimer(TIMER_EPGCAP_STATSU, 1000, NULL);
			BtnUpdate(GUI_CANCEL_ONLY);
			ChgIconStatus();
		}
		break;
	case WM_RESERVE_EPGCAP_STOP:
		{
			ChgIconStatus();
		}
		break;
	case WM_CHG_TUNER:
		{
			wstring bonDriver = L"";
			this->main.GetOpenBonDriver(&bonDriver);
			this->iniBonDriver = bonDriver.c_str();
			ReloadBonDriver();
			ChgIconStatus();
		}
		break;
	case WM_CHG_CH:
		{
			WORD ONID;
			WORD TSID;
			WORD SID;
			this->main.GetCh(&ONID, &TSID, &SID);
			this->initONID = ONID;
			this->initTSID = TSID;
			this->initSID = SID;
			ReloadServiceList();
			ChgIconStatus();
		}
		break;
	case WM_RESERVE_REC_STANDBY:
		{
			if( wParam == 1 ){
				BtnUpdate(GUI_REC_STANDBY);
				SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�\��^��ҋ@��\r\n");
			}else if( wParam == 2 ){
				BtnUpdate(GUI_NORMAL);
				SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�������[�h\r\n");
			}else{
				BtnUpdate(GUI_NORMAL);
				SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"");
			}
		}
		break;
	case WM_INVOKE_CTRL_CMD:
		this->main.CtrlCmdCallbackInvoked();
		break;
	case WM_VIEW_APP_OPEN:
		this->main.ViewAppOpen();
		break;
	case WM_TRAY_PUSHICON:
		{
			//�^�X�N�g���C�֌W
			switch(LOWORD(lParam)){
				case WM_LBUTTONDOWN:
					{
						this->iniMin = FALSE;
						ShowWindow(m_hWnd, SW_RESTORE);
						SetForegroundWindow(m_hWnd);
						KillTimer(RETRY_ADD_TRAY);
						DeleteTaskBar(GetSafeHwnd(), TRAYICON_ID);
					}
					break ;
				default :
					break ;
				}
		}
		break;
	default:
		break;
	}

	return 0;
}


BOOL CEpgDataCap_BonDlg::AddTaskBar(HWND wnd, UINT msg, UINT id, HICON icon, wstring tips)
{ 
	BOOL ret=TRUE;
	NOTIFYICONDATA data = {};

	data.cbSize = sizeof(NOTIFYICONDATA); 
	data.hWnd = wnd; 
	data.uID = id; 
	data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
	data.uCallbackMessage = msg; 
	data.hIcon = icon; 

	wcsncpy_s(data.szTip, tips.c_str(), _TRUNCATE);
 
	ret = Shell_NotifyIcon(NIM_ADD, &data);
  
	return ret; 
}

BOOL CEpgDataCap_BonDlg::ChgTipsTaskBar(HWND wnd, UINT id, HICON icon, wstring tips)
{ 
	BOOL ret=TRUE;
	NOTIFYICONDATA data = {};

	data.cbSize = sizeof(NOTIFYICONDATA); 
	data.hWnd = wnd; 
	data.uID = id; 
	data.hIcon = icon; 
	data.uFlags = NIF_ICON | NIF_TIP; 

	wcsncpy_s(data.szTip, tips.c_str(), _TRUNCATE);
 
	ret = Shell_NotifyIcon(NIM_MODIFY, &data); 
 
	return ret; 
}

BOOL CEpgDataCap_BonDlg::DeleteTaskBar(HWND wnd, UINT id)
{ 
	BOOL ret=TRUE; 
	NOTIFYICONDATA data = {};
 
	data.cbSize = sizeof(NOTIFYICONDATA); 
	data.hWnd = wnd; 
	data.uID = id; 
         
	ret = Shell_NotifyIcon(NIM_DELETE, &data); 

	return ret; 
}

void CEpgDataCap_BonDlg::ChgIconStatus(){
	if( this->minTask == TRUE){
		wstring buff=L"";
		wstring bonFile = L"";
		this->main.GetOpenBonDriver(&bonFile);
		WCHAR szBuff2[256]=L"";
		GetWindowText(GetDlgItem(IDC_COMBO_SERVICE), szBuff2, 256);
		Format(buff, L"%s �F %s", bonFile.c_str(), szBuff2);

		HICON setIcon = this->iconBlue;
		if( this->main.IsRec() == TRUE ){
			setIcon = this->iconRed;
		}else if( this->main.GetEpgCapStatus(NULL) == CBonCtrl::ST_WORKING ){
			setIcon = this->iconGreen;
		}else if( this->main.GetOpenBonDriver(NULL) == FALSE ){
			setIcon = this->iconGray;
		}

		ChgTipsTaskBar( GetSafeHwnd(),
				TRAYICON_ID,
				setIcon,
				buff );
	}
}

LRESULT CEpgDataCap_BonDlg::OnTaskbarCreated(WPARAM, LPARAM)
{
	if( IsWindowVisible(m_hWnd) == FALSE && this->minTask == TRUE){
		SetTimer(RETRY_ADD_TRAY, 0, NULL);
	}

	return 0;
}

#define ENABLE_ITEM(nItem,bEnable) EnableWindow(GetDlgItem(nItem),(bEnable))

void CEpgDataCap_BonDlg::BtnUpdate(DWORD guiMode)
{
	switch(guiMode){
		case GUI_NORMAL:
			ENABLE_ITEM(IDC_COMBO_TUNER, TRUE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, TRUE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, TRUE);
			ENABLE_ITEM(IDC_BUTTON_EPG, TRUE);
			ENABLE_ITEM(IDC_BUTTON_SET, TRUE);
			ENABLE_ITEM(IDC_BUTTON_REC, TRUE);
			ENABLE_ITEM(IDC_COMBO_REC_H, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_M, FALSE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, FALSE);
			Button_SetCheck(GetDlgItem(IDC_CHECK_REC_SET), BST_UNCHECKED);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, FALSE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		case GUI_CANCEL_ONLY:
			ENABLE_ITEM(IDC_COMBO_TUNER, FALSE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, FALSE);
			ENABLE_ITEM(IDC_BUTTON_EPG, FALSE);
			ENABLE_ITEM(IDC_BUTTON_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_REC, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_H, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_M, FALSE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, TRUE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		case GUI_OPEN_FAIL:
			ENABLE_ITEM(IDC_COMBO_TUNER, TRUE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, FALSE);
			ENABLE_ITEM(IDC_BUTTON_EPG, FALSE);
			ENABLE_ITEM(IDC_BUTTON_SET, TRUE);
			ENABLE_ITEM(IDC_BUTTON_REC, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_H, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_M, FALSE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, FALSE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		case GUI_REC:
			ENABLE_ITEM(IDC_COMBO_TUNER, FALSE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, FALSE);
			ENABLE_ITEM(IDC_BUTTON_EPG, FALSE);
			ENABLE_ITEM(IDC_BUTTON_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_REC, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_H, TRUE);
			ENABLE_ITEM(IDC_COMBO_REC_M, TRUE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, TRUE);
			Button_SetCheck(GetDlgItem(IDC_CHECK_REC_SET), BST_UNCHECKED);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, TRUE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		case GUI_REC_SET_TIME:
			ENABLE_ITEM(IDC_COMBO_TUNER, FALSE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, FALSE);
			ENABLE_ITEM(IDC_BUTTON_EPG, FALSE);
			ENABLE_ITEM(IDC_BUTTON_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_REC, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_H, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_M, FALSE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, TRUE);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, TRUE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		case GUI_OTHER_CTRL:
			ENABLE_ITEM(IDC_COMBO_TUNER, FALSE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, FALSE);
			ENABLE_ITEM(IDC_BUTTON_EPG, FALSE);
			ENABLE_ITEM(IDC_BUTTON_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_REC, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_H, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_M, FALSE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, TRUE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		case GUI_REC_STANDBY:
			ENABLE_ITEM(IDC_COMBO_TUNER, FALSE);
			ENABLE_ITEM(IDC_COMBO_SERVICE, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CHSCAN, FALSE);
			ENABLE_ITEM(IDC_BUTTON_EPG, FALSE);
			ENABLE_ITEM(IDC_BUTTON_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_REC, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_H, FALSE);
			ENABLE_ITEM(IDC_COMBO_REC_M, FALSE);
			ENABLE_ITEM(IDC_CHECK_REC_SET, FALSE);
			ENABLE_ITEM(IDC_BUTTON_CANCEL, FALSE);
			ENABLE_ITEM(IDC_BUTTON_VIEW, TRUE);
			break;
		default:
			break;
	}
}



void CEpgDataCap_BonDlg::OnCbnSelchangeComboTuner()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	WCHAR buff[512];
	if( GetWindowText(GetDlgItem(IDC_COMBO_TUNER), buff, 512) > 0 ){
		SelectBonDriver(buff);

		int sel = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_SERVICE));
		if( sel != CB_ERR ){
			DWORD index = (DWORD)ComboBox_GetItemData(GetDlgItem(IDC_COMBO_SERVICE), sel);
			SelectService(this->serviceList[index].originalNetworkID, this->serviceList[index].transportStreamID, this->serviceList[index].serviceID, this->serviceList[index].space, this->serviceList[index].ch );
		}
	}
	ChgIconStatus();
}


void CEpgDataCap_BonDlg::OnCbnSelchangeComboService()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	int sel = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_SERVICE));
	if( sel != CB_ERR ){
		DWORD index = (DWORD)ComboBox_GetItemData(GetDlgItem(IDC_COMBO_SERVICE), sel);
		SelectService(this->serviceList[index].originalNetworkID, this->serviceList[index].transportStreamID, this->serviceList[index].serviceID, this->serviceList[index].space, this->serviceList[index].ch );
	}
	ChgIconStatus();
}


void CEpgDataCap_BonDlg::OnBnClickedButtonSet()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	CSettingDlg setDlg(m_hWnd);
	disableKeyboardHook = TRUE;
	INT_PTR result = setDlg.DoModal();
	disableKeyboardHook = FALSE;
	if( result == IDOK ){

		this->main.ReloadSetting();

		ReloadNWSet();

		WORD ONID;
		WORD TSID;
		WORD SID;
		this->main.GetCh(&ONID, &TSID, &SID);
		this->initONID = ONID;
		this->initTSID = TSID;
		this->initSID = SID;
		ReloadServiceList();
		
		this->minTask = GetPrivateProfileInt( L"Set", L"MinTask", 0, this->moduleIniPath.c_str() );
	}
}

void CEpgDataCap_BonDlg::ReloadNWSet()
{
	this->main.SendUDP(FALSE);
	this->main.SendTCP(FALSE);
	if( this->main.GetCountUDP() > 0 ){
		EnableWindow(GetDlgItem(IDC_CHECK_UDP), TRUE);
	}else{
		EnableWindow(GetDlgItem(IDC_CHECK_UDP), FALSE);
		Button_SetCheck(GetDlgItem(IDC_CHECK_UDP), BST_UNCHECKED);
	}
	if( this->main.GetCountTCP() > 0 ){
		EnableWindow(GetDlgItem(IDC_CHECK_TCP), TRUE);
	}else{
		EnableWindow(GetDlgItem(IDC_CHECK_TCP), FALSE);
		Button_SetCheck(GetDlgItem(IDC_CHECK_TCP), BST_UNCHECKED);
	}
	this->main.SendUDP(Button_GetCheck(GetDlgItem(IDC_CHECK_UDP)));
	this->main.SendTCP(Button_GetCheck(GetDlgItem(IDC_CHECK_TCP)));
}

void CEpgDataCap_BonDlg::ReloadBonDriver()
{
	this->bonList.clear();
	ComboBox_ResetContent(GetDlgItem(IDC_COMBO_TUNER));

	this->bonList = this->main.EnumBonDriver();

	int selectIndex = 0;
	vector<wstring>::iterator itr;
	for( itr = this->bonList.begin(); itr != this->bonList.end(); itr++ ){
		int index = ComboBox_AddString(GetDlgItem(IDC_COMBO_TUNER), itr->c_str());
		if( this->iniBonDriver.empty() == false ){
			if( this->iniBonDriver.compare(*itr) == 0 ){
				selectIndex = index;
			}
		}
	}
	if( this->bonList.size() > 0){
		ComboBox_SetCurSel(GetDlgItem(IDC_COMBO_TUNER), selectIndex);
	}
}

void CEpgDataCap_BonDlg::ReloadServiceList(BOOL ini)
{
	this->serviceList.clear();
	ComboBox_ResetContent(GetDlgItem(IDC_COMBO_SERVICE));

	DWORD ret = this->main.GetServiceList(&this->serviceList);
	if( ret != NO_ERR || this->serviceList.size() == 0 ){
		WCHAR log[512 + 64] = L"";
		GetDlgItemText(m_hWnd, IDC_EDIT_LOG, log, 512);
		wcscat_s(log, L"�`�����l�����̓ǂݍ��݂Ɏ��s���܂���\r\n");
		SetDlgItemText(m_hWnd, IDC_EDIT_LOG, log);
	}else{
		int selectSel = 0;
		for( size_t i=0; i<this->serviceList.size(); i++ ){
			if( this->serviceList[i].useViewFlag == TRUE ){
				int index = ComboBox_AddString(GetDlgItem(IDC_COMBO_SERVICE), this->serviceList[i].serviceName.c_str());
				ComboBox_SetItemData(GetDlgItem(IDC_COMBO_SERVICE), index, i);
				if( this->serviceList[i].originalNetworkID == this->initONID &&
					this->serviceList[i].transportStreamID == this->initTSID &&
					this->serviceList[i].serviceID == this->initSID ){
						if( ini == FALSE ){
							this->initONID = -1;
							this->initTSID = -1;
							this->initSID = -1;
						}
						selectSel = index;
				}
			}
		}
		if( ComboBox_GetCount(GetDlgItem(IDC_COMBO_SERVICE)) > 0 ){
			ComboBox_SetCurSel(GetDlgItem(IDC_COMBO_SERVICE), selectSel);
		}

	}

}

DWORD CEpgDataCap_BonDlg::SelectBonDriver(LPCWSTR fileName, BOOL ini)
{
	this->main.CloseBonDriver();
	DWORD err = this->main.OpenBonDriver(fileName);
	if( err != NO_ERR ){
		wstring log;
		Format(log, L"BonDriver�̃I�[�v�����ł��܂���ł���\r\n%s\r\n", fileName);
		SetDlgItemText(m_hWnd, IDC_EDIT_LOG, log.c_str());
		BtnUpdate(GUI_OPEN_FAIL);
	}else{
		SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"");
		BtnUpdate(GUI_NORMAL);
	}
	ReloadServiceList(ini);
	return err;
}

DWORD CEpgDataCap_BonDlg::SelectService(WORD ONID, WORD TSID, WORD SID)
{
	DWORD err = this->main.SetCh(ONID, TSID, SID);
	return err;
}

DWORD CEpgDataCap_BonDlg::SelectService(WORD ONID, WORD TSID, WORD SID,	DWORD space, DWORD ch)
{
	DWORD err = this->main.SetCh(ONID, TSID, SID, space, ch);
	return err;
}

void CEpgDataCap_BonDlg::OnBnClickedButtonChscan()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	if( this->main.StartChScan() == FALSE ){
		SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�`�����l���X�L�������J�n�ł��܂���ł���\r\n");
		return;
	}
	SetTimer(TIMER_CHSCAN_STATSU, 1000, NULL);
	BtnUpdate(GUI_CANCEL_ONLY);
}


void CEpgDataCap_BonDlg::OnBnClickedButtonEpg()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	if( this->main.StartEpgCap() == FALSE ){
		SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"EPG�擾���J�n�ł��܂���ł���\r\n");
		return;
	}
	SetTimer(TIMER_EPGCAP_STATSU, 1000, NULL);
	BtnUpdate(GUI_CANCEL_ONLY);
	ChgIconStatus();
}


void CEpgDataCap_BonDlg::OnBnClickedButtonRec()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	if( this->main.StartRec() != NO_ERR ){
		SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�^����J�n�ł��܂���ł���\r\n");
		return;
	}
	SYSTEMTIME end;
	ConvertSystemTime(GetNowI64Time() + 30 * 60 * I64_1SEC, &end);

	ComboBox_SetCurSel(GetDlgItem(IDC_COMBO_REC_H), end.wHour);
	ComboBox_SetCurSel(GetDlgItem(IDC_COMBO_REC_M), end.wMinute);

	SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�^�撆\r\n");

	BtnUpdate(GUI_REC);
	ChgIconStatus();
}


void CEpgDataCap_BonDlg::OnBnClickedButtonCancel()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	if( this->main.IsRec() == TRUE ){
		WCHAR caption[128] = L"";
		GetWindowText(m_hWnd, caption, 128);
		disableKeyboardHook = TRUE;
		int result = MessageBox( m_hWnd, L"�^����~���܂����H", caption, MB_YESNO | MB_ICONQUESTION );
		disableKeyboardHook = FALSE;
		if( result == IDNO ){
			return ;
		}
	}
	SetDlgItemText(m_hWnd, IDC_EDIT_LOG, L"�L�����Z������܂���\r\n");

	this->main.StopChScan();
	KillTimer(TIMER_CHSCAN_STATSU);
	this->main.StopEpgCap();
	KillTimer(TIMER_EPGCAP_STATSU);
	this->main.StopRec();
	KillTimer(TIMER_REC_END);
	this->main.StopReserveRec();


	BtnUpdate(GUI_NORMAL);
	ChgIconStatus();
}


void CEpgDataCap_BonDlg::OnBnClickedButtonView()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	this->main.ViewAppOpen();
}


void CEpgDataCap_BonDlg::OnBnClickedCheckUdp()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	this->main.SendUDP(Button_GetCheck(GetDlgItem(IDC_CHECK_UDP)));
}


void CEpgDataCap_BonDlg::OnBnClickedCheckTcp()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	this->main.SendTCP(Button_GetCheck(GetDlgItem(IDC_CHECK_TCP)));
}


void CEpgDataCap_BonDlg::OnBnClickedCheckRecSet()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	if( Button_GetCheck(GetDlgItem(IDC_CHECK_REC_SET)) != BST_UNCHECKED ){
		BtnUpdate(GUI_REC_SET_TIME);

		int selH = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_REC_H));
		int selM = ComboBox_GetCurSel(GetDlgItem(IDC_COMBO_REC_M));

		DWORD nowTime = (DWORD)(GetNowI64Time() / I64_1SEC % (24*60*60));
		DWORD endTime = selH*60*60 + selM*60;

		if( nowTime > endTime ){
			endTime += 24*60*60;
		}
		SetTimer(TIMER_REC_END, (endTime-nowTime)*1000, NULL );
	}else{
		BtnUpdate(GUI_REC);
		KillTimer(TIMER_REC_END);
	}
}


void CEpgDataCap_BonDlg::OnBnClickedCheckNextpg()
{
	// TODO: �����ɃR���g���[���ʒm�n���h���[ �R�[�h��ǉ����܂��B
	wstring info = L"";
	this->main.GetEpgInfo(Button_GetCheck(GetDlgItem(IDC_CHECK_NEXTPG)), &info);
	WCHAR pgInfo[512] = L"";
	GetDlgItemText(m_hWnd, IDC_EDIT_PG_INFO, pgInfo, 512);
	if( info.substr(0, 511).compare(pgInfo) != 0 ){
		SetDlgItemText(m_hWnd, IDC_EDIT_PG_INFO, info.c_str());
	}
}


BOOL CEpgDataCap_BonDlg::OnQueryEndSession()
{
	// TODO:  �����ɓ���ȃN�G���̏I���Z�b�V���� �R�[�h��ǉ����Ă��������B
	if( this->main.IsRec() == TRUE ){
		ShowWindow(m_hWnd, SW_SHOW);
		return FALSE;
	}
	return TRUE;
}


void CEpgDataCap_BonDlg::OnEndSession(BOOL bEnding)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h���[ �R�[�h��ǉ����܂��B
	if( bEnding == TRUE ){
		if( this->main.IsRec() == TRUE ){
			this->main.StopReserveRec();
			this->main.StopRec();
		}
	}
}


LRESULT CALLBACK CEpgDataCap_BonDlg::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//Enter,Esc�𖳎�����
	if( disableKeyboardHook == FALSE && nCode == HC_ACTION && (wParam == VK_RETURN || wParam == VK_ESCAPE) && (lParam & (1 << 30)) == 0 ){
		return TRUE;
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


INT_PTR CALLBACK CEpgDataCap_BonDlg::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CEpgDataCap_BonDlg* pSys = (CEpgDataCap_BonDlg*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if( pSys == NULL && uMsg != WM_INITDIALOG ){
		return FALSE;
	}
	switch( uMsg ){
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		pSys = (CEpgDataCap_BonDlg*)lParam;
		pSys->m_hWnd = hDlg;
		pSys->m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, GetCurrentThreadId());
		return pSys->OnInitDialog();
	case WM_NCDESTROY:
		UnhookWindowsHookEx(pSys->m_hKeyboardHook);
		pSys->m_hWnd = NULL;
		break;
	case WM_DESTROY:
		pSys->OnDestroy();
		break;
	case WM_TIMER:
		pSys->OnTimer(wParam);
		break;
	case WM_SIZE:
		pSys->OnSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_QUERYENDSESSION:
		SetWindowLongPtr(hDlg, DWLP_MSGRESULT, pSys->OnQueryEndSession());
		return TRUE;
	case WM_ENDSESSION:
		pSys->OnEndSession((BOOL)wParam);
		break;
	case WM_COMMAND:
		switch( LOWORD(wParam) ){
		case IDC_COMBO_TUNER:
			if( HIWORD(wParam) == CBN_SELCHANGE ){
				pSys->OnCbnSelchangeComboTuner();
			}
			break;
		case IDC_COMBO_SERVICE:
			if( HIWORD(wParam) == CBN_SELCHANGE ){
				pSys->OnCbnSelchangeComboService();
			}
			break;
		case IDC_BUTTON_SET:
			pSys->OnBnClickedButtonSet();
			break;
		case IDC_BUTTON_CHSCAN:
			pSys->OnBnClickedButtonChscan();
			break;
		case IDC_BUTTON_EPG:
			pSys->OnBnClickedButtonEpg();
			break;
		case IDC_BUTTON_REC:
			pSys->OnBnClickedButtonRec();
			break;
		case IDC_CHECK_REC_SET:
			pSys->OnBnClickedCheckRecSet();
			break;
		case IDC_BUTTON_CANCEL:
			pSys->OnBnClickedButtonCancel();
			break;
		case IDC_CHECK_TCP:
			pSys->OnBnClickedCheckTcp();
			break;
		case IDC_CHECK_UDP:
			pSys->OnBnClickedCheckUdp();
			break;
		case IDC_BUTTON_VIEW:
			pSys->OnBnClickedButtonView();
			break;
		case IDC_CHECK_NEXTPG:
			pSys->OnBnClickedCheckNextpg();
			break;
		case IDOK:
		case IDCANCEL:
			//�f�b�h���b�N����̂��߃��b�Z�[�W�|���v���ێ����T�[�o���I��点��
			pSys->main.StopServer(true);
			pSys->SetTimer(TIMER_TRY_STOP_SERVER, 20, NULL);
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
			return TRUE;
		}
		break;
	case WM_SYSCOMMAND:
		{
			BOOL bProcessed = FALSE;
			pSys->OnSysCommand((UINT)(wParam & 0xFFF0), lParam, &bProcessed);
			if( bProcessed != FALSE ){
				SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
				return TRUE;
			}
		}
		break;
	default:
		if( uMsg == taskbarCreated ){
			pSys->OnTaskbarCreated(wParam, lParam);
		}else if( uMsg >= WM_USER ){
			pSys->WindowProc(uMsg, wParam, lParam);
		}
		break;
	}
	return FALSE;
}
