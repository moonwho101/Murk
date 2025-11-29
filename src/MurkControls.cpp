//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"
#include "game.h"
#include <afxmt.h>
#include "mapiwin.h"
#include "winuser.h"

extern BOOL CALLBACK DlgProcChooseProvider(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID lpContext,
                                   LPDWORD lpdwTimeOut, DWORD dwFlags);
extern BOOL CALLBACK DlgProcSelSession(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern void serversettings(int go);
void send_drop_treasure(int treasure);
extern void kill_dplay(void);
extern void find_tcp_ipx();

int sayduration;

void reset_game(int load);
extern int save_game();
extern int load_game();

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
int backgroundcolour = 0;

void OnCharDown(WPARAM wParam) {

	char junk[512];

	int x;

	cancelmove = 0;

	if (chaton == 1 && wParam == 13) {
		if (PlayerLocation[1].image == 1)
			strcpy(junk, "Barbarian: ");
		else if (PlayerLocation[1].image == 3)
			strcpy(junk, "Thief: ");
		else if (PlayerLocation[1].image == 4)
			strcpy(junk, "Mage: ");
		else if (PlayerLocation[1].image == 5)
			strcpy(junk, "Holy Man: ");

		strcat(junk, networkresponse);

		strcpy(networkresponse, junk);
		networkresponse[strlen(networkresponse) - 1] = '\0';

		if (networkgame)
			send_chat(1);

		if (ishost && networkgame && !networkserver) {
			strcpy(chat[0].say, networkresponse);
			chat[0].duration = 50;
		}
		strcpy(networkresponse, "");
		chaton = 0;
		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferProtection);

		return;
	}

	if (chaton == 1 && wParam == 8) {

		x = strlen(networkresponse) - 2;
		if (x <= 0)
			x = 0;

		networkresponse[x] = '>';
		networkresponse[x + 1] = '\0';
	}

	if (chaton == 1 && wParam >= 32 && wParam <= 122) {
		if (strlen(networkresponse) <= 35) {
			sprintf(junk, "%c", wParam);
			networkresponse[strlen(networkresponse) - 1] = '\0';
			strcat(networkresponse, junk);
			strcat(networkresponse, ">");
		}
	}

	if (!chaton) {
		if (wParam == 67 || wParam == 99) {
			strcpy(networkresponse, ">");
			chaton = 1;
		}
	}
	if (chaton == 1 && wParam == VK_ESCAPE) {
		chaton = 0;
		return;
	}
}

void OnKeyDown(WPARAM wParam) {

	int result;

	cancelmove = 0;

	int ring;
	int ability;
	int skill;

	int image;
	int armr;

	int d1;
	int d2;
	int mmissle;
	int frate;

	debug_me("key", "key", wParam, 0);

	if (wParam == 32 && networkserver) {
		find_tcp_ipx();
		return;
	}

	if (wParam == VK_F5 && showmission == 3) {

		if (!chaton) {
			strcpy(networkresponse, ">");
			chaton = 1;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferProtection);

			return;
		}
	}

	if (wParam == VK_F1 && showmission == 3) {
		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferProtection);

		//		KillTimer(hWindow, 2);
		KillTimer(hWindow, 3);
		hitanykey = 0;
		showmission = 2;
		bdown = 0;
		SetTimer(hWindow, 1, 50, TimerProc);
		helpscreen = 1;
		//		KillTimer(hWindow, 1);
	}

	if (wParam == VK_F2 && showmission == 3) {

		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferProtection);
		mousehitx = 0;
		mousehity = 0;

		//		KillTimer(hWindow, 2);
		KillTimer(hWindow, 3);
		hitanykey = 0;
		showmission = 2;
		bdown = 0;
		SetTimer(hWindow, 1, 50, TimerProc);
		//		helpscreen=2;
		//		KillTimer(hWindow, 1);
	}

	if (wParam == VK_F11) {

		backgroundcolour++;
	}

	if (wParam == VK_F12) {

		backgroundcolour--;

		// jump level
		//	PlayerLocation[1].skill = advancelevel+1;
		//	went_up_level();
	}

	if (wParam == VK_F3 && showmission == 3) {

		mousehitx = 0;
		mousehity = 0;

		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferProtection);

		hitanykey = 0;

		if (networkgame) {

			strcpy(chat[0].say, "Murk: Cannot save in network game");
			chat[0].duration = 50;

		} else if (shareware) {
			strcpy(chat[0].say, "Murk: Cannot save in SHAREWARE version");
			chat[0].duration = 50;

		} else {
			result = save_game();

			if (result) {
				strcpy(chat[0].say, "Murk: Game Saved...");
				chat[0].duration = 50;
			} else {
				strcpy(chat[0].say, "Murk: Could not save game...");
				chat[0].duration = 50;
			}
		}
	}

	if (wParam == VK_F4 && showmission == 3) {

		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferProtection);

		hitanykey = 0;

		if (networkgame) {
			strcpy(chat[0].say, "Murk: Cannot load in network game");
			chat[0].duration = 50;

		} else if (shareware) {

			strcpy(chat[0].say, "Murk: Cannot load in SHAREWARE version");
			chat[0].duration = 50;

		} else {

			result = load_game();

			ring = PlayerLocation[1].ring;
			ability = PlayerLocation[1].ability;
			skill = PlayerLocation[1].skill;

			image = PlayerLocation[1].image;
			armr = PlayerLocation[1].armour;

			d1 = PlayerLocation[1].damagemin;
			d2 = PlayerLocation[1].damagemax;
			mmissle = PlayerLocation[1].maxmissle;
			frate = PlayerLocation[1].firerate;

			if (result) {
				strcpy(chat[0].say, "Game Loaded...");

				chat[0].duration = 50;
				KillTimer(hWindow, 3);

				reset_game(1);
				//		result=load_game() ;
				new_level(0);
				//		result=load_game() ;

				PlayerLocation[1].ring = ring;
				PlayerLocation[1].ability = ability;
				PlayerLocation[1].skill = skill;
				PlayerLocation[1].image = image;
				PlayerLocation[1].armour = armr;

				PlayerLocation[1].damagemin = d1;
				PlayerLocation[1].damagemax = d2;
				PlayerLocation[1].maxmissle = mmissle;
				PlayerLocation[1].firerate = frate;

				if (PlayerLocation[1].image == 3)
					dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 1;

				if (PlayerLocation[1].image == 5)
					dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 3;

				if (PlayerLocation[1].image == 4)
					dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 2;

				if (PlayerLocation[1].image == 1)
					dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 0;

				//);
				mousehit = 0;
				hitanykey = 0;
				showmission = 2;

				SetTimer(hWindow, 1, 50, TimerProc);
			} else {
				strcpy(chat[0].say, "Murk: There are no saved games...");
				chat[0].duration = 50;
			}
		}
	}

	if (wParam == VK_ESCAPE && showmission == 0) {
		if (showmission == 1 || showmission == 2) {
			showmission = 0;
			hitanykey = 0;
		}
		PostMessage(hWindow, WM_CLOSE, 0, 0L);
	}

	if (wParam == VK_ESCAPE && showmission == 3) {

		//		KillTimer(hWindow, 2);
		KillTimer(hWindow, 3);

		if (!networkserver) {

			display_dungeon(1);
			pDirDraw->FlipToGDISurface();
		}

		UINT resultclick = MessageBox(hWindow, "Leave Murk?", "Exit Murk", MB_ICONQUESTION | MB_YESNO);
		if (resultclick == IDYES) {
			mousehit = 0;
			hitanykey = 0;
			showmission = 0;
			mousehitx = 0;
			mousehity = 0;
			bdown = 0;

			if (!ishost && networkgame) {
				send_deleteme();
				//					Sleep(2000);
				kill_dplay();
			}

			SetTimer(hWindow, 1, 50, TimerProc);

			return;
		} else {
			//			SetTimer(hWindow, 2, 4000, TimerProc);
			SetTimer(hWindow, 3, timerlength, TimerProc);
		}

		//		PostMessage(hWindow, WM_CLOSE, 0, 0L);
	}

	switch (wParam) {
		/*
		case VK_LEFT :
		    if (hitkeyboard == 0 &&	PlayerLocation[1].stairs ==0 &&	PlayerLocation[1].dead ==0)		{
		        handle_left();
		        if (cancelmove==0)
		            hitkeyboard=1;
		    }
		    break;
		case VK_RIGHT:
		    if (hitkeyboard == 0 &&	PlayerLocation[1].stairs ==0 &&	PlayerLocation[1].dead ==0){
		        handle_right();
		        if (cancelmove==0)
		            hitkeyboard=1;
		    }
		    break;
		case VK_UP :
		    if (hitkeyboard == 0 &&	PlayerLocation[1].stairs ==0 &&	PlayerLocation[1].dead ==0) {
		        handle_up();
		        if (cancelmove==0)
		            hitkeyboard=1;
		    }
		    break;
		case VK_DOWN :
		    if (hitkeyboard == 0 &&	PlayerLocation[1].stairs ==0 &&	PlayerLocation[1].dead ==0) {
		        handle_down();
		        if (cancelmove==0)
		            hitkeyboard=1;
		    }
		    break;

		case 16 :
		    if (hitkeyboard == 0 &&	PlayerLocation[1].stairs ==0) {
		        handle_stairs();
		        if (cancelmove==0)
		            hitkeyboard=1;
		    }
		    break;

		case 17:


		if (PlayerLocation[1].firecount==0) {
		    if (PlayerLocation[1].firecount==0){
		        PlayerLocation[1].firecount = PlayerLocation[1].firerate;
		    }

		        start_missle  (PlayerLocation[1].mapx,PlayerLocation[1].mapy,PlayerLocation[1].direction,
		        PlayerLocation[1].level,
		        PlayerLocation[1].x,PlayerLocation[1].y,1);
		}

		    break;
	*/
	case 84:

		if (!chaton) {
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
		break;
	case 82:
		if (!chaton) {
			if (PlayerLocation[1].protection == 0 && PlayerLocation[1].ring != 0) {
				if (m_directSoundOK)
					m_pDirSound->PlaySound(m_bufferTadaCleric);

				PlayerLocation[1].protection = 70;
				PlayerLocation[1].ring--;
			}
		}
		break;

	case 191:
		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0) {
			if (PlayerLocation[1].direction == 1)
				PlayerLocation[1].direction = 4;
			else if (PlayerLocation[1].direction == 4)
				PlayerLocation[1].direction = 1;
			else if (PlayerLocation[1].direction == 2)
				PlayerLocation[1].direction = 3;
			else if (PlayerLocation[1].direction == 3)
				PlayerLocation[1].direction = 2;

			cancelmove = 1;
			mousehit = 0;
		}
		break;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void movedown() {
	if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].type == 's') {
		if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 1) {
			PlayerLocation[CurrentPlayer].stairs = 2;
		} else {
			PlayerLocation[CurrentPlayer].stairs = 1;
		}
		strcpy(PlayerLocation[CurrentPlayer].lastmove, "-");
		check_dungeon();
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void OnLButtonUp(LPARAM lParam) {
	if (networkserver)
		return;
	bdown = 0;

	if (showmission == 3) {
		cancelmove = 1;
		mousehit = 0;
	}
}

void OnLButtonDown(LPARAM lParam) {

	UINT x1 = LOWORD(lParam);
	UINT y1 = HIWORD(lParam);

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
		if (PlayerLocation[1].ring > 0 && x1 > 166 && y1 > 575 && x1 < 220 && y1 < 613 && PlayerLocation[1].protection == 0) {

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

		if (x1 < 150 && y1 > 575 && x1 > 95 && y1 < 615) {
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
	//	if (PlayerLocation[1].dead == 0) {
	//		mousehit =1;
	//		x=x1;
	//		y=y1;

	//		handle_mouse(x,y);

	//	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void OnLButtonDblClk(LPARAM lParam) {
	handle_stairs();
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void handle_mouse(int x, int y) {

	int playerdir;

	playerdir = 0;

	int movex;
	int movey;

	if (x <= PlayerLocation[1].x + 30 && y <= PlayerLocation[1].y + 54) {
		movex = x;
		movey = y;
		playerdir = 1;
		mouse_adjust(x, y, PlayerLocation[1].x + 30, PlayerLocation[1].y + 54, 0);
		movex = calcmousex;
		movey = calcmousey;
	}
	if (x >= PlayerLocation[1].x + 30 && y >= PlayerLocation[1].y + 54) {
		movex = x;
		movey = y;
		playerdir = 4;
		mouse_adjust(x, y, PlayerLocation[1].x + 30, PlayerLocation[1].y + 54, 0);
		movex = calcmousex;
		movey = calcmousey;
	}
	if (x >= PlayerLocation[1].x + 30 && y <= PlayerLocation[1].y + 54) {
		movex = x;
		movey = y;
		playerdir = 2;
		mouse_adjust(x, y, PlayerLocation[1].x + 30, PlayerLocation[1].y + 54, 1);
		movex = calcmousex;
		movey = calcmousey;
	}
	if (x <= PlayerLocation[1].x + 30 && y >= PlayerLocation[1].y + 54) {
		movex = x;
		movey = y;
		playerdir = 3;
		mouse_adjust(x, y, PlayerLocation[1].x + 30, PlayerLocation[1].y + 54, 1);
		movex = calcmousex;
		movey = calcmousey;
	}

	if (PlayerLocation[1].frame == 0) {
		mousehitx = movex;
		mousehity = movey;
		PlayerLocation[1].direction = playerdir;
	} else {
		newmovex = movex;
		newmovey = movey;
		newmove = playerdir;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void mouse_adjust(int mx, int my, int px, int py, int type) {

	int newmx;
	int newmy;
	int b;

	// find the intsersction of two lines to adjust the mouse
	// to work on the 3d grid ... holy moly

	// intersection of y = -1/2x + b and y = x/2,  x=b

	newmx = mx - px;
	newmy = my - py;

	if (type == 1) {
		// new one
		b = (newmx / 2) - newmy;
		calcmousex = (b) + px;
		// Extra +b term compensates for screen Y-axis inversion, making clicks align with isometric tiles.
		calcmousey = (b / 2) + b + py;
	} else {
		b = (int)(newmy + (newmx / 2));
		calcmousex = (b) + px;
		calcmousey = (b / 2) + py;
	}
}


///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

// Adjust mouse coordinates for isometric projection
// mx,my = mouse position
// px,py = reference point (e.g., tile origin)
// type   = axis selector (0 = slope +0.5, 1 = slope -0.5)

void mouse_adjust_shear(int mx, int my, int px, int py, int type) {
	// Relative mouse offset
	int dx = mx - px;
	int dy = my - py;

	// Use integer-safe math (avoid truncation surprises)
	int val;

	if (type == 1) {
		// Project onto axis with slope -0.5
		// v = dy - dx/2
		val = dy - (dx >> 1); // use bitshift for exact /2
		calcmousex = px - val;
		calcmousey = py + (val >> 1);
	} else {
		// Project onto axis with slope +0.5
		// u = dy + dx/2
		val = dy + (dx >> 1);
		calcmousex = px + val;
		calcmousey = py + (val >> 1);
	}
}

void OnRButtonDown(LPARAM lParam) {

	UINT x1 = LOWORD(lParam);
	UINT y1 = HIWORD(lParam);
	int x, y;

	int playerdir;
	x = x1;
	y = y1;
	if (x <= PlayerLocation[1].x + 30 && y <= PlayerLocation[1].y + 54) {
		playerdir = 1;
	}
	if (x >= PlayerLocation[1].x + 30 && y >= PlayerLocation[1].y + 54) {
		playerdir = 4;
	}
	if (x >= PlayerLocation[1].x + 30 && y <= PlayerLocation[1].y + 54) {
		playerdir = 2;
	}
	if (x <= PlayerLocation[1].x + 30 && y >= PlayerLocation[1].y + 54) {
		playerdir = 3;
	}
	if (PlayerLocation[1].direction != playerdir)
		mousehit = 0;
	PlayerLocation[1].direction = playerdir;
	if (PlayerLocation[1].firecount == 0) {
		if (PlayerLocation[1].firecount == 0) {
			PlayerLocation[1].firecount = PlayerLocation[1].firerate;
		}
		start_missle(PlayerLocation[1].mapx, PlayerLocation[1].mapy, PlayerLocation[1].direction,
		             PlayerLocation[1].level,
		             PlayerLocation[1].x, PlayerLocation[1].y, 1);
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void handle_left() {
	if (PlayerLocation[1].tolx > 2) {
		PlayerLocation[1].direction = 1;
		calc_player_move();
	} else if (dungeon[PlayerLocation[1].mapx - 1][PlayerLocation[1].mapy][PlayerLocation[1].level].type == 'f' ||
	           dungeon[PlayerLocation[1].mapx - 1][PlayerLocation[1].mapy][PlayerLocation[1].level].type == 's') {
		PlayerLocation[1].direction = 1;
		calc_player_move();
	} else {
		cancelmove = 1;
		mousehit = 0;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void handle_right() {
	if (PlayerLocation[1].tolx < 5) {
		PlayerLocation[1].direction = 4;
		calc_player_move();
	} else if (dungeon[PlayerLocation[1].mapx + 1][PlayerLocation[1].mapy][PlayerLocation[1].level].type == 'f' ||
	           dungeon[PlayerLocation[1].mapx + 1][PlayerLocation[1].mapy][PlayerLocation[1].level].type == 's') {
		PlayerLocation[1].direction = 4;
		calc_player_move();
	} else {
		cancelmove = 1;
		mousehit = 0;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void handle_up() {
	if (PlayerLocation[1].toly > 2) {
		PlayerLocation[1].direction = 2;
		calc_player_move();
	} else if (dungeon[PlayerLocation[1].mapx][PlayerLocation[1].mapy - 1][PlayerLocation[1].level].type == 'f' ||
	           dungeon[PlayerLocation[1].mapx][PlayerLocation[1].mapy - 1][PlayerLocation[1].level].type == 's') {
		PlayerLocation[1].direction = 2;
		calc_player_move();
	} else {
		cancelmove = 1;
		mousehit = 0;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void handle_down() {
	if (PlayerLocation[1].toly < 5) {
		PlayerLocation[1].direction = 3;
		calc_player_move();
	} else if (dungeon[PlayerLocation[1].mapx][PlayerLocation[1].mapy + 1][PlayerLocation[1].level].type == 'f' ||
	           dungeon[PlayerLocation[1].mapx][PlayerLocation[1].mapy + 1][PlayerLocation[1].level].type == 's') {
		PlayerLocation[1].direction = 3;
		calc_player_move();
	} else {
		mousehit = 0;
		cancelmove = 1;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void handle_stairs() {
	if (dungeon[PlayerLocation[1].mapx][PlayerLocation[1].mapy][PlayerLocation[1].level].type == 's') {
		if (dungeon[PlayerLocation[1].mapx][PlayerLocation[1].mapy][PlayerLocation[1].level].item == 1) {
			PlayerLocation[1].stairs = 2;
			PlayerLocation[1].staircounter = 6;
		} else {
			PlayerLocation[1].stairs = 1;
			PlayerLocation[1].staircounter = 6;
		}

		///					PlayerLocation[1].stairs =2;
		//			PlayerLocation[1].staircounter = 6;

		PlayerLocation[1].frame = 1;
		PlayerLocation[1].framehuman = 1;
		check_dungeon();
	} else
		cancelmove = 1;
}

void reset_game(int load) {

	networkgame = 0;
	receiveok = 0;
	istimerset = 0;
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
	setability = 1;
	dropt = 1;
	startthegame = 1;
	select1 = 0;
	select2 = 0;
	select3 = 0;
	select4 = 0;
	bdown = 0;
	movelevel = 0;
	chatin = 0;
	chatcurrent = 0;
	gamescreen = 0;
	chaton = 0;

	if (!load) {
		advancelevel = 20;
		advanceattrib = 0;

		countlevels = 0;
		levelcounter = 1;
	}
	hitanykey = 0;
	oktosend = 1;
	helpscreen = 0;
}