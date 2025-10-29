//Murk Common Variables


#include <afx.h>
//#include <windows.h>
#include <string.h>
#include <winuser.h>
#include "resource.h"
#include "cdirdraw.h"
#include "cdirsnd.h"
#include "cwave.h"
#include "cmydd.h"

#include "Game.h"
#include "Resource.h"

struct thepits {
	int x;
	int y;
	int lvl;
	int frame;
};

extern CGame* m_pGame;
extern struct thepits pits[50];
extern LPGUID  lpGuid;
extern int oktosend;

struct murkregistry {
	char name[50];
	char key[50];
	char version[50];
	int sound;
	int joystick;
	int registered;
};
extern struct murkregistry murkreg[1];

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
};

extern PlayerDat PlayerLocation[400];

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
};
extern missle_struct missle[100];

struct dungeon_struct {
	char type;
	BOOL explored;
	char item;
	int image;
	int mark;
	int order;
};

extern struct dungeon_struct dungeon[140][140][40];

typedef	struct {
	int dir;
	int x;
	int y;
} mazestack;

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
extern struct deathscene dscene[100];




extern DWORD dwsession;
extern BOOL moveok;
extern BOOL computersmove;

extern int gametype;
extern int showmission;
extern int calcmousex;
extern int calcmousey;
extern int levelcounter;
extern  LPDIRECTPLAY  m_pDirectPlay;
extern  DPID          m_dpidPlayer[10];
extern  HANDLE        m_hDPEvent;

//extern  TileState     m_tsGrid[3][3];
extern  UINT          m_uiTurns;
extern  BOOL          m_bMyTurn;


extern int startthegame;
extern BOOL bIsActive;
extern int startsolve;
extern int hitanykey;
extern int whoisnext;
extern int dialogresponse;
extern int shortestroute;
extern int circ;
extern int camera;
extern int framemove;
extern int cancelmove;
extern int startgame;
extern int shiftkey;
extern int newwarrior;
extern int mousehit;
extern int mousehitx;
extern int mousehity;
extern int timerlength;
extern int pitcounter;
extern int moveitx;
extern int moveity;

extern int moveitxfin;
extern int moveityfin;

extern int orderglobal;
extern int saveshort;


extern int select1, select2, select3, select4;


extern int start_recursion;

extern int hitkeyboard;
extern int setability;
extern int resultok;
extern int playery, playerx;
extern char lastmove[100];
extern int dx;
extern int dy;
extern int savelevel;
extern int movelevel;
extern int CurrentPlayer;
extern int NumPlayers;
extern int moved;
extern int speed;
extern int beholder;
extern int maxmove;
extern int fx;
extern int fx2;
extern int fy;
extern int fy2;
extern int animateon;
extern int treasurex;
extern int treasurey;
extern int treasurelvl;
extern int foundtreasure;

extern int treasurex2;
extern int treasurey2;
extern int treasurelvl2;
extern int foundtreasure2;

extern  int maxlevel;
extern int playerfx;
extern int playerfy;
extern int playerfx2;
extern int playerfy2;
extern int frame;
extern int startx;
extern int starty;
extern int playerdx;
extern int playerdy;
extern int playerdl;
extern int newmove;
extern int newmovex;
extern int newmovey;
extern int dimage[100];
extern  int DUNGEONX;
extern  int DUNGEONY;
extern int frame_num;
extern int MoveNum;
extern int savelastmove;
extern char response[900];

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



extern struct gamet gamedef;

extern mazestack* stack;

extern UINT partsASurfaceNum, partsBSurfaceNum, panelSurfaceNum;
extern UINT murksplash;
extern UINT scroll;
extern UINT selectplayer;

extern UINT DungeonSurface2;
extern UINT DungeonSurface3;

extern int tile_type[40];

extern CWave* m_pWave;
extern CDirSound* m_pDirSound;

extern UINT m_bufferNum;

extern CWave* m_pMusic;
extern UINT m_bufferMusic;

extern CWave* m_pDrip;
extern UINT m_bufferDrip;

extern CWave* m_pScream;
extern UINT m_bufferScream;

//CWave* m_pBeholder;
//UINT m_bufferBeholder;

extern CWave* m_pTreasure;
extern UINT m_bufferTreasure;

extern CWave* m_pDead;
extern UINT m_bufferDead;

extern CWave* m_pWin;
extern UINT m_bufferWin;

extern CWave* m_pReward;
extern UINT m_bufferReward;

extern CWave* m_pBow;
extern UINT m_bufferBow;

extern CWave* m_pSword;
extern UINT m_bufferSword;

extern CWave* m_pYell;
extern UINT m_bufferYell;

extern CWave* m_pHit;
extern UINT m_bufferHit;

extern CWave* m_pMonsterdie;
extern UINT m_bufferMonsterdie;

extern CWave* m_pMumble;
extern UINT m_bufferMumble;

extern CWave* m_pDragon;
extern UINT m_bufferDragon;

extern CWave* m_pRegen;
extern UINT m_bufferRegen;

extern CWave* m_pSkeleton;
extern UINT m_bufferSkeleton;

extern CWave* m_pDorb;
extern UINT m_bufferDorb;

extern CWave* m_pFireball;
extern UINT m_bufferFireball;

extern CWave* m_pOuch;
extern UINT m_bufferOuch;

extern CWave* m_pButton;
extern UINT m_bufferButton;

extern CWave* m_pSwordThief;
extern UINT m_bufferSwordThief;

extern CWave* m_pSwordBarb;
extern UINT m_bufferSwordBarb;


extern CWave* m_pEyeswizard;
extern UINT m_bufferEyeswizard;

extern CWave* m_pTadaCleric;
extern UINT m_bufferTadaCleric;

extern CWave* m_pProtection;
extern UINT m_bufferProtection;



extern BOOL m_directSoundOK;

//extern CONST MAXMOVE ;
extern HINSTANCE hInstance;

//extern CDirDraw* pDirDraw;


extern CMyDirDraw* pDirDraw;
extern BOOL ddok;
extern BOOL setupOK;
extern HWND hWindow;

typedef enum { North, East, South, West } directionT;

extern int ishost;

extern int networkgame;

extern int netplayernum;

extern int networkserver;

extern int items;

extern int bdown;


extern int dropt;

extern char networkresponse[512];

struct chatit {
	char say[300];
	char duration;
};


extern struct chatit chat[10];

extern char chatin;
extern char chatcurrent;
extern char gamescreen;
extern char chaton;

extern int advancelevel;
extern char advanceattrib;
extern int countlevels;



extern HANDLE mainthread;
extern HANDLE globalthread;
extern HANDLE quitthread;
extern DWORD IDThread;

extern BOOL sendonce;

extern int countsquares;
extern int stopthetime;
extern int cronit;
extern BOOL receivedata;

extern int initplayer;
extern int istimerset;


extern int receiveok;

extern int pingplayer[10];
extern int helpscreen;


struct droptreasure {

	int towner;
	int tdx;
	int tdy;
	int tdlvl;
};

extern struct droptreasure dtreasure[3];

extern int gameerror;


extern int shareware;
extern int sharewarelevel;


extern int abortgame;



