
//  Murk Ver 1.12 By Mark Longo 
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////
extern int play_again();
extern void send_play_sound(int sound);
extern void send_reward(int slot, int reward);
extern void play_random_music();
extern void stop_song();






extern int beholderability;
extern int moutherability;
extern int orcability;
extern int skeletonability;
extern int deathorbability;





extern void send_death(int player, int x, int y, int lvl, int image, int
	direction, int counter, int mapx, int mapy, int dead);

void kill_monster(int t, int winner);

void computermove() {

	int px, py;

	int rnum;
	char mvx[14], mvy[14];
	char playermv[14];
	BOOL gox, goy;
	int gothere;
	int findmonster;

	char lastmovesave[100];
	int slide;
	int len;
	int counter;
	int subcounter;
	//	int shortestroute;
	int lastdir=0;
	char response[50];
	int saveroute;

	BOOL flag, flag2;

	int SUBMAX = 20;
	int COUNTMAX = 20;

	int backatstart = 0;
	char* pdest;
	int num;
	int level;

	level = PlayerLocation[CurrentPlayer].level;

	flag = 1;
	findmonster = 0;

	subcounter = 0;
	px = PlayerLocation[CurrentPlayer].mapx;
	py = PlayerLocation[CurrentPlayer].mapy;


	shortestroute = 9999;

	counter = 0;
	gothere = 0;
	strcpy(playermv, "");
	len = random_num(55);

	strcpy(lastmovesave, PlayerLocation[CurrentPlayer].lastmove);

	while (flag) {
		if (px == dx && py == dy) {
			if (counter < shortestroute) {
				shortestroute = counter;
				saveroute = gothere;
				px = PlayerLocation[CurrentPlayer].mapx;
				py = PlayerLocation[CurrentPlayer].mapy;
				backatstart = 0;
				counter = 0;
				subcounter++;
				strcpy(lastmove, lastmovesave);
				if (subcounter > SUBMAX) {
					flag = 0;
					break;
				}
			}
		}
		strcpy(playermv, "");
		strcpy(mvx, "");
		strcpy(mvy, "");
		gox = 0;
		goy = 0;
		len = 0;
		slide = 1;
		if (px > dx) {
			len = random_num(24);
			if (len == 1)
				strcat(mvx, "3");
			else
				strcat(mvx, "1");
		}
		else if (px < dx) {
			len = random_num(24);
			if (len == 1)
				strcat(mvx, "1");
			else
				strcat(mvx, "3");
		}
		else {
			len = random_num(100);
			if (len == 0)
				strcat(mvx, "1");
			else if (len == 1)
				strcat(mvx, "3");
			else
				strcat(mvx, "-");
		}
		if (py > dy) {
			len = random_num(24);
			if (len == 1)
				strcat(mvy, "4");
			else
				strcat(mvy, "2");
		}
		else if (py < dy) {
			len = random_num(24);
			if (len == 1)
				strcat(mvy, "2");
			else
				strcat(mvy, "4");
		}
		else {

			len = random_num(100);
			if (len == 0)
				strcat(mvy, "2");
			else if (len == 1)
				strcat(mvy, "4");
			else
				strcat(mvy, "-");
		}
		if (dungeon[px - 1][py][level].type == 'f' || dungeon[px - 1][py][level].type == 's') {
			strcat(playermv, "1");
		}
		if (dungeon[px + 1][py][level].type == 'f' || dungeon[px + 1][py][level].type == 's') {
			strcat(playermv, "3");
		}
		if (dungeon[px][py - 1][level].type == 'f' || dungeon[px][py - 1][level].type == 's') {
			strcat(playermv, "2");
		}
		if (dungeon[px][py + 1][level].type == 'f' || dungeon[px][py + 1][level].type == 's') {
			strcat(playermv, "4");
		}

		strcpy(response, playermv);
		strcpy(playermv, "");
		if (dungeon[px - 1][py][level].type == 'f' || dungeon[px - 1][py][level].type == 's') {
			if (!findbeholder(px - 1, py, level) || strlen(response) == 1) {
				strcat(playermv, "1");
			}
		}
		if (dungeon[px + 1][py][level].type == 'f' || dungeon[px + 1][py][level].type == 's') {
			if (!findbeholder(px + 1, py, level) || strlen(response) == 1) {
				strcat(playermv, "3");
			}
		}
		if (dungeon[px][py - 1][level].type == 'f' || dungeon[px][py - 1][level].type == 's') {
			if (!findbeholder(px, py - 1, level) || strlen(response) == 1)
				strcat(playermv, "2");
		}
		if (dungeon[px][py + 1][level].type == 'f' || dungeon[px][py + 1][level].type == 's') {
			if (!findbeholder(px, py + 1, level) || strlen(response) == 1)
				strcat(playermv, "4");
		}
		if (strlen(playermv) == 0)
			strcpy(playermv, response);
		len = random_num(100);
		if (strlen(playermv) > 1 && len != 1) {
			pdest = strstr(playermv, lastmove);
			if (pdest != NULL) {
				num = pdest - playermv;
				playermv[num] = '!';
			}
		}
		sprintf(response, "%d", lastdir);
		response[1] = '\0';

		if (strstr(playermv, mvx) != NULL) {
			gox = 1;
		}
		if (strstr(playermv, mvy) != NULL) {
			goy = 1;
		}

		if (gox == 1) {
			slide = atoi(mvx);
		}

		if (goy == 1) {
			slide = atoi(mvy);
		}

		if (gox == 1 && goy == 1) {
			rnum = random_num(2);
			if (rnum == 0)
				slide = atoi(mvx);
			else
				slide = atoi(mvy);
		}

		len = random_num(19);

		if (len == 0 && counter != 0) {
			gox = 0;
			goy = 0;
		}

		if (gox == 0 && goy == 0) {
			strcpy(response, "");
			flag2 = TRUE;
			while (flag2) {
				len = strlen(playermv);
				len = random_num(len);
				response[0] = playermv[len];
				response[1] = '\0';
				slide = atoi(response);
				if (strcmp(response, "!") != 0)
					flag2 = FALSE;
			}
		}

		if (counter == 0) {
			//		if (counter ==0 || backatstart==0) {



			gothere = slide;
			lastdir = gothere;
			saveroute = gothere;
			backatstart = 1;

		}
		if (slide == 1) {
			px = px - 1;
		}
		if (slide == 2) {
			py = py - 1;
		}
		if (slide == 3) {
			px = px + 1;
		}
		if (slide == 4) {
			py = py + 1;
		}
		if (px == PlayerLocation[CurrentPlayer].mapx && py == PlayerLocation[CurrentPlayer].mapy)
			backatstart = 0;

		if (slide == 1) {
			strcpy(lastmove, "3");
		}
		if (slide == 2) {
			strcpy(lastmove, "4");
		}
		if (slide == 3) {
			strcpy(lastmove, "1");
		}
		if (slide == 4) {
			strcpy(lastmove, "2");
		}
		counter++;
		if (counter > COUNTMAX) {
			counter = 0;
			backatstart = 0;
			subcounter++;
			px = PlayerLocation[CurrentPlayer].mapx;
			py = PlayerLocation[CurrentPlayer].mapy;
			strcpy(lastmove, lastmovesave);

			if (subcounter > SUBMAX) {
				flag = 0;
				break;
			}
		}
	}

	gothere = saveroute;

	lastdir = gothere;

	if (gothere == 1)
		PlayerLocation[CurrentPlayer].direction = 1;
	if (gothere == 2)
		PlayerLocation[CurrentPlayer].direction = 2;
	if (gothere == 3)
		PlayerLocation[CurrentPlayer].direction = 4;
	if (gothere == 4)
		PlayerLocation[CurrentPlayer].direction = 3;

	if (gothere == 1) {
		strcpy(lastmove, "3");
	}
	if (gothere == 2) {
		strcpy(lastmove, "4");
	}
	if (gothere == 3) {
		strcpy(lastmove, "1");
	}
	if (gothere == 4) {
		strcpy(lastmove, "2");
	}

	strcpy(lastmovesave, lastmove);
	strcpy(PlayerLocation[CurrentPlayer].lastmove, lastmove);

	if (shortestroute == 9999) {
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

void check_dungeon() {


	//	if (CurrentPlayer==camera) 
	if (PlayerLocation[CurrentPlayer].asleep == 0 || CurrentPlayer <= newwarrior) {
		if (dungeon[PlayerLocation[CurrentPlayer].mapx - 1][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].type == 'f' || dungeon[PlayerLocation[CurrentPlayer].mapx - 1][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].type == 's') {
			dungeon[PlayerLocation[CurrentPlayer].mapx - 1][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].explored = 1;
		}
		if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy - 1][PlayerLocation[CurrentPlayer].level].type == 'f' || dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy - 1][PlayerLocation[CurrentPlayer].level].type == 's') {
			dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy - 1][PlayerLocation[CurrentPlayer].level].explored = 1;
		}
		if (dungeon[PlayerLocation[CurrentPlayer].mapx + 1][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].type == 'f' || dungeon[PlayerLocation[CurrentPlayer].mapx + 1][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].type == 's') {
			dungeon[PlayerLocation[CurrentPlayer].mapx + 1][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].explored = 1;
		}
		if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy + 1][PlayerLocation[CurrentPlayer].level].type == 'f' || dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy + 1][PlayerLocation[CurrentPlayer].level].type == 's') {
			dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy + 1][PlayerLocation[CurrentPlayer].level].explored = 1;
		}
		dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].explored = 1;
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

void make_your_move() {

	switch (PlayerLocation[CurrentPlayer].direction) {
	case 1:
		PlayerLocation[CurrentPlayer].mapx = PlayerLocation[CurrentPlayer].mapx - 1;
		break;
	case 2:
		PlayerLocation[CurrentPlayer].mapy = PlayerLocation[CurrentPlayer].mapy - 1;
		break;
	case 3:
		PlayerLocation[CurrentPlayer].mapy = PlayerLocation[CurrentPlayer].mapy + 1;
		break;
	case 4:
		PlayerLocation[CurrentPlayer].mapx = PlayerLocation[CurrentPlayer].mapx + 1;
		break;
	}




}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////


void wakeup_sleepy_heads() {

	int i;
	int j, k;
	//	int rand;	
	int saveCurrent;

	saveCurrent = CurrentPlayer;

	for (i = 1; i <= NumPlayers; i++) {
		if (foundtreasure2 != 0 && PlayerLocation[i].asleep == 1 && gametype == 1) {
			PlayerLocation[i].asleep = 0;
			CurrentPlayer = i;
			check_dungeon();
			CurrentPlayer = saveCurrent;
			if (m_directSoundOK) {
				if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 2)
					m_pDirSound->PlaySound(m_bufferScream);

				if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 8)
					m_pDirSound->PlaySound(m_bufferYell);

				if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 9)
					m_pDirSound->PlaySound(m_bufferDorb);

				if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 6)
					m_pDirSound->PlaySound(m_bufferSkeleton);

				if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 7)
					m_pDirSound->PlaySound(m_bufferMumble);

			}
		}
		else if (PlayerLocation[i].asleep == 1) {
			for (k = -3; k <= 3; k++) {
				for (j = -3; j <= 3; j++) {
					if (((PlayerLocation[i].mapx + j) >= 1) && ((PlayerLocation[i].mapx + j) <= DUNGEONX) && ((PlayerLocation[i].mapy + k) >= 1) && ((PlayerLocation[i].mapy + k) <= DUNGEONY)) {
						if (((PlayerLocation[i].mapx + j) == PlayerLocation[CurrentPlayer].mapx) && ((PlayerLocation[i].mapy + k) == PlayerLocation[CurrentPlayer].mapy) && ((PlayerLocation[i].level) == PlayerLocation[CurrentPlayer].level)) {
							PlayerLocation[i].asleep = 0;
							CurrentPlayer = i;
							check_dungeon();
							CurrentPlayer = saveCurrent;

							if (m_directSoundOK) {
								if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 2)
									m_pDirSound->PlaySound(m_bufferScream);

								if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 8)
									m_pDirSound->PlaySound(m_bufferYell);

								if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 9)
									m_pDirSound->PlaySound(m_bufferDorb);

								if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 6)
									m_pDirSound->PlaySound(m_bufferSkeleton);

								if (PlayerLocation[i].character == 3 && PlayerLocation[i].image == 7)
									m_pDirSound->PlaySound(m_bufferMumble);
							}
						}
					}
				}
			}
		}
	}
	CurrentPlayer = saveCurrent;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

int attack_player() {

	int loop2;
	int i;
	int savecurrent;
	int savex;
	int savey;
	int savecopx;
	int savecopy;
	int savecoplvl;
	int savemaxmove;
	int saveit;
	int char2;
	char type[20];



	if (!PlayerLocation[CurrentPlayer].active)
		return 0;

	savecurrent = CurrentPlayer;
	savex = PlayerLocation[CurrentPlayer].mapx;
	savey = PlayerLocation[CurrentPlayer].mapy;
	savecopx = PlayerLocation[CurrentPlayer].copx;
	savecopy = PlayerLocation[CurrentPlayer].copy;
	savecoplvl = PlayerLocation[CurrentPlayer].coplvl;

	saveit = PlayerLocation[CurrentPlayer].level;
	savemaxmove = PlayerLocation[CurrentPlayer].maxmove;
	strcpy(type, PlayerLocation[CurrentPlayer].type);

	char2 = PlayerLocation[CurrentPlayer].character;

	for (i = 1; i <= NumPlayers; i++) {
		if (i != savecurrent) {
			if (PlayerLocation[i].mapx == savex && PlayerLocation[i].mapy == savey
				&& PlayerLocation[i].level == saveit
				&& PlayerLocation[i].dead == 0 && PlayerLocation[i].active
				) {
				if (char2 == 2) {
					if (PlayerLocation[i].character == 3) {
						//human on monster u lose
						apply_body_damage(savecurrent, i);
						break;
					}
					else {
						if (PlayerLocation[savecurrent].team == PlayerLocation[i].team) {
						}
						else {
							apply_body_damage(savecurrent, i);
							break;
						}
					}
				}
				else {
					if (PlayerLocation[i].character == 2) {


						if (PlayerLocation[i].protection > 0) {
							kill_monster(savecurrent, i);

						}
						else {
							apply_body_damage(i, savecurrent);

						}
						break;
					}
				}
			}
		}
	}

	CurrentPlayer = savecurrent;
	if (PlayerLocation[CurrentPlayer].mapx == treasurex &&
		PlayerLocation[CurrentPlayer].hit == 0 &&
		dropt == 1 &&
		PlayerLocation[CurrentPlayer].mapy == treasurey &&
		PlayerLocation[CurrentPlayer].level == treasurelvl &&
		PlayerLocation[CurrentPlayer].character == 2 && foundtreasure == 0
		&& CurrentPlayer != foundtreasure2 &&
		ishost) {

		if (PlayerLocation[CurrentPlayer].team == 1 && PlayerLocation[CurrentPlayer].protecttreasure == 0 && CurrentPlayer != 1 && networkgame == 0) {


		}
		else {


			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferTreasure);



			if (ishost) {
				foundtreasure = CurrentPlayer;
			}

			if (networkgame && ishost) {
				if (dtreasure[0].towner != CurrentPlayer) {

					send_treasure(0, 0);
					send_play_sound(1);
					strcpy(PlayerLocation[CurrentPlayer].lastmove, "-");
				}
				else {
					foundtreasure = 0;
				}
			}
			//			if (PlayerLocation[CurrentPlayer].ring > 0)
			//				PlayerLocation[CurrentPlayer].protection=80;

		}
	}

	if (PlayerLocation[CurrentPlayer].mapx == treasurex2 &&
		dropt == 1 &&
		PlayerLocation[CurrentPlayer].mapy == treasurey2 &&
		PlayerLocation[CurrentPlayer].level == treasurelvl2 &&
		PlayerLocation[CurrentPlayer].hit == 0 &&
		PlayerLocation[CurrentPlayer].character == 2 && foundtreasure2 == 0
		&& CurrentPlayer != foundtreasure && ishost) {
		if (PlayerLocation[CurrentPlayer].team == 2 && PlayerLocation[CurrentPlayer].protecttreasure == 0 && CurrentPlayer != 1 && networkgame == 0) {
		}
		else {

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferTreasure);

			if (ishost)
				foundtreasure2 = CurrentPlayer;
			if (networkgame && ishost) {
				if (dtreasure[1].towner != CurrentPlayer) {

					send_treasure2(0, 0);
					send_play_sound(1);
					strcpy(PlayerLocation[CurrentPlayer].lastmove, "-");
				}
				else {
					foundtreasure2 = 0;
				}
			}


			//			if (PlayerLocation[CurrentPlayer].ring > 0)
			//				PlayerLocation[CurrentPlayer].protection=80;

		}

	}


	if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 'e') {
		if (ishost || CurrentPlayer == 1) {

			PlayerLocation[CurrentPlayer].ability = PlayerLocation[CurrentPlayer].ability + 2;

		}
		dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item = 0;

		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferReward);

		if (ishost)
			items--;
	}

	if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 'a') {
		if (PlayerLocation[CurrentPlayer].character == 2
			&& PlayerLocation[CurrentPlayer].image == 1 ||
			PlayerLocation[CurrentPlayer].character == 2
			&& PlayerLocation[CurrentPlayer].image == 3) {
			if (ishost)
				items--;

			if (ishost || CurrentPlayer == 1) {
				PlayerLocation[CurrentPlayer].armour = PlayerLocation[CurrentPlayer].armour + 5;

			}

			dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item = 0;

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferReward);
		}
	}

	if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 's') {
		if (PlayerLocation[CurrentPlayer].character == 2
			&& PlayerLocation[CurrentPlayer].image == 4 ||
			PlayerLocation[CurrentPlayer].character == 2
			&& PlayerLocation[CurrentPlayer].image == 5) {

			if (ishost || CurrentPlayer == 1) {
				PlayerLocation[CurrentPlayer].armour = PlayerLocation[CurrentPlayer].armour + 5;

			}

			dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item = 0;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferReward);

		}
	}

	if (dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 'r' && PlayerLocation[CurrentPlayer].ring <= 4) {
		if (PlayerLocation[CurrentPlayer].character == 2) {

			if (ishost)
				items--;

			if (ishost || CurrentPlayer == 1) {
				PlayerLocation[CurrentPlayer].ring = PlayerLocation[CurrentPlayer].ring + 1;

			}
			dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item = 0;
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferReward);

		}
	}
	//got treasure
	if (CurrentPlayer == foundtreasure && PlayerLocation[CurrentPlayer].mapx == PlayerLocation[CurrentPlayer].copx && PlayerLocation[CurrentPlayer].mapy == PlayerLocation[CurrentPlayer].copy && PlayerLocation[CurrentPlayer].level == PlayerLocation[CurrentPlayer].coplvl) {

		if (PlayerLocation[CurrentPlayer].team == 2 && ishost) {
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferWin);

			if (ishost) {
				PlayerLocation[CurrentPlayer].skill = PlayerLocation[CurrentPlayer].skill + 8;
				if (networkgame && ishost) {
					send_reward(CurrentPlayer, 8);
					send_play_sound(3);
				}

			}
			return 1;
		}
		else if (CurrentPlayer != 1) {


			if (ishost && !networkgame) {
				treasurex = PlayerLocation[CurrentPlayer].mapx;
				treasurey = PlayerLocation[CurrentPlayer].mapy;
				treasurelvl = PlayerLocation[CurrentPlayer].level;
				foundtreasure = 0;
				if (networkgame && ishost)
					send_treasure(0, 0);

				for (loop2 = 2; loop2 <= NumPlayers; loop2++) {

					if (PlayerLocation[loop2].team == PlayerLocation[CurrentPlayer].team)
						PlayerLocation[loop2].protecttreasure = 0;
				}
			}

		}
	}


	if (CurrentPlayer == foundtreasure2 && PlayerLocation[CurrentPlayer].mapx == PlayerLocation[CurrentPlayer].copx && PlayerLocation[CurrentPlayer].mapy == PlayerLocation[CurrentPlayer].copy && PlayerLocation[CurrentPlayer].level == PlayerLocation[CurrentPlayer].coplvl) {

		if (PlayerLocation[CurrentPlayer].team == 1 && ishost) {


			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferWin);


			//			for (loop=1;loop<=NumPlayers;loop++) {
			//				if (PlayerLocation[loop].team == PlayerLocation[CurrentPlayer].team)
			//					PlayerLocation[CurrentPlayer].skill=PlayerLocation[CurrentPlayer].skill + 8;
			//			}
			if (ishost) {
				PlayerLocation[CurrentPlayer].skill = PlayerLocation[CurrentPlayer].skill + 8;
				if (networkgame && ishost) {
					send_reward(CurrentPlayer, 8);
					send_play_sound(3);
				}

			}


			return 1;
		}
		else if (CurrentPlayer != 1) {


			if (ishost && !networkgame) {
				treasurex2 = PlayerLocation[CurrentPlayer].mapx;
				treasurey2 = PlayerLocation[CurrentPlayer].mapy;
				treasurelvl2 = PlayerLocation[CurrentPlayer].level;
				foundtreasure2 = 0;
				if (networkgame && ishost)
					send_treasure2(0, 0);
				for (loop2 = 2; loop2 <= NumPlayers; loop2++) {

					if (PlayerLocation[loop2].team == PlayerLocation[CurrentPlayer].team)
						PlayerLocation[loop2].protecttreasure = 0;
				}

			}

		}
	}
	return 0;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

void destination() {

	int i;
	int saveit;
	//	int junk;
	//aaa
	//	int len;
	int level;
	char work[100], work2[100];
	int image;

	saveit = CurrentPlayer;
	if (PlayerLocation[CurrentPlayer].character == 2 ||
		PlayerLocation[CurrentPlayer].character == 3 &&
		CurrentPlayer % 2 == 0) {
		if (PlayerLocation[CurrentPlayer].image == 1) {
			//fighter
			i = findelixir(2);
			if (i == 0)
				i = findelixir(1);
			if (i == 0)
				i = findelixir(4);
		}

		else if (PlayerLocation[CurrentPlayer].image == 3) {
			//thief
			i = findelixir(2);
			if (i == 0)
				i = findelixir(1);
			if (i == 0)
				i = findelixir(4);

		}
		else if (PlayerLocation[CurrentPlayer].image == 4) {
			//mage
			i = findelixir(3);
			if (i == 0)
				i = findelixir(1);
			if (i == 0)
				i = findelixir(4);
		}
		else if (PlayerLocation[CurrentPlayer].image == 5) {
			//cleric
			i = findelixir(3);
			if (i == 0)
				i = findelixir(1);
			if (i == 0)
				i = findelixir(4);
		}
		else {

			i = random_num(2);

			if (i == 1)
				i = findelixir(1);
		}
	}
	else {
		i = 0;
	}


	if (PlayerLocation[CurrentPlayer].lasttarget != '-') {

	}
	else if (i == 1 && foundtreasure2 == 0 && foundtreasure == 0) {
		return;
	}

	image = PlayerLocation[CurrentPlayer].image;
	level = PlayerLocation[CurrentPlayer].level;

	if (PlayerLocation[CurrentPlayer].character == 3) {
		strcpy(work, "");



		for (i = 1; i <= newwarrior; i++) {
			if (i != CurrentPlayer) {


				//				if (PlayerLocation[i].level	== PlayerLocation[CurrentPlayer].level &&  PlayerLocation[i].active) {
				//					sprintf(work2,"%d",i);
				//					strcat (work,work2);
				//				}

			}
		}
		/*
			if (strlen(work) == 0) {
				savep=0;
				for (i=1;i<=newwarrior;i++) {
					if (i!=CurrentPlayer)
						if (PlayerLocation[i].active) {
							savep=i;
						}

				}
				if (savep==0)
					i=1;
				else
					i=savep;

			}
			else {
				i = random_num(strlen(work));
				work2[0] = work[i];
				work2[1] = '\0';
				i = atoi(work2);
				work2[0] =  PlayerLocation[CurrentPlayer].lasttarget;
				work2[1] = '\0';
				if (strstr(work,work2) != NULL) {
					i  = atoi(work2);
				}
				sprintf(work2,"%d",i);
				PlayerLocation[CurrentPlayer].lasttarget=work2[0];
			}
	*/


		if (PlayerLocation[CurrentPlayer].lasttarget != '-') {
			work2[0] = PlayerLocation[CurrentPlayer].lasttarget;
			work2[1] = '\0';

			i = atoi(work2);

		}
		else
			i = CurrentPlayer;



		if (foundtreasure != 0 || foundtreasure2 != 0) {
			i = 0;
			if (foundtreasure)
				i++;
			if (foundtreasure2)
				i++;
			if (i == 2) {
				i = random_num(2) + 1;
				if (i == 1)
					i = foundtreasure;
				else
					i = foundtreasure2;
			}
			else {
				if (foundtreasure)
					i = foundtreasure;
				if (foundtreasure2)
					i = foundtreasure2;
			}

		}


		saveit = PlayerLocation[i].level;
		if (saveit > level) {
			findstairs(0);
			if (dx == PlayerLocation[CurrentPlayer].mapx
				&& dy == PlayerLocation[CurrentPlayer].mapy ||
				dungeon[PlayerLocation[CurrentPlayer].mapx]
				[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].type == 's'
				&& dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
				[PlayerLocation[CurrentPlayer].level].item == 0) {
				movedown();
			}
		}
		else if (saveit < level) {
			findstairs(1);
			if (dx == PlayerLocation[CurrentPlayer].mapx &&
				dy == PlayerLocation[CurrentPlayer].mapy ||
				dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
				[PlayerLocation[CurrentPlayer].level].type == 's' &&
				dungeon[PlayerLocation[CurrentPlayer].mapx]
				[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 1) {
				movedown();
			}
		}
		else {
			dx = PlayerLocation[i].mapx;
			dy = PlayerLocation[i].mapy;
		}
	}

	// PLAYERS MOVE
	if (PlayerLocation[CurrentPlayer].team == 1 && treasurelvl != 1 && foundtreasure2 != CurrentPlayer)
		PlayerLocation[CurrentPlayer].protecttreasure = 1;

	if (PlayerLocation[CurrentPlayer].team == 2 && treasurelvl2 != maxlevel && foundtreasure != CurrentPlayer)
		PlayerLocation[CurrentPlayer].protecttreasure = 2;


	if (PlayerLocation[CurrentPlayer].team == 2 && PlayerLocation[CurrentPlayer].protecttreasure != 2 || PlayerLocation[CurrentPlayer].protecttreasure == 1) {
		if (treasurelvl > level) {
			findstairs(0);
			if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy
				|| dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
				[PlayerLocation[CurrentPlayer].level].type == 's' &&
				dungeon[PlayerLocation[CurrentPlayer].mapx]
				[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 0) {
				movedown();
			}
		}
		else if (treasurelvl < level) {
			findstairs(1);
			if (dx == PlayerLocation[CurrentPlayer].mapx &&
				dy == PlayerLocation[CurrentPlayer].mapy ||
				dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
				[PlayerLocation[CurrentPlayer].level].type == 's' &&
				dungeon[PlayerLocation[CurrentPlayer].mapx]
				[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 1) {
				movedown();

			}
		}
		else {
			dx = treasurex;
			dy = treasurey;
		}

		if (foundtreasure != 0 && CurrentPlayer != foundtreasure) {
			if (treasurelvl > level) {
				findstairs(0);
				if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx]
					[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 0) {
					movedown();
				}
			}
			else if (treasurelvl < level) {
				findstairs(1);
				if (dx == PlayerLocation[CurrentPlayer].mapx &&
					dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].item == 1) {
					movedown();
				}
			}
			else {
				dx = PlayerLocation[foundtreasure].mapx;
				dy = PlayerLocation[foundtreasure].mapy;
			}
		}

		if (foundtreasure != 0 && foundtreasure == CurrentPlayer) {
			if (PlayerLocation[foundtreasure].coplvl > level) {
				findstairs(0);
				if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].item == 0) {
					movedown();
				}
			}
			else if (PlayerLocation[foundtreasure].coplvl < level) {
				findstairs(1);
				if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].item == 1) {
					movedown();
				}
			}
			else {
				dx = PlayerLocation[foundtreasure].copx;
				dy = PlayerLocation[foundtreasure].copy;
			}
		}
	}

	if (PlayerLocation[CurrentPlayer].team == 1 && PlayerLocation[CurrentPlayer].protecttreasure != 1 || PlayerLocation[CurrentPlayer].protecttreasure == 2) {


		if (treasurelvl2 > level) {
			findstairs(0);
			if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy
				|| dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
				[PlayerLocation[CurrentPlayer].level].type == 's' &&
				dungeon[PlayerLocation[CurrentPlayer].mapx]
				[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 0) {
				movedown();
			}
		}
		else if (treasurelvl2 < level) {
			findstairs(1);
			if (dx == PlayerLocation[CurrentPlayer].mapx &&
				dy == PlayerLocation[CurrentPlayer].mapy ||
				dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
				[PlayerLocation[CurrentPlayer].level].type == 's' &&
				dungeon[PlayerLocation[CurrentPlayer].mapx]
				[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 1) {
				movedown();

			}
		}
		else {
			dx = treasurex2;
			dy = treasurey2;
		}

		if (foundtreasure2 != 0 && CurrentPlayer != foundtreasure2) {
			if (treasurelvl2 > level) {
				findstairs(0);
				if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx]
					[PlayerLocation[CurrentPlayer].mapy][PlayerLocation[CurrentPlayer].level].item == 0) {
					movedown();
				}
			}
			else if (treasurelvl2 < level) {
				findstairs(1);
				if (dx == PlayerLocation[CurrentPlayer].mapx &&
					dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].item == 1) {
					movedown();
				}
			}
			else {
				dx = PlayerLocation[foundtreasure2].mapx;
				dy = PlayerLocation[foundtreasure2].mapy;
			}
		}


		if (foundtreasure2 != 0 && foundtreasure2 == CurrentPlayer) {

			if (PlayerLocation[foundtreasure2].coplvl > level) {
				findstairs(0);
				if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].item == 0) {
					movedown();
				}
			}
			else if (PlayerLocation[foundtreasure2].coplvl < level) {
				findstairs(1);
				if (dx == PlayerLocation[CurrentPlayer].mapx && dy == PlayerLocation[CurrentPlayer].mapy ||
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].type == 's' &&
					dungeon[PlayerLocation[CurrentPlayer].mapx][PlayerLocation[CurrentPlayer].mapy]
					[PlayerLocation[CurrentPlayer].level].item == 1) {
					movedown();
				}
			}
			else {
				dx = PlayerLocation[foundtreasure2].copx;
				dy = PlayerLocation[foundtreasure2].copy;
			}
		}
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////


int findstairs(char way) {
	int x, y;
	int countstairs = 0;
	int len = 0;
	int level;

	level = PlayerLocation[CurrentPlayer].level;

	for (y = 0; y < DUNGEONY; y++) {
		for (x = 0; x < DUNGEONX; x++) {
			if (dungeon[x][y][level].type == 's') {
				countstairs++;
				if (dungeon[x][y][level].item == way) {
					dx = x;
					dy = y;
					if (CurrentPlayer == 1 || CurrentPlayer % 3 == 0)
						return 0;
					if (countstairs == 2) {
						if (CurrentPlayer % 2 == 0)
							return 0;
					}
				}
			}
		}
	}

	return 0;

}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

int findelixir(int item) {

	int x, y;
	int countstairs = 0;
	int len = 0;
	int level;

	char thing;



	switch (item) {
	case 1:
		thing = 'e';
		break;
	case 2:
		thing = 'a';
		break;
	case 3:
		thing = 's';
		break;
	case 4:
		thing = 'r';
		break;

	}

	level = PlayerLocation[CurrentPlayer].level;
	len = random_num(4);
	if (CurrentPlayer % 2 == 0) {

		for (y = 0; y < DUNGEONY; y++) {
			for (x = 0; x < DUNGEONX; x++) {
				if (dungeon[x][y][level].item == thing) {
					dx = x;
					dy = y;
					return 1;
				}
			}
		}

	}

	else {
		for (x = 0; x < DUNGEONX; x++) {
			for (y = 0; y < DUNGEONY; y++) {
				if (dungeon[x][y][level].item == thing) {
					dx = x;
					dy = y;
					return 1;
				}
			}
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

int findbeholder(int x, int y, int lvl) {

	int savecurrent;
	int i;
	int len;


	if (PlayerLocation[CurrentPlayer].character == 3)
		return 0;

	savecurrent = CurrentPlayer;
	for (i = 1; i <= NumPlayers; i++) {
		CurrentPlayer = i;
		if (savecurrent != i) {
			if (PlayerLocation[CurrentPlayer].character == 3 &&
				PlayerLocation[i].asleep == 0 &&
				x == PlayerLocation[i].mapx &&
				y == PlayerLocation[i].mapy &&
				lvl == PlayerLocation[i].level &&
				PlayerLocation[i].dead == 0) {
				CurrentPlayer = savecurrent;
				strcpy(PlayerLocation[CurrentPlayer].lastmove, "-");
				return 1;
			}
			if (foundtreasure == savecurrent &&
				PlayerLocation[i].character == 2 &&
				PlayerLocation[i].asleep == 0 &&
				x == PlayerLocation[i].mapx &&
				y == PlayerLocation[i].mapy &&
				lvl == PlayerLocation[i].level &&
				PlayerLocation[i].dead == 0) {
				CurrentPlayer = savecurrent;
				strcpy(PlayerLocation[i].lastmove, "-");
				len = random_num(2);
				if (len == 1)
					return 1;
				else
					return 0;
			}
			if (foundtreasure == savecurrent &&
				PlayerLocation[i].character == 1 &&
				PlayerLocation[i].asleep == 0 &&
				x == PlayerLocation[i].mapx &&
				y == PlayerLocation[i].mapy &&
				lvl == PlayerLocation[i].level &&
				PlayerLocation[i].dead == 0) {
				CurrentPlayer = savecurrent;
				strcpy(PlayerLocation[i].lastmove, "-");
				len = random_num(2);
				if (len == 1)
					return 1;
				else
					return 0;
			}
		}
	}
	CurrentPlayer = savecurrent;

	for (i = 1; i <= 20; i++) {
		if (missle[i].frame != 0) {

			if (missle[i].misslex == x && missle[i].missley == y && missle[i].misslelvl == lvl) {
				return 1;
			}
		}
	}


	return 0;
}
///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////


void check_dead() {

	int flag;

	if (MoveNum <= 0) {
		flag = 1;
		while (flag) {
			CurrentPlayer++;
			if (CurrentPlayer > NumPlayers)
				CurrentPlayer = 1;
			if (PlayerLocation[CurrentPlayer].asleep == 0 && PlayerLocation[CurrentPlayer].dead == 0) {
				flag = 0;
			}
		}
		if (CurrentPlayer > NumPlayers) {
			CurrentPlayer = 1;
		}
		MoveNum = PlayerLocation[CurrentPlayer].maxmove;
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////



void apply_damage() {

	int t, i;
	int savecurrent;
	savecurrent = CurrentPlayer;
	int result;
	int rand;
	int hurt;
	int randnum;
	//	for (t=1;t<=NumPlayers;t++) {

	for (t = NumPlayers; t >= 0; t--) {
		for (i = 1; i <= 20; i++) {
			if (missle[i].frame != 0) {
				if (missle[i].misslex == PlayerLocation[t].mapx &&
					missle[i].missley == PlayerLocation[t].mapy &&
					missle[i].misslelvl == PlayerLocation[t].level &&
					PlayerLocation[t].active &&
					PlayerLocation[t].dead == 0 && missle[i].owner != t
					) {


					PlayerLocation[missle[i].owner].numshots = PlayerLocation[missle[i].owner].numshots - 1;
					if (foundtreasure == t && PlayerLocation[t].protection == 0 && ishost) {
						treasurex = PlayerLocation[t].mapx;
						treasurey = PlayerLocation[t].mapy;
						treasurelvl = PlayerLocation[t].level;
						foundtreasure = 0;
						PlayerLocation[t].hit = 1;
						PlayerLocation[t].lasttarget = '-';
						strcpy(PlayerLocation[t].lastmove, "-");
						PlayerLocation[t].track = 0;



						if (networkgame && ishost) {

							dtreasure[0].towner = t;
							dtreasure[0].tdx = PlayerLocation[t].mapx;
							dtreasure[0].tdy = PlayerLocation[t].mapy;
							dtreasure[0].tdlvl = PlayerLocation[t].level;


							send_treasure(0, 0);

						}

					}

					if (foundtreasure2 == t && PlayerLocation[t].protection == 0 && ishost) {
						treasurex2 = PlayerLocation[t].mapx;
						treasurey2 = PlayerLocation[t].mapy;
						treasurelvl2 = PlayerLocation[t].level;
						foundtreasure2 = 0;
						PlayerLocation[t].hit = 1;
						PlayerLocation[t].lasttarget = '-';
						strcpy(PlayerLocation[t].lastmove, "-");
						PlayerLocation[t].track = 0;
						if (networkgame && ishost) {
							dtreasure[1].towner = t;
							dtreasure[1].tdx = PlayerLocation[t].mapx;
							dtreasure[1].tdy = PlayerLocation[t].mapy;
							dtreasure[1].tdlvl = PlayerLocation[t].level;



							send_treasure2(0, 0);

						}

					}

					missle[i].frame = 0;

					if (ishost || t == 1) {


						hurt = random_num((PlayerLocation[missle[i].owner].damagemax - PlayerLocation[missle[i].owner].damagemin) + 1) + 1;
						hurt = hurt + (PlayerLocation[missle[i].owner].damagemin - 1);

						if (PlayerLocation[t].protection == 0) {
							if (PlayerLocation[t].armour > 0) {
								if (PlayerLocation[t].armour - hurt < 0) {
									hurt = hurt - PlayerLocation[t].armour;
									PlayerLocation[t].armour = 0;
									PlayerLocation[t].ability = PlayerLocation[t].ability - hurt;

								}
								else {
									PlayerLocation[t].armour = PlayerLocation[t].armour - hurt;
								}

							}
							else {
								PlayerLocation[t].ability = PlayerLocation[t].ability - hurt;

							}


							if (t == 1) {
								if (m_directSoundOK)
									m_pDirSound->PlaySound(m_bufferOuch);
							}

						}

					}

					if (PlayerLocation[t].asleep == 1 && ishost)
						PlayerLocation[t].asleep = 0;
					if (missle[i].misslelvl == PlayerLocation[camera].level)
						if (!networkserver)
							result = pDirDraw->BlitImage(&CPoint(PlayerLocation[t].x, PlayerLocation[t].y),
								partsASurfaceNum, &CRect(0, 1152, 63, 1152 + 63));

					if (m_directSoundOK)
						m_pDirSound->PlaySound(m_bufferHit);
					if (PlayerLocation[t].ability <= 0) {


						if (t <= newwarrior) {
							if (ishost || t == 1) {


								if (ishost) {
									PlayerLocation[missle[i].owner].skill = PlayerLocation[missle[i].owner].skill + PlayerLocation[t].damagemax;

									if (networkgame && ishost) {
										send_reward(missle[i].owner, PlayerLocation[t].damagemax);
									}

								}

								PlayerLocation[t].mapx = PlayerLocation[t].copx;
								PlayerLocation[t].mapy = PlayerLocation[t].copy;
								PlayerLocation[t].level = PlayerLocation[t].coplvl;
								PlayerLocation[t].frame = 0;
								strcpy(PlayerLocation[t].lastmove, "-");
								PlayerLocation[t].stairs = 0;
								PlayerLocation[t].staircounter = 0;
								startgame = 2;
								PlayerLocation[t].justdied = 1;
								PlayerLocation[t].tolx = 3;
								PlayerLocation[t].toly = 4;
								PlayerLocation[t].track = 0;
								PlayerLocation[t].moved = 0;
								PlayerLocation[t].dead = 64;
								PlayerLocation[t].speed = 2;
								PlayerLocation[t].protection = 0;
								PlayerLocation[t].hit = 0;
								PlayerLocation[t].asleep = 0;
								//									PlayerLocation[t].ability = 0;
								PlayerLocation[t].skill = 0;
								PlayerLocation[t].ring = 0;
								PlayerLocation[t].armour = 0;
								randnum = random_num(20) + 1;
								PlayerLocation[t].ability = 30 + randnum + countlevels;
								PlayerLocation[t].damagemin = 2;
								PlayerLocation[t].damagemax = 2;
								PlayerLocation[t].maxmissle = 4;
								PlayerLocation[t].firerate = 5;



								if (m_directSoundOK)
									m_pDirSound->PlaySound(m_bufferDead);


								if (ishost && networkgame)
									send_player(t, 0, 0);


								if (t == 1) {
									advancelevel = 20;
									advanceattrib = 0;
									hitkeyboard = 0;
									mousehit = 0;
									rand = play_again();

									if (rand == 1)
										return;

								}






							}
						}
						else {
							kill_monster(t, missle[i].owner);


						}
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

void calc_player_move() {

	if (PlayerLocation[1].direction == 1)
		PlayerLocation[1].tolx = PlayerLocation[1].tolx - 1;
	if (PlayerLocation[1].direction == 2)
		PlayerLocation[1].toly = PlayerLocation[1].toly - 1;
	if (PlayerLocation[1].direction == 3)
		PlayerLocation[1].toly = PlayerLocation[1].toly + 1;
	if (PlayerLocation[1].direction == 4)
		PlayerLocation[1].tolx = PlayerLocation[1].tolx + 1;

	if (PlayerLocation[1].tolx >= 7) {
		PlayerLocation[1].mapx = PlayerLocation[1].mapx + 1;
		PlayerLocation[1].tolx = 1;
		dropt = 1;
		wakeup_sleepy_heads();
	}
	if (PlayerLocation[1].tolx <= 0) {
		PlayerLocation[1].mapx = PlayerLocation[1].mapx - 1;
		PlayerLocation[1].tolx = 6;
		dropt = 1;
		wakeup_sleepy_heads();
	}

	if (PlayerLocation[1].toly >= 7) {
		PlayerLocation[1].mapy = PlayerLocation[1].mapy + 1;
		PlayerLocation[1].toly = 1;
		dropt = 1;
		wakeup_sleepy_heads();
	}
	if (PlayerLocation[1].toly <= 0) {
		PlayerLocation[1].mapy = PlayerLocation[1].mapy - 1;
		PlayerLocation[1].toly = 6;
		dropt = 1;
		wakeup_sleepy_heads();
	}
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////


void calc_animation() {


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
	}

	fx = framex;
	fy = framey;
	fx2 = frame2x;
	fy2 = frame2y;

	PlayerLocation[CurrentPlayer].fx = framex;
	PlayerLocation[CurrentPlayer].fy = framey;
	PlayerLocation[CurrentPlayer].fx2 = frame2x;
	PlayerLocation[CurrentPlayer].fy2 = frame2y;


}



///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////


bool OutsideMaze(int x, int y)
{


	return (x <= 0 || y <= 0 ||
		x >= DUNGEONX || y >= DUNGEONY);
}/* OutsideMaze */

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

bool AtFinish(int x, int y)
{

	if (x == dx && y == dy)
		return 1;
	else
		return 0;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

bool WallExists(int x, int y, int level, int dir)
{

	switch (dir)
	{
	case 2: {
		y--;
		break;
	}
	case 3: {
		x++;
		break;
	}
	case 4: {
		y++;
		break;
	}
	case 1: {
		x--;
		break;
	}
	}/* switch */

   /* In this version, we never wander outside the maze, since we
	* are looking for "F". Hence being outside the maze is like
	* finding a wall.
	*/

	if (dungeon[x][y][level].type == 0 || dungeon[x][y][level].type == 'u')
		return 1;


	if (OutsideMaze(x, y) == 1)
		return 1;




	return 0;

}/* WallExists */

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

void MarkSquare(int x, int y, int level)
{
	shortestroute++;
	dungeon[x][y][level].mark = 'X';
	dungeon[x][y][level].order = ++orderglobal;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

void UnmarkSquare(int x, int y, int level)
{
	shortestroute--;
	dungeon[x][y][level].mark = 0;
	dungeon[x][y][level].order = 0;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

bool IsMarked(int x, int y, int level)
{
	if (dungeon[x][y][level].mark == 'X')
		return 1;
	else
		return 0;
}

static bool SolveMaze(int x, int y, int level, int dir)
{

	int x1, y1;
	int count;
	//	int len;
	int direction;

	count = 1;

	if (x == dx && y == dy) {
		if (startsolve < saveshort) {
			saveshort = startsolve;
			//			printf("\nallsquare %d saveshort %d startsolve %d movex %d movey %d\n",allsquare,saveshort,startsolve,moveitx,moveity);
			moveitxfin = moveitx;
			moveityfin = moveity;


		}


		if (PlayerLocation[CurrentPlayer].stairsx > 0) {
			return (TRUE);
		}
	}

	if (dungeon[x][y][level].mark == 'X') {
		return (FALSE);
	}

	//				printf("\nSolveMaze %d %d",x,y);

	//	shortestroute++;
	dungeon[x][y][level].mark = 'X';
	//	dungeon[x][y][level].order = ++orderglobal;

	//	MarkSquare(x,y,level);
	startsolve++;
	for (direction = 1; direction < 5; direction++) {
		if (!WallExists(x, y, level, direction)) {
			x1 = x;
			y1 = y;
			switch (direction)
			{
			case 2:
				y1--;
				break;
			case 3:
				x1++;
				break;
			case 4:
				y1++;
				break;
			case 1: x1--;
				break;
			}

			if (startsolve == 1) {
				moveitx = x1;
				moveity = y1;
			}

			if (SolveMaze(x1, y1, level, 1)) {
				startsolve--;
				if (startsolve == 0) {
					moveitx = x1;
					moveity = y1;
				}
				return (TRUE);
			}


		}
	}
	//	UnmarkSquare(x,y,level);

	//	shortestroute--;
	dungeon[x][y][level].mark = 0;
	//  dungeon[x][y][level].order = 0;

	startsolve--;
	return (FALSE);
}/* SolveMaze */


///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

int computer_m(int x, int y, int level) {

	int len;
	//	int size;
	int dir;
	int px, py;
	char playermv[100];
	char response[100];
	//	int i;
	//	int saveroute;
	//	bool solve;
	orderglobal = 0;
	start_recursion = 1;
	int loopx, loopy;


	//	FILE *fp;

//	i = random_num(5);
//	if (PlayerLocation[CurrentPlayer].character ==3 && i!=1) {
//		computermove();
//		return 0;
//	}
	if (PlayerLocation[CurrentPlayer].mapx == dx &&
		PlayerLocation[CurrentPlayer].mapy == dy
		||
		dungeon[dx][dy][level].type == 'u' ||
		dungeon[dx][dy][level].type == 0) {

		px = x;
		py = y;
		PlayerLocation[CurrentPlayer].track = 0;
		strcpy(playermv, "");

		if (dungeon[px - 1][py][level].type == 'f' || dungeon[px - 1][py][level].type == 's') {
			strcat(playermv, "1");
		}
		if (dungeon[px + 1][py][level].type == 'f' || dungeon[px + 1][py][level].type == 's') {
			strcat(playermv, "4");
		}
		if (dungeon[px][py - 1][level].type == 'f' || dungeon[px][py - 1][level].type == 's') {
			strcat(playermv, "2");
		}
		if (dungeon[px][py + 1][level].type == 'f' || dungeon[px][py + 1][level].type == 's') {
			strcat(playermv, "3");
		}
		len = strlen(playermv);

		len = random_num(len);
		response[0] = playermv[len];
		response[1] = '\0';
		dir = atoi(response);
		PlayerLocation[CurrentPlayer].direction = dir;

		//		if (dungeon[px][py][level].type != 's') {
		//			PlayerLocation[CurrentPlayer].rest = 9;
		//		}


		return 0;
	}

	for (loopy = 0; loopy <= DUNGEONY; loopy++) {
		for (loopx = 0; loopx <= DUNGEONX; loopx++) {
			dungeon[loopx][loopy][level].mark = 0;
			dungeon[loopx][loopy][level].order = 0;
		}
	}

	startsolve = 0;
	shortestroute = 0;
	saveshort = 99999;

	SolveMaze3(x, y, level, 1);

	if (moveitxfin < x)
		dir = 1;

	if (moveitxfin > x)
		dir = 4;

	if (moveityfin < y)
		dir = 2;
	if (moveityfin > y)
		dir = 3;

	PlayerLocation[CurrentPlayer].direction = dir;
	//	out_map(PlayerLocation[CurrentPlayer].mapx,PlayerLocation[CurrentPlayer].mapy,dir,saveshort,saveroute,size);		

	return dir;

}



///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////

static bool SolveMaze3(int x, int y, int level, int dir)
{


	int x1, y1;
	int counter = 0;
	int i;
	int flag;
	int flag2;
	int short2 = 0;




	for (i = 0; i < DUNGEONX * DUNGEONY; i++) {
		stack[i].dir = 1;
		stack[i].x = 0;
		stack[i].y = 0;
	}

	stack[0].x = x;
	stack[0].y = y;
	flag = TRUE;
	flag2 = TRUE;
	saveshort = 99999;

	while (flag2) {
		if (AtFinish(x, y)) {
			if (short2 < saveshort) {
				saveshort = short2;
				moveitxfin = moveitx;
				moveityfin = moveity;
				if (saveshort <= 2)
					break;

			}

		}
		dir = stack[counter].dir;
		x = stack[counter].x;
		y = stack[counter].y;
		flag = TRUE;

		while (flag) {
			if (dir > 4) {
				UnmarkSquare(stack[counter].x, stack[counter].y, level);
				stack[counter].dir = 1;
				short2--;
				counter--;
				dir = stack[counter].dir;
				x = stack[counter].x;
				y = stack[counter].y;
				if (counter < 0) {
					flag2 = FALSE;
					break;
				}
			}

			x1 = 0;
			y1 = 0;
			if (!WallExists(x, y, level, dir)) {
				x1 = x;
				y1 = y;
				switch (dir)
				{
				case 2:
					y1--;
					break;
				case 3:
					x1++;
					break;
				case 4:
					y1++;
					break;
				case 1: x1--;
					break;
				}

			}

			if (!IsMarked(x1, y1, level) && x1 != 0) {
				stack[counter].dir = dir;
				stack[counter].dir++;
				stack[counter].x = x;
				stack[counter].y = y;
				MarkSquare(x, y, level);
				flag = FALSE;
			}
			else
				dir++;
		}

		if (flag2) {
			x1 = x;
			y1 = y;
			switch (dir)
			{
			case 2:
				y1--;
				break;
			case 3:
				x1++;
				break;
			case 4:
				y1++;
				break;
			case 1: x1--;
				break;
			}
			MarkSquare(x1, y1, level);
			short2++;

			if (short2 == 1) {
				moveitx = x1;
				moveity = y1;
			}
			x = x1;
			y = y1;
			counter++;
			stack[counter].x = x;
			stack[counter].y = y;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////



int count_nodes(int lvlnum) {


	int x, y;
	int count;
	int node;
	node = 0;


	for (y = 0; y <= DUNGEONY; y++) {
		for (x = 0; x <= DUNGEONX; x++) {


			if (dungeon[x][y][lvlnum].type == 'f' || dungeon[x][y][lvlnum].type == 's') {
				count = 0;

				if (dungeon[x - 1][y][lvlnum].type == 'f' || dungeon[x - 1][y][lvlnum].type == 's')
					count++;
				if (dungeon[x + 1][y][lvlnum].type == 'f' || dungeon[x + 1][y][lvlnum].type == 's')
					count++;
				if (dungeon[x][y - 1][lvlnum].type == 'f' || dungeon[x][y - 1][lvlnum].type == 's')
					count++;
				if (dungeon[x][y + 1][lvlnum].type == 'f' || dungeon[x][y + 1][lvlnum].type == 's')
					count++;

				if (count >= 3) {
					node++;

				}

			}

		}
	}


	return node;

}


void apply_body_damage(int loser, int winner) {

	int hurt;
	int 			randnum;
	int r;
	if (PlayerLocation[loser].protection == 0 && PlayerLocation[loser].hit == 0) {


		if (ishost || loser == 1) {


			if (winner == 99) {
				hurt = 4;
			}
			else {
				hurt = random_num((PlayerLocation[winner].damagemax - PlayerLocation[winner].damagemin) + 1) + 1;
				hurt = hurt + (PlayerLocation[winner].damagemin - 1);
			}


			if (PlayerLocation[loser].armour > 0) {
				if (PlayerLocation[loser].armour - hurt < 0) {
					hurt = hurt - PlayerLocation[loser].armour;
					PlayerLocation[loser].armour = 0;
					PlayerLocation[loser].ability = PlayerLocation[loser].ability - hurt;
				}
				else {
					PlayerLocation[loser].armour = PlayerLocation[loser].armour - hurt;
				}
			}
			else {
				PlayerLocation[loser].ability = PlayerLocation[loser].ability - hurt;
			}

			PlayerLocation[loser].hit = 1;

		}

		if (loser == 1) {

			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferOuch);


		}

		//		if (!networkserver)
		//			result = pDirDraw->BlitImage(&CPoint(PlayerLocation[loser].x, PlayerLocation[loser].y),
		//			partsASurfaceNum, 	&CRect(0, 1152, 63, 1152+63));


		if (foundtreasure == loser && PlayerLocation[loser].protection == 0) {

			if (ishost) {
				treasurex = PlayerLocation[loser].mapx;
				treasurey = PlayerLocation[loser].mapy;
				treasurelvl = PlayerLocation[loser].level;
				foundtreasure = 0;
				PlayerLocation[loser].hit = 1;
				PlayerLocation[loser].lasttarget = '-';
				strcpy(PlayerLocation[loser].lastmove, "-");
				PlayerLocation[loser].track = 0;
				if (networkgame && ishost)
					send_treasure(0, 0);


			}
		}

		if (foundtreasure2 == loser && PlayerLocation[loser].protection == 0) {


			if (ishost) {
				treasurex2 = PlayerLocation[loser].mapx;
				treasurey2 = PlayerLocation[loser].mapy;
				treasurelvl2 = PlayerLocation[loser].level;
				foundtreasure2 = 0;
				PlayerLocation[loser].hit = 1;
				PlayerLocation[loser].lasttarget = '-';
				strcpy(PlayerLocation[loser].lastmove, "-");
				PlayerLocation[loser].track = 0;
				if (networkgame && ishost)
					send_treasure2(0, 0);

			}
		}

		if (PlayerLocation[loser].ability <= 0) {
			PlayerLocation[loser].mapx = PlayerLocation[loser].copx;
			PlayerLocation[loser].mapy = PlayerLocation[loser].copy;
			PlayerLocation[loser].level = PlayerLocation[loser].coplvl;
			PlayerLocation[loser].frame = 0;
			strcpy(PlayerLocation[loser].lastmove, "-");
			PlayerLocation[loser].stairs = 0;
			PlayerLocation[loser].staircounter = 0;
			startgame = 2;
			PlayerLocation[loser].justdied = 1;
			PlayerLocation[loser].tolx = 3;
			PlayerLocation[loser].toly = 4;
			PlayerLocation[loser].track = 0;
			PlayerLocation[loser].hit = 0;
			PlayerLocation[loser].moved = 0;
			PlayerLocation[loser].dead = 64;
			PlayerLocation[loser].speed = 2;
			PlayerLocation[loser].protection = 0;
			//			PlayerLocation[loser].ability = 0;
			PlayerLocation[loser].skill = 0;
			PlayerLocation[loser].ring = 0;
			PlayerLocation[loser].armour = 0;
			randnum = random_num(20) + 1;
			PlayerLocation[loser].ability = 20 + randnum + countlevels;
			PlayerLocation[loser].damagemin = 1;
			PlayerLocation[loser].damagemax = 1;
			PlayerLocation[loser].maxmissle = 4;
			PlayerLocation[loser].firerate = 5;


			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferDead);



			hitkeyboard = 0;
			mousehit = 0;

			if (ishost && networkgame)
				send_player(loser, 0, 0);

			if (loser == 1) {
				advancelevel = 20;
				advanceattrib = 0;
				r = play_again();
				if (r == 1)
					return;

			}

		}
	}
}



void went_up_level() {

	if (PlayerLocation[1].skill >= advancelevel) {
		advancelevel = advancelevel + (50) + (levelcounter * 2);

		PlayerLocation[1].ability = PlayerLocation[1].ability + 20;
		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferWin);


		switch (advanceattrib) {
		case 0:
			PlayerLocation[1].maxmissle++;
			strcpy(chat[0].say, "You went up a level in fire power.");
			chat[0].duration = 120;
			if (PlayerLocation[1].maxmissle > 4)
				PlayerLocation[1].maxmissle = 4;
			break;
		case 1:
			PlayerLocation[1].damagemax++;
			strcpy(chat[0].say, "You went up a level in maximum damage.");
			chat[0].duration = 120;
			if (PlayerLocation[1].damagemax > 5)
				PlayerLocation[1].damagemax = 5;

			break;
		case 2:
			PlayerLocation[1].firerate--;
			if (PlayerLocation[1].firerate <= 2)
				PlayerLocation[1].firerate = 2;
			strcpy(chat[0].say, "You went up a level in fire rate.");
			chat[0].duration = 120;

			break;
		case 3:
			strcpy(chat[0].say, "You went up a level in minmum damage.");
			chat[0].duration = 120;

			PlayerLocation[1].damagemin++;
			if (PlayerLocation[1].damagemin > 3)
				PlayerLocation[1].damagemin = 3;

			break;
		}
		advanceattrib++;
		if (advanceattrib >= 4)
			advanceattrib = 0;

		if (ishost && networkgame) {
			send_player(1, 1, 0);
		}

	}
}


int play_again() {



	if (ishost && !networkgame) {

		if (m_directSoundOK)
			stop_song();
		KillTimer(hWindow, 3);

		if (!networkserver) {
			display_dungeon(1);
			pDirDraw->FlipToGDISurface();
		}
		UINT resultclick = MessageBox(hWindow, "Play Again?", "Murk Play Again", MB_ICONQUESTION | MB_YESNO);
		if (resultclick == IDYES) {

			initplayer = 0;
			startgame = 1;
			movelevel = 0;
			istimerset = 0;
			countlevels = 0;
			levelcounter = 1;
			mousehit = 0;
			hitanykey = 0;
			new_level(0);

			if (m_directSoundOK)
				play_random_music();

			SetTimer(hWindow, 3, timerlength, TimerProc);
			return 1;
		}
		else
		{
			showmission = 0;

			mousehit = 0;
			hitanykey = 0;
			showmission = 0;
			mousehitx = 0;
			mousehity = 0;
			bdown = 0;
			if (m_directSoundOK)
				play_random_music();
			SetTimer(hWindow, 1, 50, TimerProc);
			return 1;

		}
	}

	return 0;

}



void kill_monster(int t, int winner) {
	int rand;
	int randnum;



	if (ishost) {
		if (items < 50 + maxlevel
			&& dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].item == 0
			&& dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].type == 'f') {
			rand = random_num(4) + 1;
			switch (rand) {
			case 1:
				dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].item = 'a';
				items++;
				break;
			case 2:
				dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].item = 's';
				items++;
				break;
			case 3:
				dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].item = 'e';
				items++;
				break;
			case 4:
				//											dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].item = 'r';
				//											items++;
				break;
			default:
				break;
			}



			if (dungeon[PlayerLocation[t].mapx][PlayerLocation[t].mapy][PlayerLocation[t].level].item
				&& ishost && networkgame) {
				send_dungeon_square(PlayerLocation[t].mapx, PlayerLocation[t].mapy, PlayerLocation[t].level, 0);

			}
		}



		dscene[t].x = PlayerLocation[t].x;
		dscene[t].y = PlayerLocation[t].y;
		dscene[t].lvl = PlayerLocation[t].level;
		dscene[t].direction = PlayerLocation[t].direction;
		dscene[t].counter = 64;
		dscene[t].mapx = PlayerLocation[t].mapx;
		dscene[t].mapy = PlayerLocation[t].mapy;

		PlayerLocation[t].mapx = PlayerLocation[t].copx;
		PlayerLocation[t].mapy = PlayerLocation[t].copy;
		PlayerLocation[t].level = PlayerLocation[t].coplvl;
		PlayerLocation[t].frame = 0;
		strcpy(PlayerLocation[t].lastmove, "-");
		PlayerLocation[t].ability = 5;


		switch (PlayerLocation[t].image) {


		case 1:
			randnum = random_num(20) + 1;
			PlayerLocation[t].ability = 20 + randnum + countlevels;
			break;
		case 3:
			randnum = random_num(20) + 1;
			PlayerLocation[t].ability = 20 + randnum + countlevels;
			break;
		case 4:
			randnum = random_num(20) + 1;
			PlayerLocation[t].ability = 20 + randnum + countlevels;
			break;

		case 5:
			randnum = random_num(20) + 1;
			PlayerLocation[t].ability = 20 + randnum + countlevels;
			break;

		case 2:
			//behold
			randnum = random_num(6) + 1;
			PlayerLocation[t].ability = 6 + randnum + countlevels;
			PlayerLocation[t].ability = beholderability;
			break;
		case 6:
			//skel
			randnum = random_num(4) + 1;
			PlayerLocation[t].ability = 4 + randnum + countlevels;
			PlayerLocation[t].ability = skeletonability;
			break;
		case 7:
			//mount
			randnum = random_num(3) + 1;
			PlayerLocation[t].ability = 3 + randnum + countlevels;
			PlayerLocation[t].ability = moutherability;
			break;
		case 8:
			//orc
			randnum = random_num(5) + 1;
			PlayerLocation[t].ability = 5 + randnum + countlevels;
			PlayerLocation[t].ability = orcability;
			break;
		case 9:
			//lord help us death orb
			randnum = random_num(10) + 1;
			PlayerLocation[t].ability = 10 + randnum + countlevels;
			PlayerLocation[t].ability = deathorbability;
			break;

		}
		PlayerLocation[t].stairs = 0;

		PlayerLocation[t].lasttarget = '-';
		PlayerLocation[t].staircounter = 0;
		startgame = 2;
		PlayerLocation[t].justdied = 1;
		PlayerLocation[t].tolx = 3;
		PlayerLocation[t].toly = 4;
		PlayerLocation[t].track = 0;
		PlayerLocation[t].moved = 0;
		PlayerLocation[t].speed = 2;
		rand = random_num(400);
		PlayerLocation[t].protection = 0;
		PlayerLocation[t].dead = 800 + rand + (NumPlayers * 5);
		PlayerLocation[t].hit = 0;

		if (ishost && networkgame) {
			send_player(t, 0, 0);


			send_death(t, dscene[t].x, dscene[t].y, dscene[t].lvl, 0,
				dscene[t].direction, dscene[t].counter,
				dscene[t].mapx, dscene[t].mapy,
				PlayerLocation[t].dead);






			send_play_sound(2);
		}

		if (m_directSoundOK)
			m_pDirSound->PlaySound(m_bufferMonsterdie);


	}


	if (ishost) {
		PlayerLocation[winner].skill = PlayerLocation[winner].skill + PlayerLocation[t].damagemax;

		if (networkgame && ishost) {
			send_reward(winner, PlayerLocation[t].damagemax);
		}

	}



}