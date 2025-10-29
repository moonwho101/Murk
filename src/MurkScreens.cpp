
//  Murk Ver 1.12 By Mark Longo 
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"
#include "mapiwin.h"

extern BOOL doit;
extern void reset_game(int load);
extern int save_game();
extern int load_game();

//////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK DlgRegister(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void showsplash() {

	int resultOK;
	int i;
	int x, y;
	int result;
	int adjust;
	BOOL flag;
	int recu1, recu2, recl1, recl2;

	int ring;
	int ability;
	int skill;
	int image;
	int armr;

	int	d1;
	int	d2;
	int mmissle;
	int	frate;


	char buffer[100];


	pDirDraw->ClearSurface(BACKBUFFERNUM);
	resultok = pDirDraw->BlitImage(&CPoint(100, 50), murksplash,
		&CRect(0, 0, 800, 600));

	flag = FALSE;
	x = 90;
	y = 640;

	for (i = 1; i <= 5; i++) {

		recu1 = 0;
		recu2 = 882;
		recl1 = 226;
		recl2 = 979;
		adjust = 0;

		if (mousehitx != 0) {
			if (mousehitx < 273 && mousehity >600 && bdown == 1 && i == 1 ||
				mousehitx < 424 && mousehitx > 273 && mousehity > 600 && bdown == 1 && i == 2 ||
				mousehitx < 575 && mousehitx > 424 && mousehity > 600 && bdown == 1 && i == 3 ||
				mousehitx < 725 && mousehitx > 575 && mousehity > 600 && bdown == 1 && i == 4 ||
				mousehitx < 875 && mousehitx > 725 && mousehity > 600 && bdown == 1 && i == 5) {
				recu1 = 226;
				recu2 = 882;
				recl1 = 452;
				recl2 = 980;
				adjust = 4;
				flag = TRUE;
			}
		}
		resultOK = pDirDraw->BlitImage(&CPoint(x, y),
			partsBSurfaceNum, &CRect(recu1, recu2, recl1, recl2));

		switch (i) {
		case 1:
			recu1 = 452;
			recu2 = 882;
			recl1 = 678;
			recl2 = 931;
			break;
		case 2:
			recu1 = 452;
			recu2 = 931;
			recl1 = 678;
			recl2 = 980;
			break;
		case 3:
			recu1 = 0;
			recu2 = 1078;
			recl1 = 226;
			recl2 = 1127;
			break;
		case 4:
			recu1 = 0;
			recu2 = 1127;
			recl1 = 226;
			recl2 = 1176;
			break;
		case 5:
			recu1 = 226;
			recu2 = 1078;
			recl1 = 452;
			recl2 = 1127;
			break;

		}

		resultOK = pDirDraw->BlitImage(&CPoint(x + adjust, y + 25 + adjust),
			partsBSurfaceNum, &CRect(recu1, recu2, recl1, recl2));

		x = x + 150;
	}


	//		resultOK = pDirDraw->BlitImage(&CPoint(775 ,580),
	//		partsBSurfaceNum, &CRect(449, 1054, 665,1115));


	resultOK = pDirDraw->BlitImage(&CPoint(775, 560),
		partsBSurfaceNum, &CRect(449, 1054, 665, 1140));



	resultok = pDirDraw->FlipSurfaces();

	if (mousehitx != 0 || mousehity != 0) {
		if (mousehitx < 273 && mousehity >600) {

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferButton);


			mousehitx = 0;
			mousehity = 0;
			showmission = 1;


		}
		if (mousehitx < 424 && mousehitx > 273 && mousehity > 600) {

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferButton);

			KillTimer(hWindow, 1);
			strcpy(networkresponse, "~Welcome to Multiplayer Murk~~");
			showdialogs();

			if (abortgame == 2) {
				UINT resultclick = MessageBox(hWindow, "DirectPlay initialization Failed.\nPlease make sure you have\nDirectX (tm) Installed.", "Direct Play Initialization Failed", MB_OK);
			}
			if (abortgame == 3) {
				UINT resultclick = MessageBox(hWindow, "DirectPlay Create Session Failed.\nPlease make sure you have\nDirectX (tm) Installed.", "Direct Play CreateSession Failed", MB_OK);
			}
			if (abortgame == 4) {
				UINT resultclick = MessageBox(hWindow, "DirectPlay Connect To Session Failed.\nPlease make sure you have\nDirectX (tm) Installed.\nOr There may be a problem with the server", "Direct Play Connect To Failed", MB_OK);
			}
			if (abortgame == 5) {
				UINT resultclick = MessageBox(hWindow, "The server is FULL.", "Server is Full", MB_OK);
			}

			if (!ishost && !doit) {

				movelevel--;
				setability = 1;
				new_level(0);

				//				SetTimer(hWindow, 2, 4000, TimerProc);		
				showmission = 4;
				SetTimer(hWindow, 1, 50, TimerProc);
				networkgame = 1;
				//				SetTimer(hWindow, 3, timerlength, TimerProc);		
				istimerset = 1;
				receiveok = 1;


			}
			else if (ishost && !doit) {
				movelevel--;
				setability = 1;
				new_level(0);

				networkgame = 1;
				SetTimer(hWindow, 3, timerlength, TimerProc);
				istimerset = 1;
				showmission = 3;
				receiveok = 1;
			}
			else {
				mousehit = 0;
				hitanykey = 0;
				showmission = 0;
				mousehitx = 0;
				mousehity = 0;
				SetTimer(hWindow, 1, 50, TimerProc);
			}

		}
		if (mousehitx < 575 && mousehitx > 424 && mousehity > 600) {
			//pppp
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferButton);

			hitanykey = 0;
			result = load_game();

			if (result && !shareware) {
				KillTimer(hWindow, 1);
				//					reset_game();
				//					result=load_game() ;
				//					new_level(0);
				//					mousehit=0;			
				//					hitanykey=0;
				//					showmission=2;

				//



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

					reset_game(1);

					new_level(0);
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


					mousehit = 0;
					hitanykey = 1;
					showmission = 3;
					SetTimer(hWindow, 1, 50, TimerProc);

					//							SetTimer(hWindow, 3, timerlength, TimerProc);		
				}
			}
			else {
				KillTimer(hWindow, 1);
				resultok = pDirDraw->FlipSurfaces();
				pDirDraw->FlipToGDISurface();
				if (shareware)
					UINT resultclick = MessageBox(hWindow, "Cannot Load in Shareware Version", "Shareware Load", MB_OK);

				SetTimer(hWindow, 1, 50, TimerProc);

				mousehit = 0;
				hitanykey = 0;
				showmission = 0;
				mousehitx = 0;
				mousehity = 0;

			}

		}


		if (mousehitx < 725 && mousehitx > 575 && mousehity > 600) {

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferButton);



			if (!networkserver) {
				resultok = pDirDraw->FlipSurfaces();
				pDirDraw->FlipToGDISurface();
			}
			bdown = 1;
			KillTimer(hWindow, 1);

			DialogBox(NULL, (LPCTSTR)IDD_REG, hWindow,
				(DLGPROC)DlgRegister);



			result = GetDlgItemText(hWindow, IDC_EDIT1, &buffer[0], 50);








			SetTimer(hWindow, 1, 50, TimerProc);
			mousehitx = 0;
			mousehity = 0;

		}
		if (mousehitx < 876 && mousehitx > 725 && mousehity > 600) {

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferButton);


			PostMessage(hWindow, WM_CLOSE, 0, 0L);
			mousehitx = 0;
			mousehity = 0;

		}



	}
	if (bdown == 0) {
		mousehitx = 0;
		mousehity = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////

void sendtext(char p1[900]) {

	int result;
	int framex = 0;
	int framey = 0;
	int frame2x = 0;
	int frame2y = 0;

	int penx, peny;
	int size = 0;
	int count;

	int whichone;
	int playergrid = 0;
	int stop;
	char c;
	char p2[600];
	char result2[600];

	stop = 0;
	//	penx = 150;

	penx = 1;
	//	peny = 630;
	peny = 605;

	count = 0;
	whichone = 0;

	strcpy(result2, "");
	strcpy(p2, "");



	sprintf(result2, "%3d", countlevels + 1);

	strcat(result2, "~");

	//	if (newwarrior >=4)
	//		newwarrior2 = 4;
	//	else
	//		newwarrior2 = newwarrior;
	//	newwarrior2 = 1;
	//	for (i=1;i<=newwarrior2;i++) {
	sprintf(p2, "%4d", PlayerLocation[1].armour);

	strcat(result2, p2);
	//		if (i !=newwarrior2)
	//			strcat(result2,"    ");
	//	}

	strcat(result2, "~");

	//	for (i=1;i<=newwarrior2;i++) {
	sprintf(p2, "%4d", PlayerLocation[1].ability);
	strcat(result2, p2);
	//		if (i !=newwarrior2)
	//			strcat(result2,"    ");

	//	}

	strcat(result2, "~");

	//	for (i=1;i<=newwarrior2;i++) {
	sprintf(p2, "%4d", PlayerLocation[1].skill);
	strcat(result2, p2);
	//		if (i !=newwarrior2)
	//			strcat(result2,"    ");

	//	}
	strcat(result2, "~");

	//	for (i=1;i<=newwarrior2;i++) {
	sprintf(p2, "%4d", PlayerLocation[1].ring);
	strcat(result2, p2);
	//		if (i !=newwarrior2)
	//			strcat(result2,"    ");
	//		
	//	}

	strcat(result2, "~\0");
	strcpy(p2, "");
	strcpy(p2, result2);

	if (showmission == 2) {


		if (networkgame == 1) {
			//		strcpy(p2,"~~Episode Objective~~");
			strcpy(p2, "Find the -p   and return it home");
		}
		else {
			strcpy(p2, "~~Episode Objective~~");
			strcat(p2, gamedef.response);
		}

		penx = 180;
		peny = 90;
	}
	if (showmission == 4) {
		strcpy(p2, networkresponse);

		if (gameerror == 1) {
			strcpy(p2, "Murk Version 1.12~~You are running an old version of Murk.~~Please visit www.murk.on.ca for an update!~~Click to return to the Main Screen.");
		}
		penx = 180;
		peny = 90;
	}
	if (gamescreen == 1) {
		strcpy(p2, chat[0].say);
		penx = 240;
		peny = 640;
	}

	if (gamescreen == 2 && chaton) {
		strcpy(p2, networkresponse);
		penx = 240;
		peny = 700;
	}


	if (helpscreen == 1) {
		strcpy(p2, "                Murk Help      ~~F1: Help Screen   F2: Episode Information~F3: Save Game     F4: Load Game~F5: Chat          F6: Stats~~Mouse Controls:~Left Mouse Botton:  Move~Right Mouse Button: Fire~Double click Left Button to use stairs.~~Keyboard Controls:~Arrows :  Move~CTRL   :  Fire~SHIFT  :  Up & Down Stairs~R Key to use Ring~T Key to Drop Treasure~~Programmed by Mark Longo (mlongo@murk.on.ca)~Graphics by Tom Portaccio~~www.murk.on.ca for updates!");
	}

	size = strlen(p2);
	for (count = 0; count < size; count++) {
		stop = 0;
		playergrid = 0;
		c = p2[count];
		switch (c) {
		case 'A':
			framex = 0;
			framey = 588;
			break;
		case 'B':
			framex = 57;
			framey = 588;
			break;
		case 'C':
			framex = 114;
			framey = 588;
			break;
		case 'D':
			framex = 171;
			framey = 588;
			break;
		case 'E':
			framex = 228;
			framey = 588;
			break;
		case 'F':
			framex = 285;
			framey = 588;
			break;
		case 'G':
			framex = 342;
			framey = 588;
			break;
		case 'H':
			framex = 399;
			framey = 588;
			break;
		case 'I':
			framex = 456;
			framey = 588;
			break;
		case 'J':
			framex = 513;
			framey = 588;
			break;
		case 'K':
			framex = 570;
			framey = 588;
			break;
		case 'L':
			framex = 627;
			framey = 588;
			break;
		case 'M':
			framex = 684;
			framey = 588;
			break;
		case 'N':
			framex = 741;
			framey = 588;
			break;
		case 'O':
			framex = 0;
			framey = 637;
			break;
		case 'P':
			framex = 57;
			framey = 637;
			break;
		case 'Q':
			framex = 114;
			framey = 637;
			break;
		case 'R':
			framex = 171;
			framey = 637;
			break;
		case 'S':
			framex = 228;
			framey = 637;
			break;
		case 'T':
			framex = 285;
			framey = 637;
			break;
		case 'U':
			framex = 342;
			framey = 637;
			break;
		case 'V':
			framex = 399;
			framey = 637;
			break;
		case 'W':
			framex = 456;
			framey = 637;
			break;
		case 'X':
			framex = 513;
			framey = 637;
			break;
		case 'Y':
			framex = 570;
			framey = 637;
			break;
		case 'Z':
			framex = 627;
			framey = 637;
			break;
		case ' ':
			framex = 684;
			framey = 637;
			break;
		case '~':
			peny = peny + 25;

			if (showmission == 2)
				penx = 180;
			else
				penx = 150;

			if (gamescreen == 1) {
				penx = 240;
			}

			stop = 1;
			break;


		case '1':
			framex = 0;
			framey = 686;
			break;
		case '2':
			framex = 57;
			framey = 686;
			break;
		case '3':
			framex = 114;
			framey = 686;
			break;
		case '4':
			framex = 171;
			framey = 686;
			break;
		case '5':
			framex = 228;
			framey = 686;
			break;
		case '6':
			framex = 285;
			framey = 686;
			break;
		case '7':
			framex = 342;
			framey = 686;
			break;
		case '8':
			framex = 399;
			framey = 686;
			break;
		case '9':
			framex = 456;
			framey = 686;
			break;
		case '0':
			framex = 513;
			framey = 686;
			break;


		case '.':
			framex = 570;
			framey = 686;
			break;

		case ',':
			framex = 627;
			framey = 686;
			break;

		case '(':
			framex = 678;
			framey = 686;
			break;

		case ')':
			framex = 729;
			framey = 686;
			break;
		case 'a':
			framex = 0;
			framey = 735;
			break;
		case 'b':
			framex = 57;
			framey = 735;
			break;
		case 'c':
			framex = 114;
			framey = 735;
			break;
		case 'd':
			framex = 171;
			framey = 735;
			break;
		case 'e':
			framex = 228;
			framey = 735;
			break;
		case 'f':
			framex = 285;
			framey = 735;
			break;
		case 'g':
			framex = 342;
			framey = 735;
			break;
		case 'h':
			framex = 399;
			framey = 735;
			break;
		case 'i':
			framex = 456;
			framey = 735;
			break;
		case 'j':
			framex = 513;
			framey = 735;
			break;
		case 'k':
			framex = 570;
			framey = 735;
			break;
		case 'l':
			framex = 627;
			framey = 735;
			break;
		case 'm':
			framex = 684;
			framey = 735;
			break;
		case 'n':
			framex = 741;
			framey = 735;
			break;
		case 'o':
			framex = 0;
			framey = 784;
			break;
		case 'p':
			framex = 57;
			framey = 784;
			break;
		case 'q':
			framex = 114;
			framey = 784;
			break;
		case 'r':
			framex = 171;
			framey = 784;
			break;
		case 's':
			framex = 228;
			framey = 784;
			break;
		case 't':
			framex = 285;
			framey = 784;
			break;
		case 'u':
			framex = 342;
			framey = 784;
			break;
		case 'v':
			framex = 399;
			framey = 784;
			break;
		case 'w':
			framex = 456;
			framey = 784;
			break;
		case 'x':
			framex = 513;
			framey = 784;
			break;
		case 'y':
			framex = 570;
			framey = 784;
			break;
		case 'z':
			framex = 627;
			framey = 784;
			break;



		case '!':
			framex = 0;
			framey = 833;
			break;
		case '@':
			framex = 57;
			framey = 833;
			break;
		case '#':
			framex = 114;
			framey = 833;
			break;
		case '$':
			framex = 171;
			framey = 833;
			break;
		case '%':
			framex = 228;
			framey = 833;
			break;
		case '^':
			framex = 285;
			framey = 833;
			break;
		case '&':
			framex = 342;
			framey = 833;
			break;
		case '*':
			framex = 399;
			framey = 833;
			break;
		case '?':
			framex = 456;
			framey = 833;
			break;
		case '\"':
			framex = 513;
			framey = 833;
			break;
		case ':':
			framex = 570;
			framey = 833;
			break;


		case '>':
			framex = 684;
			framey = 784;
			break;
		case '-':
			c = p2[++count];
			switch (c) {

			case 'a':
				playergrid = 1;
				framex = 384;
				framey = 0 + 64;
				break;
			case 'b':
				playergrid = 1;
				framex = 384;
				framey = 128 + 64;
				break;
			case 'c':
				playergrid = 1;
				framex = 384;
				framey = 256 + 64;
				break;
			case 'd':
				playergrid = 1;
				framex = 384;
				framey = 384 + 64;
				break;
			case 'e':
				playergrid = 1;
				framex = 384;
				framey = 512 + 64;
				break;
			case 'f':
				playergrid = 1;
				framex = 384;
				framey = 640 + 64;
				break;
			case 'g':
				playergrid = 1;
				framex = 384;
				framey = 768 + 64;
				break;
			case 'h':
				playergrid = 1;
				framex = 384;
				framey = 896 + 64;
				break;
			case 'i':
				playergrid = 1;
				framex = 384;
				framey = 1024 + 64;
				break;


			case 'j':
				playergrid = 2;
				framex = 0;
				framey = 294;
				break;
			case 'k':
				playergrid = 2;
				framex = 113;
				framey = 294;
				break;
			case 'l':
				playergrid = 2;
				framex = 226;
				framey = 294;
				break;
			case 'm':
				playergrid = 2;
				framex = 339;
				framey = 294;
				break;
			case 'n':
				playergrid = 2;
				framex = 452;
				framey = 294;
				break;
			case 'o':
				playergrid = 2;
				framex = 565;
				framey = 294;
				break;
			case 'p':
				playergrid = 2;
				framex = 678;
				framey = 294;
				break;
			case 'q':
				playergrid = 2;
				framex = 458;
				framey = 0;
				break;
			case 'r':
				playergrid = 2;
				framex = 226;
				framey = 490;
				break;
			case 's':
				playergrid = 2;
				framex = 571;
				framey = 0;
				break;
			case 't':
				playergrid = 2;
				framex = 684;
				framey = 0;
				break;
			case 'u':
				playergrid = 2;
				framex = 0;
				framey = 294;
				break;
			case 'v':
				playergrid = 2;
				framex = 339;
				framey = 490;
				break;
			case 'w':
				playergrid = 2;
				framex = 565;
				framey = 490;
				break;
			case 'x':
				playergrid = 2;
				framex = 113;
				framey = 0;
				break;
			case 'y':
				playergrid = 2;
				framex = 226;
				framey = 0;
				break;

			}
			break;
		}
		if (stop == 0) {
			if (playergrid == 0) {
				result = pDirDraw->BlitImage(&CPoint(penx, peny),
					partsBSurfaceNum, &CRect(framex, framey, framex + 56, framey + 49));
				penx = penx + 15;

			}
			else if (playergrid == 1) {

				peny = peny - 20;

				result = pDirDraw->BlitImage(&CPoint(penx, peny),
					partsASurfaceNum, &CRect(framex, framey, framex + 64, framey + 64));

				penx = penx + 30;

				peny = peny + 20;
			}
			else {
				peny = peny - 20;
				result = pDirDraw->BlitImage(&CPoint(penx, peny),
					partsBSurfaceNum, &CRect(framex, framey, framex + 113, framey + 98));
				penx = penx + 20;
				peny = peny + 20;


			}
		}
		if (penx > 980) {
			peny = peny + 25;
			penx = 5;
		}

	}

}

//////////////////////////////////////////////////////////////////////////////

void showscroll() {

	int resultok;
	int recu1, recu2, recl1, recl2;
	int adjust;
	BOOL flag;

	flag = FALSE;

	pDirDraw->ClearSurface(BACKBUFFERNUM);
	resultok = pDirDraw->BlitImage(&CPoint(100, 80), scroll,
		&CRect(0, 0, 800, 600));


	recu1 = 0;
	recu2 = 882;
	recl1 = 226;
	recl2 = 979;

	adjust = 0;

	if ((showmission != 4 && helpscreen == 0))
		if (mousehitx < 1000 && mousehitx > 0 && mousehity > 0 && bdown == 1 && showmission == 2 ||
			GetAsyncKeyState(32) < 0 && showmission == 2 || GetAsyncKeyState(13) && showmission == 2 < 0
			) {
			recu1 = 226;
			recu2 = 882;
			recl1 = 452;
			recl2 = 980;
			adjust = 4;
			flag = TRUE;
			//			if (m_directSoundOK)
			//				m_pDirSound->PlaySound(m_bufferButton);
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferProtection);

		}

	if (helpscreen == 1 && (showmission != 4) && showmission == 2)
		if (mousehitx < 1000 && mousehitx > 0 && mousehity > 0 && bdown == 1 ||
			GetAsyncKeyState(32) < 0 || GetAsyncKeyState(13) < 0
			) {
			recu1 = 226;
			recu2 = 882;
			recl1 = 452;
			recl2 = 980;
			adjust = 4;
			helpscreen = 0;
			flag = TRUE;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferProtection);

			//			SetTimer(hWindow, 2, 4000, TimerProc);		
			//			SetTimer(hWindow, 3, timerlength, TimerProc);		

		}




	if (flag) {
		pDirDraw->ClearSurface(BACKBUFFERNUM);


		helpscreen = 0;
		//		KillTimer(hWindow, 1);	
		hitanykey = 1;
		showmission = 3;
		if (ishost && networkgame == 0 || !ishost && networkgame) {
			SetTimer(hWindow, 3, timerlength, TimerProc);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void showselect() {


	//ooops
	int resultok;
	if (mousehitx != 0 && mousehity != 0) {
		if (mousehitx < 512 && mousehity < 384) {
			//thief

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferSwordThief);

			if (select1 == 0)
				select1 = 1;
			else
				select1 = 0;
		}
		if (mousehitx > 512 && mousehity < 384) {
			//cleric
			if (select2 == 0)
				select2 = 1;
			else
				select2 = 0;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferTadaCleric);

		}
		if (mousehitx < 512 && mousehity>384) {
			//wizard
			if (select3 == 0)
				select3 = 1;
			else
				select3 = 0;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferEyeswizard);


		}
		if (mousehitx > 512 && mousehity > 384) {
			//figher
			if (select4 == 0)
				select4 = 1;
			else
				select4 = 0;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferSwordBarb);


		}
	}
	mousehitx = 0;
	mousehity = 0;

	if (select1 == 0)
		resultok = pDirDraw->BlitImage(&CPoint(90, 80), selectplayer,
			&CRect(0, 0, 400, 325));
	else
		resultok = pDirDraw->BlitImage(&CPoint(90, 80), selectplayer,
			&CRect(400, 0, 800, 325));

	if (select2 == 0)
		resultok = pDirDraw->BlitImage(&CPoint(490, 80), selectplayer,
			&CRect(0, 325, 400, 650));
	else
		resultok = pDirDraw->BlitImage(&CPoint(490, 80), selectplayer,
			&CRect(400, 325, 800, 650));

	if (select3 == 0)
		resultok = pDirDraw->BlitImage(&CPoint(90, 400), selectplayer,
			&CRect(0, 650, 400, 975));
	else
		resultok = pDirDraw->BlitImage(&CPoint(90, 400), selectplayer,
			&CRect(400, 650, 800, 975));

	if (select4 == 0)
		resultok = pDirDraw->BlitImage(&CPoint(490, 400), selectplayer,
			&CRect(0, 975, 400, 1300));
	else
		resultok = pDirDraw->BlitImage(&CPoint(540, 400), selectplayer,
			&CRect(400, 975, 800, 1300));

	resultok = pDirDraw->BlitImage(&CPoint(220, 20), selectplayer,
		&CRect(39, 1375, 590, 1462));
	if (select1 != 0 || select2 != 0 || select3 != 0 || select4 != 0) {
		swap_players();
		resultok = pDirDraw->FlipSurfaces();

		movelevel--;
		new_level(0);
		Sleep(1200);
		pDirDraw->ClearSurface(BACKBUFFERNUM);
		//		showmission=2;
		hitanykey = 1;
		showmission = 3;
		mousehitx = 0;
		mousehity = 0;
	}


}

//////////////////////////////////////////////////////////////////////////////

void read_levels() {
	FILE* fp;
	int work;
	int count = 1;
	int abort;


	char fname[20];

	abort = 0;

	strcpy(fname, "levelsb.dat");

	if (PlayerLocation[1].image == 1)
		strcpy(fname, "levelsb.dat");
	else if (PlayerLocation[1].image == 3)
		strcpy(fname, "levelst.dat");
	else if (PlayerLocation[1].image == 4)
		strcpy(fname, "levelsw.dat");
	else if (PlayerLocation[1].image == 5)
		strcpy(fname, "levelsc.dat");


	if (ishost && networkgame) {
		strcpy(fname, "levelsn.dat");
	}


	while (abort == 0) {
		if ((fp = fopen(fname, "r")) == NULL) {
			return;
		}

		if (feof(fp)) {
			debug_me("read_levels", "Eof ..resetting", 0, 0);
			levelcounter = 1;
		}

		for (count = 1; count <= levelcounter; count++)
		{

			fgets(response, 890, fp);
			strcpy(gamedef.response, response);

			fgets(response, 890, fp);
			strcpy(gamedef.title, response);


			if (networkgame) {
				//			strcpy(gamedef.title,"~~Episode Objective~~");
				strcpy(gamedef.title, "Find the -p   and return it home");
				//			strcpy(gamedef.response,"~~Episode Objective~~");
				strcpy(gamedef.response, "Find the -p   and return it home");
			}



			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.gametype = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.beholders = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.bspeed = work;


			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.deathorb = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.dspeed = work;


			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.skeletons = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.sspeed = work;


			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.mouther = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.mspeed = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.ogre = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.ospeed = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.fighter = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.fteam = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.fspeed = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.cleric = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.cteam = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.cspeed = work;


			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.thief = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.tteam = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.tspeed = work;


			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.wizard = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.wteam = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.wspeed = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.dungeonx = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.dungeony = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.dungeonlvl = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.pits = work;

			fgets(response, 890, fp);
			work = atoi(response);
			gamedef.maxpit = work;

			if (feof(fp)) {
				debug_me("read_levels", "Eof ..resetting", 0, 0);
				levelcounter = 1;
				count = 1;
				fclose(fp);
				abort = 0;
				break;
			}
			else
				abort = 1;
		}
	}



	/*
		char response[900];
		char title[80];
		int gametype;
		int beholders;
		int bspeed;
		int deathorb;
		int dspeed;
		int skeletons;
		int sspeed;
		int mouther;
		int mspeed;
		int ogre;
		int ospeed;
		int fighter;
		int fspeed;
		int fteam;
		int thief;
		int tspeed;
		int tteam;
		int cleric;
		int cspeed;
		int cteam;
		int wizard;
		int wspeed;
		int wteam;
		int dungeonx;
		int dungeony;
		int dungeonlvl;
		int pits;
		int maxpit;
	*/


}



void swap_players() {

	if (select1 == 1) {
		PlayerLocation[1].image = 3;
		dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 1;
	}
	else if (select2 == 1) {
		PlayerLocation[1].image = 5;
		dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 3;
	}
	else if (select3 == 1) {
		PlayerLocation[1].image = 4;
		dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 2;
	}
	else if (select4 == 1) {
		dungeon[PlayerLocation[1].copx][PlayerLocation[1].copy][PlayerLocation[1].coplvl].image = 0;
		PlayerLocation[1].image = 1;
	}
}


void show_panel() {

	int i;

	int adjust;
	for (i = 1; i <= 1; i++) {
		adjust = 0;
		if (i == 1)
			adjust = 0;
		else if (i == 2)
			adjust = adjust + 256;
		else if (i == 3)
			adjust = adjust + 256 + 256;
		else if (i == 4)
			adjust = adjust + 256 + 256 + 256;

		resultok = pDirDraw->BlitImage(&CPoint(0 + adjust, 575), panelSurfaceNum,
			&CRect(0, 0, 259, 189));

		if (i == 1) {
			if (foundtreasure == 1 || foundtreasure2 == 1) {
				if (mousehitx < 150 && mousehitx > 100 && mousehity > 600 && bdown == 1) {
					resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 90, 575 + 14),
						partsBSurfaceNum, &CRect(265, 986, 335, 1034));
				}
				else {
					resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 90, 575 + 14),
						partsBSurfaceNum, &CRect(39, 984, 109, 1035));
				}
				if (foundtreasure == 1) {
					resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 82, 575 + 14),
						partsBSurfaceNum, &CRect(14, 313, 84, 359));
				}
				if (foundtreasure2 == 1) {
					resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 81, 575 + 18),
						partsBSurfaceNum, &CRect(688, 318, 761, 361));
				}
			}
			if (PlayerLocation[1].ring > 0) {
				if (mousehitx < 220 && mousehitx > 160 && mousehity > 600 && bdown == 1) {
					resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 90 + 70, 575 + 14),
						partsBSurfaceNum, &CRect(265, 986, 335, 1034));
				}
				else {
					resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 90 + 70, 575 + 14),
						partsBSurfaceNum, &CRect(39, 984, 109, 1035));
				}
				resultok = pDirDraw->BlitImage(&CPoint(0 + adjust + 90 + 70 + 18, 575 + 14 + 8),
					partsBSurfaceNum, &CRect(711, 33, 745, 67));
			}
		}
	}
	gamescreen = 0;
	sendtext("");


	if (strlen(chat[0].say)) {
		gamescreen = 1;
		sendtext("");
		chat[0].duration--;
		if (chat[0].duration < 1) {
			chat[0].duration = 0;
			strcpy(chat[0].say, "");
		}


	}

	if (strlen(networkresponse)) {
		gamescreen = 2;
		sendtext("");

	}

	gamescreen = 0;

	return;
}
