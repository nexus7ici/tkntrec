#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <LM.h>
#pragma comment(lib, "netapi32.lib")

static const TCHAR CLASS_NAME[] = TEXT("EpgTimerAdminProxy");

static bool g_denySetTime;

enum {
	// �V�X�e��������ݒ肷��
	WM_APP_SETTIME = WM_APP,
	// ����g���q�̃t�@�C���ɋ��L�A�N�Z�X�����邩�ǂ������ׂ�
	WM_APP_NETFIND,
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		if (wParam == 1) {
			KillTimer(hwnd, 1);
			g_denySetTime = false;
		}
		break;
	case WM_APP_SETTIME:
		if (!g_denySetTime) {
			FILETIME ft;
			ft.dwLowDateTime = static_cast<DWORD>(wParam);
			ft.dwHighDateTime = static_cast<DWORD>(lParam);
			FILETIME ftNow;
			GetSystemTimeAsFileTime(&ftNow);
			__int64 t = ft.dwLowDateTime | static_cast<__int64>(ft.dwHighDateTime) << 32;
			__int64 tNow = ftNow.dwLowDateTime | static_cast<__int64>(ftNow.dwHighDateTime) << 32;
			// ���ϋ֎~(24����)
			if (tNow - 24 * 3600 * 1000000LL < t && t < tNow + 24 * 3600 * 10000000LL) {
				SYSTEMTIME st;
				if (FileTimeToSystemTime(&ft, &st) && SetSystemTime(&st)) {
					// ���S�̂���30�b�Ԃ͍Đݒ�����ۂ���
					g_denySetTime = true;
					SetTimer(hwnd, 1, 30000, nullptr);
					return TRUE;
				}
			}
		}
		return -1;
	case WM_APP_NETFIND:
		{
			// 8�����ȉ��̊g���q��wParam��lParam�Ŏ󂯎��
			WCHAR ext[10] = L".";
			for (int i = 1; i < 9; ++i) {
				ext[i] = (static_cast<DWORD>(i < 5 ? wParam : lParam) >> ((i - 1) % 4 * 8)) & 0xFF;
				// �p�����Ɍ���
				if (ext[i] && (ext[i] < L'0' || L'9' < ext[i]) && (ext[i] < L'A' || L'Z' < ext[i]) && (ext[i] < L'a' || L'z' < ext[i])) {
					ext[1] = L'\0';
					break;
				}
			}
			LRESULT ret = -1;
			if (ext[1]) {
				FILE_INFO_3 *info;
				DWORD entriesread;
				DWORD totalentries;
				if (NetFileEnum(nullptr, nullptr, nullptr, 3, reinterpret_cast<LPBYTE*>(&info), MAX_PREFERRED_LENGTH, &entriesread, &totalentries, nullptr) == NERR_Success) {
					for (DWORD i = 0; i < entriesread; ++i) {
						if (wcslen(info[i].fi3_pathname) >= wcslen(ext) && _wcsicmp(info[i].fi3_pathname + wcslen(info[i].fi3_pathname) - wcslen(ext), ext) == 0) {
							ret = TRUE;
							break;
						}
					}
					NetApiBufferFree(info);
				}
			}
			return ret;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	static_cast<void>(hPrevInstance);
	static_cast<void>(nCmdShow);

	SetDllDirectory(TEXT(""));

	if (_tcsicmp(lpCmdLine, TEXT("/TestSetTime")) == 0) {
		// ����e�X�g: ���ݎ����ŃV�X�e��������ݒ肷��
		LPCTSTR text = TEXT("Proxy not found.");
		HWND hwnd = FindWindowEx(HWND_MESSAGE, nullptr, CLASS_NAME, nullptr);
		if (hwnd) {
			FILETIME ft;
			GetSystemTimeAsFileTime(&ft);
			LRESULT ret = SendMessage(hwnd, WM_APP_SETTIME, ft.dwLowDateTime, ft.dwHighDateTime);
			text = (ret > 0 ? TEXT("Succeeded.") : ret < 0 ? TEXT("Failed.") : TEXT("Denied."));
		}
		MessageBox(nullptr, text, CLASS_NAME, MB_OK);
		return 0;
	}
	else if (_tcsicmp(lpCmdLine, TEXT("/TestNetFind")) == 0) {
		// ����e�X�g: �g���q.txt�ɂ��ċ��L�A�N�Z�X�����邩�ǂ������ׂ�
		LPCTSTR text = TEXT("Proxy not found.");
		HWND hwnd = FindWindowEx(HWND_MESSAGE, nullptr, CLASS_NAME, nullptr);
		if (hwnd) {
			LRESULT ret = SendMessage(hwnd, WM_APP_NETFIND, 0x00747874, 0x00000000);
			text = (ret > 0 ? TEXT("Succeeded.") : ret < 0 ? TEXT("Failed or no access.") : TEXT("Denied."));
		}
		MessageBox(nullptr, text, CLASS_NAME, MB_OK);
		return 0;
	}

	// ���b�Z�[�W��p�E�B���h�E���쐬
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);
	if (CreateWindow(CLASS_NAME, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr)) {
		// ���������x���̒Ⴂ�v���Z�X����̃��b�Z�[�W���󂯎��悤�ɂ���
		ChangeWindowMessageFilter(WM_APP_SETTIME, MSGFLT_ADD);
		ChangeWindowMessageFilter(WM_APP_NETFIND, MSGFLT_ADD);
		// ���b�Z�[�W���[�v
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}
