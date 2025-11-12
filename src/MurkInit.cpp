//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"
#include "mapiwin.h"
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

int beholderability = 30;
int moutherability = 6;
int orcability = 15;
int skeletonability = 10;
int deathorbability = 50;

void init_ping_player();
extern void send_pit(int slot, int forsure, int x, int y, int lvl, int frame);
void set_tile();
extern void init_death_scene();

extern int mousestate;
void assign_player_cop() {

	CONST int XADJUST = 49;
	CONST int YADJUST = 25;

	int i;

	BOOL check;

	int randx, randy;
	int lvlnum;
	int randnum;

	int j;
	int countm;

	countm = 1;
	NumPlayers = 0;

	if (networkgame) {
		countlevels = 0;
	}

	newwarrior = gamedef.fighter + gamedef.thief + gamedef.cleric + gamedef.wizard + 1;

	for (j = 0; j <= 9; j++) {

		if (j == 0) {
			countm = 1;
		} else
			countm = countm + NumPlayers;
		if (j == 0)
			NumPlayers = 1;
		if (j == 1) {
			NumPlayers = gamedef.fighter;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 2) {
			NumPlayers = gamedef.cleric;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 3) {
			NumPlayers = gamedef.thief;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 4) {
			NumPlayers = gamedef.wizard;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 5) {
			NumPlayers = gamedef.beholders;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 6) {
			NumPlayers = gamedef.mouther;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 7) {
			NumPlayers = gamedef.ogre;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 8) {
			NumPlayers = gamedef.skeletons;
			if (NumPlayers == 0)
				j++;
		}
		if (j == 9) {

			NumPlayers = gamedef.deathorb;
			if (NumPlayers == 0)
				j++;
		}

		for (i = countm; i <= countm + NumPlayers - 1; i++) {
			if (j == 0) {

				if (networkgame && ishost) {
					if (gamedef.gametype == 1)
						PlayerLocation[i].team = 1;
					if (gamedef.gametype == 2)
						PlayerLocation[i].team = 1;
				} else {

					PlayerLocation[i].team = 1;
				}
				if (startthegame == 1)
					PlayerLocation[i].image = 1;
				PlayerLocation[i].rest = 0;
				PlayerLocation[i].limitspeed = FALSE;
				PlayerLocation[i].shoot = 1;
				PlayerLocation[i].maxspeed = 1;

				if (networkgame && ishost && networkserver) {

				} else {
					PlayerLocation[i].active = TRUE;
				}

				//				PlayerLocation[i].maxspeed = gamedef.fspeed;
				if (setability) {

					randnum = random_num(10) + 1;
					PlayerLocation[i].ability = 30 + randnum;
					PlayerLocation[i].damagemin = 2;
					PlayerLocation[i].damagemax = 2;
					PlayerLocation[i].maxmissle = 4;
					PlayerLocation[i].firerate = 5;
					//					if (j==0)
					//						PlayerLocation[i].ability = 999;
				}
			}
			if (j == 1) {
				PlayerLocation[i].image = 1;
				PlayerLocation[i].rest = 0;
				PlayerLocation[i].limitspeed = TRUE;
				PlayerLocation[i].firerate = 5;
				PlayerLocation[i].shoot = 1;
				if (setability == 1 && networkgame)
					PlayerLocation[i].active = FALSE;

				if (networkgame == 0 && ishost)
					PlayerLocation[i].active = TRUE;
				PlayerLocation[i].damagemin = 1;
				PlayerLocation[i].damagemax = 3;
				PlayerLocation[i].maxmissle = 4;

				if (ishost && networkgame) {
					if (gamedef.gametype == 1)
						PlayerLocation[i].team = 1;
					if (gamedef.gametype == 2)
						PlayerLocation[i].team = 1;
				}

				if (networkgame && ishost) {
					if (gamedef.gametype == 1)
						PlayerLocation[i].team = 1;
					if (gamedef.gametype == 2)
						PlayerLocation[i].team = 1;
				} else {
					PlayerLocation[i].team = gamedef.fteam;
				}

				PlayerLocation[i].maxspeed = gamedef.fspeed;
				randnum = random_num(20) + 1;
				PlayerLocation[i].ability = 20 + randnum + countlevels;
			}

			if (j == 2) {
				if (setability == 1 && networkgame)
					PlayerLocation[i].active = FALSE;
				if (networkgame == 0 && ishost)
					PlayerLocation[i].active = TRUE;
				PlayerLocation[i].shoot = 1;
				PlayerLocation[i].image = 5;
				PlayerLocation[i].rest = 0;
				PlayerLocation[i].limitspeed = TRUE;
				PlayerLocation[i].firerate = 5;
				PlayerLocation[i].damagemin = 1;
				PlayerLocation[i].damagemax = 3;
				PlayerLocation[i].maxmissle = 4;
				PlayerLocation[i].maxspeed = gamedef.cspeed;
				randnum = random_num(20) + 1;
				PlayerLocation[i].ability = 20 + randnum + countlevels;

				if (networkgame && ishost) {
					if (gamedef.gametype == 1)
						PlayerLocation[i].team = 1;
					if (gamedef.gametype == 2)
						PlayerLocation[i].team = 2;
				} else {

					PlayerLocation[i].team = gamedef.cteam;
				}
			}
			if (j == 3) {
				if (setability == 1 && networkgame)
					PlayerLocation[i].active = FALSE;
				if (networkgame == 0 && ishost)
					PlayerLocation[i].active = TRUE;

				PlayerLocation[i].image = 3;
				PlayerLocation[i].rest = 0;
				PlayerLocation[i].limitspeed = TRUE;
				PlayerLocation[i].firerate = 5;
				PlayerLocation[i].shoot = 1;
				PlayerLocation[i].damagemin = 1;
				PlayerLocation[i].damagemax = 3;
				PlayerLocation[i].maxmissle = 4;
				PlayerLocation[i].team = gamedef.tteam;

				if (networkgame && ishost) {
					if (gamedef.gametype == 1)
						PlayerLocation[i].team = 1;
					if (gamedef.gametype == 2)
						PlayerLocation[i].team = 1;
				} else {

					PlayerLocation[i].maxspeed = gamedef.tspeed;
				}
				randnum = random_num(20) + 1;
				PlayerLocation[i].ability = 20 + randnum + countlevels;
			}

			if (j == 4) {

				if (setability == 1 && networkgame)
					PlayerLocation[i].active = FALSE;

				if (networkgame == 0 && ishost)
					PlayerLocation[i].active = TRUE;
				PlayerLocation[i].image = 4;
				PlayerLocation[i].rest = 0;
				PlayerLocation[i].limitspeed = TRUE;
				PlayerLocation[i].firerate = 5;
				PlayerLocation[i].shoot = 1;
				PlayerLocation[i].damagemin = 1;
				PlayerLocation[i].damagemax = 2;
				PlayerLocation[i].maxmissle = 4;
				PlayerLocation[i].maxspeed = gamedef.wspeed;
				randnum = random_num(20) + 1;
				PlayerLocation[i].ability = 20 + randnum + countlevels;

				if (networkgame && ishost) {
					if (gamedef.gametype == 1)
						PlayerLocation[i].team = 1;
					if (gamedef.gametype == 2)
						PlayerLocation[i].team = 2;
				} else {

					PlayerLocation[i].team = gamedef.wteam;
				}
			}
			check = 0;
			while (check == 0) {
				randnum = random_num(maxlevel) + 1;
				lvlnum = randnum;
				if (j >= 0 && j <= 4) {
					if (PlayerLocation[i].team == 1) {
						lvlnum = 1;
					} else {
						lvlnum = maxlevel;
					}
					/*
					if (networkgame) {
					switch (PlayerLocation[i].image) {
					case 1:
					case 5:
					lvlnum =1;
					PlayerLocation[i].team=1;
					break;
					case 3:
					case 4:
					lvlnum =1;
					PlayerLocation[i].team=1;
					break;
					}

					}
					*/
				} else {

					if (gamedef.gametype == 2) {
						randnum = random_num(maxlevel - 2) + 1;
						randnum++;
						lvlnum = randnum;
					} else {

						randnum = random_num(maxlevel - 1) + 1;
						randnum++;
						lvlnum = randnum;
					}
				}

				randx = random_num(DUNGEONX) + 1;
				randy = random_num(DUNGEONY) + 1;
				if (dungeon[randx][randy][lvlnum].type == 'f' && dungeon[randx][randy][lvlnum].item == 0) {
					check = 1;
				} else {
					check = 0;
				}
			}
			PlayerLocation[i].intelligence = 1;
			PlayerLocation[i].x = playerx;
			PlayerLocation[i].y = playery;
			PlayerLocation[i].framehuman = 0;
			PlayerLocation[i].protection = 0;
			PlayerLocation[i].lasttarget = '-';
			PlayerLocation[i].track = 0;
			PlayerLocation[i].justdied = 0;
			PlayerLocation[i].direction = 3;
			PlayerLocation[i].level = lvlnum;
			PlayerLocation[i].stairsx = 0;
			PlayerLocation[i].stairsy = 0;
			PlayerLocation[i].mapx = randx;
			PlayerLocation[i].mapy = randy;
			PlayerLocation[i].copx = randx;
			PlayerLocation[i].copy = randy;
			PlayerLocation[i].coplvl = lvlnum;
			PlayerLocation[i].firecount = 0;
			PlayerLocation[i].dead = 0;
			PlayerLocation[i].numshots = 0;
			PlayerLocation[i].damage = 5;
			PlayerLocation[i].frame = 0;
			PlayerLocation[i].fx = 0;
			PlayerLocation[i].fy = 0;
			PlayerLocation[i].fx2 = 0;
			PlayerLocation[i].fy2 = 0;
			PlayerLocation[i].tolx = 3;
			PlayerLocation[i].toly = 4;
			PlayerLocation[i].dx = 0;
			PlayerLocation[i].protecttreasure = 0;
			PlayerLocation[i].dy = 0;
			PlayerLocation[i].dl = 0;
			PlayerLocation[i].moved = 0;
			PlayerLocation[i].speed = 2;

			//			PlayerLocation[i].maxspeed = 2;
			PlayerLocation[i].stairs = 0;
			PlayerLocation[i].character = 2;
			PlayerLocation[i].staircounter = 0;
			PlayerLocation[i].asleep = 0;
			strcpy(PlayerLocation[i].type, "Computer");
			strcpy(PlayerLocation[i].name, "Player");
			if (setability == 1) {
				PlayerLocation[i].skill = 0;
				PlayerLocation[i].ring = 1;
				PlayerLocation[i].armour = 0;
			}
			if (i <= newwarrior) {
				PlayerLocation[i].dead = 65;
			}
			if (i == 2)
				PlayerLocation[i].calcmove = 1;
			else
				PlayerLocation[i].calcmove = 0;

			whoisnext = 2;

			if (j >= 0 && j <= 4) {
				if (networkserver)
					PlayerLocation[1].active = FALSE;
				if (networkgame && ishost && PlayerLocation[i].active)
					dungeon[randx][randy][lvlnum].item = 'c';
				else if (networkgame && ishost)
					dungeon[randx][randy][lvlnum].item = 'm';
				else
					dungeon[randx][randy][lvlnum].item = 'c';
			}

			if (PlayerLocation[i].image == 1)
				dungeon[randx][randy][lvlnum].image = 0;
			else if (PlayerLocation[i].image == 5)
				dungeon[randx][randy][lvlnum].image = 3;
			else if (PlayerLocation[i].image == 3)
				dungeon[randx][randy][lvlnum].image = 1;
			else if (PlayerLocation[i].image == 4)
				dungeon[randx][randy][lvlnum].image = 2;

			if (j == 1) {
			}
			maxmove = 8;

			if (i > newwarrior) {
				PlayerLocation[i].active = TRUE;
				PlayerLocation[i].rest = 0;
				PlayerLocation[i].ability = 6;
				strcpy(PlayerLocation[i].type, "Beholder");
				PlayerLocation[i].limitspeed = TRUE;
				dungeon[randx][randy][lvlnum].item = 1;
				PlayerLocation[i].asleep = 1;
				PlayerLocation[i].shoot = 1;
				PlayerLocation[i].maxspeed = 2;
				PlayerLocation[i].team = 0;
				maxmove = 3;
				PlayerLocation[i].image = 2;
				PlayerLocation[i].character = 3;
				randnum = random_num(2);
				PlayerLocation[i].limitspeed = TRUE;
				randnum = random_num(2);
				if (randnum == 1) {
					PlayerLocation[i].maxspeed = 3;
				}

				randnum = random_num(5);
				if (j == 5) {
					// beholder
					randnum = random_num(6) + 1;
					PlayerLocation[i].ability = 26 + randnum + countlevels;
					PlayerLocation[i].maxspeed = 2;
					PlayerLocation[i].limitspeed = TRUE;
					PlayerLocation[i].image = 2;
					PlayerLocation[i].shoot = 1;
					PlayerLocation[i].maxmissle = 3;
					PlayerLocation[i].firerate = 5;
					PlayerLocation[i].intelligence = 1;
					PlayerLocation[i].damagemin = 2;
					PlayerLocation[i].damagemax = 4;
					PlayerLocation[i].maxspeed = gamedef.bspeed;
					//					PlayerLocation[i].ability = 20;
					PlayerLocation[i].ability = beholderability;

				} else if (j == 6) {
					// mouther
					randnum = random_num(3) + 1;
					PlayerLocation[i].ability = 3 + randnum + countlevels;
					PlayerLocation[i].maxspeed = 2;
					PlayerLocation[i].limitspeed = TRUE;
					PlayerLocation[i].image = 7;
					PlayerLocation[i].shoot = 1;
					PlayerLocation[i].maxmissle = 2;
					PlayerLocation[i].intelligence = 3;
					PlayerLocation[i].firerate = 8;
					PlayerLocation[i].damagemin = 1;
					PlayerLocation[i].damagemax = 1;
					PlayerLocation[i].maxspeed = gamedef.mspeed;
					PlayerLocation[i].ability = moutherability;
				} else if (j == 7) {
					// orc
					randnum = random_num(5) + 1;
					PlayerLocation[i].ability = 5 + randnum + countlevels;
					PlayerLocation[i].maxspeed = 2;
					PlayerLocation[i].limitspeed = TRUE;
					PlayerLocation[i].image = 8;
					PlayerLocation[i].shoot = 1;
					PlayerLocation[i].firerate = 7;
					PlayerLocation[i].intelligence = 1;
					PlayerLocation[i].maxmissle = 4;
					PlayerLocation[i].damagemin = 1;
					PlayerLocation[i].damagemax = 3;
					PlayerLocation[i].maxspeed = gamedef.ospeed;
					PlayerLocation[i].ability = orcability;

				} else if (j == 8) {
					// skeleton
					randnum = random_num(4) + 1;
					PlayerLocation[i].ability = 4 + randnum + countlevels;
					PlayerLocation[i].maxspeed = 2;
					PlayerLocation[i].limitspeed = TRUE;
					PlayerLocation[i].image = 6;
					PlayerLocation[i].shoot = 1;
					PlayerLocation[i].firerate = 9;
					PlayerLocation[i].intelligence = 2;
					PlayerLocation[i].maxmissle = 2;
					PlayerLocation[i].damagemin = 1;
					PlayerLocation[i].damagemax = 1;
					PlayerLocation[i].maxspeed = gamedef.sspeed;
					PlayerLocation[i].ability = skeletonability;

				} else if (j == 9) {
					// god help us - death orb

					randnum = random_num(10) + 1;
					PlayerLocation[i].ability = 30 + randnum + countlevels;
					PlayerLocation[i].maxspeed = 1;
					PlayerLocation[i].limitspeed = TRUE;
					PlayerLocation[i].image = 9;
					PlayerLocation[i].shoot = 1;
					PlayerLocation[i].firerate = 3;
					PlayerLocation[i].intelligence = 1;
					PlayerLocation[i].maxmissle = 8;
					PlayerLocation[i].damagemin = 2;
					PlayerLocation[i].damagemax = 6;

					PlayerLocation[i].maxspeed = gamedef.dspeed;
					PlayerLocation[i].ability = deathorbability;
				}
			}

			PlayerLocation[i].maxmove = maxmove;
			check_dungeon();
		}
	}
	NumPlayers = newwarrior +
	             gamedef.beholders + gamedef.mouther + gamedef.ogre + gamedef.skeletons +
	             gamedef.deathorb;

	monster_cop();
}

BOOL InitGraphics() {
	// pDirDraw = new CDirDraw(hWindow);
	pDirDraw = new CMyDirDraw(hWindow);
	ddok = pDirDraw->DirectDrawOK();
	if (!ddok)
		return FALSE;
	partsASurfaceNum = pDirDraw->CreateOffScreenSurface(800, 1280);
	if (partsASurfaceNum == 0)
		return FALSE;
	partsBSurfaceNum = pDirDraw->CreateOffScreenSurface(800, 1176);
	if (partsBSurfaceNum == 0)
		return FALSE;
	panelSurfaceNum = pDirDraw->CreateOffScreenSurface(259, 189);
	if (panelSurfaceNum == 0)
		return FALSE;
	murksplash = pDirDraw->CreateOffScreenSurface(800, 600);
	if (murksplash == 0)
		return FALSE;

	DungeonSurface2 = pDirDraw->CreateOffScreenSurface(800, 1000);
	if (DungeonSurface2 == 0)
		return FALSE;

	DungeonSurface3 = pDirDraw->CreateOffScreenSurface(800, 1000);
	if (DungeonSurface3 == 0)
		return FALSE;

	scroll = pDirDraw->CreateOffScreenSurface(800, 600);
	if (scroll == 0)
		return FALSE;

	selectplayer = pDirDraw->CreateOffScreenSurface(800, 1650);
	if (selectplayer == 0)
		return FALSE;

	LoadFloorDibs("../artwork/dungeon.bmp");
	LoadDudeDibs(1);
	LoadSplash();
	LoadScroll();
	LoadSelect();

	return TRUE;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void LoadFloorDibs(char *p) {
	CDib *pDib;
	pDib = new CDib(p);
	pDirDraw->CopyDibToSurface(partsBSurfaceNum,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 1176));
	delete pDib;

	pDib = new CDib("../artwork/dungeon2.bmp");
	pDirDraw->CopyDibToSurface(DungeonSurface2,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 600));
	delete pDib;

	pDib = new CDib("../artwork/dungeon3.bmp");
	pDirDraw->CopyDibToSurface(DungeonSurface3,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 600));
	delete pDib;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void LoadScroll() {

	CString splash("../artwork/scroll.bmp");
	CDib *pDib = new CDib(splash);
	pDirDraw->CopyDibToSurface(scroll,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 600));
	delete pDib;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void LoadSelect() {

	CString splash("../artwork/select.bmp");
	CDib *pDib = new CDib(splash);
	pDirDraw->CopyDibToSurface(selectplayer,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 1650));
	delete pDib;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void LoadSplash() {

	CString splash("../artwork/murk256.bmp");
	CDib *pDib = new CDib(splash);
	pDirDraw->CopyDibToSurface(murksplash,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 600));
	delete pDib;
}
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void LoadDudeDibs(UINT floor) {
	char s[5];
	wsprintf(s, "%d", floor);
	CString floorFileName("../artwork/p256.BMP");

	CDib *pDib = new CDib(floorFileName);
	BOOL resultOK = pDirDraw->SetPalette(pDib);
	pDirDraw->CopyDibToSurface(partsASurfaceNum,
	                           &CPoint(0, 0), pDib, &CRect(0, 0, 800, 1280));
	delete pDib;
}
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void load_panel() {
	CString floorFileName("../artwork/pan3.BMP");
	CDib *pDib = new CDib(floorFileName);
	//	254172
	pDirDraw->CopyDibToSurface(panelSurfaceNum, &CPoint(0, 0), pDib, &CRect(0, 0, 259, 189));
	delete pDib;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void clear_dungeon() {
	int lvlnum, x, y;

	for (lvlnum = 1; lvlnum <= 10; lvlnum++) {
		for (y = 0; y <= 30; y++) {
			for (x = 0; x <= 30; x++) {
				dungeon[x][y][lvlnum].mark = 0;
				dungeon[x][y][lvlnum].order = 0;
				dungeon[x][y][lvlnum].type = 0;
				dungeon[x][y][lvlnum].item = 0;
				dungeon[x][y][lvlnum].explored = 0;
			}
		}
	}
}

void init_dungeon() {

	int x, y;

	int work1;
	int work2;
	int level;

	for (level = 0; level <= maxlevel; level++) {
		work1 = random_num(2);
		if (work1 == 1)
			dimage[level] = 1;
		else
			dimage[level] = 2;
	}
	for (level = 1; level <= maxlevel; level++) {
		for (y = 0; y < DUNGEONY; y++) {
			for (x = 0; x < DUNGEONX; x++) {
				dungeon[x][y][level].type = 'f';
				dungeon[x][y][level].item = 0;
				dungeon[x][y][level].explored = 0;
			}
		}
		work1 = 8;
		for (y = 1; y < 8; y++) {
			for (x = 1; x < work1; x++) {
				dungeon[x][y][level].type = 'u';
				dungeon[x][y][level].item = 0;
				dungeon[x][y][level].explored = 0;
			}
			work1--;
		}
		work1 = 25;
		work2 = 10;
		for (y = 1; y < 13; y++) {
			for (x = work2; x < work1; x++) {
				dungeon[x][y][level].type = 'u';
				dungeon[x][y][level].item = 0;
				dungeon[x][y][level].explored = 0;
			}
			work2++;
		}
		work1 = 1;
		for (y = 12; y < 25; y++) {
			for (x = 1; x < work1; x++) {
				dungeon[x][y][level].type = 'u';
				dungeon[x][y][level].item = 0;
				dungeon[x][y][level].explored = 0;
			}
			work1++;
		}
		work1 = 25;
		work2 = 21;
		for (y = 15; y < 25; y++) {
			for (x = work2; x < work1; x++) {
				dungeon[x][y][level].type = 'u';
				dungeon[x][y][level].item = 0;
				dungeon[x][y][level].explored = 0;
			}
			work2--;
		}
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void assign_treasure() {

	int check;
	int randx, randy;
	int lvl;
	check = 0;

	if (gametype == 1) {
		treasurex = 0;
		treasurey = 0;
		treasurelvl = 1;
		return;
	}
	while (check == 0) {
		randx = random_num(DUNGEONX) + 1;
		randy = random_num(DUNGEONY) + 1;
		lvl = 1;
		if (dungeon[randx][randy][lvl].type == 'f' && dungeon[randx][randy][lvl].item == 0) {
			check = 1;
		} else {
			check = 0;
		}
	}
	dungeon[randx][randy][lvl].explored = 0;
	treasurex = randx;
	treasurey = randy;
	treasurelvl = lvl;
	foundtreasure = 0;
}
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void assign_treasure2() {

	int check;
	int randx, randy;
	int lvl;
	int randnum;
	check = 0;

	while (check == 0) {

		lvl = random_num(maxlevel) + 1;
		randx = random_num(DUNGEONX) + 1;
		randy = random_num(DUNGEONY) + 1;

		if (newwarrior == 1 || gametype == 1) {
			randnum = random_num(maxlevel - 1) + 1;
			randnum++;
			lvl = randnum;
		}
		if (gametype == 2) {
			randnum = random_num(maxlevel - 1) + 1;
			randnum++;
			lvl = maxlevel;
		}

		if (dungeon[randx][randy][lvl].type == 'f' && dungeon[randx][randy][lvl].item == 0) {
			check = 1;
		} else {
			check = 0;
		}
	}
	dungeon[randx][randy][lvl].explored = 0;
	treasurex2 = randx;
	treasurey2 = randy;
	treasurelvl2 = lvl;

	foundtreasure2 = 0;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void assign_elixir(int length) {

	int check;
	int randx, randy;
	int lvl;
	int i;
	int rand;

	for (i = 0; i < length; i++) {
		check = 0;
		while (check == 0) {
			randx = random_num(20) + 1;
			randy = random_num(20) + 1;
			lvl = random_num(maxlevel) + 1;
			if (dungeon[randx][randy][lvl].type == 'f' && dungeon[randx][randy][lvl].item == 0) {
				check = 1;
			} else {
				check = 0;
			}
		}

		rand = random_num(5) + 1;
		if (rand == 1)
			dungeon[randx][randy][lvl].item = 'a';
		//		else if (rand==2)
		//			dungeon[randx][randy][lvl].item = 'r' ;
		else if (rand == 3)
			dungeon[randx][randy][lvl].item = 's';
		else
			dungeon[randx][randy][lvl].item = 'e';
	}

	for (i = 0; i < 2; i++) {
		check = 0;
		while (check == 0) {
			randx = random_num(20) + 1;
			randy = random_num(20) + 1;
			lvl = random_num(maxlevel) + 1;
			if (dungeon[randx][randy][lvl].type == 'f' && dungeon[randx][randy][lvl].item == 0) {
				check = 1;
			} else {
				check = 0;
			}
		}

		dungeon[randx][randy][lvl].item = 'r';
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void monster_cop() {

	int check;
	int randx, randy;
	int lvl;
	check = 0;
	int i;
	int flip;
	flip = 4;

	for (i = newwarrior + 1; i <= NumPlayers; i++) {
		flip++;
		check = 0;

		if (flip == 5) {
			while (check == 0) {

				randx = random_num(DUNGEONX) + 1;
				randy = random_num(DUNGEONY) + 1;

				lvl = random_num(maxlevel - 1) + 2;
				if (lvl == maxlevel)
					lvl = maxlevel - 1;
				if (dungeon[randx][randy][lvl].type == 'f' && dungeon[randx][randy][lvl].item == 0) {
					check = 1;
					flip = 0;
				} else {
					check = 0;
				}
			}
		}

		dungeon[randx][randy][lvl].explored = 0;
		dungeon[randx][randy][lvl].item = 'c';
		dungeon[randx][randy][lvl].image = 4;
		PlayerLocation[i].copx = randx;
		PlayerLocation[i].copy = randy;
		PlayerLocation[i].coplvl = lvl;
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void assign_pit(int level) {

	int check;
	int randx, randy;
	int lvl;
	int count;
	int i;
	check = 0;

	if (pitcounter > 8)
		return;

	for (count = 1; count <= 3; count++) {
		check = 0;
		while (check == 0) {
			randx = random_num(DUNGEONX) + 1;
			randy = random_num(DUNGEONY) + 1;
			lvl = random_num(maxlevel - 1) + 1;
			if (dungeon[randx][randy][lvl + 1].type == 'f' &&
			    dungeon[randx][randy][lvl].type == 'f' && dungeon[randx][randy][lvl].item == 0) {
				for (i = 1; i <= 48; i++) {
					if (pits[i].x == randx && pits[i].y == randy && pits[i].lvl == lvl && pits[i].frame != 0) {
						return;
					} else
						check = 1;
				}

			} else {
				check = 0;
			}
		}

		for (i = 1; i <= 48; i++) {
			if (pits[i].frame == 0)
				break;
		}
		pits[i].x = randx;
		pits[i].y = randy;
		pits[i].lvl = lvl;
		pits[i].frame = 1;

		if (networkgame && ishost)
			send_pit(0, 0, randx, randy, lvl, 1);
	}
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void new_level(int nextlev) {

	int randnum;

	sharewarelevel++;

	srand((unsigned)time(NULL));

	init_ping_player();

	if (nextlev) {
		levelcounter++;
	}

	read_levels();
	if (ishost && networkgame) {
		if (networkserver)
			gamedef.fighter = 1;
		else
			gamedef.fighter = 0;
		gamedef.cleric = 1;
		gamedef.thief = 1;
		gamedef.wizard = 1;
	}

	DUNGEONX = gamedef.dungeonx;
	DUNGEONY = gamedef.dungeony;
	maxlevel = gamedef.dungeonlvl;
	gametype = gamedef.gametype;

	movelevel++;

	if (nextlev)
		countlevels++;

	if (nextlev && movelevel >= 1) {
		movelevel = 0;
	}

	for (randnum = 1; randnum <= 48; randnum++) {
		pits[randnum].x = 0;
		pits[randnum].y = 0;
		pits[randnum].lvl = 0;
		pits[randnum].frame = 0;
	}
	for (randnum = 0; randnum <= 1; randnum++) {
		dtreasure[randnum].towner = 0;
		dtreasure[randnum].tdx = 0;
		dtreasure[randnum].tdy = 0;
		dtreasure[randnum].tdlvl = 0;
	}

	gameerror = 0;

	pitcounter = 0;
	countsquares = 0;
	initplayer = 0;
	savelastmove = 1;
	foundtreasure = 0;
	foundtreasure2 = 0;
	computersmove = 0;
	CurrentPlayer = 1;
	frame_num = 1;
	startx = 10;
	starty = 370;
	newmove = 0;
	newmovex = 0;
	newmovey = 0;
	mousehit = 0;
	mousehitx = 0;
	mousehity = 0;
	hitkeyboard = 0;
	animateon = 0;
	framemove = 0;
	cancelmove = 0;
	startgame = 1;
	beholder = 0;
	init_chat();

	if (networkgame) {
		strcpy(chat[0].say, "Find the -p   and return it home");
		chat[0].duration = 90;

	} else {

		strcpy(chat[0].say, gamedef.response);
		chat[0].duration = 90;
	}

	strcpy(lastmove, "-");
	select1 = 0;
	select2 = 0;
	select3 = 0;
	select4 = 0;
	mousestate = 0;
	set_tile();

	clear_dungeon();

	init_death_scene();
	if (ishost) {
		init_dungeon();
		generate_dungeon();
		assign_player_cop();
		assign_elixir(16 + (maxlevel * 2));
		assign_treasure();
		assign_treasure2();
	}

	if (startthegame == 1)
		startthegame = 2;
	setability = 0;
	if (nextlev && networkgame && ishost) {

		send_new_level();
	}

	if (nextlev && !networkgame) {
		showmission = 2;

		if (ishost && !networkgame) {
			bdown = 0;
		}
	}
}

// ------------------------------
// Dungeon generation helpers (C-style, internal linkage)
// ------------------------------

static int is_inside_bounds(int x, int y) {
	// Playable area is 1..DUNGEONX and 1..DUNGEONY; we also keep a padded border at [0] and [DUNGEON?+1]
	return (x >= 1 && x <= DUNGEONX && y >= 1 && y <= DUNGEONY);
}

static void reset_level_grid(int lvlnum) {
	// Reset all cells (including padded border) for the level
	int x, y;
	for (y = 0; y <= DUNGEONY + 1; y++) {
		for (x = 0; x <= DUNGEONX + 1; x++) {
			dungeon[x][y][lvlnum].mark = 0;
			dungeon[x][y][lvlnum].order = 0;
			dungeon[x][y][lvlnum].type = 0;
			dungeon[x][y][lvlnum].item = 0;
			dungeon[x][y][lvlnum].explored = 0;
		}
	}
}

static void prefill_blocked_regions(int lvlnum) {
	// Recreate the original four 'u' fill passes, but guard by bounds to avoid OOB on small maps
	int x, y;
	int work1, work2;

	work1 = 8;
	for (y = 1; y < 8; y++) {
		for (x = 1; x < work1; x++) {
			if (is_inside_bounds(x, y)) {
				dungeon[x][y][lvlnum].type = 'u';
				dungeon[x][y][lvlnum].item = 0;
				dungeon[x][y][lvlnum].explored = 0;
			}
		}
		work1--;
	}

	work1 = 25;
	work2 = 9;
	for (y = 1; y < 11; y++) {
		for (x = work2; x < work1; x++) {
			if (is_inside_bounds(x, y)) {
				dungeon[x][y][lvlnum].type = 'u';
				dungeon[x][y][lvlnum].item = 0;
				dungeon[x][y][lvlnum].explored = 0;
			}
		}
		work2++;
	}

	work1 = 1;
	for (y = 12; y < 25; y++) {
		for (x = 1; x < work1; x++) {
			if (is_inside_bounds(x, y)) {
				dungeon[x][y][lvlnum].type = 'u';
				dungeon[x][y][lvlnum].item = 0;
				dungeon[x][y][lvlnum].explored = 0;
			}
		}
		work1++;
	}

	work1 = 25;
	work2 = 18;
	for (y = 11; y < 22; y++) {
		for (x = work2; x < work1; x++) {
			if (is_inside_bounds(x, y)) {
				dungeon[x][y][lvlnum].type = 'u';
				dungeon[x][y][lvlnum].item = 0;
				dungeon[x][y][lvlnum].explored = 0;
			}
		}
		work2--;
	}
}

static void seed_floor_at(int lvlnum, int x, int y) {
	// Safe seed of a floor tile (accepting padded border range)
	if (x >= 0 && x <= DUNGEONX + 1 && y >= 0 && y <= DUNGEONY + 1) {
		dungeon[x][y][lvlnum].type = 'f';
		dungeon[x][y][lvlnum].explored = 0;
		dungeon[x][y][lvlnum].item = 0;
	}
}

static void carve_random_walk(int lvlnum, int startx, int starty, int steps) {
	// Random walk that carves floor tiles while attempting to avoid 3-sided dead-ends
	int i;
	int x = startx;
	int y = starty;
	int xpos = 0, ypos = 0;
	int way, randval;
	int good;
	int quit;

	for (i = 0; i < steps; i++) {
		xpos = 0;
		ypos = 0;
		way = random_num(4);
		switch (way) {
		case 0:
			xpos = -1;
			break;
		case 1:
			xpos = +1;
			break;
		case 2:
			ypos = -1;
			break;
		case 3:
			ypos = +1;
			break;
		}

		quit = 0;
		good = 0;

		if ((x + xpos) > 0 && (x + xpos) < DUNGEONX &&
		    (y + ypos) > 0 && (y + ypos) < DUNGEONY &&
		    dungeon[x + xpos][y + ypos][lvlnum].type != 'u' &&
		    dungeon[x + xpos][y + ypos][lvlnum].type != 's') {

			// Occasionally skip the adjacency check for variety
			randval = random_num(30);
			if (randval != 1) {
				// Upper left
				if (dungeon[x + xpos - 1][y + ypos - 1][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos - 1][y + ypos][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos][y + ypos - 1][lvlnum].type == 'f')
					good++;
				if (good == 3)
					quit = 1;

				// Lower left
				good = 0;
				if (dungeon[x + xpos - 1][y + ypos + 1][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos - 1][y + ypos][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos][y + ypos + 1][lvlnum].type == 'f')
					good++;
				if (good == 3)
					quit = 1;

				// Upper right
				good = 0;
				if (dungeon[x + xpos + 1][y + ypos - 1][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos + 1][y + ypos][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos][y + ypos - 1][lvlnum].type == 'f')
					good++;
				if (good == 3)
					quit = 1;

				// Lower right
				good = 0;
				if (dungeon[x + xpos + 1][y + ypos + 1][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos][y + ypos + 1][lvlnum].type == 'f')
					good++;
				if (dungeon[x + xpos + 1][y + ypos][lvlnum].type == 'f')
					good++;
				if (good == 3)
					quit = 1;
			}

			if (quit == 0) {
				dungeon[x + xpos][y + ypos][lvlnum].type = 'f';
				dungeon[x + xpos][y + ypos][lvlnum].explored = 0;
				x = x + xpos;
				y = y + ypos;
			}
		}
	}
}

static void carve_level_from_center(int lvlnum, int sx, int sy) {
	// Perform four random walks of 75 steps each, starting from the center
	int jump;
	seed_floor_at(lvlnum, sx, sy);
	for (jump = 0; jump < 4; jump++) {
		carve_random_walk(lvlnum, sx, sy, 75);
	}
}

static void clear_stairs_on_level(int lvl) {
	// Convert any 's' (stairs) back to floor on a specific level
	int x, y;
	for (x = 0; x <= DUNGEONX + 1; x++) {
		for (y = 0; y <= DUNGEONY + 1; y++) {
			if (dungeon[x][y][lvl].type == 's') {
				dungeon[x][y][lvl].type = 'f';
				dungeon[x][y][lvl].item = 0;
			}
		}
	}
}

static void clear_stairs_pair(int lvl) {
	// Clear stairs both on lvl and on lvl+1 (if in range)
	clear_stairs_on_level(lvl);
	if (lvl + 1 <= maxlevel) {
		clear_stairs_on_level(lvl + 1);
	}
}

static int find_floor_cell_on_level(int lvlnum, int *px, int *py, int maxTries) {
	// Try random coordinates until we find a 'f' (floor) or we give up
	int tries = 0;
	int x, y;

	while (tries < maxTries) {
		x = random_num(DUNGEONX) + 1;
		y = random_num(DUNGEONY) + 1;
		if (dungeon[x][y][lvlnum].type == 'f') {
			*px = x;
			*py = y;
			return 1;
		}
		tries++;
	}
	return 0;
}

// ------------------------------
// Refactored dungeon generation
// ------------------------------

void generate_dungeon() {

	int x, y, i;
	int randx, randy;
	int lvlnum;
	int numnodes;
	int counttry;
	int leave;
	int sx, sy;

	int numtry = 0; // number of times we have tried to place stairs for the current level
	int stopit = 0; // global fail-safe to avoid infinite outer loops

	// Start carving near the center of the playable area
	sx = (int)(DUNGEONX) / 2;
	sy = (int)(DUNGEONY) / 2;

	randx = sx;
	randy = sy;

	lvlnum = 0;
	while (lvlnum < maxlevel) {
		lvlnum++;
		stopit++;
		if (stopit >= (DUNGEONX * DUNGEONY * maxlevel) * 2) {
			// Emergency reset if something goes terribly wrong
			lvlnum = 0;
			numtry = 0;
			debug_me("generate_dungeon", "could not generate dungeon", 0, 0);
			// return; // original code did not return; keep same behavior
		}

		// 1) Prepare the level grid and prefill "unreachable" spaces
		reset_level_grid(lvlnum);
		prefill_blocked_regions(lvlnum);

		// 2) Carve paths from the center
		carve_level_from_center(lvlnum, sx, sy);

		// 3) Validate node count and roll-back if too many nodes
		numnodes = count_nodes(lvlnum);
		counttry = 0;

		if (numnodes >= 30) {
			// Too many nodes for this level: roll back one level and clean up any stairs
			lvlnum--;

			if (lvlnum == 1 || lvlnum == 0) {
				lvlnum = 0;
				numtry = 0;
				debug_me("generate_dungeon", "NumNodes Start Again lvlnum==1", numtry, counttry);
			} else {
				debug_me("generate_dungeon", "Too many nodes", numnodes, 0);
				clear_stairs_pair(lvlnum);
			}
			continue;
		}

		// 4) Connect with stairs to the previous level if not at the first level
		if (lvlnum > 1) {
			leave = 0;

			for (i = 1; i < 4; i++) {
				int check = 0;

				while (check == 0) {
					// 4.a) Find a random floor on current level (bounded attempts)
					int found = find_floor_cell_on_level(lvlnum, &randx, &randy, (DUNGEONX * DUNGEONY * 2));
					if (!found) {
						// Could not find a floor tile - give up and restart
						leave = 1;
						check = 1;
						break;
					}
					debug_me("generate_dungeon", "found floor", numtry, counttry);

					// 4.b) If floor aligns with the same coordinate on previous level, accept
					counttry++;
					if (counttry >= 350) {
						// Too many attempts: roll back a level and clean up stairs
						lvlnum--;
						debug_me("generate_dungeon", "Failed numtry counttry", numtry, counttry);

						if (lvlnum == 1 || numtry >= 10 || lvlnum == 0) {
							// keep rolling back to restart
						} else {
							clear_stairs_pair(lvlnum);
						}
						leave = 1;
						check = 1;

						numtry++;
						i = 4; // break outer for loop
						break;
					}

					if (dungeon[randx][randy][lvlnum - 1].type == 'f' &&
					    dungeon[randx][randy][lvlnum].type == 'f') {
						check = 1;
					} else {
						check = 0;
					}
				}

				if (leave)
					break;

				// 4.c) Place the stairs pair (up/down) between lvlnum-1 and lvlnum
				dungeon[randx][randy][lvlnum - 1].type = 's';
				dungeon[randx][randy][lvlnum - 1].item = 0;
				dungeon[randx][randy][lvlnum].type = 's';
				dungeon[randx][randy][lvlnum].item = 1;
			}
		}
	}
}

void init_chat() {

	int i;

	for (i = 0; i < 10; i++) {
		strcpy(chat[i].say, "");
		chat[i].duration = 6;
	}
}

void init_ping_player() {

	int i;

	for (i = 0; i < 10; i++) {
		pingplayer[i] = 0;
	}
}

void set_tile() {

	int i;
	int randnum;

	randnum = 1;

	for (i = 1; i < 39; i++) {
		//		randnum = random_num(3)+1;

		tile_type[i] = randnum;
		randnum++;
		if (randnum > 3)
			randnum = 1;
	}
}

void init_death_scene() {

	int i;

	for (i = 0; i < 99; i++) {
		dscene[i].x = 0;
		dscene[i].y = 0;
		dscene[i].lvl = 0;
		dscene[i].image = 0;
		dscene[i].direction = 0;
		dscene[i].counter = 0;
		dscene[i].mapx = 0;
		dscene[i].mapy = 0;
	}
}