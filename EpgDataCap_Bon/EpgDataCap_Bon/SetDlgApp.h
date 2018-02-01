#pragma once


// CSetDlgApp �_�C�A���O

class CSetDlgApp
{
public:
	CSetDlgApp();   // �W���R���X�g���N�^�[
	~CSetDlgApp();
	BOOL Create(LPCTSTR lpszTemplateName, HWND hWndParent);
	HWND GetSafeHwnd() const{ return m_hWnd; }
	void SaveIni(void);

// �_�C�A���O �f�[�^
	enum { IDD = IDD_DIALOG_SET_APP };

protected:
	HWND m_hWnd;

	BOOL OnInitDialog();
	static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND GetDlgItem(int nID) const{ return ::GetDlgItem(m_hWnd, nID); }
};
