//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.

#include "stdafx.h"
#include "murkdecl.h"
#include "murkcommon.h"

extern void serversettings(int go);
extern BOOL CALLBACK DlgNetwork(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT PerformMenuCommand(HWND hWnd, WPARAM wParam) {

	switch (LOWORD(wParam)) {

	case IDM_GAME_CREATE:
		//        serversettings(0);

		DialogBox(NULL, (LPCTSTR)IDD_NETWORK, hWindow,
		          (DLGPROC)DlgNetwork);

		return 0;

	case IDM_GAME_EXIT:
		SendMessage(hWnd, WM_CLOSE, 0, 0L);
		return 0;
	}

	return 0;
}