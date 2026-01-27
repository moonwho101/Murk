//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserved.
//
//  Here lies the guts of Murk.
//  The display_dungeon() function is called by the timer.  It is our main function.
//

// COMPILE NOTES

// Make sure your Tools->Options Directories is set to where your murk
// source is for BOTH libaries and includes.  It must search this
// directory first.  We need DX5 libs as murk runs on NT/95/98 by only
// using DIRECTX 3.0 objects.

#include "stdafx.h"
#include "murkdecl.h"
#include "murk.h"
#include <stdlib.h>
#include <crtdbg.h>

void check_leftmousedown();
void check_rightmousedown();

extern BOOL InitMIDI();
extern UINT m_uiDevID;

int keyboardlock;

int readyornot;

extern int joystickhit;
extern int usejstick;

extern int xmin;
extern int xmax;
extern int ymin;
extern int ymax;

int compserv;
int serverbusy = 0;
int controlsinuse;
int mousestate;

char serverscreen[100];

BOOL check_joystick(int fire);
void check_input();
void check_keys();
void check_mouse();

///////////////////////////////////////////////////////////
// WinMain
///////////////////////////////////////////////////////////

extern int networktype;
extern void send_pinghost();
extern void serversettings(int go);
extern void build_server_screen();
extern void kill_dplay();
extern void send_pingserver();
extern int is_shareware();
extern int joystickhit;

extern BOOL joy_button(UINT Button);
extern void play_random_music();
extern void stop_song();
extern void CleanupMIDI();
extern void write_reg();
extern BOOL InitJoystick();
extern LONG OnJoyMove(UINT wParam, LONG lParam);
extern LONG OnJoyButtonDown(UINT wParam, LONG lParam);
extern void send_drop_treasure(int treasure);

extern bool WriteMurkReg();
extern bool SaveMurkReg();
extern bool ReadMurkReg();
extern bool IsShareware();

#define _CRTDBG_MAP_ALLOC

int APIENTRY WinMain(HINSTANCE hCurrentInst,
                     HINSTANCE hPreviousInst, LPSTR lpszCmdLine,
                     int nCmdShow) {

	WNDCLASS wndClass;
	HWND hWnd;
	MSG msg;
	_CrtSetDbgFlag(
	    _CRTDBG_ALLOC_MEM_DF |
	    _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR,
	                  _CRTDBG_MODE_DEBUG);

	CDialog dialog;

	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hCurrentInst;
	wndClass.hIcon = LoadIcon(hCurrentInst, MAKEINTRESOURCE(IDI_ICON3));
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	readyornot = 0;

	// IMPORTANT!!  Change compserv=1 to compile MURK into a windowed server
	// compserv=0 while make the game run with graphics

	compserv = 0; // client
	//	compserv=1;  //server
	//
	if (strstr(lpszCmdLine, "/s") != NULL || compserv) {
		wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	} else {
		wndClass.hbrBackground = NULL;
	}

	wndClass.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN);
	wndClass.lpszClassName = "Aztec";
	bIsActive = FALSE;

	RegisterClass(&wndClass);

	globalthread = NULL;
	quitthread = NULL;
	networkserver = 0;
	networkgame = 0;

	if (strstr(lpszCmdLine, "/s") != NULL || compserv) {
		networkserver = 1;
		networkgame = 1;
	}

	if (networkserver) {
		timerlength = 60;
	} else {
		timerlength = 24;
	}

	hInstance = hCurrentInst;

	if (!networkserver) {
		// Create a window of the previously defined class.
		hWnd = CreateWindow(
		    "Aztec",         // Window class's name.
		    "Murk Ver 1.12", // Title bar text.
		    WS_POPUP,        // The window's style.
		    0,               // X position.
		    0,               // Y position.
		    240,             // Width.
		    280,             // Height.
		    NULL,            // Parent window's handle.
		    NULL,            // Menu handle.
		    hCurrentInst,    // Instance handle.
		    NULL);           // No additional data.

	} else {

		hWnd = CreateWindow(
		    "Aztec",             // Window class's name.
		    "Murk Ver 1.12",     // Title bar text.
		    WS_OVERLAPPEDWINDOW, // The window's style.
		    0,                   // X position.
		    0,                   // Y position.
		    240,                 // Width.
		    280,                 // Height.
		    NULL,                // Parent window's handle.
		    NULL,                // Menu handle.
		    hCurrentInst,        // Instance handle.
		    NULL);               // No additional data.
	}

	SetMenu(hWnd, NULL);

	clear_log();
	strcpy(serverscreen, "Welcome to Murk...");
	// Display the window on the screen.
	networktype = 0;

	ShowWindow(hWnd, nCmdShow);

	// Force the window to repaint itself.
	UpdateWindow(hWnd);

	// Save the window handle.
	hWindow = hWnd;
	DUNGEONX = 18;
	DUNGEONY = 18;

	SystemParametersInfo(SPI_SETKEYBOARDSPEED, 31,
	                     (LPVOID)NULL, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);

	SystemParametersInfo(SPI_SETKEYBOARDDELAY, 0,
	                     (LPVOID)NULL, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);

	if (!networkserver) {
		setupOK = InitGraphics();
		if (!setupOK)
			return 0;
	}

	if (!networkserver) {
		BOOL resultOk = pDirDraw->SetTransparentColors(partsASurfaceNum, 255, 255);
		resultOk = pDirDraw->SetTransparentColors(partsBSurfaceNum, 255, 255);
		resultOk = pDirDraw->SetTransparentColors(panelSurfaceNum, 255, 255);
		resultOk = pDirDraw->SetTransparentColors(scroll, 255, 255);
		resultOk = pDirDraw->SetTransparentColors(selectplayer, 223, 223);
		resultOk = pDirDraw->SetTransparentColors(DungeonSurface2, 255, 255);
		resultOk = pDirDraw->SetTransparentColors(DungeonSurface3, 255, 255);
		load_panel();
		play_sound();
		m_directSoundOK = TRUE;
	} else {
		m_directSoundOK = FALSE;
	}

	if (m_directSoundOK) {
		// InitMIDI();
	}

	shareware = 0;
	sharewarelevel = 1;

	if (m_directSoundOK) {
		m_pDirSound->PlaySound(m_bufferMusic);
		play_random_music();
	}

	countlevels = 0;
	levelcounter = 1;

	abortgame = 0;

	receiveok = 0;
	istimerset = 0;
	m_pDirectPlay = NULL;
	m_hDPEvent = NULL;
	ishost = 1;
	countsquares = 0;
	stopthetime = 0;
	cronit = 0;
	sendonce = FALSE;
	items = 0;
	initplayer = 0;
	showmission = 0;
	camera = 1;
	maxlevel = 4;
	newwarrior = 1;
	NumPlayers = 10;
	setability = 1;
	dropt = 1;
	startthegame = 1;
	select1 = 0;
	select2 = 0;
	select3 = 0;
	select4 = 0;
	bdown = 0;

	stack = (mazestack *)calloc(56 * 56, sizeof(mazestack));

	movelevel = 0;
	advancelevel = 50;
	advanceattrib = 0;
	chatin = 0;
	chatcurrent = 0;
	gamescreen = 0;
	new_level(0);
	chaton = 0;
	hitanykey = 0;
	oktosend = 1;
	helpscreen = 0;
	joystickhit = 0;

	if (networkserver) {
		serversettings(1);
	} else {
		SetTimer(hWindow, 1, 50, TimerProc);
		SetTimer(hWindow, 2, 4000, TimerProc);
	}

	keyboardlock = 0;
	readyornot = 1;
	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (!GetMessage(&msg, NULL, 0, 0)) {
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

///////////////////////////////////////////////////////////
// WndProc
///////////////////////////////////////////////////////////

LRESULT APIENTRY WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
	// Handle the messages to which the application
	// must respond.

	HDC hDC;
	PAINTSTRUCT paintStruct;
	int i = 0;

	switch (message) {

	case WM_PAINT:
		if (networkserver) {

			hDC = BeginPaint(hWnd, &paintStruct);
			if (networktype == 0) {
				strcpy(serverscreen, "Welcome to Murk...");
				TextOut(hDC, 1, 1, serverscreen, strlen(serverscreen));
			}
			if (networktype == 1 || networktype == 2) {
				if (networktype == 1) {
					strcpy(serverscreen, "Murk TCP/IP Server (Active)");
					TextOut(hDC, 1, 1, serverscreen, strlen(serverscreen));
				} else {
					strcpy(serverscreen, "Murk IPX/SPX Server (Active)");
					TextOut(hDC, 1, 1, serverscreen, strlen(serverscreen));
				}

				for (i = 1; i <= newwarrior; i++) {
					switch (PlayerLocation[i].image) {
					case 1:
						// barbarian
						if (PlayerLocation[i].active) {
							strcpy(serverscreen, "Barbarian is connected");
						} else
							strcpy(serverscreen, "                           ");
						TextOut(hDC, 1, 20, serverscreen, strlen(serverscreen));
						break;
					case 3:
						// barbarian
						if (PlayerLocation[i].active) {
							strcpy(serverscreen, "Thief is connected");
						} else
							strcpy(serverscreen, "                           ");

						TextOut(hDC, 1, 40, serverscreen, strlen(serverscreen));
						break;
					case 4:
						// barbarian
						if (PlayerLocation[i].active) {
							strcpy(serverscreen, "Mage is connected");
						} else
							strcpy(serverscreen, "                           ");

						TextOut(hDC, 1, 60, serverscreen, strlen(serverscreen));
						break;
					case 5:
						// barbarian
						if (PlayerLocation[i].active) {
							strcpy(serverscreen, "Cleric is connected");
						} else
							strcpy(serverscreen, "                           ");

						TextOut(hDC, 1, 80, serverscreen, strlen(serverscreen));
						break;
					}
				}
			}

			EndPaint(hWnd, &paintStruct);
			return 0;
		}
		break;

	case WM_COMMAND:
		return PerformMenuCommand(hWnd, wParam);
	case WM_CHAR:
		if (!networkserver)
			OnCharDown(wParam);
		return 0;
	case WM_KEYDOWN:
		if (!networkserver)
			OnKeyDown(wParam);
		return 0;
	case WM_DESTROY:
		OnDestroy();
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		OnTimer();
		return 0;
	case WM_LBUTTONDOWN:

		if (showmission != 3) {

			if (!networkserver)
				OnLButtonDown(lParam);
			return 0;
		}

		break;
	case WM_LBUTTONUP:

		if (showmission != 3) {
			if (!networkserver)
				OnLButtonUp(lParam);
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
		if (!networkserver)
			OnLButtonDblClk(lParam);
		return 0;
	case WM_ACTIVATEAPP:
		if (wParam) {
			// active

			if (showmission == 3 && !networkserver) {
				display_dungeon(1);
				SetTimer(hWindow, 3, timerlength, TimerProc);
				SetTimer(hWindow, 2, 4000, TimerProc);
				if (m_directSoundOK)
					play_random_music();

			} else if (readyornot == 1 && !networkserver) {
				SetTimer(hWindow, 1, 50, TimerProc);
				SetTimer(hWindow, 2, 4000, TimerProc);
				display_dungeon(1);

				if (m_directSoundOK)
					play_random_music();
			}

		} else {
			// deactivate

			if (showmission == 3 && !networkserver) {
				KillTimer(hWindow, 3);
				KillTimer(hWindow, 2);
				if (m_directSoundOK)
					stop_song();

			} else if (readyornot == 1 && !networkserver) {
				KillTimer(hWindow, 1);
				KillTimer(hWindow, 2);

				if (m_directSoundOK)
					stop_song();
			}
		}

		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////
// OnDestroy
///////////////////////////////////////////////////////////

void OnDestroy() {
	if (showmission == 3) {

		KillTimer(hWindow, 3);
	}
	if (showmission == 0 || showmission == 1 || showmission == 2) {

		KillTimer(hWindow, 1);
	}
	KillTimer(hWindow, 2);

	kill_dplay();
	CleanupMIDI();
	delete pDirDraw;
	delete m_pDirSound;
	delete m_pWave;
	delete m_pMusic;
	delete m_pDrip;
	delete m_pScream;
	delete m_pWin;
	delete m_pTreasure;
	delete m_pDead;
	delete m_pReward;
	delete m_pHit;
	delete m_pYell;
	delete m_pSword;
	delete m_pBow;
	delete m_pMonsterdie;
	delete m_pMumble;
	delete m_pDragon;
	delete m_pRegen;
	free(stack);
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void ShowPlayer(int animate) {
	BOOL resultOK;
	int framey, framex;
	int frame2y, frame2x;
	int deadadjust = 0;
	int deadadjust2 = 0;

	switch (PlayerLocation[CurrentPlayer].direction) {
	case 1:
		framex = 0;
		framey = 0;
		frame2x = 63;
		frame2y = 63;
		break;
	case 2:
		framex = 384;
		framey = 0;
		frame2x = 447;
		frame2y = 63;
		break;
	case 3:
		framex = 0;
		framey = 64;
		frame2x = 63;
		frame2y = 127;
		break;
	case 4:
		framex = 384;
		framey = 64;
		frame2x = 447;
		frame2y = 127;
		break;
	}

	if (PlayerLocation[CurrentPlayer].character == 3) {
		if (PlayerLocation[CurrentPlayer].image == 9) {
			framex = framex + 64;
			frame2x = frame2x + 64;
			if (animate != 2)
				animate = 0;
		}
		if (PlayerLocation[CurrentPlayer].image == 2) {
			if (animate != 2)
				animate = 0;
		}
	}

	switch (PlayerLocation[CurrentPlayer].image) {
	case 2:

		framey = framey + 128;
		frame2y = frame2y + 128;
		break;
	case 3:
		framey = framey + 256;
		frame2y = frame2y + 256;
		break;
	case 4:
		framey = framey + 384;
		frame2y = frame2y + 384;
		break;
	case 5:
		framey = framey + 512;
		frame2y = frame2y + 512;
		break;
	case 6:
		framey = framey + 768;
		frame2y = frame2y + 768;
		break;

	case 7:
		framey = framey + 896;
		frame2y = frame2y + 896;
		break;

	case 8:
		framey = framey + 1024;
		frame2y = frame2y + 1024;
		break;
	case 9:

		framey = framey + 128;
		frame2y = frame2y + 128;
		break;
	}

	if (PlayerLocation[CurrentPlayer].dead <= 64 && PlayerLocation[CurrentPlayer].dead > 1) {
		deadadjust = PlayerLocation[CurrentPlayer].dead - 5;
		deadadjust2 = PlayerLocation[CurrentPlayer].dead;
	}

	if (animateon == 1 && animate == 1) {
		framex = PlayerLocation[CurrentPlayer].fx;
		framey = PlayerLocation[CurrentPlayer].fy;
		frame2x = PlayerLocation[CurrentPlayer].fx2;
		frame2y = PlayerLocation[CurrentPlayer].fy2;
	}

	if (animate == 2) {
		// death scene
		if (!networkserver)
			resultOK = pDirDraw->BlitImage(&CPoint(dscene[CurrentPlayer].x, dscene[CurrentPlayer].y + (64 - dscene[CurrentPlayer].counter)), partsASurfaceNum,
			                               &CRect(framex, framey, frame2x, frame2y - (64 - dscene[CurrentPlayer].counter)));

		return;
	}

	if (!networkserver)
		resultOK = pDirDraw->BlitImage(&CPoint(PlayerLocation[CurrentPlayer].x, PlayerLocation[CurrentPlayer].y + deadadjust), partsASurfaceNum,
		                               &CRect(framex, framey, frame2x, frame2y - deadadjust2));
	check_dungeon();
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void animate_player(int startpos) {

	int framey, framex;
	int frame2y, frame2x, adjust;
	int move1, move2;

	move1 = 0;
	move2 = 0;
	framex = 0;
	framey = 0;
	frame2x = 0;
	frame2y = 0;
	adjust = 0;

	switch (frame_num) {
	case 1:
		adjust = 64;
		break;
	case 2:
		adjust = 128;
		break;
	case 3:
		adjust = 192;
		break;
	case 4:
		adjust = 256;
		break;
	case 5:
		adjust = 320;
		break;
	}

	if (PlayerLocation[CurrentPlayer].character == 3) {
		if (PlayerLocation[CurrentPlayer].image == 9)
			adjust = 0;
		if (PlayerLocation[CurrentPlayer].image == 2)
			adjust = 64;
	}

	switch (PlayerLocation[CurrentPlayer].direction) {
	case 1:
		framex = 0 + adjust;
		framey = 0;
		frame2x = 63 + adjust;
		frame2y = 63;
		move1 = -1;
		move2 = -1;
		break;
	case 2:
		framex = 384 + adjust;
		framey = 0;
		frame2x = 447 + adjust;
		frame2y = 63;
		move1 = +1;
		move2 = -1;
		break;
	case 3:
		framex = 0 + adjust;
		framey = 64;
		frame2x = 63 + adjust;
		frame2y = 127;
		move1 = -1;
		move2 = +1;
		break;
	case 4:
		framex = 384 + adjust;
		framey = 64;
		frame2x = 447 + adjust;
		frame2y = 127;
		move1 = +1;
		move2 = +1;
		break;
	}

	switch (PlayerLocation[CurrentPlayer].image) {
	case 2:
		framey = framey + 128;
		frame2y = frame2y + 128;
		break;
	case 3:
		framey = framey + 256;
		frame2y = frame2y + 256;
		break;
	case 4:
		framey = framey + 384;
		frame2y = frame2y + 384;
		break;
	case 5:
		framey = framey + 512;
		frame2y = frame2y + 512;
		break;
	case 6:
		framey = framey + 768;
		frame2y = frame2y + 768;
		break;
	case 7:
		framey = framey + 896;
		frame2y = frame2y + 896;
		break;
	case 8:
		framey = framey + 1024;
		frame2y = frame2y + 1024;
		break;
	case 9:
		framey = framey + 128;
		frame2y = frame2y + 128;
		break;
	}

	fx = framex;
	fy = framey;
	fx2 = frame2x;
	fy2 = frame2y;

	if (ishost || networkgame == 0 || CurrentPlayer == 1) {
		PlayerLocation[CurrentPlayer].fx = framex;
		PlayerLocation[CurrentPlayer].fy = framey;
		PlayerLocation[CurrentPlayer].fx2 = frame2x;
		PlayerLocation[CurrentPlayer].fy2 = frame2y;
		if (PlayerLocation[CurrentPlayer].stairs != 0) {
			if (PlayerLocation[CurrentPlayer].stairs == 1) {
				PlayerLocation[CurrentPlayer].y = PlayerLocation[CurrentPlayer].y + 2;
			} else {
				PlayerLocation[CurrentPlayer].y = PlayerLocation[CurrentPlayer].y - 2;
			}
		} else {
			PlayerLocation[CurrentPlayer].x = PlayerLocation[CurrentPlayer].x + move1 * 8;
			PlayerLocation[CurrentPlayer].y = PlayerLocation[CurrentPlayer].y + move2 * 4;
		}
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void OnTimer() {

	if (frame_num == 1 || frame_num == 4) {
		if (m_directSoundOK && CurrentPlayer == 1)
			m_pDirSound->PlaySound(m_bufferNum);
	}

	animateon = 1;
	animate_player(1);

	if (frame_num == 3 && PlayerLocation[CurrentPlayer].framehuman == 0) {
		if (PlayerLocation[CurrentPlayer].stairs == 0) {
			make_your_move();
		}
		savelevel = PlayerLocation[CurrentPlayer].level;
		wakeup_sleepy_heads();
		check_dungeon();
	}

	if (CurrentPlayer == 1 && PlayerLocation[CurrentPlayer].stairs == 0) {
		PlayerLocation[1].framehuman = 0;
	}

	frame_num++;

	//if (PlayerLocation[CurrentPlayer].stairs > 0 && CurrentPlayer == 1) {
	//	//			PlayerLocation[CurrentPlayer].staircounter=0;
	//	PlayerLocation[CurrentPlayer].staircounter--;
	//	if (PlayerLocation[CurrentPlayer].staircounter == 3) {

	//		if (PlayerLocation[CurrentPlayer].stairs == 1) {
	//			PlayerLocation[CurrentPlayer].level++;
	//			PlayerLocation[CurrentPlayer].track = 0;

	//			PlayerLocation[CurrentPlayer].y = PlayerLocation[CurrentPlayer].y - 12;
	//		} else if (PlayerLocation[CurrentPlayer].stairs == 2) {
	//			if (PlayerLocation[CurrentPlayer].level > 1)
	//				PlayerLocation[CurrentPlayer].level--;

	//			PlayerLocation[CurrentPlayer].track = 0;
	//			PlayerLocation[CurrentPlayer].y = PlayerLocation[CurrentPlayer].y + 12;
	//		}
	//		wakeup_sleepy_heads();
	//	} else if (PlayerLocation[CurrentPlayer].staircounter <= 0) {
	//		PlayerLocation[CurrentPlayer].staircounter = 0;
	//		frame_num = 0;
	//		animateon = 0;
	//		PlayerLocation[1].frame = frame_num;
	//		PlayerLocation[1].framehuman = 0;
	//		PlayerLocation[CurrentPlayer].stairs = 0;
	//		wakeup_sleepy_heads();
	//		check_dungeon();

	//	} else
	//		PlayerLocation[1].framehuman = 1;

	//	return;
	//}
	if (frame_num == 3) {
		if (PlayerLocation[CurrentPlayer].stairs > 0) {
		}
		if (PlayerLocation[CurrentPlayer].stairs == 1) {
			PlayerLocation[CurrentPlayer].level++;
			PlayerLocation[CurrentPlayer].track = 0;
		} else if (PlayerLocation[CurrentPlayer].stairs == 2) {
			if (PlayerLocation[CurrentPlayer].level > 1)
				PlayerLocation[CurrentPlayer].level--;
			PlayerLocation[CurrentPlayer].track = 0;
		}
		check_dungeon();
		PlayerLocation[CurrentPlayer].frame = frame_num;
	}

	if (frame_num > 5) {
		frame_num = 0;
		animateon = 0;
		PlayerLocation[CurrentPlayer].stairs = 0;
		check_dungeon();
		PlayerLocation[CurrentPlayer].frame = frame_num;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

inline void display_dungeon(int player) {
	CONST int XADJUST = 49;
	CONST int YADJUST = 25;

	int x, y, startxreset, startyreset;

	BOOL mademove = FALSE;

	int level;
	int i;
	int work1;
	int work2;
	int lvlnum;
	char response[10];
	int final;
	int monstershoot;
	int restmonster;
	int loop;
	int rand;
	BOOL endgame;
	int recu1, recu2, recl1, recl2;
	int savecurrent;
	BOOL resultOK;

	endgame = FALSE;

	startx = 930;
	starty = -200;
	startxreset = startx;
	startyreset = starty - 100;

	startx = 530;
	starty = -100;
	startxreset = 530;
	startyreset = -200;

	if (!networkserver) {
		pDirDraw->ClearSurface(BACKBUFFERNUM);
		if (sharewarelevel >= 6 && shareware) {

			if (networkgame && !ishost)
				send_deleteme();
			//				KillTimer(hWindow,3);
			pDirDraw->FlipToGDISurface();
			UINT resultclick = MessageBox(hWindow, "Thank You for Playing Murk Shareware.\nPlease Register at http://www.murk.on.ca", "Murk Shareware", MB_OK);
			PostMessage(hWindow, WM_CLOSE, 0, 0L);
			return;
		}
	}

	if (abortgame == 1) {
		if (networkgame && !ishost)
			send_deleteme();

		//				KillTimer(hWindow,3);
		pDirDraw->FlipToGDISurface();
		UINT resultclick = MessageBox(hWindow, "You are running an OLD version.\nPlease Vist http://www.murk.on.ca\nFor the latest Version.", "Update Your Version\nA purchased Registration key will work on Updates.", MB_OK);
		PostMessage(hWindow, WM_CLOSE, 0, 0L);
		return;
	}

	level = PlayerLocation[camera].level;
	for (i = 1; i <= NumPlayers; i++)
		PlayerLocation[i].moved = 0;

	for (y = 0; y < DUNGEONY + 1; y++) {
		for (x = 0; x < DUNGEONX + 1; x++) {
			if (dungeon[x][y][level].type == 'f' && dungeon[x][y][level].explored == 1 || dungeon[x][y][level].type == 's' && dungeon[x][y][level].explored == 1) {
				lvlnum = level;
				strcpy(response, "");
				if (dungeon[x - 1][y][lvlnum].type == 'f' || dungeon[x - 1][y][lvlnum].type == 's') {
					strcat(response, "1");
				}
				if (dungeon[x][y - 1][lvlnum].type == 'f' || dungeon[x][y - 1][lvlnum].type == 's') {
					strcat(response, "2");
				}
				if (dungeon[x + 1][y][lvlnum].type == 'f' || dungeon[x + 1][y][lvlnum].type == 's') {
					strcat(response, "3");
				}
				if (dungeon[x][y + 1][lvlnum].type == 'f' || dungeon[x][y + 1][lvlnum].type == 's') {
					strcat(response, "4");
				}
				strcat(response, "\0");
				final = atoi(response);
				switch (final) {
				case 1:
					recu1 = 0;
					recu2 = 98;
					recl1 = 112;
					recl2 = 195;
					break;
				case 2:
					recu1 = 113;
					recu2 = 98;
					recl1 = 225;
					recl2 = 195;
					break;
				case 3:
					recu1 = 226;
					recu2 = 98;
					recl1 = 338;
					recl2 = 195;
					break;
				case 4:
					recu1 = 339;
					recu2 = 98;
					recl1 = 451;
					recl2 = 195;
					break;
				case 12:
					recu1 = 452;
					recu2 = 98;
					recl1 = 564;
					recl2 = 195;
					break;
				case 23:
					recu1 = 565;
					recu2 = 98;
					recl1 = 677;
					recl2 = 195;
					break;
				case 34:
					recu1 = 678;
					recu2 = 98;
					recl1 = 790;
					recl2 = 195;
					break;
				case 14:
					recu1 = 0;
					recu2 = 196;
					recl1 = 112;
					recl2 = 293;
					break;
				case 13:
					recu1 = 113;
					recu2 = 196;
					recl1 = 225;
					recl2 = 293;
					break;
				case 24:
					recu1 = 226;
					recu2 = 196;
					recl1 = 338;
					recl2 = 293;
					break;
				case 123:
					recu1 = 339;
					recu2 = 196;
					recl1 = 451;
					recl2 = 293;
					break;
				case 234:
					recu1 = 452;
					recu2 = 196;
					recl1 = 564;
					recl2 = 293;
					break;
				case 134:
					recu1 = 565;
					recu2 = 196;
					recl1 = 677;
					recl2 = 293;
					break;
				case 124:
					recu1 = 678;
					recu2 = 196;
					recl1 = 790;
					recl2 = 293;
					break;
				case 1234:
					recu1 = 0;
					recu2 = 0;
					recl1 = 112;
					recl2 = 97;
					break;
				}
				if (!networkserver) {

					switch (tile_type[PlayerLocation[1].level]) {

					case 1:
						resultOK = pDirDraw->BlitImage(&CPoint(startx, starty),
						                               partsBSurfaceNum, &CRect(recu1, recu2, recl1, recl2));
						break;

					case 2:
						resultOK = pDirDraw->BlitImage(&CPoint(startx, starty),
						                               DungeonSurface2, &CRect(recu1, recu2, recl1, recl2));
						break;
					case 3:
						resultOK = pDirDraw->BlitImage(&CPoint(startx, starty),
						                               DungeonSurface3, &CRect(recu1, recu2, recl1, recl2));
						break;
					case 4:
						resultOK = pDirDraw->BlitImage(&CPoint(startx, starty),
						                               DungeonSurface3, &CRect(recu1, recu2, recl1, recl2));
						break;
					}
				}
			}
			if (dungeon[x][y][level].type == 's' && dungeon[x][y][level].explored == 1) {
				if (dungeon[x][y][level].item == 1) {
					if (!networkserver)
						resultOK = pDirDraw->BlitImage(&CPoint(startx, starty),
						                               partsBSurfaceNum, &CRect(224, 0, 336, 97));
				} else {
					/*down*/
					if (!networkserver)
						resultOK = pDirDraw->BlitImage(&CPoint(startx, starty),
						                               partsBSurfaceNum, &CRect(112, 0, 224, 97));
				}
			}
			if (dungeon[x][y][level].item == 'c' && dungeon[x][y][level].explored == 1 && !networkserver) {
				if (dungeon[x][y][level].image == 0) {
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                                    &CRect(113, 294, 225, 391));
				} else if (dungeon[x][y][level].image == 1) {
					resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                               &CRect(113 + 112 + 112, 294, 225 + 112 + 112, 391));
				} else if (dungeon[x][y][level].image == 2) {
					resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                               &CRect(113 + 112 + 112 + 112, 294, 225 + 112 + 112 + 112, 391));
				} else if (dungeon[x][y][level].image == 3) {
					resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                               &CRect(113 + 112 + 112 + 112 + 112, 294, 225 + 112 + 112 + 112 + 112, 391));
				} else if (dungeon[x][y][level].image == 4) {
					resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                               &CRect(224, 294, 337, 391));
				}
			}

			if (dungeon[x][y][level].item == 'e' && dungeon[x][y][level].explored == 1) {
				if (!networkserver)
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                                    &CRect(226, 490, 338, 587));
			}
			if (dungeon[x][y][level].item == 's' && dungeon[x][y][level].explored == 1) {
				if (!networkserver)
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty), partsBSurfaceNum,
					                                    &CRect(565, 0, 565 + 113, 97));
			}
			if (dungeon[x][y][level].item == 'r' && dungeon[x][y][level].explored == 1) {
				if (!networkserver)
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty), partsBSurfaceNum,
					                                    &CRect(678, 0, 678 + 113, 97));
			}
			if (dungeon[x][y][level].item == 'a' && dungeon[x][y][level].explored == 1) {
				if (!networkserver)
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty), partsBSurfaceNum,
					                                    &CRect(224 + 113 + 113, 0, 449 + 113, 97));
			}
			if (x == treasurex && y == treasurey && level == treasurelvl && dungeon[x][y][level].explored == 1 && foundtreasure == 0) {
				if (!networkserver)
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                                    &CRect(0, 294, 112, 391));
			}
			if (x == treasurex2 && y == treasurey2 && level == treasurelvl2 && dungeon[x][y][level].explored == 1 && foundtreasure2 == 0) {
				if (!networkserver)
					BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
					                                    &CRect(678, 294, 790, 391));
			}
			for (i = 1; i <= 48; i++) {

				if (dungeon[x][y][level].explored) {
					if (pits[i].x == x &&
					    pits[i].y == y &&
					    pits[i].lvl == level &&
					    pits[i].frame != 0) {
						if (pits[i].frame <= 6 || pits[i].frame >= 30) {
							if (!networkserver)
								BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
								                                    &CRect(452, 490, 452 + 113, 490 + 98));
						} else if (pits[i].frame <= 12 || pits[i].frame >= 24) {
							if (!networkserver)
								BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
								                                    &CRect(452 + 113, 490, 452 + 113 + 113, 490 + 98));

						} else if (pits[i].frame <= 18 || pits[i].frame > 18) {
							if (!networkserver)
								BOOL resultOK = pDirDraw->BlitImage(&CPoint(startx, starty + 20), partsBSurfaceNum,
								                                    &CRect(452 + 113 + 113, 490, 452 + 113 + 113 + 113, 490 + 98));
						}
					}
				}
			}
			savecurrent = CurrentPlayer;
			savelevel = level;
			for (i = NumPlayers; i > 0; i--) {
				CurrentPlayer = i;
				if (foundtreasure != 0 && CurrentPlayer == foundtreasure && ishost) {
					treasurex = PlayerLocation[i].mapx;
					treasurey = PlayerLocation[i].mapy;
					treasurelvl = PlayerLocation[i].level;
				}
				if (foundtreasure2 != 0 && CurrentPlayer == foundtreasure2 && ishost) {
					treasurex2 = PlayerLocation[i].mapx;
					treasurey2 = PlayerLocation[i].mapy;
					treasurelvl2 = PlayerLocation[i].level;
				}
				if (PlayerLocation[i].mapx == x && PlayerLocation[i].mapy == y && PlayerLocation[i].asleep == 1 && PlayerLocation[i].moved == 0 && PlayerLocation[i].dead <= 64) {
					PlayerLocation[i].x = startx + 20;
					PlayerLocation[i].y = starty + 10;
				}

				else if (PlayerLocation[i].mapx == x && PlayerLocation[i].mapy == y && PlayerLocation[i].asleep == 0 && PlayerLocation[i].moved == 0 && PlayerLocation[i].dead <= 64) {

					if (ishost && PlayerLocation[i].active || i == 1 && PlayerLocation[i].active) {
						for (rand = 1; rand <= 48; rand++) {
							if (PlayerLocation[i].mapx == pits[rand].x &&
							    PlayerLocation[i].mapy == pits[rand].y &&
							    PlayerLocation[i].level == pits[rand].lvl &&
							    pits[rand].frame >= 18 && pits[rand].frame <= 29 &&
							    PlayerLocation[i].frame == 0) {
								PlayerLocation[i].track = 0;
								if (i == 1) {
									hitkeyboard = 0;
									mousehit = 0;
									PlayerLocation[i].stairs = 1;
									PlayerLocation[i].staircounter = 6;
									PlayerLocation[i].frame = 1;
									PlayerLocation[i].framehuman = 1;
								} else {
									PlayerLocation[i].frame = 1;
									PlayerLocation[i].rest = 0;
									PlayerLocation[i].stairs = 1;
									strcpy(PlayerLocation[i].lastmove, "-");
								}
							}
						}
					}

					if (m_directSoundOK && PlayerLocation[i].dead == 50 || m_directSoundOK && PlayerLocation[i].dead == 49)
						m_pDirSound->PlaySound(m_bufferRegen);

					if (startgame == 1 || PlayerLocation[i].justdied == 1 || PlayerLocation[i].x == 0 && PlayerLocation[i].y == 0) {
						// save player co-ords at start of level!
						PlayerLocation[i].x = startx + 20;
						PlayerLocation[i].y = starty + 10;
						PlayerLocation[i].justdied = 0;
					}

					monstershoot = 0;
					if (PlayerLocation[i].shoot == 1 && PlayerLocation[i].active) {

						if (ishost && i > newwarrior || networkgame == 0)
							monstershoot = monster_shoot();
					}

					if (monstershoot == 1) {
						if (monstershoot == 1) {
							start_missle(PlayerLocation[i].mapx, PlayerLocation[i].mapy, PlayerLocation[i].direction,
							             PlayerLocation[i].level,
							             PlayerLocation[i].x, PlayerLocation[i].y, i);

							if (i <= newwarrior && i != 1) {

								if (PlayerLocation[i].ability <= 10 &&
								    !PlayerLocation[i].protection &&
								    PlayerLocation[i].ring > 0) {

									PlayerLocation[i].protection = 70;
									PlayerLocation[i].ring--;
									if (m_directSoundOK)
										m_pDirSound->PlaySound(m_bufferTadaCleric);
								}
							}
						}
					}
					//if (i == 1 && hitkeyboard == 1 && PlayerLocation[i].framehuman == 0 || i == 1 && mousehit == 1 && PlayerLocation[i].framehuman == 0) {

					//	// human move

					//	if (newmove != 0) {

					//		PlayerLocation[i].direction = newmove;
					//		newmove = 0;
					//		mousehit = 1;
					//		mousehitx = newmovex;
					//		mousehity = newmovey;
					//	}

					//	if (mousehit == 1 && joystickhit != 1) {
					//		if (PlayerLocation[1].direction == 1 && PlayerLocation[1].x + 30 <= mousehitx && PlayerLocation[1].y + 54 <= mousehity) {
					//			cancelmove = 1;
					//		}
					//		if (PlayerLocation[1].direction == 2 && PlayerLocation[1].x + 30 >= mousehitx && PlayerLocation[1].y + 54 <= mousehity) {
					//			cancelmove = 1;
					//		}
					//		if (PlayerLocation[1].direction == 3 && PlayerLocation[1].x + 30 <= mousehitx && PlayerLocation[1].y + 54 >= mousehity) {
					//			cancelmove = 1;
					//		}
					//		if (PlayerLocation[1].direction == 4 && PlayerLocation[1].x + 30 >= mousehitx && PlayerLocation[1].y + 54 >= mousehity) {
					//			cancelmove = 1;
					//		}
					//	}
					//	if (cancelmove == 0) {

					//		if (mousehit == 1 && PlayerLocation[1].direction == 1) {
					//			handle_left();
					//		}
					//		if (mousehit == 1 && PlayerLocation[1].direction == 4)
					//			handle_right();
					//		if (mousehit == 1 && PlayerLocation[1].direction == 2)
					//			handle_up();
					//		if (mousehit == 1 && PlayerLocation[1].direction == 3)
					//			handle_down();
					//	}

					//	if (cancelmove == 1) {
					//		frame_num = 0;
					//		cancelmove = 0;
					//		PlayerLocation[1].framehuman = 0;
					//		PlayerLocation[1].frame = frame_num;
					//		mousehit = 0;
					//		joystickhit = 0;
					//	} else {

					//		PlayerLocation[1].framehuman = 1;

					//		if (PlayerLocation[1].frame == 0) {
					//			PlayerLocation[1].frame = 1;
					//		}

					//		if (networkgame) {
					//			sendonce = FALSE;
					//			if (!ishost) {
					//				send_playerquick(1, 1, 0);
					//			} else if (!networkserver && ishost) {
					//				send_playerquick(1, 0, 0);
					//			}
					//		}
					//	}
					//} else if (i == 1 && hitkeyboard == 0 && PlayerLocation[CurrentPlayer].stairs == 0 || i == 1 && mousehit == 0 && PlayerLocation[CurrentPlayer].stairs == 0) {

					//	PlayerLocation[1].framehuman = 0;
					//	PlayerLocation[1].frame = 0;

					//	// here
					//	if (networkgame && !ishost)
					//		send_player2(1, 1, 0);
					//	else if (networkgame && ishost && !networkserver)
					//		send_player2(1, 0, 0);
					//}

					if (PlayerLocation[i].limitspeed && PlayerLocation[i].speed != 1 && i != 1 || PlayerLocation[i].rest > 0 && i != 1) {
						if (PlayerLocation[i].active && ishost && networkgame && PlayerLocation[i].character == 2) {
							if (PlayerLocation[i].frame == 1) {
								wakeup_sleepy_heads();
							}
						}

						// monster is slow ... skip him
					} else {

						if (PlayerLocation[i].frame == 0) {

							// frame 0 ... computer move

								restmonster = random_num(8);
								restmonster = 0;

								if (restmonster == 1 && i > newwarrior && ishost) {
									restmonster = random_num(7) + 7;
									PlayerLocation[i].rest = restmonster;
								} else {
									if (PlayerLocation[i].track == 0) {

										strcpy(PlayerLocation[i].lastmove, "-");
										destination();
										if (PlayerLocation[CurrentPlayer].character == 13) {
											computermove();
										} else
											computer_m(PlayerLocation[i].mapx, PlayerLocation[i].mapy, PlayerLocation[i].level);
										PlayerLocation[i].track = 10;

										PlayerLocation[i].dx = dx;
										PlayerLocation[i].dy = dy;
										PlayerLocation[i].dl = PlayerLocation[i].level;
									} else {
										PlayerLocation[i].track--;
										if (PlayerLocation[i].track < 0)
											PlayerLocation[i].track = 0;
										destination();
										dx = PlayerLocation[i].dx;
										dy = PlayerLocation[i].dy;
										if (PlayerLocation[CurrentPlayer].character == 13) {
											computermove();
										} else {
											computer_m(PlayerLocation[i].mapx, PlayerLocation[i].mapy, PlayerLocation[i].level);
										}
									}
									mademove = TRUE;
									PlayerLocation[i].x = startx + 20;
									PlayerLocation[i].y = starty + 10;
									frame_num = 1;
									PlayerLocation[i].frame = frame_num;
									calc_animation();
								}
								PlayerLocation[i].calcmove = 0;
							} // new one
						 
						if (PlayerLocation[i].frame == 0) {

							animateon = 0;
						}
						else {
							// animate the character
							frame_num = PlayerLocation[i].frame;

							OnTimer();
							PlayerLocation[i].frame = frame_num;

							animateon = 1;
							animateon = 0;
						}
					}
					if (foundtreasure == i && !networkserver) {
						if (treasurelvl >= level)
							BOOL resultOK = pDirDraw->BlitImage(&CPoint(PlayerLocation[i].x, PlayerLocation[i].y - 20), partsBSurfaceNum,
							                                    &CRect(0, 490, 56, 530));
						else
							BOOL resultOK = pDirDraw->BlitImage(&CPoint(PlayerLocation[i].x, PlayerLocation[i].y - 20), partsBSurfaceNum,
							                                    &CRect(56, 490, 112, 530));
					}

					if (foundtreasure2 == i && !networkserver) {
						if (treasurelvl2 >= level)
							BOOL resultOK = pDirDraw->BlitImage(&CPoint(PlayerLocation[i].x, PlayerLocation[i].y - 20), partsBSurfaceNum,
							                                    &CRect(113, 490, 169, 587));
						else
							BOOL resultOK = pDirDraw->BlitImage(&CPoint(PlayerLocation[i].x, PlayerLocation[i].y - 20), partsBSurfaceNum,
							                                    &CRect(170, 490, 225, 587));
					}

					if (PlayerLocation[i].limitspeed && ishost) {
						PlayerLocation[i].speed++;
						if (PlayerLocation[i].speed > PlayerLocation[i].maxspeed)
							PlayerLocation[i].speed = 1;
					}
					if (PlayerLocation[i].rest > 0 && ishost) {

						PlayerLocation[i].rest--;
						if (PlayerLocation[i].rest <= 0)
							PlayerLocation[i].rest = 0;
					}

					if (PlayerLocation[i].hit > 0 && ishost || i == 1 && PlayerLocation[i].hit > 0) {
						PlayerLocation[i].hit++;
						if (PlayerLocation[i].hit > 10)
							PlayerLocation[i].hit = 0;
					}

					if (PlayerLocation[i].firecount > 0 && ishost || i == 1 && PlayerLocation[i].firecount > 0) {
						PlayerLocation[i].firecount--;
						if (PlayerLocation[i].firecount <= 0)
							PlayerLocation[i].firecount = 0;
					}

					if (PlayerLocation[i].protection > 0 && ishost || i == 1 && PlayerLocation[i].protection > 0) {
						//						PlayerLocation[i].hit=4;
						PlayerLocation[i].protection--;
						if (PlayerLocation[i].protection == 25) {
							if (m_directSoundOK)
								m_pDirSound->PlaySound(m_bufferProtection);
							if (m_directSoundOK)
								m_pDirSound->PlaySound(m_bufferEyeswizard);
						}

						if (PlayerLocation[i].protection <= 0)
							PlayerLocation[i].protection = 0;
					}

					if (i == dtreasure[0].towner && networkgame && ishost) {

						if (dtreasure[0].tdx != PlayerLocation[i].mapx ||
						    dtreasure[0].tdy != PlayerLocation[i].mapy ||
						    dtreasure[0].tdlvl != PlayerLocation[i].level) {
							dtreasure[0].towner = 0;
							dtreasure[0].tdx = 0;
							dtreasure[0].tdy = 0;
							dtreasure[0].tdlvl = 0;
						}
					}
					if (i == dtreasure[1].towner && networkgame && ishost) {

						if (dtreasure[1].tdx != PlayerLocation[i].mapx ||
						    dtreasure[1].tdy != PlayerLocation[i].mapy ||
						    dtreasure[1].tdlvl != PlayerLocation[i].level) {

							dtreasure[1].towner = 0;
							dtreasure[1].tdx = 0;
							dtreasure[1].tdy = 0;
							dtreasure[1].tdlvl = 0;
						}
					}

					if (ishost)
						PlayerLocation[i].moved = 1;

					// old attack player
					if (attack_player() == 1) {
						endgame = TRUE;
						break;
					}
				}

				if (dscene[i].mapx == x && dscene[i].mapy == y - 1 && dscene[i].x != 0) {

					if (dscene[i].lvl == PlayerLocation[camera].level)
						ShowPlayer(2);

					// the death scene!
					dscene[i].counter = dscene[i].counter - 6;
					if (dscene[i].counter <= 0) {

						dscene[i].x = 0;
						dscene[i].y = 0;
						dscene[i].lvl = 0;
						dscene[i].counter = 0;
						dscene[i].mapx = 0;
						dscene[i].mapy = 0;
					}
				}

				if (PlayerLocation[i].mapx == x && PlayerLocation[i].mapy == y - 1 && PlayerLocation[i].asleep == 0 && PlayerLocation[i].dead <= 64 &&
				    PlayerLocation[i].active != FALSE) {
					if (PlayerLocation[i].protection > 0 && PlayerLocation[i].level == level && !networkserver) {
						if (PlayerLocation[i].protection <= 20 && PlayerLocation[i].protection >= 25 ||
						    PlayerLocation[i].protection <= 15 && PlayerLocation[i].protection >= 10 ||
						    PlayerLocation[i].protection <= 5 && PlayerLocation[i].protection >= 1) {
							if (PlayerLocation[i].protection == 25 ||
							    PlayerLocation[i].protection == 15 ||
							    PlayerLocation[i].protection == 5)
								if (m_directSoundOK)
									m_pDirSound->PlaySound(m_bufferProtection);

						} else {
							BOOL resultOK = pDirDraw->BlitImage(&CPoint(PlayerLocation[i].x - 24, PlayerLocation[i].y - 15), partsBSurfaceNum,
							                                    &CRect(226 + 113, 490, 338 + 113, 587));
						}
					}
					if (PlayerLocation[i].frame == 0 || PlayerLocation[i].fx == 0) {
						if (PlayerLocation[i].level == PlayerLocation[camera].level)
							ShowPlayer(0);
					} else {
						animateon = 1;
						if (PlayerLocation[i].level == PlayerLocation[camera].level)
							ShowPlayer(1);
						animateon = 0;
					}

					if (ishost && PlayerLocation[i].speed == 1 && networkgame) {

						if (PlayerLocation[i].frame == 0) {
							send_player2(i, 0, 0);
						} else {

							send_playerquick(i, 0, 0);
						}
					}
				}
			}
			CurrentPlayer = savecurrent;

			level = PlayerLocation[camera].level;

			startx = startx + XADJUST;
			starty = starty + YADJUST;
		}
		startx = startxreset - ((y + 1) * 48);
		starty = startyreset + ((y + 1) * 24);
	}

	startgame = 0;
	show_missle();

	apply_damage();
	went_up_level();
	work1 = 1;
	whoisnext++;
	if (whoisnext > 6)
		whoisnext = 2;
	i = whoisnext;
	for (loop = 1; loop <= NumPlayers; loop++) {
		PlayerLocation[loop].calcmove = 0;
		if (PlayerLocation[loop].dead >= 1 && ishost || loop == 1 && PlayerLocation[loop].dead >= 1) {
			PlayerLocation[loop].dead -= 2;
			if (PlayerLocation[loop].dead == 1) {
				PlayerLocation[loop].dead = 0;
			}
		}
	}

	work2 = i;
	i = 0;
	if (ishost) {
		while (work1) {
			i++;
			if (i > 6)
				break;
			if (work2 > 6) {
				work2 = 2;
			}
			for (loop = work2; loop <= NumPlayers; loop += 5) {
				if (PlayerLocation[loop].asleep == 0 && PlayerLocation[loop].dead == 0 && PlayerLocation[loop].justdied == 0 && PlayerLocation[loop].frame == 0) {
					PlayerLocation[loop].calcmove = 1;
				}
			}
			work2++;
		}
	}

	rand = 49;

	for (i = 1; i <= 48; i++) {

		if (pits[i].frame > 0)
			pits[i].frame++;

		if (pits[i].frame >= 36) {
			pits[i].frame = 0;
			pitcounter--;
		}
	}

	rand = random_num(15);

	//	rand=1;
	if (rand == 1 && gamedef.pits == 1 && ishost) {
		assign_pit(level);
	}
	if (!networkserver) {

		if (endgame) {

			strcpy(chat[0].say, "Level Complete...Stand by.");
			chat[0].duration = 60;
		}
		show_panel();

		resultOK = pDirDraw->FlipSurfaces();
		if (player != 1)
			pDirDraw->ClearSurface(BACKBUFFERNUM);
	}

	if (endgame) {

		if (ishost) {

			// FINISHED LEVEL

			KillTimer(hWindow, 3);
			//			KillTimer(hWindow, 2);

			hitanykey = 0;
			hitkeyboard = 0;
			mousehit = 0;
			bdown = 0;

			if (networkgame)
				send_stoptimer(0);

			startgame = 1;
			receiveok = 0;
			new_level(1);
			if (m_directSoundOK)
				stop_song();

			if (m_directSoundOK)
				play_random_music();

			if (!networkgame) {
				showmission = 3;
				hitanykey = 1;
				SetTimer(hWindow, 1, 50, TimerProc);
			}
		}
	}

	cronit++;
	if (cronit > 20) {
		cronit = 0;

		if (ishost && networkgame) {
			send_treasure(0, 0);
			send_treasure2(0, 0);
			send_pingserver();
		}

		if (!ishost) {
			send_pinghost();
		}
	}

	if (ishost && networkgame) {

		for (loop = 1; loop <= newwarrior; loop++) {

			if (PlayerLocation[loop].active && networkserver)
				pingplayer[loop] = pingplayer[loop]++;
			if (PlayerLocation[loop].active && !networkserver) {

				if (loop != 1)
					pingplayer[loop] = pingplayer[loop]++;
			}

			if (pingplayer[loop] > 250) {

				pingplayer[loop] = 0;
				debug_me("display_dungeon", "Player Timed Out:", loop, 0);
				switch (PlayerLocation[loop].image) {
				case 1:
					strcpy(networkresponse, "Murk: Barbarian has timed out...");

					break;
				case 3:
					strcpy(networkresponse, "Murk: Thief timed out...");

					break;
				case 4:
					strcpy(networkresponse, "Murk: Mage has timed out...");

					break;
				case 5:
					strcpy(networkresponse, "Murk: Cleric timed out...");

					break;
				}

				send_chat(0);

				dungeon[PlayerLocation[loop].copx][PlayerLocation[loop].copy][PlayerLocation[loop].coplvl].item = 'm';
				send_dungeon_square(PlayerLocation[loop].copx, PlayerLocation[loop].copy, PlayerLocation[loop].coplvl, 1);
				if (m_pDirectPlay->DestroyPlayer(m_dpidPlayer[loop]) == DP_OK) {
				}

				PlayerLocation[loop].active = FALSE;
				clear_player(loop);
			}
		}
	}
	if (!ishost && networkgame) {

		pingplayer[0] = pingplayer[0]++;

		if (pingplayer[0] > 300) {

			pingplayer[0] = 900;
			strcpy(chat[0].say, "No response from Murk Server!!!");
			chat[0].duration = 100;
			// del player
		}
	}

	if (networkserver) {
		reset_networkserver();
	}

	keyboardlock = 0;
	hitkeyboard = 0;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void show_missle(void) {

	int i;
	BOOL resultOK;
	int framey, framex;
	int frame2y, frame2x;
	int frm, adjust, move1, move2;

	move1 = 0;
	move2 = 0;

	for (i = 1; i <= 20; i++) {
		if (missle[i].frame != 0) {
			frm = missle[i].frame;

			if (missle[i].image == 1) {
				adjust = 0;
			}
			if (missle[i].image == 2) {
				adjust = 64;
			}
			if (missle[i].image == 3) {
				adjust = 64 + 64;
			}
			if (missle[i].image == 4) {
				adjust = 64 + 64 + 64;
			}

			if (missle[i].image == 5) {
				adjust = 64 + 64 + 64 + 64;
			}
			if (missle[i].image == 6) {
				adjust = 64 + 64 + 64 + 64 + 64;
			}
			switch (missle[i].direction) {
			case 1:
				framex = 0 + adjust;
				framey = 0;
				frame2x = 63 + adjust;
				frame2y = 63;
				move1 = -1;
				move2 = -1;
				break;
			case 2:
				framex = 384 + adjust;
				framey = 0;
				frame2x = 447 + adjust;
				frame2y = 63;
				move1 = +1;
				move2 = -1;
				break;
			case 3:
				framex = 0 + adjust;
				framey = 64;
				frame2x = 63 + adjust;
				frame2y = 127;
				move1 = -1;
				move2 = +1;
				break;
			case 4:
				framex = 384 + adjust;
				framey = 64;
				frame2x = 447 + adjust;
				frame2y = 127;
				move1 = +1;
				move2 = +1;
				break;
			}
			framey = framey + 640;
			frame2y = frame2y + 640;
			if (missle[i].misslelvl == PlayerLocation[camera].level) {
				if (!networkserver)
					resultOK = pDirDraw->BlitImage(&CPoint(missle[i].missledx, missle[i].missledy), partsASurfaceNum,
					                               &CRect(framex, framey, frame2x, frame2y));
			}

			missle[i].frame++;
			missle[i].missledx = missle[i].missledx + move1 * 8 * 2;
			missle[i].missledy = missle[i].missledy + move2 * 4 * 2;
			if (missle[i].frame > 3) {
				if (missle[i].direction == 1)
					missle[i].misslex = missle[i].misslex - 1;
				if (missle[i].direction == 2)
					missle[i].missley = missle[i].missley - 1;
				if (missle[i].direction == 3)
					missle[i].missley = missle[i].missley + 1;
				if (missle[i].direction == 4)
					missle[i].misslex = missle[i].misslex + 1;
				if (dungeon[missle[i].misslex][missle[i].missley][missle[i].misslelvl].type != 'f' && dungeon[missle[i].misslex][missle[i].missley][missle[i].misslelvl].type != 's') {
					missle[i].frame = 0;
					PlayerLocation[missle[i].owner].numshots = PlayerLocation[missle[i].owner].numshots - 1;

				} else
					missle[i].frame = 1;
			}
		}
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void start_missle(int x, int y, int dir, int lvl, int sx, int sy, int owner) {

	int i;
	int rand;

	if (PlayerLocation[owner].numshots >= PlayerLocation[owner].maxmissle) {
		if (ishost || owner == 1)
			return;
	}

	for (i = 1; i <= 20; i++) {
		if (missle[i].frame == 0)
			break;
	}

	switch (PlayerLocation[owner].image) {
	case 2:
	case 4:
	case 5:
	case 9:
		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferFireball);
		break;
	default:
		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferBow);
		break;
	}

	missle[i].misslex = x;
	missle[i].missley = y;
	missle[i].direction = dir;
	missle[i].misslelvl = lvl;
	missle[i].missledx = sx;
	missle[i].missledy = sy;
	missle[i].frame = 1;
	missle[i].owner = owner;

	switch (PlayerLocation[owner].image) {

	case 1:
		missle[i].image = 1;
		break;
	case 2:
		missle[i].image = 3;
		break;
	case 3:
		missle[i].image = 1;
		break;
	case 4:
		missle[i].image = 3;
		break;
	case 5:
		missle[i].image = 4;
		break;
	case 6:
		missle[i].image = 1;
		break;
	case 7:
		missle[i].image = 6;
		break;
	case 8:
		rand = random_num(2);
		if (rand == 1)
			missle[i].image = 2;
		else
			missle[i].image = 5;
		break;
	case 9:
		missle[i].image = 4;
		break;
	}

	PlayerLocation[owner].numshots = PlayerLocation[owner].numshots + 1;

	// fix it
	if (networkgame) {
		if (ishost)
			send_missle(0, missle[i].misslex, missle[i].missley, missle[i].misslelvl, missle[i].frame, missle[i].image, missle[i].direction, owner, missle[i].missledx, missle[i].missledy, i);
		else if (owner == 1)
			send_missle(1, missle[i].misslex, missle[i].missley, missle[i].misslelvl, missle[i].frame, missle[i].image, missle[i].direction, owner, missle[i].missledx, missle[i].missledy, i);
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

int monster_shoot(void) {

	int t, i;
	int savecurrent;
	int x1;
	int y1;
	int agression;
	int monster;
	int level;
	int savedir;
	int j;
	//	int restmonster;
	//	int work;
	//	int work2;
	char junk[30];
	int rand;

	BOOL flag;

	int dir2;

	monster = 0;
	savecurrent = CurrentPlayer;
	savelevel = PlayerLocation[savecurrent].level;
	level = savelevel;
	savedir = PlayerLocation[savecurrent].direction;
	i = CurrentPlayer;
	int image;

	/*if (CurrentPlayer == 1) {

		return 0;
	}*/

	image = PlayerLocation[savecurrent].team;

	j = PlayerLocation[CurrentPlayer].direction;
	dir2 = 1;

	while (1) {

		flag = TRUE;
		x1 = PlayerLocation[i].mapx;
		y1 = PlayerLocation[i].mapy;

		if (PlayerLocation[savecurrent].character == 3)
			monster = 1;
		if (PlayerLocation[savecurrent].asleep == 0 &&
		    PlayerLocation[savecurrent].numshots < PlayerLocation[savecurrent].maxmissle && PlayerLocation[savecurrent].dead == 0 &&
		    PlayerLocation[savecurrent].active) {

			while (flag) {
				for (t = 1; t <= NumPlayers; t++) {
					if (t != savecurrent) {
						if (PlayerLocation[t].mapx == x1 &&
						    PlayerLocation[t].mapy == y1 &&
						    PlayerLocation[t].level == savelevel &&
						    PlayerLocation[t].dead == 0 &&
						    PlayerLocation[t].active) {

							if (PlayerLocation[t].character == 3 && monster) {
								flag = FALSE;
							}

							else if (PlayerLocation[t].team == image && !monster) {
								flag = FALSE;
							} else {
								CurrentPlayer = savecurrent;
								if (PlayerLocation[CurrentPlayer].frame == 0 && PlayerLocation[CurrentPlayer].rest == 0) {
									if (PlayerLocation[CurrentPlayer].direction != j)
										PlayerLocation[CurrentPlayer].direction = j;

									switch (PlayerLocation[CurrentPlayer].intelligence) {

									case 1:
										agression = 5;
										break;
									case 2:
										agression = 4;
										break;
									case 3:
										agression = 2;
										break;
									default:
										agression = 2;
										break;
									}

									rand = random_num(agression);
									if (rand != 1) {

										switch (PlayerLocation[CurrentPlayer].intelligence) {

										case 1:
											PlayerLocation[CurrentPlayer].rest = 2;
											break;
										case 2:
											PlayerLocation[CurrentPlayer].rest = 4;
											break;
										case 3:
											PlayerLocation[CurrentPlayer].rest = 6;
											break;
										default:
											PlayerLocation[CurrentPlayer].rest = 6;
											break;
										}

										PlayerLocation[CurrentPlayer].track = 10;
										PlayerLocation[CurrentPlayer].dx = x1;
										PlayerLocation[CurrentPlayer].dy = y1;
										PlayerLocation[CurrentPlayer].dl = PlayerLocation[CurrentPlayer].level;
										sprintf(junk, "%d", t);
										PlayerLocation[CurrentPlayer].lasttarget = junk[0];
										strcpy(PlayerLocation[CurrentPlayer].lastmove, "-");
									}
								}
								if (flag) {
									if (PlayerLocation[savecurrent].firecount == 0) {
										PlayerLocation[savecurrent].firecount = PlayerLocation[savecurrent].firerate;
										return 1;
									} else {
										return 0;
									}
								}
							}
						}
					}
				}
				switch (j) {
				case 1:
					x1--;
					break;
				case 2:
					y1--;
					break;
				case 3:
					y1++;
					break;
				case 4:
					x1++;
					break;
				}

				if (dungeon[x1][y1][level].type == 'f' || dungeon[x1][y1][level].type == 's') {
				} else {
					flag = FALSE;
				}
			}
		}

		CurrentPlayer = savecurrent;

		j++;
		if (j > 4)
			j = 1;

		dir2++;
		if (dir2 >= 5)
			return 0;

		if (PlayerLocation[savecurrent].intelligence == 3) {
			return 0;
		}
		if (foundtreasure == CurrentPlayer && dir2 == 2 || foundtreasure2 == CurrentPlayer && dir2 == 2)
			return 0;
	}
	return 0;
}

void build_server_screen() {

	HDC hDC;
	int i;

	hDC = GetDC(hWindow);

	if (networkserver && ishost) {
		if (networktype == 0) {
			strcpy(serverscreen, "Welcome to Murk...");
			TextOut(hDC, 1, 1, serverscreen, strlen(serverscreen));
		}
		if (networktype == 1 || networktype == 2) {
			if (networktype == 1) {
				strcpy(serverscreen, "Murk TCP/IP Server (Active)");
				TextOut(hDC, 1, 1, serverscreen, strlen(serverscreen));
			} else {
				strcpy(serverscreen, "Murk IPX/SPX Server (Active)");
				TextOut(hDC, 1, 1, serverscreen, strlen(serverscreen));
			}

			for (i = 1; i <= newwarrior; i++) {

				switch (PlayerLocation[i].image) {
				case 1:
					// barbarian
					if (PlayerLocation[i].active) {
						strcpy(serverscreen, "Barbarian is connected");
					} else
						strcpy(serverscreen, "                           ");
					TextOut(hDC, 1, 20, serverscreen, strlen(serverscreen));
					break;
				case 3:
					// barbarian
					if (PlayerLocation[i].active) {
						strcpy(serverscreen, "Thief is connected");
					} else
						strcpy(serverscreen, "                           ");

					TextOut(hDC, 1, 30, serverscreen, strlen(serverscreen));
					break;
				case 4:
					// barbarian
					if (PlayerLocation[i].active) {
						strcpy(serverscreen, "Mage is connected");
					} else
						strcpy(serverscreen, "                           ");

					TextOut(hDC, 1, 40, serverscreen, strlen(serverscreen));
					break;
				case 5:
					// barbarian
					if (PlayerLocation[i].active) {
						strcpy(serverscreen, "Cleric is connected");
					} else
						strcpy(serverscreen, "                           ");

					TextOut(hDC, 1, 50, serverscreen, strlen(serverscreen));
					break;
				}
			}
		}
	}

	ReleaseDC(hWindow, hDC);
}

void check_input() {
	controlsinuse = 0;
	int fire = 0;

	if (GetAsyncKeyState(17) < 0) {
		if (PlayerLocation[1].firecount == 0) {
			if (PlayerLocation[1].firecount == 0) {
				PlayerLocation[1].firecount = PlayerLocation[1].firerate;
			}
			start_missle(PlayerLocation[1].mapx, PlayerLocation[1].mapy, PlayerLocation[1].direction,
			             PlayerLocation[1].level,
			             PlayerLocation[1].x, PlayerLocation[1].y, 1);
			fire = 1;
		}
	}

	if (usejstick && mousestate == 0) {
		check_joystick(fire);
		if (joystickhit == 1)
			return;
	}

	if (mousestate == 0)
		check_keys();

	if (keyboardlock) {
		return;
	}

	if (GetKeyState(VK_LBUTTON) >= 0) {
		// mouse is up
		// check to see if it was down a sec ago
		if (mousestate == 1) {
			bdown = 0;
			cancelmove = 1;
			mousehit = 0;
			newmove = 0;
			hitkeyboard = 0;
			mousestate = 0;
			//			debug_me("MOUSE L UP","UP",0,0);

			PlayerLocation[1].framehuman = 0;
			return;
		}

	} else if (::GetAsyncKeyState(VK_LBUTTON) < 0) {
		// mouse is down
		mousestate = 1;
		//			debug_me("MOUSE L DOWN","DOWN",0,0);
		check_leftmousedown();
	}

	if (GetKeyState(VK_RBUTTON) < 0 && fire == 0) {
		check_rightmousedown();
	}
}

void check_mouse() {
	if (::GetAsyncKeyState(VK_LBUTTON) < 0) {

		check_leftmousedown();
	}
}

void check_rightmousedown() {

	UINT x1;
	UINT y1;
	int x, y;

	int playerdir;
	CPoint ptCurPos;

	::GetCursorPos(&ptCurPos);

	x1 = ptCurPos.x;
	y1 = ptCurPos.y;

	x = x1;
	y = y1;

	//	if (::GetAsyncKeyState (VK_LBUTTON) <0) {
	//	}
	//	else {

	if (x <= PlayerLocation[1].x + 30 && y <= PlayerLocation[1].y + 54) {
		playerdir = 1;
	}
	if (x >= PlayerLocation[1].x + 30 && y > PlayerLocation[1].y + 54) {
		playerdir = 4;
	}
	if (x >= PlayerLocation[1].x + 30 && y <= PlayerLocation[1].y + 54) {
		playerdir = 2;
	}
	if (x <= PlayerLocation[1].x + 30 && y > PlayerLocation[1].y + 54) {
		playerdir = 3;
	}
	//	if (PlayerLocation[1].direction != playerdir)
	//		mousehit =0;

	if (mousestate == 0)
		PlayerLocation[1].direction = playerdir;
	//	}

	if (PlayerLocation[1].firecount == 0) {
		if (PlayerLocation[1].firecount == 0) {
			PlayerLocation[1].firecount = PlayerLocation[1].firerate;
		}
		start_missle(PlayerLocation[1].mapx, PlayerLocation[1].mapy, PlayerLocation[1].direction,
		             PlayerLocation[1].level,
		             PlayerLocation[1].x, PlayerLocation[1].y, 1);
	}
}

void check_leftmousedown() {

	UINT x1;
	UINT y1;
	int x, y;

	CPoint ptCurPos;

	::GetCursorPos(&ptCurPos);

	x1 = ptCurPos.x;
	y1 = ptCurPos.y;

	//	debug_me("CHECKLEFT","x1,y1",x1,y1);
	//	debug_me("CHECKLEFT","Player x,y",PlayerLocation[1].x,PlayerLocation[1].y);

	if (showmission == 1 || showmission == 0 || showmission == 2) {

		bdown = 1;
		mousehitx = x1;
		mousehity = y1;
		return;
	}
	if (showmission == 4) {
		bdown = 1;
		mousehitx = 0;
		mousehity = 0;
		return;
	}

	if (y1 > 600 && x1 < 240) {
		if (PlayerLocation[1].ring > 0 && x1 > 166 && y1 > 575 && x1 < 220 && y1 < 630 && PlayerLocation[1].protection == 0) {

			bdown = 1;
			mousehitx = x1;
			mousehity = y1;

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferButton);
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferTadaCleric);

			PlayerLocation[1].protection = 70;
			PlayerLocation[1].ring--;
			return;
		}

		if (x1 < 150 && y1 > 575 && x1 > 95 && y1 < 630) {
			bdown = 1;
			mousehitx = x1;
			mousehity = y1;

			if (foundtreasure2 == 1 || foundtreasure == 1) {
				dropt = 0;
				if (m_directSoundOK)
					m_pDirSound->PlaySound(m_bufferButton);

				if (foundtreasure2 == 1) {

					if (ishost) {
						treasurex2 = PlayerLocation[1].mapx;
						treasurey2 = PlayerLocation[1].mapy;
						treasurelvl2 = PlayerLocation[1].level;
						foundtreasure2 = 0;
					}
					if (networkgame)
						send_drop_treasure(2);
				}
				if (foundtreasure == 1) {

					if (ishost) {
						treasurex = PlayerLocation[1].mapx;
						treasurey = PlayerLocation[1].mapy;
						treasurelvl = PlayerLocation[1].level;
						foundtreasure = 0;
					}
					if (networkgame)
						send_drop_treasure(1);
				}

				//			PlayerLocation[1].hit=10;
			}
			// sendplayer
			return;
		}
		return;
	}

	if (PlayerLocation[1].x + 30 > (int)x1 - 5 && PlayerLocation[1].x + 30 < (int)x1 + 5)
		return;
	if (PlayerLocation[1].y + 54 > (int)y1 - 5 && PlayerLocation[1].y + 54 < (int)y1 + 5)
		return;

	if (PlayerLocation[1].dead == 0) {
		mousehit = 1;
		x = x1;
		y = y1;

		handle_mouse(x, y);
	}
}

BOOL check_joystick(int fire) {

	//  if (m_bJoyActive)
	//  {

	JOYINFO jiInfo;
	CPoint ptJoyPos(0, 0);

	::joyGetPos(m_uiDevID, &jiInfo);
	ptJoyPos.x = jiInfo.wXpos;
	ptJoyPos.y = jiInfo.wYpos;

	if (fire == 0)
		joy_button(jiInfo.wButtons);

	// Convert joystick position to client coordinates
	//    if (JoyToClient(&ptJoyPos))
	//    {

	// Convert client joystick position to screen coordinates
	//      ClientToScreen(&ptJoyPos);

	// Move the ghost cursor
	//      ::SetCursorPos(ptJoyPos.x, ptJoyPos.y);
	//    }
	//  }

	if (ptJoyPos.x == 0 && ptJoyPos.y == 0) {
		//		mousehit=0;
		joystickhit = 0;

		return TRUE;
	}

	if (ptJoyPos.x < (xmax / 2) + 5000 && ptJoyPos.x > (xmax / 2) - 5000 && ptJoyPos.y < ymin + 10000) {
		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 2;
		}
		return TRUE;
	}

	if (ptJoyPos.x < (xmax / 2) + 5000 && ptJoyPos.x > (xmax / 2) - 5000 && ptJoyPos.y > ymax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 3;
		}
		return TRUE;
	}

	if (ptJoyPos.y < (ymax / 2) + 5000 && ptJoyPos.y > (xmax / 2) - 5000 && ptJoyPos.x < xmin + 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 1;
		}
		return TRUE;
	}
	if (ptJoyPos.y < (ymax / 2) + 5000 && ptJoyPos.y > (xmax / 2) - 5000 && ptJoyPos.x > xmax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 4;
		}
		return TRUE;
	}

	if (ptJoyPos.x < xmin + 10000 && ptJoyPos.y < ymin + 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 1;
		}
		return TRUE;
	}
	if (ptJoyPos.x > xmax - 10000 && ptJoyPos.y < ymin + 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 2;
		}
		return TRUE;
	}

	if (ptJoyPos.x < xmin + 10000 && ptJoyPos.y > ymax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 3;
		}
		return TRUE;
	}

	if (ptJoyPos.x > xmax - 10000 && ptJoyPos.y > ymax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 4;
		}
		return TRUE;
	}

	if (ptJoyPos.x < (xmax / 2) + 10000 && ptJoyPos.x > (xmax / 2) - 10000 &&
	    ptJoyPos.y < (ymax / 2) + 10000 && ptJoyPos.x > (ymax / 2) - 10000) {

		if (joystickhit == 1)
			mousehit = 0;
		joystickhit = 0;
		return TRUE;
	}

	joystickhit = 0;

	return TRUE;
}

BOOL joy_button(UINT Button) {

	if (Button & JOY_BUTTON1) {
		if (PlayerLocation[1].firecount == 0) {
			if (PlayerLocation[1].firecount == 0) {
				PlayerLocation[1].firecount = PlayerLocation[1].firerate;
			}

			start_missle(PlayerLocation[1].mapx, PlayerLocation[1].mapy, PlayerLocation[1].direction,
			             PlayerLocation[1].level,
			             PlayerLocation[1].x, PlayerLocation[1].y, 1);
		}
	}
	if (Button & JOY_BUTTON2) {

		if (PlayerLocation[1].stairs == 0)
			handle_stairs();
	}

	if (Button & JOY_BUTTON3) {
		if (foundtreasure == 1) {
			treasurex = PlayerLocation[1].mapx;
			treasurey = PlayerLocation[1].mapy;
			treasurelvl = PlayerLocation[1].level;
			foundtreasure = 0;
			PlayerLocation[1].hit = 1;
			dropt = 0;
			if (networkgame)
				send_drop_treasure(1);
		}

		if (foundtreasure2 == 1) {
			treasurex2 = PlayerLocation[1].mapx;
			treasurey2 = PlayerLocation[1].mapy;
			treasurelvl2 = PlayerLocation[1].level;
			foundtreasure2 = 0;
			PlayerLocation[1].hit = 1;
			dropt = 0;
			if (networkgame)
				send_drop_treasure(2);
		}
	}
	if (Button & JOY_BUTTON4) {
		if (PlayerLocation[1].protection == 0 && PlayerLocation[1].ring != 0) {
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferTadaCleric);

			PlayerLocation[1].protection = 70;
			PlayerLocation[1].ring--;
		}
	}

	return TRUE;
}

void check_keys() {

	if (!networkserver) {
		if (keyboardlock == 0) {
			cancelmove = 0;
			if (GetAsyncKeyState(VK_LEFT) < 0 || GetAsyncKeyState(36) < 0) {

				if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
					handle_left();
					if (cancelmove == 0) {
						hitkeyboard = 1;
						keyboardlock = 1;
					}
				}
			} else if (GetAsyncKeyState(VK_RIGHT) < 0 || GetAsyncKeyState(34) < 0) {

				if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
					handle_right();
					if (cancelmove == 0) {
						hitkeyboard = 1;
						keyboardlock = 1;
					}
				}
			} else if (GetAsyncKeyState(VK_UP) < 0 || GetAsyncKeyState(33) < 0) {

				if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
					handle_up();
					if (cancelmove == 0) {
						hitkeyboard = 1;
						keyboardlock = 1;
					}
				}
			} else if (GetAsyncKeyState(VK_DOWN) < 0 || GetAsyncKeyState(35) < 0) {

				if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
					handle_down();
					if (cancelmove == 0) {
						hitkeyboard = 1;
						keyboardlock = 1;
					}
				}
			}

			/*

			            if (GetAsyncKeyState(17) <0 ) {
			                if (PlayerLocation[1].firecount==0) {
			                    if (PlayerLocation[1].firecount==0){
			                        PlayerLocation[1].firecount = PlayerLocation[1].firerate;
			                    }

			                    start_missle  (PlayerLocation[1].mapx,PlayerLocation[1].mapy,PlayerLocation[1].direction,
			                        PlayerLocation[1].level,
			                        PlayerLocation[1].x,PlayerLocation[1].y,1);
			                }

			            }
			*/
			if (GetAsyncKeyState(190) < 0) {
				cancelmove = 1;
				mousehit = 0;
				//				hitkeyboard=0;
			}
			if (GetAsyncKeyState(16) < 0) {
				if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0) {
					handle_stairs();
					if (cancelmove == 0)
						hitkeyboard = 1;
				}
			}
		}
	}
}
