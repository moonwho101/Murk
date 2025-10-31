//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.

#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"
#include "dplay.h"
#include "game.h"
// #include <atlbase.h>

#include "registry.h"

BOOL FAR PASCAL EnumSP(LPGUID lpGuid, LPSTR lpDesc, DWORD dwMajorVersion,
                       DWORD dwMinorVersion, LPVOID lpv);

BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID lpContext,
                            LPDWORD lpdwTimeOut, DWORD dwFlags);

BOOL FAR PASCAL EnumSPNEW(LPGUID lpGd, LPSTR lpDesc, DWORD dwMajorVersion,
                          DWORD dwMinorVersion, LPVOID lpv);

BOOL CALLBACK DlgNetwork(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgRegister(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void find_tcp_ipx();
void RegCrypt(char *user, char *key, char *result);
extern GUID TICTACTOE_10;
DWORD Globalid;
extern int save_server_settings(int network);
extern int is_shareware();
extern BOOL doit;
;
extern UINT m_uiDevID;

extern void play_random_music();
extern void stop_song();

extern int usejstick;
extern int usesound;

bool WriteMurkReg();
bool SaveMurkReg();
bool ReadMurkReg();
bool IsShareware();

extern int networktype;
LPDPSESSIONDESC lpGameID;
void write_reg();
BOOL CALLBACK DlgProcQCreate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	// This handles the messages from the Create/Connect dialog box
	switch (msg) {
	case WM_COMMAND:
		switch (wParam) {
		case IDC_CREATE:
			dialogresponse = 0;
			EndDialog(hDlg, -1);
			return (TRUE);
			break;
		case IDC_CONNECT:
			dialogresponse = 1;
			EndDialog(hDlg, -1);
			return (TRUE);
			break;

		case IDCANCEL:
			doit = TRUE;
			// Escape was pressed
			EndDialog(hDlg, FALSE);
			return (TRUE);
		}
		break;
	}
	return (FALSE);
}

BOOL CALLBACK DlgProcChooseProvider(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	static LONG iIndex;
	static HWND hWndCtl;

	switch (msg) {
	case WM_INITDIALOG:
		// Get the list control
		hWndCtl = GetDlgItem(hDlg, IDC_LIST1);
		if (hWndCtl == NULL) {
			EndDialog(hDlg, TRUE);
			return (TRUE);
		}
		// Fill the list with DirectPlay service providers
		DirectPlayEnumerate(EnumSP, (LPVOID)hWndCtl);
		// Highlight the first one
		SetFocus(hWndCtl);
		SendMessage(hWndCtl, LB_SETCURSEL, 0, 0);
		return (FALSE);

	case WM_COMMAND:

		switch (HIWORD(wParam)) {
		case LBN_SELCHANGE:
			// Move cursor
			iIndex = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
			hWndCtl = (HWND)lParam;
			return (FALSE);

		case LBN_DBLCLK:
			// They've selected one, so get it
			iIndex = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
			if (iIndex != LB_ERR) {
				lpGuid = (LPGUID)SendMessage((HWND)lParam, LB_GETITEMDATA, iIndex, 0);
				// Create a DirectPlay object for the chosen provider
				//                DirectPlayCreate(lpGuid, &lpIDC, NULL);
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;

		case 0:
			// Was enter pressed?
			if (LOWORD(wParam) == IDOK) {
				// Is there anything in the listbox?
				if (iIndex != LB_ERR) {
					// Get the selected provider
					lpGuid = (LPGUID)SendMessage(hWndCtl, LB_GETITEMDATA, iIndex, 0);
					if (lpGuid) {
						// Create a DirectPlay object for the chosen provider
						//                        DirectPlayCreate(lpGuid, &lpIDC, NULL);
						EndDialog(hDlg, TRUE);
					} else
						EndDialog(hDlg, FALSE);
					return (TRUE);
				}
			} else if (LOWORD(wParam) == IDCANCEL) {
				doit = TRUE;
				// Escape was pressed
				EndDialog(hDlg, FALSE);
				return (TRUE);
			}
			break;
		}
	}
	return (FALSE);
}

BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID lpContext,
                            LPDWORD lpdwTimeOut, DWORD dwFlags) {
	LONG iIndex;
	HWND hWnd = (HWND)lpContext;

	// Have we timed out?
	if (dwFlags & DPESC_TIMEDOUT) {
		return FALSE; // don't try again
	}

	// Add the session to the listbox
	iIndex = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)lpDPGameDesc->szSessionName);
	if (iIndex != LB_ERR)
		SendMessage(hWnd, LB_SETITEMDATA, iIndex, (LPARAM)lpDPGameDesc->dwSession);

	SetFocus(hWnd);
	SendMessage(hWnd, LB_SETCURSEL, 0, 0);
	return (TRUE);
}

//**********

BOOL FAR PASCAL EnumSP(LPGUID lpGuid, LPSTR lpDesc, DWORD dwMajorVersion,
                       DWORD dwMinorVersion, LPVOID lpv) {
	LONG iIndex;
	HWND hWnd = (HWND)lpv;

	// Add the service provider to the listbox
	iIndex = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)lpDesc);
	if (iIndex != LB_ERR) {
		SendMessage(hWnd, LB_SETITEMDATA, iIndex, (LPARAM)lpGuid);
		Globalid = (DWORD)lpGuid;
	}
	SetFocus(hWnd);
	SendMessage(hWnd, LB_SETCURSEL, 0, 0);
	return (TRUE);
}

//**********

BOOL CALLBACK DlgProcSelSession(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static LONG iIndex;
	static HWND hWndCtl;
	DPSESSIONDESC dpDesc;
	HRESULT hr = DP_OK + 10;

	switch (msg) {
	case WM_INITDIALOG:
		// Get the listbox
		hWndCtl = GetDlgItem(hDlg, IDC_LB_SESSION);
		if (hWndCtl == NULL) {
			EndDialog(hDlg, TRUE);
			return (TRUE);
		}

		//	::ZeroMemory(&dpsdDesc, sizeof(DPSESSIONDESC));
		//	dpsdDesc.dwSize = sizeof(DPSESSIONDESC);
		//	dpsdDesc.guidSession = TICTACTOE_10;
		//	if (m_pDirectPlay->EnumSessions(&dpsdDesc, 5000, EnumSessions,
		//	    (LPVOID)this, DPENUMSESSIONS_ALL) == DP_OK)
		//	    m_lbSessions.SetCurSel(0);
		//	else
		//	    EndDialog(FALSE);

		// Initialize the session description structure
		memset(&dpDesc, 0x00, sizeof(DPSESSIONDESC));
		dpDesc.dwSize = sizeof(dpDesc);
		dpDesc.guidSession = TICTACTOE_10;
		// Enum sessions with 5 second timeout
		//        lpIDC->lpVtbl->EnumSessions(lpIDC, &dpDesc, (DWORD)5000, EnumSession, (LPVOID) hWndCtl, (DWORD)NULL);

		m_pDirectPlay->EnumSessions(&dpDesc, (DWORD)6000, EnumSession,
		                            (LPVOID)hWndCtl, DPENUMSESSIONS_ALL);

		//  if (m_pDirectPlay->EnumSessions(&dpsdDesc, 5000, EnumSessions,
		//    (LPVOID)this, DPENUMSESSIONS_ALL) == DP_OK)
		//    m_lbSessions.SetCurSel(0);
		//  else
		//    EndDialog(FALSE);

		SetFocus(hWndCtl);
		return (FALSE);

	case WM_COMMAND:

		switch (HIWORD(wParam)) {
		case LBN_SELCHANGE:
			// Change selection
			iIndex = SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
			hWndCtl = (HWND)lParam;
			return (FALSE);
		case 0:
			if (LOWORD(wParam) == IDCANCEL) {
				// Player pressed escape
				//                lpIDC->lpVtbl->Close(lpIDC);
				//                lpIDC->lpVtbl->Release(lpIDC);
				//                lpIDC = NULL;
				doit = TRUE;

				// fix
				EndDialog(hDlg, FALSE);
				return (TRUE);
			}
			//
			// Fall Through.
			//
		case LBN_DBLCLK:
			if (HIWORD(wParam) == LBN_DBLCLK) {
				// They selected an entry, so get it
				hWndCtl = (HWND)lParam;
				iIndex = SendMessage(hWndCtl, LB_GETCURSEL, 0, 0);
			}

			// Do we have an entry?
			if (iIndex != LB_ERR) {
				// Yes, so initialize session description struc to open it
				//                memset(&dpDesc, 0x00, sizeof(DPSESSIONDESC));
				//                dpDesc.dwSize       = sizeof(dpDesc);
				//                dpDesc.guidSession  = *g_lpGuid;
				//                dpDesc.dwFlags      = DPOPEN_OPENSESSION;
				dwsession = SendMessage((HWND)hWndCtl, LB_GETITEMDATA, iIndex, 0);
				//                hr = lpIDC->lpVtbl->Open(lpIDC, &dpDesc);

				// Successful?
				//                if (hr != DP_OK)
				//                {
				// Failed, so clean up
				//                    lpIDC->lpVtbl->Close(lpIDC);
				//                    lpIDC->lpVtbl->Release(lpIDC);
				//                    lpIDC = NULL;
				//                    EndDialog(hDlg, FALSE);
				//                }

				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
		}
	}
	return (FALSE);
}

void find_tcp_ipx() {

	DirectPlayEnumerate(EnumSPNEW, (LPVOID)hWindow);
}

BOOL FAR PASCAL EnumSPNEW(LPGUID lpGd, LPSTR lpDesc, DWORD dwMajorVersion,
                          DWORD dwMinorVersion, LPVOID lpv) {
	HWND hWnd = (HWND)lpv;

	char a[50];
	char *p;

	p = &a[0];

	// Add the service provider to the listbox
	//    iIndex = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) lpDesc);
	//    if (iIndex != LB_ERR) {
	//        SendMessage(hWnd, LB_SETITEMDATA, iIndex, (LPARAM) lpGuid);
	//		Globalid = (DWORD) lpGuid;

	//	}

	if (networktype == 1)
		strcpy(a, "TCP");
	if (networktype == 2)
		strcpy(a, "IPX");

	if (strstr(lpDesc, p) != NULL) {

		lpGuid = lpGd;
	}

	return (TRUE);
}

BOOL CALLBACK DlgNetwork(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	char keyresult[100];

	strcpy(keyresult, "1");
	// This handles the messages from the Create/Connect dialog box

	static HWND hWndCtl;

	switch (msg) {

	case WM_INITDIALOG:

		if (networktype == 1) {
			hWndCtl = GetDlgItem(hDlg, IDC_RADIO1);
			SendMessage(hWndCtl, BM_SETCHECK, 1, 0);
		} else {
			hWndCtl = GetDlgItem(hDlg, IDC_RADIO2);
			SendMessage(hWndCtl, BM_SETCHECK, 1, 0);
		}

		break;

	case WM_COMMAND:
		switch (wParam) {
		case IDC_OK:

			EndDialog(hDlg, -1);
			return (TRUE);
			break;
		case IDC_CANCEL:

			EndDialog(hDlg, -1);
			return (TRUE);
			break;
		case IDC_RADIO1:

			save_server_settings(1);
			networktype = 1;
			break;
		case IDC_RADIO2:

			save_server_settings(2);
			networktype = 2;
			break;
		}
		break;
	}
	return (FALSE);
}

BOOL CALLBACK DlgRegister(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	CRegistry Registry;
	CString Path;
	CString DefaultTitle;
	char buffer[100];
	int result;
	static HWND hWndCtl;
	// This handles the messages from the Create/Connect dialog box
	switch (msg) {

	case WM_INITDIALOG:

		ReadMurkReg();

		result = SetDlgItemText(hDlg, IDC_EDIT1, murkreg->name);
		result = SetDlgItemText(hDlg, IDC_EDIT2, murkreg->key);

		if (murkreg->registered)
			result = SetDlgItemText(hDlg, IDC_EDIT3, "REGISTERED");
		else
			result = SetDlgItemText(hDlg, IDC_EDIT3, "PLEASE REGISTER");

		if (murkreg->sound == 1) {
			hWndCtl = GetDlgItem(hDlg, IDC_RADIO1);
			SendMessage(hWndCtl, BM_SETCHECK, 1, 0);
		}

		if (murkreg->joystick == 1) {
			hWndCtl = GetDlgItem(hDlg, IDC_RADIO2);
			SendMessage(hWndCtl, BM_SETCHECK, 1, 0);
		}

		break;

	case WM_COMMAND:
		switch (wParam) {

		case IDC_OK:

			result = GetDlgItemText(hDlg, IDC_EDIT1, &buffer[0], 50);

			strcpy(murkreg->name, buffer);
			result = GetDlgItemText(hDlg, IDC_EDIT2, &buffer[0], 50);
			strcpy(murkreg->key, buffer);
			SaveMurkReg();

			EndDialog(hDlg, -1);
			return (TRUE);
			break;
		case IDC_CANCEL:

			EndDialog(hDlg, -1);
			return (TRUE);
			break;
		case IDC_RADIO1:

			if (usesound == 1) {

				usesound = 0;
				stop_song();
				m_directSoundOK = FALSE;
				murkreg->sound = 0;
			} else {

				usesound = 1;

				murkreg->sound = 1;
				m_directSoundOK = TRUE;
				play_random_music();
			}
			SaveMurkReg();

			break;
		case IDC_RADIO2:

			Path = "SOFTWARE\\Murk\\";

			//		  save_server_settings(2) ;

			if (usejstick == 1) {
				usejstick = 0;
				joyReleaseCapture(m_uiDevID);
				murkreg->joystick = 0;

			} else {
				usejstick = 1;
				murkreg->joystick = 1;
				joySetCapture(hWindow, m_uiDevID, NULL, TRUE);
			}

			SaveMurkReg();
			break;
		}
		break;
	}
	return (FALSE);
}

void RegCrypt(char *user, char *key, char *result) {

	// Eric's Bizzare Encryption Method - EBEM encryption

	char encstring[1024], string1[1024], string2[1024];
	char encstring2[1024];
	int encstringlen;
	char resultstring[17];
	long int i, c, d;

	//	strcpy(encstring,"023456789ABCDEFGHJKMNOPQRSTUVWXYZabcdefghjkmnopqrstuvwxyz");

	strcpy(encstring, "0KMabcdrst0234WXYefghuDEvwxdVWXYZabcdefghjkmnopqrstuvwxyz");
	strcpy(encstring2, "BCNOPQkmnopq567BCNOPQkmnopq567BCNOPQkmnopq567ZFRSTUV8jGH");
	strcpy(encstring, "KMabcdrst0234WXYefghuDEvwxyzAJ9ZFRSTUVBCNOPQkmnopq5678jGH");
	strcpy(encstring2, "XYZ023456789ABCDETUVWabcdFGHJKMNOPQRSefghjkmnopqrstuvwxyz");
	strcpy(encstring2, "vwxyWXYefghuDEzAJKMabcdrst02349ZFRSTUVBCNOPQkmnopq5678jGH");
	strcpy(encstring2, "023456789ABCDEFGHJKMNOPQRSTUVWXYZabcdefghjkmnopqrstuvwxyz");
	strcpy(encstring, "XYZ023456789ABCDETUVWabcdFGHJKMNOPQRSefghjkmnopqrstuvwxyz");
	strcpy(encstring, "vwxyWXYefghuDEzAJKMabcdrst02349ZFRSTUVBCNOPQkmnopq5678jGH");
	strcpy(encstring, "023456789ABCDEFGHJKMNOPQRSTUVWXYZabcdefghjkmnopqrstuvwxyz");
	strcpy(encstring, "WXYefghuDEvwxyzAJKMabcdrst02349ZFRSTUVBCNOPQkmnopq5678jGH");
	strcpy(encstring, "op02345RST6789ABCDEghjkmnqrstFGHJKMNOPQUVWXYZabcdefuvwxyz");
	strcpy(encstring, "KMaWXYefghuDEvwxyzAJbcdrst02ZF34ST9RUVBCNOPQkmnopq5678jGH");
	strcpy(encstring, "QRSTUVWXYZabcdef023NOPghjkmnopq456789ABCDEFGHJKMrstuvwxyz");
	strcpy(encstring2, "wxyz89ABCD0234567fghjkmEFGHJKMNOPQRSnopqrstuvTUVWXYZabcde");
	strcpy(encstring2, "WXYFRSTUVBCNOPQkefghuDEvwxyzAJKMabcdrst02349Zmnopq5678jGH");
	strcpy(encstring, "wxyz89ABCD0234567fghjkmEFGHJKMNOPQRSnopqrstuvTUVWXYZabcde");
	strcpy(encstring, "WXYFRSTUVBCNOPQkefghuDEvwxyzAJKMabcdrst02349Zmnopq5678jGH");
	strcpy(encstring2, "JKMabcdrst02349ZFRSTUVWXYefghuDEvwxyzABCNOPQkmnopq5678jGH");
	strcpy(encstring, "JKMaTUVWXYefghuDopq5678jGHEvwxyzABbcdrst02349ZFRSCNOPQkmn");
	strcpy(encstring, "JKMabcdrst02349ZFRSTUVWXYefghuDEvwxyzABCNOPQkmnopq5678jGH");
	strcpy(encstring2, "WXYefghuDEvwxyzBCNOPQkmnopq5678jGHAJKMabcdrst02349ZFRSTUV");
	strcpy(encstring2, "op02345MNOPQUVWXYZabcdRST6789ABCDEghjkmnqrstFGHJKefuvwxyz");
	strcpy(encstring2, "KMaWXYefghuDEvwxyzAJbcNOPQkmnopqdrst02ZF34ST9RUVBC5678jGH");
	strcpy(encstring2, "QRSWXYZabcdef023NOPghjkmnopq456789ATUVBCDEFGHJKMrstuvwxyz");

	encstringlen = strlen(encstring);

	strcpy(resultstring, "               ");
	d = 0;
	strcpy(string1, user);
	strcpy(string2, key);
	for (i = 0; i < (long int)strlen(string1); i++)
		d = d + string1[i];
	for (i = 0; i < (long int)strlen(string2); i++)
		d = d + string2[i];
	for (i = 0; i < 16; i++) {
		c = encstring[i] + d;
		if (i < (long int)strlen(string1))
			c = c + string1[i];
		if (i < (long int)strlen(string2))
			c = c + string2[i];
		resultstring[i] = encstring[(c % encstringlen)];
		d = c;
	}
	resultstring[16] = 0;
	strcpy(result, resultstring);
}

void write_reg() {

	CRegistry Registry;
	CString Path;
	CString DefaultTitle;
	Path = "SOFTWARE\\Murk\\";
	if (!Registry.VerifyKey(HKEY_LOCAL_MACHINE, Path)) {
		// Check to see if the key exists
		// if it doesn’t then create it
		Registry.CreateKey(HKEY_LOCAL_MACHINE, Path);
		Registry.Open(HKEY_LOCAL_MACHINE, Path); // Open the Registry
		Registry.Write("Name", "");              // Write the default title to the Registry
		Registry.Write("Sound", "1");            // Write the default title to the Registry
		Registry.Write("Joystick", "0");         // Write the default title to the Registry
		Registry.Write("Key", "");               // Write the default title to the Registry
		Registry.Write("Version", "1.0");        // Write the default title to the Registry
		Registry.Close();                        // Close the Registry
	}

	if (!Registry.Read2("Joystick", DefaultTitle)) {
		Registry.Open(HKEY_LOCAL_MACHINE, Path); // Open the Registry
		Registry.Write("Sound", "1");            // Write the default title to the Registry
		Registry.Write("Joystick", "0");         // Write the default title to the Registry
		Registry.Close();                        // Close the Registry
	}
	Registry.Open(HKEY_LOCAL_MACHINE, Path); // Open the Registry
	Registry.Write("Version", "1.12");       // Write the default title to the Registry
	Registry.Close();                        // Close the Registry
}

bool WriteMurkReg() {

	char junk[25];
	HKEY hkey;
	DWORD dwDisposition;
	long hresult;

	strcpy(junk, "test");

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Calamity\\Murk"), 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS) {
		RegCloseKey(hkey);
		ReadMurkReg();

	} else {
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Calamity\\Murk"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) == ERROR_SUCCESS) {

			strcpy(murkreg->name, "DEMO");
			strcpy(murkreg->key, "DEMO");
			strcpy(murkreg->version, "1.12");
			murkreg->sound = 1;
			murkreg->joystick = 0;

			usejstick = 0;
			usesound = 1;

			hresult = RegSetValueEx(hkey, "Name", 0, REG_SZ, (PBYTE)LPCTSTR(murkreg->name), strlen(murkreg->name));
			hresult = RegSetValueEx(hkey, "Key", 0, REG_SZ, (PBYTE)LPCTSTR(murkreg->key), strlen(murkreg->key));
			hresult = RegSetValueEx(hkey, "Sound", 0, REG_SZ, (PBYTE)LPCTSTR("1"), strlen("1"));
			hresult = RegSetValueEx(hkey, "Joystick", 0, REG_SZ, (PBYTE)LPCTSTR("0"), strlen("0"));
			hresult = RegSetValueEx(hkey, "Version", 0, REG_SZ, (PBYTE)LPCTSTR(murkreg->version), strlen(murkreg->version));

			RegCloseKey(hkey);
		}
	}
	return 1;
}

bool SaveMurkReg() {

	char junk[50];
	HKEY hkey;
	DWORD dwDisposition;
	long hresult;

	strcpy(junk, "test");

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Calamity\\Murk"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) == ERROR_SUCCESS) {
		hresult = RegSetValueEx(hkey, "Name", 0, REG_SZ, (PBYTE)LPCTSTR(murkreg->name), strlen(murkreg->name));
		hresult = RegSetValueEx(hkey, "Key", 0, REG_SZ, (PBYTE)LPCTSTR(murkreg->key), strlen(murkreg->key));

		sprintf(junk, "%d", murkreg->sound);
		hresult = RegSetValueEx(hkey, "Sound", 0, REG_SZ, (PBYTE)LPCTSTR(junk), strlen(junk));

		sprintf(junk, "%d", murkreg->joystick);
		hresult = RegSetValueEx(hkey, "Joystick", 0, REG_SZ, (PBYTE)LPCTSTR(junk), strlen(junk));
		hresult = RegSetValueEx(hkey, "Version", 0, REG_SZ, (PBYTE)LPCTSTR(murkreg->version), strlen(murkreg->version));
		RegCloseKey(hkey);
	}

	return 1;
}

bool ReadMurkReg() {

	char junk[4255];
	HKEY hkey;
	DWORD dwType;
	DWORD dwSize;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Calamity\\Murk"), 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS) {

		dwSize = sizeof(junk);
		dwType = REG_SZ;
		RegQueryValueEx(hkey, "Name", NULL, &dwType, (BYTE *)&junk, &dwSize);

		strcpy(murkreg->name, junk);

		dwSize = sizeof(junk);
		dwType = REG_SZ;
		RegQueryValueEx(hkey, "key", NULL, &dwType, (BYTE *)&junk, &dwSize);

		strcpy(murkreg->key, junk);

		dwSize = sizeof(junk);
		dwType = REG_SZ;
		RegQueryValueEx(hkey, "version", NULL, &dwType, (BYTE *)&junk, &dwSize);

		strcpy(murkreg->version, junk);

		dwSize = sizeof(junk);
		dwType = REG_SZ;
		RegQueryValueEx(hkey, "sound", NULL, &dwType, (BYTE *)&junk, &dwSize);

		if (strcmp(junk, "1") == 0) {
			murkreg->sound = 1;

			usesound = 1;

		} else {
			murkreg->sound = 0;
			usesound = 0;
		}

		dwSize = sizeof(junk);
		dwType = REG_SZ;
		RegQueryValueEx(hkey, "joystick", NULL, &dwType, (BYTE *)&junk, &dwSize);

		if (strcmp(junk, "1") == 0) {
			murkreg->joystick = 1;
			usejstick = 1;
		} else {
			murkreg->joystick = 0;
			usejstick = 0;
		}

		IsShareware();
	}

	return 1;
}

bool IsShareware() {

	char *p;
	char keyresult[100];
	// This handles the messages from the Create/Connect dialog box

	p = &keyresult[0];
	RegCrypt(murkreg->name, "Murk", p);
	debug_me("KEY", p, 0, 0);
	if (strcmp(murkreg->key, p) == 0) {
		shareware = 0;
		murkreg->registered = 1;
		return TRUE;

	} else {
		shareware = 0;
		murkreg->registered = 1;
	}
	return FALSE;
}
