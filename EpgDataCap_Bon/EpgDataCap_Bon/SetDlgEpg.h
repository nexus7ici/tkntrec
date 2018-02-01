#pragma once

#include "../../Common/ParseTextInstances.h"

// CSetDlgEpg �_�C�A���O

class CSetDlgEpg
{
public:
	CSetDlgEpg();   // �W���R���X�g���N�^�[
	~CSetDlgEpg();
	BOOL Create(LPCTSTR lpszTemplateName, HWND hWndParent);
	HWND GetSafeHwnd() const{ return m_hWnd; }
	void SaveIni(void);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_DIALOG_SET_EPG };
	
protected:
	HWND m_hWnd;
	CParseChText5 chSet;

	BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAllChk();
	afx_msg void OnBnClickedButtonVideoChk();
	afx_msg void OnBnClickedButtonAllClear();
	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND GetDlgItem(int nID) const{ return ::GetDlgItem(m_hWnd, nID); }
};
