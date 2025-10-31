
//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
extern void check_mouse();
extern void play_random_music();
extern void stop_song();

extern void check_input();

void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime) {

	int dripit;

	if (nIDEvent == 2) {
		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferMusic);
		dripit = random_num(200);
		if (dripit == 1) {
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferDrip);
		}

		dripit = random_num(5);
		dripit = 1;
		if (dripit == 1) {
			//		assign_pit();
			//			assign_elixir(2+maxlevel);
		}
	} else if (nIDEvent == 1) {
		if (hitanykey == 1) {
			showmission++;
			hitanykey = 0;
			if (showmission >= 3) {
				hitanykey = 1;

				KillTimer(hWindow, 1);
				if (ishost && networkgame == 0 || bdown == 1 && networkgame && !ishost) {

					showmission = 3;
					//					if (m_directSoundOK)
					//						stop_song() ;

					//					if (m_directSoundOK)
					//					play_random_music();
					SetTimer(hWindow, 3, timerlength, TimerProc);
				}
			}
		} else {
			if (showmission == 0) {
				if (!networkserver) {
					countlevels = 0;
					levelcounter = 1;
					setability = 1;
					showsplash();
				}
			} else if (showmission == 1) {
				if (!networkserver) {
					pDirDraw->ClearSurface(BACKBUFFERNUM);
					showselect();
					resultok = pDirDraw->FlipSurfaces();
				}
			} else if (showmission == 2) {
				if (!networkserver) {
					pDirDraw->ClearSurface(BACKBUFFERNUM);
					showscroll();
					sendtext("test");
					resultok = pDirDraw->FlipSurfaces();
				}
			} else if (showmission == 4) {
				if (!networkserver) {
					pDirDraw->ClearSurface(BACKBUFFERNUM);
					showscroll();
					sendtext("test");
					resultok = pDirDraw->FlipSurfaces();
				}
			}
		}
	} else if (nIDEvent == 3) {
		if (framemove == 0) {
			framemove = 1;

			display_dungeon(0);

			if (!networkserver)
				check_input();
			framemove = 0;
		}
	}
}
