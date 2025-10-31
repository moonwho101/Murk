
//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.

#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void delay_s(unsigned char seconds) {

	time_t tstart, tend;
	time(&tstart);
	time(&tend);
	while (difftime(tend, tstart) < (double)seconds) {
		time(&tend);
	}
}
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void clear_log() {

	//	if (!networkserver)
	//		remove("log");
}

void debug_me(char *u, char *s, int a, int b) {

	// Turn off logging for release version
	return;

	FILE *fp;
	CTime t;
	t = CTime::GetCurrentTime();
	if ((fp = fopen("log.txt", "a")) == NULL) {
	}

	fprintf(fp, "%s %-21s %-34s %-4d %-4d\n", t.Format("%m/%d %H:%M"), u, s, a, b);

	fclose(fp);
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

int random_num(int num) {
	UINT rndNum;
	rndNum = rand() % num;

	return rndNum;
}

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void out_map(int sx, int sy, int dir, int shortestroute, int saveroute, int size) {

	FILE *fp;

	int x, y;
	if ((fp = fopen("map", "a")) == NULL) {
	}

	for (y = 0; y < DUNGEONY; y++) {
		for (x = 0; x < DUNGEONX; x++) {

			if (x == sx && y == sy)
				fputc('S', fp);
			else if (x == dx && y == dy)
				fputc('D', fp);
			else if (dungeon[x][y][1].mark == 'X')
				fputc('X', fp);
			else
				fputc(dungeon[x][y][1].type, fp);
		}
		fputc('\n', fp);
	}

	fputc('\n', fp);
	fprintf(fp, "dir %d shortest %d saveroute %d size %d x %d y %d dx %d dy %d", dir, shortestroute, saveroute, size, x, y, dx, dy);
	fprintf(fp, "mx %d my%d", moveitx, moveity);

	fputc('\n', fp);

	fclose(fp);
}
