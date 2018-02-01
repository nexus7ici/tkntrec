#pragma once


#include "../../BonCtrl/BonCtrlDef.h"

// CSetDlgNetwork �_�C�A���O

class CSetDlgNetwork
{
public:
	CSetDlgNetwork();   // �W���R���X�g���N�^�[
	~CSetDlgNetwork();
	BOOL Create(LPCTSTR lpszTemplateName, HWND hWndParent);
	HWND GetSafeHwnd() const{ return m_hWnd; }
	void SaveIni(void);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_DIALOG_SET_NW };

protected:
	HWND m_hWnd;
	vector<NW_SEND_INFO> udpSendList;
	vector<NW_SEND_INFO> tcpSendList;

	BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAddUdp();
	afx_msg void OnBnClickedButtonDelUdp();
	afx_msg void OnBnClickedButtonAddTcp();
	afx_msg void OnBnClickedButtonDelTcp();
	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND GetDlgItem(int nID) const{ return ::GetDlgItem(m_hWnd, nID); }
};
