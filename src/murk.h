// Main Murk Variables

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif

#include <winsock2.h>
#include <afx.h>

// #include <windows.h>
#include <string.h>
#include <winuser.h>
#include "resource.h"
#include "cdirdraw.h"
#include "cdirsnd.h"
#include "cwave.h"
#include "cmydd.h"

#include "Game.h"

#include "Resource.h"

CGame *m_pGame;
LPGUID lpGuid;

struct PlayerDat {
	int protection;
	int skill;
	int stairsx;
	int stairsy;
	int justdied;
	int x;
	int y;
	int direction;
	int mapx;
	int mapy;
	int level;
	int copx;
	int copy;
	int coplvl;
	int asleep;
	char name[20];
	char type[20];
	int maxmove;
	char lastmove[20];
	int dead;
	int fx;
	int fy;
	int fx2;
	int fy2;
	int frame;
	BOOL limitspeed;
	int dx;
	int dy;
	int dl;
	int maxspeed;
	int moved;
	int speed;
	int calcmove;
	int image;
	int character;
	int stairs;
	int ability;
	int damage;
	int hit;
	int tolx;
	int toly;
	int rest;
	int staircounter;
	int shoot;
	int track;
	int framehuman;
	char lasttarget;
	int numshots;
	int protecttreasure;
	int team;
	int firerate;
	int firecount;
	int maxmissle;
	int intelligence;
	int armour;
	int ring;
	BOOL active;
	int damagemin;
	int damagemax;
} PlayerLocation[400];

struct missle_struct {
	int misslex;
	int missley;
	int misslelvl;
	int frame;
	int image;
	int direction;
	int owner;
	int missledx;
	int missledy;
} missle[100];

struct dungeon_struct {
	char type;
	BOOL explored;
	char item;
	int image;
	int mark;
	int order;
};

// struct dungeon_struct dungeon [40][40][40];

typedef struct {
	int dir;
	int x;
	int y;
} mazestack;

//	typedef mazestack *stack;
LPDIRECTPLAY m_pDirectPlay;

DPID m_dpidPlayer[10];
HANDLE m_hDPEvent;

//  TileState     m_tsGrid[3][3];
UINT m_uiTurns;
BOOL m_bMyTurn;

BOOL moveok;
BOOL computersmove;

BOOL bIsActive;

struct dungeon_struct dungeon[140][140][40];

int DUNGEONX;
int DUNGEONY;
int maxlevel;

DWORD dwsession;

int oktosend;

int dialogresponse;

int gametype = 0;
int showmission;
int calcmousex;
int calcmousey;
int levelcounter;

int startsolve = 0;
int hitanykey;
int whoisnext;

int shortestroute;
int circ;
int camera;
int framemove;
int cancelmove;
int startgame;
int shiftkey;
int newwarrior;
int mousehit;
int mousehitx;
int mousehity;
int timerlength;
int pitcounter;
int moveitx;
int moveity;

int moveitxfin;
int moveityfin;

int orderglobal;
int saveshort;

int select1, select2, select3, select4;

int start_recursion = 0;

int hitkeyboard;
int setability;
int resultok;
int playery, playerx;
char lastmove[100];
int dx;
int dy;
int savelevel;
int movelevel;
int CurrentPlayer;
int NumPlayers;
int moved;
int speed;
int beholder;
int maxmove;
int fx;
int fx2;
int fy;
int fy2;
int animateon;
int treasurex;
int treasurey;
int treasurelvl;
int foundtreasure;

int treasurex2;
int treasurey2;
int treasurelvl2;
int foundtreasure2;

int playerfx;
int playerfy;
int playerfx2;
int playerfy2;
int frame;
int startx;
int starty;
int playerdx;
int playerdy;
int playerdl;
int newmove;
int newmovex;
int newmovey;
int dimage[100];
int frame_num;
int MoveNum;
int savelastmove;
char response[900];

int startthegame;

struct gamet {
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
};

struct gamet gamedef;

struct thepits {
	int x;
	int y;
	int lvl;
	int frame;
};
struct thepits pits[50];

struct murkregistry {
	char name[50];
	char key[50];
	char version[50];
	int sound;
	int joystick;
	int registered;
};
struct murkregistry murkreg[1];

struct deathscene {
	int x;
	int y;
	int lvl;
	int image;
	int direction;
	int counter;
	int mapx;
	int mapy;
};
struct deathscene dscene[100];

mazestack *stack;

UINT partsASurfaceNum, partsBSurfaceNum, panelSurfaceNum;
UINT murksplash;
UINT scroll;
UINT selectplayer;
UINT DungeonSurface2;
UINT DungeonSurface3;

int tile_type[40];

CWave *m_pWave;
CDirSound *m_pDirSound;

UINT m_bufferNum;

CWave *m_pMusic;
UINT m_bufferMusic;

CWave *m_pDrip;
UINT m_bufferDrip;

CWave *m_pScream;
UINT m_bufferScream;

// CWave* m_pBeholder;
// UINT m_bufferBeholder;

CWave *m_pTreasure;
UINT m_bufferTreasure;

CWave *m_pDead;
UINT m_bufferDead;

CWave *m_pWin;
UINT m_bufferWin;

CWave *m_pReward;
UINT m_bufferReward;

CWave *m_pBow;
UINT m_bufferBow;

CWave *m_pSword;
UINT m_bufferSword;

CWave *m_pYell;
UINT m_bufferYell;

CWave *m_pHit;
UINT m_bufferHit;

CWave *m_pMonsterdie;
UINT m_bufferMonsterdie;

CWave *m_pMumble;
UINT m_bufferMumble;

CWave *m_pDragon;
UINT m_bufferDragon;

CWave *m_pRegen;
UINT m_bufferRegen;

CWave *m_pSkeleton;
UINT m_bufferSkeleton;

CWave *m_pDorb;
UINT m_bufferDorb;

CWave *m_pFireball;
UINT m_bufferFireball;

CWave *m_pOuch;
UINT m_bufferOuch;

CWave *m_pButton;
UINT m_bufferButton;

CWave *m_pSwordThief;
UINT m_bufferSwordThief;

CWave *m_pSwordBarb;
UINT m_bufferSwordBarb;

CWave *m_pEyeswizard;
UINT m_bufferEyeswizard;

CWave *m_pTadaCleric;
UINT m_bufferTadaCleric;

CWave *m_pProtection;
UINT m_bufferProtection;

BOOL m_directSoundOK;

// CONST MAXMOVE = 7 ;
HINSTANCE hInstance;

// CDirDraw* pDirDraw;

CMyDirDraw *pDirDraw;
BOOL ddok;
BOOL setupOK = FALSE;
HWND hWindow;

typedef enum {
	North,
	East,
	South,
	West
} directionT;

int ishost;

int networkgame;

int netplayernum;

int networkserver;

int items;

int dropt;

int bdown;

char networkresponse[512];

struct chatit {
	char say[300];
	char duration;
} chat[10];

char chatin;
char chatcurrent;

char gamescreen;

char chaton;

int advancelevel;
char advanceattrib;

int countlevels;
int istimerset;

HANDLE mainthread;
HANDLE globalthread;
HANDLE quitthread;
DWORD IDThread;

BOOL sendonce;

int countsquares;
int stopthetime;
int cronit;
BOOL receivedata;

int initplayer;

int receiveok;

int pingplayer[10];

int helpscreen;

struct droptreasure {

	int towner;
	int tdx;
	int tdy;
	int tdlvl;
};

struct droptreasure dtreasure[3];

int gameerror;

int shareware;
int sharewarelevel;

int abortgame;