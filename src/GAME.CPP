//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.

// The guts of NETWORK PLAY.

#include "StdAfx.h"
#include <windowsx.h>
#include "Game.h"
#include "GUID.h"
#include "Resource.h"
#include <afxmt.h>
#include "murkcommon.h"
#include "murkdecl.h"
#include "mapiwin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define MSG_NUMPLAYERS 0x11
#define MSG_SENDMAN 0x12
#define MSG_PLAYERNUM 0x13
#define MSG_MISSLE 0x14
#define MSG_QUIT 0x15
#define MSG_PLAYERQUICK 0x16
#define MSG_DUNGEONSQUARE 0x17
#define MSG_CHAT 0x18
#define MSG_DUNGEONSIZE 0x19
#define MSG_REWARD 0x20
#define MSG_PLAYER 0x22
#define MSG_STARTTIMER 0x33
#define MSG_DELETEPLAYER 0x34
#define MSG_ERROR 0x35
#define MSG_VERSION 0x36
#define MSG_DEATH 0x37
#define MSG_STOPTIMER 0x44
#define MSG_PIT 0x45
#define MSG_TREASURE 0x55
#define MSG_PLAYERDIR 0x66
#define MSG_PLAYSOUND 0x67
#define MSG_SENDDUNGEON 0x77
#define MSG_DROPTREASURE 0x78
#define MSG_HOSTPING 0x79
#define MSG_DUNGEON 0x88

#define MAX_BUFFER_SIZE 512

BYTE CommBuff[512];
void send_reward(int slot, int reward);
void send_dungeon_size(int slot);
void send_pinghost();
void send_play_sound(int sound);
void send_pit(int slot, int forsure, int x, int y, int lvl, int frame);
void send_pingserver();
void send_client_error(int slot, int problem);
void send_version(int slot);
void kill_dplay();
void send_death(int player, int x, int y, int lvl, int image, int direction, int counter, int mapx, int mapy, int dead);

extern void play_random_music();
extern void stop_song();

extern void build_server_screen();
BOOL doit;
extern DWORD Globalid;

extern void find_tcp_ipx();

typedef struct _MSGINT {
	BYTE MsgCode;
	int miscint1;
	int miscint2;
} MSGINT, *LPMSGINT;

typedef struct _MSGCHAR {
	BYTE MsgCode;
	int miscchar1;
	int miscchar2;
} MSGCHAR, *LPMSGCHAR;

typedef struct _CHATMSG {
	BYTE MsgCode;
	char say[80];
} CHATMSG, *LPCHATMSG;

typedef struct _DUNGEONMSG {
	BYTE MsgCode;
	char type;
	BYTE explored;
	char item;
	BYTE image;
	BYTE mark;
	BYTE order;
	BYTE x;
	BYTE y;
	BYTE lvl;

} DUNGEONMSG, *LPDUNGEON;

typedef struct _DSCENE {
	BYTE MsgCode;
	BYTE player;
	int x;
	int y;
	BYTE lvl;
	BYTE image;
	BYTE direction;
	BYTE counter;
	BYTE mapx;
	BYTE mapy;
	int dead;
} DSCENE, *LPDEATH;

typedef struct _MISSLEMSG {
	BYTE MsgCode;
	WORD slot;
	WORD misslex;
	WORD missley;
	WORD misslelvl;
	WORD frame;
	WORD image;
	WORD direction;
	WORD owner;
	WORD missledx;
	WORD missledy;
} MISSLEMSG, *LPMISSLE;

typedef struct _PLAYERMSG {
	BYTE MsgCode;
	BYTE PlayerNum;
	BYTE protection;
	BYTE skill;
	BYTE stairsx;
	BYTE stairsy;
	BYTE justdied;
	WORD x;
	WORD y;
	BYTE direction;
	BYTE mapx;
	BYTE mapy;
	BYTE level;
	BYTE copx;
	BYTE copy;
	BYTE coplvl;
	BYTE asleep;
	char name[20];
	char type[20];
	BYTE maxmove;
	char lastmove[20];
	int dead;
	WORD fx;
	WORD fy;
	WORD fx2;
	WORD fy2;
	WORD frame;
	BYTE limitspeed;
	BYTE dx;
	BYTE dy;
	BYTE dl;
	BYTE maxspeed;
	BYTE moved;
	BYTE speed;
	BYTE calcmove;
	BYTE image;
	BYTE character;
	BYTE stairs;
	BYTE ability;
	BYTE damage;
	BYTE hit;
	BYTE tolx;
	BYTE toly;
	BYTE rest;
	BYTE staircounter;
	BYTE shoot;
	BYTE track;
	BYTE framehuman;
	char lasttarget;
	BYTE numshots;
	BYTE protecttreasure;
	BYTE team;
	BYTE firerate;
	BYTE firecount;
	BYTE maxmissle;
	BYTE intelligence;
	BYTE armour;
	BYTE ring;
	BYTE damagemin;
	BYTE damagemax;
	BYTE active;
} PLAYERMSG, *LPPLAYER;

typedef struct _PLAYERDIR {
	BYTE MsgCode;
	BYTE PlayerNum;
	BYTE justdied;
	WORD x;
	WORD y;
	BYTE direction;
	BYTE mapx;
	BYTE mapy;
	BYTE level;
	BYTE asleep;
	char lastmove[20];
	WORD fx;
	WORD fy;
	WORD fx2;
	WORD fy2;
	int dead;
	BYTE frame;
	BYTE dx;
	BYTE dy;
	BYTE dl;
	BYTE hit;
	BYTE tolx;
	BYTE toly;
	BYTE rest;
	BYTE track;
	char lasttarget;
	BYTE numshots;
	BYTE firecount;
	BYTE speed;
	BYTE protection;
	BYTE calcmove;
	BYTE moved;
} PLAYERDIR, *LPPLAYERDIR;

typedef struct _PLAYERQUICK {
	BYTE MsgCode;
	BYTE PlayerNum;
	WORD x;
	WORD y;
	BYTE direction;
	BYTE mapx;
	BYTE mapy;
	BYTE level;
	WORD fx;
	WORD fy;
	WORD fx2;
	WORD fy2;
	BYTE frame;
	BYTE speed;
	BYTE rest;
	BYTE calcmove;
	BYTE moved;
} PLAYERQUICK, *LPPLAYERQUICK;

BOOL CALLBACK DlgProcChooseProvider(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID lpContext,
                            LPDWORD lpdwTimeOut, DWORD dwFlags);
BOOL CALLBACK DlgProcSelSession(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HRESULT ShutdownConnection();
HRESULT SetupConnection();
void serversettings(int go);
void send_drop_treasure(int treasure);
extern char serverscreen[100];

int networktype;

CGame::CGame() {
}

CGame::~CGame() {
	//  DPCleanup();
}

BOOL CGame::DPInit() {

	int i;

	for (i = 0; i < 10; i++) {
		m_dpidPlayer[i] = 0;
	}

	return (::DirectPlayCreate(lpGuid,
	                           &m_pDirectPlay, NULL) == DP_OK);

	return FALSE;
}

void CGame::DPCleanup() {

	// End the DP session
	//  DPEndSession();

	// Cleanup the DP object
	//  if (m_pDirectPlay)
	//  {
	//    m_pDirectPlay->Release();
	//    m_pDirectPlay = NULL;
	//  }
}

BOOL CGame::DPCreateSession() {
	if (m_pDirectPlay) {
		DPSESSIONDESC dpsdDesc;
		::ZeroMemory(&dpsdDesc, sizeof(DPSESSIONDESC));
		dpsdDesc.dwSize = sizeof(DPSESSIONDESC);
		dpsdDesc.dwMaxPlayers = 4;
		dpsdDesc.dwFlags = DPOPEN_CREATESESSION;
		dpsdDesc.guidSession = TICTACTOE_10;
		strcpy(dpsdDesc.szSessionName, "Murk");
		if (m_pDirectPlay->Open(&dpsdDesc) == DP_OK) {
			// Create local player and set game info
			m_pDirectPlay->EnableNewPlayers(TRUE);

			if (DPCreateLocalPlayer()) {
				m_bMyTurn = TRUE;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CGame::DPConnectSession() {

	HRESULT hr;

	if (m_pDirectPlay) {
		if (1) {
			// Open remote DP session
			DPSESSIONDESC dpsdDesc;
			::ZeroMemory(&dpsdDesc, sizeof(DPSESSIONDESC));
			dpsdDesc.dwSize = sizeof(DPSESSIONDESC);
			dpsdDesc.dwFlags = DPOPEN_OPENSESSION;
			dpsdDesc.guidSession = TICTACTOE_10;

			dpsdDesc.dwSession = dwsession;

			hr = m_pDirectPlay->Open(&dpsdDesc);

			switch (hr) {

			case DPERR_ACCESSDENIED:
				debug_me("DPConnectSession", "Open Session Access Denied", 0, 0);
				break;
			case DPERR_ALREADYINITIALIZED:
				debug_me("DPConnectSession", "Initialize Session", 0, 0);
				break;
			case DPERR_CONNECTING:
				debug_me("DPConnectSession", "Connecting Error", 0, 0);
				break;
			case DPERR_NOCONNECTION:
				debug_me("DPConnectSession", "No Connection", 0, 0);
				break;
			case DPERR_NONEWPLAYERS:
				debug_me("DPConnectSession", "No New Players", 0, 0);
				abortgame = 5;
				break;
			case DPERR_TIMEOUT:
				debug_me("DPConnectSession", "Timeout", 0, 0);
				break;
			case DPERR_UNINITIALIZED:
				debug_me("DPConnectSession", "Uninitialized", 0, 0);
				break;
			case DPERR_USERCANCEL:
				debug_me("DPConnectSession", "User cancelled", 0, 0);
				break;
			}

			if (hr == DP_OK) {
				m_dpidPlayer[1] = 1;

				debug_me("DPConnectSession", "Attached and ready...", 0, 0);
				m_pDirectPlay->EnableNewPlayers(TRUE);
				if (DPCreateLocalPlayer()) {
					debug_me("DPConnectSession", "Returned Success", 0, 0);
					m_bMyTurn = FALSE;
					return TRUE;
				}
			}
		}
	}
	debug_me("DPConnectSession", "Failed...", 0, 0);
	return FALSE;
}

void CGame::DPEndSession() {

	//  Destroy the local DP player and clear the remote player
	DPDestroyLocalPlayer();
	m_dpidPlayer[1] = 0;

	// Close the DP object
	if (m_pDirectPlay)
		m_pDirectPlay->Close();

	if (m_pDirectPlay) {
		m_pDirectPlay->Release();
		m_pDirectPlay = NULL;
	}

	delete m_pGame;
}

//-----------------------------------------------------------------
// CGame Protected Methods
//-----------------------------------------------------------------
BOOL CGame::DPCreateLocalPlayer() {
	char friendname[100];
	char formalname[100];

	strcpy(friendname, "friend");
	strcpy(formalname, "friend");

	globalthread = NULL;
	if (m_pDirectPlay->CreatePlayer(&m_dpidPlayer[0], "Friend", "Formal", &globalthread) == DP_OK) {
		debug_me("DPCreateLocalPlayer", "CreatePlayer Success", 0, 0);
		make_thread2();
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CGame::DPDestroyLocalPlayer() {
	// Destroy local DP player
	if (m_pDirectPlay->DestroyPlayer(m_dpidPlayer[0]) == DP_OK) {
		m_dpidPlayer[0] = 0;
		return TRUE;
	}

	return FALSE;
}

void send_dungeon_info(int slot) {

	int x, y, lvl;
	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages
	for (lvl = 1; lvl <= maxlevel; lvl++) {
		for (y = 0; y <= DUNGEONY + 1; y++) {
			for (x = 0; x <= DUNGEONX + 1; x++) {
				if (dungeon[x][y][lvl].type == 's' || dungeon[x][y][lvl].type == 'f') {
					lpDungeon = (LPDUNGEON)Comm;
					lpDungeon->MsgCode = MSG_DUNGEON;
					lpDungeon->type = dungeon[x][y][lvl].type;
					lpDungeon->explored = dungeon[x][y][lvl].explored;
					lpDungeon->item = dungeon[x][y][lvl].item;
					lpDungeon->image = dungeon[x][y][lvl].image;
					lpDungeon->mark = dungeon[x][y][lvl].mark;
					lpDungeon->order = dungeon[x][y][lvl].order;
					lpDungeon->x = x;
					lpDungeon->y = y;
					lpDungeon->lvl = lvl;
					nBytes = sizeof(DUNGEONMSG);
					Sleep(8);
					if (slot != 0) {
						if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
						                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
						}
					} else {
						if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
						                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
						}
					}
				}
			}
		}
	}
}

void send_player(int i, int client, int slot) {

	DWORD nBytes;
	LPPLAYER lpPlayer;

	BYTE Comm[512]; // Communication buffer for messages

	lpPlayer = (LPPLAYER)Comm;
	lpPlayer->MsgCode = MSG_PLAYER;

	if (client == 1)
		lpPlayer->PlayerNum = netplayernum;
	else
		lpPlayer->PlayerNum = i;

	lpPlayer->protection = PlayerLocation[i].protection;
	lpPlayer->firecount = PlayerLocation[i].firecount;
	lpPlayer->skill = PlayerLocation[i].skill;
	lpPlayer->stairsx = PlayerLocation[i].stairsx;
	lpPlayer->stairsy = PlayerLocation[i].stairsy;
	lpPlayer->justdied = PlayerLocation[i].justdied;
	lpPlayer->x = PlayerLocation[i].x;
	lpPlayer->y = PlayerLocation[i].y;
	lpPlayer->direction = PlayerLocation[i].direction;
	lpPlayer->mapx = PlayerLocation[i].mapx;
	lpPlayer->mapy = PlayerLocation[i].mapy;
	lpPlayer->level = PlayerLocation[i].level;
	lpPlayer->copx = PlayerLocation[i].copx;
	lpPlayer->copy = PlayerLocation[i].copy;
	lpPlayer->coplvl = PlayerLocation[i].coplvl;
	lpPlayer->asleep = PlayerLocation[i].asleep;
	strcpy(lpPlayer->name, PlayerLocation[i].name);
	strcpy(lpPlayer->type, PlayerLocation[i].type);
	lpPlayer->maxmove = PlayerLocation[i].maxmove;
	strcpy(lpPlayer->lastmove, PlayerLocation[i].lastmove);
	lpPlayer->dead = PlayerLocation[i].dead;
	lpPlayer->fx = PlayerLocation[i].fx;
	lpPlayer->fy = PlayerLocation[i].fy;
	lpPlayer->fx2 = PlayerLocation[i].fx2;
	lpPlayer->fy2 = PlayerLocation[i].fy2;
	lpPlayer->frame = PlayerLocation[i].frame;
	lpPlayer->limitspeed = PlayerLocation[i].limitspeed;
	lpPlayer->dx = PlayerLocation[i].dx;
	lpPlayer->dy = PlayerLocation[i].dy;
	lpPlayer->dl = PlayerLocation[i].dl;
	lpPlayer->maxspeed = PlayerLocation[i].maxspeed;
	lpPlayer->moved = PlayerLocation[i].moved;
	lpPlayer->speed = PlayerLocation[i].speed;
	lpPlayer->calcmove = PlayerLocation[i].calcmove;
	lpPlayer->image = PlayerLocation[i].image;
	lpPlayer->character = PlayerLocation[i].character;
	lpPlayer->stairs = PlayerLocation[i].stairs;
	lpPlayer->ability = PlayerLocation[i].ability;
	lpPlayer->damage = PlayerLocation[i].damage;
	lpPlayer->hit = PlayerLocation[i].hit;
	lpPlayer->tolx = PlayerLocation[i].tolx;
	lpPlayer->toly = PlayerLocation[i].toly;
	lpPlayer->rest = PlayerLocation[i].rest;
	lpPlayer->staircounter = PlayerLocation[i].staircounter;
	lpPlayer->shoot = PlayerLocation[i].shoot;
	lpPlayer->track = PlayerLocation[i].track;
	lpPlayer->framehuman = PlayerLocation[i].framehuman;
	lpPlayer->lasttarget = PlayerLocation[i].lasttarget;
	lpPlayer->numshots = PlayerLocation[i].numshots;
	lpPlayer->protecttreasure = PlayerLocation[i].protecttreasure;
	lpPlayer->team = PlayerLocation[i].team;
	lpPlayer->firerate = PlayerLocation[i].firerate;
	lpPlayer->firecount = PlayerLocation[i].firecount;
	lpPlayer->maxmissle = PlayerLocation[i].maxmissle;
	lpPlayer->intelligence = PlayerLocation[i].intelligence;
	lpPlayer->armour = PlayerLocation[i].armour;
	lpPlayer->ring = PlayerLocation[i].ring;
	lpPlayer->damagemax = PlayerLocation[i].damagemax;
	lpPlayer->damagemin = PlayerLocation[i].damagemin;
	lpPlayer->active = PlayerLocation[i].active;
	nBytes = sizeof(PLAYERMSG);

	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (client == 1) {
			if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
			                        0, (LPSTR)Comm, nBytes) == DP_OK) {
			}
		} else {
			if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
			}
		}
	}
}

void send_player2(int i, int client, int slot) {
	DWORD nBytes;
	LPPLAYERDIR lpPlayer;

	BYTE Comm[512]; // Communication buffer for messages

	lpPlayer = (LPPLAYERDIR)Comm;

	if (client == 1)
		lpPlayer->PlayerNum = netplayernum;
	else
		lpPlayer->PlayerNum = i;

	lpPlayer->MsgCode = MSG_PLAYERDIR;
	lpPlayer->justdied = PlayerLocation[i].justdied;
	lpPlayer->x = PlayerLocation[i].x;
	lpPlayer->y = PlayerLocation[i].y;
	lpPlayer->direction = PlayerLocation[i].direction;
	lpPlayer->mapx = PlayerLocation[i].mapx;
	lpPlayer->mapy = PlayerLocation[i].mapy;
	lpPlayer->level = PlayerLocation[i].level;
	lpPlayer->asleep = PlayerLocation[i].asleep;
	lpPlayer->dead = PlayerLocation[i].dead;
	strcpy(lpPlayer->lastmove, PlayerLocation[i].lastmove);
	lpPlayer->fx = PlayerLocation[i].fx;
	lpPlayer->fy = PlayerLocation[i].fy;
	lpPlayer->fx2 = PlayerLocation[i].fx2;
	lpPlayer->fy2 = PlayerLocation[i].fy2;
	lpPlayer->frame = PlayerLocation[i].frame;
	lpPlayer->dx = PlayerLocation[i].dx;
	lpPlayer->dy = PlayerLocation[i].dy;
	lpPlayer->dl = PlayerLocation[i].dl;
	lpPlayer->hit = PlayerLocation[i].hit;
	lpPlayer->tolx = PlayerLocation[i].tolx;
	lpPlayer->toly = PlayerLocation[i].toly;
	lpPlayer->rest = PlayerLocation[i].rest;
	lpPlayer->track = PlayerLocation[i].track;
	lpPlayer->lasttarget = PlayerLocation[i].lasttarget;
	lpPlayer->numshots = PlayerLocation[i].numshots;
	lpPlayer->firecount = PlayerLocation[i].firecount;
	lpPlayer->speed = PlayerLocation[i].speed;
	lpPlayer->rest = PlayerLocation[i].rest;
	lpPlayer->hit = PlayerLocation[i].hit;
	lpPlayer->firecount = PlayerLocation[i].firecount;
	lpPlayer->protection = PlayerLocation[i].protection;
	lpPlayer->dead = PlayerLocation[i].dead;
	lpPlayer->calcmove = PlayerLocation[i].calcmove;
	lpPlayer->moved = PlayerLocation[i].moved;

	nBytes = sizeof(PLAYERDIR);

	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (client == 1) {
			if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
			                        0, (LPSTR)Comm, nBytes) == DP_OK) {
			}
		} else {
			if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                        0, (LPSTR)Comm, nBytes) == DP_OK) {
			}
		}
	}
}

void send_playerquick(int i, int client, int slot) {
	DWORD nBytes;
	LPPLAYERQUICK lpPlayer;

	BYTE Comm[512]; // Communication buffer for messages

	lpPlayer = (LPPLAYERQUICK)Comm;

	if (client == 1)
		lpPlayer->PlayerNum = netplayernum;
	else
		lpPlayer->PlayerNum = i;

	lpPlayer->MsgCode = MSG_PLAYERQUICK;
	lpPlayer->x = PlayerLocation[i].x;
	lpPlayer->y = PlayerLocation[i].y;
	lpPlayer->direction = PlayerLocation[i].direction;
	lpPlayer->mapx = PlayerLocation[i].mapx;
	lpPlayer->mapy = PlayerLocation[i].mapy;
	lpPlayer->level = PlayerLocation[i].level;
	lpPlayer->fx = PlayerLocation[i].fx;
	lpPlayer->fy = PlayerLocation[i].fy;
	lpPlayer->fx2 = PlayerLocation[i].fx2;
	lpPlayer->fy2 = PlayerLocation[i].fy2;
	lpPlayer->frame = PlayerLocation[i].frame;
	lpPlayer->speed = PlayerLocation[i].speed;
	lpPlayer->rest = PlayerLocation[i].rest;
	lpPlayer->calcmove = PlayerLocation[i].calcmove;
	lpPlayer->moved = PlayerLocation[i].moved;
	nBytes = sizeof(PLAYERQUICK);
	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (client == 1) {
			if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
			                        0, (LPSTR)Comm, nBytes) == DP_OK) {
			}
		} else {
			if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                        0, (LPSTR)Comm, nBytes) == DP_OK) {
			}
		}
	}
}

void DPEventMsg2() {

	int i;
	int freeslot;

	DWORD count;
	DPID dpidFrom, dpidTo;
	DWORD nBytes;
	nBytes = MAX_BUFFER_SIZE;
	DWORD dwLen = 128;

	LPDUNGEON lpDungeon;
	LPPLAYER lpPlayer;
	LPPLAYERDIR lpPlayerDir;
	LPPLAYERQUICK lpPlayerQuick;
	LPMISSLE lpMissle;
	LPCHATMSG lpChat;
	LPMSGINT lpMsgInt;
	LPDEATH lpDeath;

	while (1) {
		m_pDirectPlay->GetMessageCount(m_dpidPlayer[0], &count);
		if (count == 0)
			return;

		if (m_pDirectPlay->Receive(&dpidFrom, &dpidTo,
		                           DPRECEIVE_ALL, CommBuff, &nBytes) == DP_OK) {
			if (dpidFrom == 0 && ishost) {
				//				debug_me("DPEventMsg2","Server Message came in",0,0);
				// Got a system message
				DPMSG_GENERIC *pmsgGeneric = (DPMSG_GENERIC *)CommBuff;
				CString sText;
				switch (pmsgGeneric->dwType) {
				case DPSYS_CONNECT:
					debug_me("DPEventMsg2", "Connected", 0, 0);
					break;
				case DPSYS_SESSIONLOST:
					debug_me("DPEventMsg2", "Lost Session", 0, 0);
					break;
				case DPSYS_ADDPLAYER:

					debug_me("DPEventMsg2", "Addplayer", 0, 0);
					if (networkserver)
						PlayerLocation[1].active = FALSE;
					freeslot = find_free_slot();

					send_version(freeslot);

					if (freeslot > newwarrior) {

						// FULL
						debug_me("DPEventMsg2", "Server is full", 0, 0);
						send_client_error(freeslot, 1);
						//					    if (m_pDirectPlay->DestroyPlayer(m_dpidPlayer[freeslot]) == DP_OK){
						//					}

					} else {
						debug_me("DPEventMsg2", "Find free slot", 0, 0);
						m_dpidPlayer[freeslot] = ((DPMSG_ADDPLAYER *)
						                              pmsgGeneric)
						                             ->dpId;

						send_playernum(freeslot);

						switch (PlayerLocation[freeslot].image) {
						case 1:
							strcpy(networkresponse, "Murk: Barbarian joins the game!");

							break;
						case 3:
							strcpy(networkresponse, "Murk: Thief joins the game!");

							break;
						case 4:
							strcpy(networkresponse, "Murk: Mage joins the game!");

							break;
						case 5:
							strcpy(networkresponse, "Murk: Cleric joins the game!");

							break;
						}

						send_chat(0);

						debug_me("DPEventMsg2", "Assigned to", freeslot, 0);
						send_numplayers(freeslot);
						send_dungeon_size(freeslot);

						send_dungeon_info(freeslot);

						for (i = 1; i <= NumPlayers; i++) {
							send_player(i, 0, freeslot);
							if (PlayerLocation[i].active) {
							}
						}
						send_player(freeslot, 0, 0);

						send_treasure(freeslot, 0);
						dungeon[PlayerLocation[freeslot].copx][PlayerLocation[freeslot].copy][PlayerLocation[freeslot].coplvl].item = 'c';
						send_dungeon_square(PlayerLocation[freeslot].copx, PlayerLocation[freeslot].copy, PlayerLocation[freeslot].coplvl, 1);

						for (i = 1; i <= 48; i++) {

							if (pits[i].frame != 0) {
								send_pit(freeslot, 0, pits[i].x, pits[i].y, pits[i].lvl, pits[i].frame);
							}
						}

						send_treasure2(freeslot, 0);

						if (networkserver) {
							InvalidateRect(hWindow, NULL, TRUE);
							UpdateWindow(hWindow);
						}
					}
					break;
				case DPSYS_DELETEPLAYER:

					debug_me("DPEventMsg2", "Got delet Player Message (not used)", 0, 0);
					break;
				}
			} else {
				if (dpidTo == m_dpidPlayer[0]) {
					if (ishost) {
						lpDungeon = (LPDUNGEON)CommBuff;
						switch (CommBuff[0]) {
						case MSG_CHAT:
							lpChat = (LPCHATMSG)CommBuff;
							strcpy(networkresponse, lpChat->say);
							send_chat(0);

							if (ishost && networkgame && !networkserver) {
								strcpy(chat[0].say, networkresponse);
								chat[0].duration = 50;
							}

							break;
						case MSG_DROPTREASURE:
							lpMsgInt = (LPMSGINT)CommBuff;
							if (lpMsgInt->miscint1 == 1) {
								PlayerLocation[lpMsgInt->miscint2].hit = 1;
								foundtreasure = 0;
								send_treasure(0, 0);
								dtreasure[0].towner = lpMsgInt->miscint2;
								dtreasure[0].tdx = PlayerLocation[lpMsgInt->miscint2].mapx;
								dtreasure[0].tdy = PlayerLocation[lpMsgInt->miscint2].mapy;
								dtreasure[0].tdlvl = PlayerLocation[lpMsgInt->miscint2].level;
								//								debug_me("DPEventMsg2","Drop Treasure 1 Owner:",lpMsgInt->miscint2,0);
							} else {
								PlayerLocation[lpMsgInt->miscint2].hit = 1;
								foundtreasure2 = 0;
								send_treasure2(0, 0);
								dtreasure[1].towner = lpMsgInt->miscint2;
								dtreasure[1].tdx = PlayerLocation[lpMsgInt->miscint2].mapx;
								dtreasure[1].tdy = PlayerLocation[lpMsgInt->miscint2].mapy;
								dtreasure[1].tdlvl = PlayerLocation[lpMsgInt->miscint2].level;

								//								debug_me("DPEventMsg2","Drop Treasure 2 Owner:",lpMsgInt->miscint2,0);
							}

							break;

						case MSG_HOSTPING:
							lpMsgInt = (LPMSGINT)CommBuff;
							pingplayer[lpMsgInt->miscint1] = 0;
							break;

						case MSG_SENDDUNGEON:
							break;
						case MSG_DELETEPLAYER:

							i = lpDungeon->x;
							debug_me("DPEventMsg2", "Delete Player:", i, 0);
							dungeon[PlayerLocation[i].copx][PlayerLocation[i].copy][PlayerLocation[i].coplvl].item = 'm';
							send_dungeon_square(PlayerLocation[i].copx, PlayerLocation[i].copy, PlayerLocation[i].coplvl, 1);
							if (m_pDirectPlay->DestroyPlayer(m_dpidPlayer[lpDungeon->x]) == DP_OK) {
								debug_me("DPEventMsg2", "Delete Player Success:", i, 0);
							}

							PlayerLocation[lpDungeon->x].active = FALSE;

							switch (PlayerLocation[lpDungeon->x].image) {
							case 1:
								strcpy(networkresponse, "Murk: Barbarian has left the game...");

								break;
							case 3:
								strcpy(networkresponse, "Murk: Thief has left the game...");

								break;
							case 4:
								strcpy(networkresponse, "Murk: Mage has left the game...");

								break;
							case 5:
								strcpy(networkresponse, "Murk: Cleric has left the game...");

								break;
							}

							send_chat(0);

							clear_player(lpDungeon->x);
							if (networkserver) {
								InvalidateRect(hWindow, NULL, TRUE);
								UpdateWindow(hWindow);
							}

							break;
						case MSG_SENDMAN:
							break;
						case MSG_QUIT:
							PostMessage(hWindow, WM_CLOSE, 0, 0L);
							break;
						case MSG_MISSLE:

							lpMissle = (LPMISSLE)CommBuff;
							i = lpMissle->owner;
							start_missle(lpMissle->misslex, lpMissle->missley, lpMissle->direction,
							             lpMissle->misslelvl,
							             lpMissle->missledx, lpMissle->missledy, i);
							break;
						case MSG_PLAYERQUICK:

							if (receiveok) {
								lpPlayerQuick = (LPPLAYERQUICK)CommBuff;
								i = lpPlayerQuick->PlayerNum;
								PlayerLocation[i].x = lpPlayerQuick->x;
								PlayerLocation[i].y = lpPlayerQuick->y;
								PlayerLocation[i].direction = lpPlayerQuick->direction;
								PlayerLocation[i].mapx = lpPlayerQuick->mapx;
								PlayerLocation[i].mapy = lpPlayerQuick->mapy;
								PlayerLocation[i].level = lpPlayerQuick->level;
								PlayerLocation[i].fx = lpPlayerQuick->fx;
								PlayerLocation[i].fy = lpPlayerQuick->fy;
								PlayerLocation[i].fx2 = lpPlayerQuick->fx2;
								PlayerLocation[i].fy2 = lpPlayerQuick->fy2;
								PlayerLocation[i].frame = lpPlayerQuick->frame;
								PlayerLocation[i].speed = lpPlayerQuick->speed;
								PlayerLocation[i].rest = lpPlayerQuick->rest;
								PlayerLocation[i].calcmove = lpPlayerQuick->calcmove;
								PlayerLocation[i].moved = lpPlayerQuick->moved;
							}
							break;

						case MSG_PLAYERDIR:

							if (receiveok) {
								lpPlayerDir = (LPPLAYERDIR)CommBuff;
								i = lpPlayerDir->PlayerNum;
								PlayerLocation[i].justdied = lpPlayerDir->justdied;
								PlayerLocation[i].x = lpPlayerDir->x;
								PlayerLocation[i].y = lpPlayerDir->y;
								PlayerLocation[i].direction = lpPlayerDir->direction;
								PlayerLocation[i].mapx = lpPlayerDir->mapx;
								PlayerLocation[i].mapy = lpPlayerDir->mapy;
								PlayerLocation[i].level = lpPlayerDir->level;
								PlayerLocation[i].asleep = lpPlayerDir->asleep;
								strcpy(PlayerLocation[i].lastmove, lpPlayerDir->lastmove);
								PlayerLocation[i].dead = lpPlayerDir->dead;
								PlayerLocation[i].fx = lpPlayerDir->fx;
								PlayerLocation[i].fy = lpPlayerDir->fy;
								PlayerLocation[i].fx2 = lpPlayerDir->fx2;
								PlayerLocation[i].fy2 = lpPlayerDir->fy2;
								PlayerLocation[i].frame = lpPlayerDir->frame;
								PlayerLocation[i].dx = lpPlayerDir->dx;
								PlayerLocation[i].dy = lpPlayerDir->dy;
								PlayerLocation[i].dl = lpPlayerDir->dl;
								//							PlayerLocation[i].hit=lpPlayerDir->hit;
								PlayerLocation[i].tolx = lpPlayerDir->tolx;
								PlayerLocation[i].toly = lpPlayerDir->toly;
								PlayerLocation[i].rest = lpPlayerDir->rest;
								PlayerLocation[i].track = lpPlayerDir->track;
								PlayerLocation[i].lasttarget = lpPlayerDir->lasttarget;
								PlayerLocation[i].numshots = lpPlayerDir->numshots;
								PlayerLocation[i].firecount = lpPlayerDir->firecount;
								PlayerLocation[i].protection = lpPlayerDir->protection;
								PlayerLocation[i].speed = lpPlayerDir->speed;
								PlayerLocation[i].calcmove = lpPlayerDir->calcmove;
								PlayerLocation[i].moved = lpPlayerDir->moved;
							}
							break;
						case MSG_PLAYER:

							if (receiveok) {
								lpPlayer = (LPPLAYER)CommBuff;
								i = lpPlayer->PlayerNum;
								PlayerLocation[i].protection = lpPlayer->protection;
								PlayerLocation[i].skill = lpPlayer->skill;
								PlayerLocation[i].stairsx = lpPlayer->stairsx;
								PlayerLocation[i].stairsy = lpPlayer->stairsy;
								PlayerLocation[i].justdied = lpPlayer->justdied;
								PlayerLocation[i].x = lpPlayer->x;
								PlayerLocation[i].y = lpPlayer->y;
								PlayerLocation[i].direction = lpPlayer->direction;
								PlayerLocation[i].mapx = lpPlayer->mapx;
								PlayerLocation[i].mapy = lpPlayer->mapy;
								PlayerLocation[i].level = lpPlayer->level;
								PlayerLocation[i].copx = lpPlayer->copx;
								PlayerLocation[i].copy = lpPlayer->copy;
								PlayerLocation[i].coplvl = lpPlayer->coplvl;
								PlayerLocation[i].asleep = lpPlayer->asleep;
								strcpy(PlayerLocation[i].name, lpPlayer->name);
								strcpy(PlayerLocation[i].type, lpPlayer->type);
								PlayerLocation[i].maxmove = lpPlayer->maxmove;
								strcpy(PlayerLocation[i].lastmove, lpPlayer->lastmove);
								PlayerLocation[i].dead = lpPlayer->dead;
								PlayerLocation[i].fx = lpPlayer->fx;
								PlayerLocation[i].fy = lpPlayer->fy;
								PlayerLocation[i].fx2 = lpPlayer->fx2;
								PlayerLocation[i].fy2 = lpPlayer->fy2;
								PlayerLocation[i].frame = lpPlayer->frame;
								PlayerLocation[i].limitspeed = lpPlayer->limitspeed;
								PlayerLocation[i].dx = lpPlayer->dx;
								PlayerLocation[i].dy = lpPlayer->dy;
								PlayerLocation[i].dl = lpPlayer->dl;
								PlayerLocation[i].maxspeed = lpPlayer->maxspeed;
								PlayerLocation[i].moved = lpPlayer->moved;
								PlayerLocation[i].speed = lpPlayer->speed;
								PlayerLocation[i].calcmove = lpPlayer->calcmove;
								PlayerLocation[i].image = lpPlayer->image;
								PlayerLocation[i].character = lpPlayer->character;
								PlayerLocation[i].stairs = lpPlayer->stairs;
								PlayerLocation[i].ability = lpPlayer->ability;
								PlayerLocation[i].damage = lpPlayer->damage;
								PlayerLocation[i].hit = lpPlayer->hit;
								PlayerLocation[i].tolx = lpPlayer->tolx;
								PlayerLocation[i].toly = lpPlayer->toly;
								PlayerLocation[i].rest = lpPlayer->rest;
								PlayerLocation[i].staircounter = lpPlayer->staircounter;
								PlayerLocation[i].shoot = lpPlayer->shoot;
								PlayerLocation[i].track = lpPlayer->track;
								PlayerLocation[i].framehuman = lpPlayer->framehuman;
								PlayerLocation[i].lasttarget = lpPlayer->lasttarget;
								PlayerLocation[i].numshots = lpPlayer->numshots;
								PlayerLocation[i].protecttreasure = lpPlayer->protecttreasure;
								PlayerLocation[i].team = lpPlayer->team;
								PlayerLocation[i].firerate = lpPlayer->firerate;
								PlayerLocation[i].firecount = lpPlayer->firecount;
								PlayerLocation[i].maxmissle = lpPlayer->maxmissle;
								PlayerLocation[i].intelligence = lpPlayer->intelligence;
								PlayerLocation[i].armour = lpPlayer->armour;
								PlayerLocation[i].ring = lpPlayer->ring;
								PlayerLocation[i].damagemax = lpPlayer->damagemax;
								PlayerLocation[i].damagemin = lpPlayer->damagemin;
								PlayerLocation[i].active = lpPlayer->active;
							}
							break;
						}
					} else {

						switch (CommBuff[0]) {

						case MSG_REWARD:
							lpMsgInt = (LPMSGINT)CommBuff;

							PlayerLocation[1].skill = PlayerLocation[1].skill + lpMsgInt->miscint1;
							break;

						case MSG_HOSTPING:
							lpMsgInt = (LPMSGINT)CommBuff;
							pingplayer[lpMsgInt->miscint1] = 0;
							break;
						case MSG_DEATH:

							if (receiveok) {
								lpDeath = (LPDEATH)CommBuff;
								i = lpDeath->player;
								if (i == 1) {
									i = netplayernum;
								}

								//							debug_me("receive_death x-----------","x=",lpDeath->x,lpDeath->y);
								dscene[i].x = lpDeath->x;
								dscene[i].y = lpDeath->y;
								dscene[i].lvl = lpDeath->lvl;
								dscene[i].image = lpDeath->image;
								dscene[i].direction = lpDeath->direction;
								dscene[i].counter = lpDeath->counter;
								dscene[i].mapx = lpDeath->mapx;
								dscene[i].mapy = lpDeath->mapy;
								PlayerLocation[i].dead = lpDeath->dead;
							}

							break;

						case MSG_STARTTIMER:

							initplayer = 2;
							// fix this killtimer
							if (stopthetime == 0) {
								KillTimer(hWindow, 1);
								stopthetime = 1;
							}

							if (m_directSoundOK)
								play_random_music();

							debug_me("DPEventMsg2", "Start Timer", 0, 0);
							SetTimer(hWindow, 3, timerlength, TimerProc);
							showmission = 3;
							break;
						case MSG_PLAYSOUND:
							lpMsgInt = (LPMSGINT)CommBuff;

							switch (lpMsgInt->miscint1) {
							case 1:

								if (m_directSoundOK)
									m_pDirSound->PlaySound(m_bufferTreasure);
								// treasure
								break;
							case 2:
								if (m_directSoundOK)
									m_pDirSound->PlaySound(m_bufferMonsterdie);
								break;
							case 3:
								if (m_directSoundOK)
									m_pDirSound->PlaySound(m_bufferWin);

								break;
							}

							break;

						case MSG_CHAT:
							lpChat = (LPCHATMSG)CommBuff;
							strcpy(chat[0].say, lpChat->say);

							chat[0].duration = 50;
							break;
						case MSG_STOPTIMER:
							debug_me("DPEventMsg2", "Stop Timer", 0, 0);

							KillTimer(hWindow, 3);
							showmission = 4;
							strcpy(networkresponse, "~Welcome to MultiPlayer Murk~~  Receiving Dungeon");

							if (m_directSoundOK)
								stop_song();

							hitanykey = 0;
							SetTimer(hWindow, 1, 50, TimerProc);
							initplayer = 0;
							startgame = 1;
							new_level(1);

							break;
						case MSG_TREASURE:
							lpDungeon = (LPDUNGEON)CommBuff;

							if (lpDungeon->image) {

								//								debug_me("DPEventMsg2","Got Treasure 1",lpDungeon->x,lpDungeon->y);
								treasurex2 = lpDungeon->x;
								treasurey2 = lpDungeon->y;
								treasurelvl2 = lpDungeon->lvl;
								if (initplayer >= 2) {
									if (lpDungeon->order == netplayernum) {
										foundtreasure2 = 1;
									} else
										foundtreasure2 = lpDungeon->order;
								}

							} else {

								//								debug_me("DPEventMsg2","Got Treasure 2",lpDungeon->x,lpDungeon->y);
								treasurex = lpDungeon->x;
								treasurey = lpDungeon->y;
								treasurelvl = lpDungeon->lvl;
								if (initplayer >= 2) {
									if (lpDungeon->order == netplayernum) {

										foundtreasure = 1;
									} else
										foundtreasure = lpDungeon->order;
								}
							}

							break;
						case MSG_NUMPLAYERS:

							lpDungeon = (LPDUNGEON)CommBuff;
							debug_me("DPEventMsg2", "Got NumPlayers", lpDungeon->x, 0);
							NumPlayers = lpDungeon->x;
							break;
						case MSG_ERROR:
							lpDungeon = (LPDUNGEON)CommBuff;
							if (lpDungeon->order == 1)
								debug_me("DPEventMsg2", "Server Full", 0, 0);
							break;
						case MSG_VERSION:
							lpDungeon = (LPDUNGEON)CommBuff;
							if (lpDungeon->order != 112) {
								debug_me("DPEventMsg2", "Wrong Version", 0, 0);
								abortgame = 1;
							}
							break;

						case MSG_PIT:

							lpDungeon = (LPDUNGEON)CommBuff;

							for (i = 1; i <= 48; i++) {

								if (pits[i].frame == 0)
									break;
							}
							pits[i].x = lpDungeon->x;
							pits[i].y = lpDungeon->y;
							pits[i].lvl = lpDungeon->lvl;
							pits[i].frame = lpDungeon->order;
							break;

						case MSG_DUNGEON:
							lpDungeon = (LPDUNGEON)CommBuff;
							countsquares++;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].type = lpDungeon->type;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].explored = lpDungeon->explored;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].item = lpDungeon->item;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].image = lpDungeon->image;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].mark = lpDungeon->mark;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].order = lpDungeon->order;

							if (countsquares > 100) {
								strcat(networkresponse, ".");
								countsquares = 0;
							}
							break;
						case MSG_DUNGEONSQUARE:
							lpDungeon = (LPDUNGEON)CommBuff;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].type = lpDungeon->type;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].explored = lpDungeon->explored;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].item = lpDungeon->item;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].image = lpDungeon->image;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].mark = lpDungeon->mark;
							dungeon[lpDungeon->x][lpDungeon->y][lpDungeon->lvl].order = lpDungeon->order;
							//							debug_me("DPEventMsg2","Got Dungeon Item (xy)",lpDungeon->x,lpDungeon->y);
							break;
						case MSG_DUNGEONSIZE:

							lpDungeon = (LPDUNGEON)CommBuff;
							debug_me("DPEventMsg2", "Got Dungeon Size (xy)", lpDungeon->x, lpDungeon->y);
							gametype = lpDungeon->order;

							DUNGEONX = lpDungeon->x;
							DUNGEONY = lpDungeon->y;
							maxlevel = lpDungeon->lvl;
							break;
						case MSG_PLAYERNUM:
							lpDungeon = (LPDUNGEON)CommBuff;
							debug_me("DPEventMsg2", "Got Dungeon PlayerNum", lpDungeon->x, 0);
							netplayernum = lpDungeon->x;

							break;
						case MSG_MISSLE:
							lpMissle = (LPMISSLE)CommBuff;
							if (lpMissle->owner == netplayernum || initplayer < 2) {

							} else {

								if (lpMissle->owner == 1)
									i = netplayernum;
								else
									i = lpMissle->owner;
								start_missle(lpMissle->misslex, lpMissle->missley, lpMissle->direction,
								             lpMissle->misslelvl,
								             lpMissle->missledx, lpMissle->missledy, i);
							}
							break;
						case MSG_PLAYERDIR:
							lpPlayerDir = (LPPLAYERDIR)CommBuff;
							i = lpPlayerDir->PlayerNum;
							if (i == netplayernum || initplayer < 2) {
							} else {
								if (i == 1)
									i = netplayernum;

								PlayerLocation[i].justdied = lpPlayerDir->justdied;
								PlayerLocation[i].x = lpPlayerDir->x;
								PlayerLocation[i].y = lpPlayerDir->y;
								PlayerLocation[i].direction = lpPlayerDir->direction;
								PlayerLocation[i].mapx = lpPlayerDir->mapx;
								PlayerLocation[i].mapy = lpPlayerDir->mapy;
								PlayerLocation[i].level = lpPlayerDir->level;
								PlayerLocation[i].asleep = lpPlayerDir->asleep;
								strcpy(PlayerLocation[i].lastmove, lpPlayerDir->lastmove);
								PlayerLocation[i].dead = lpPlayerDir->dead;
								PlayerLocation[i].fx = lpPlayerDir->fx;
								PlayerLocation[i].fy = lpPlayerDir->fy;
								PlayerLocation[i].fx2 = lpPlayerDir->fx2;
								PlayerLocation[i].fy2 = lpPlayerDir->fy2;
								PlayerLocation[i].frame = lpPlayerDir->frame;
								PlayerLocation[i].dx = lpPlayerDir->dx;
								PlayerLocation[i].dy = lpPlayerDir->dy;
								PlayerLocation[i].dl = lpPlayerDir->dl;
								PlayerLocation[i].hit = lpPlayerDir->hit;
								PlayerLocation[i].tolx = lpPlayerDir->tolx;
								PlayerLocation[i].toly = lpPlayerDir->toly;
								PlayerLocation[i].rest = lpPlayerDir->rest;
								PlayerLocation[i].track = lpPlayerDir->track;
								PlayerLocation[i].lasttarget = lpPlayerDir->lasttarget;
								PlayerLocation[i].numshots = lpPlayerDir->numshots;
								PlayerLocation[i].firecount = lpPlayerDir->firecount;
								PlayerLocation[i].protection = lpPlayerDir->protection;
								PlayerLocation[i].speed = lpPlayerDir->speed;
								PlayerLocation[i].calcmove = lpPlayerDir->calcmove;
								PlayerLocation[i].moved = lpPlayerDir->moved;
							}
							break;
						case MSG_PLAYERQUICK:
							lpPlayerQuick = (LPPLAYERQUICK)CommBuff;
							i = lpPlayerQuick->PlayerNum;
							if (i == netplayernum || initplayer < 2) {
							} else {

								if (i == 1)
									i = netplayernum;
								PlayerLocation[i].x = lpPlayerQuick->x;
								PlayerLocation[i].y = lpPlayerQuick->y;
								PlayerLocation[i].direction = lpPlayerQuick->direction;
								PlayerLocation[i].mapx = lpPlayerQuick->mapx;
								PlayerLocation[i].mapy = lpPlayerQuick->mapy;
								PlayerLocation[i].level = lpPlayerQuick->level;
								PlayerLocation[i].fx = lpPlayerQuick->fx;
								PlayerLocation[i].fy = lpPlayerQuick->fy;
								PlayerLocation[i].fx2 = lpPlayerQuick->fx2;
								PlayerLocation[i].fy2 = lpPlayerQuick->fy2;
								PlayerLocation[i].frame = lpPlayerQuick->frame;
								PlayerLocation[i].speed = lpPlayerQuick->speed;
								PlayerLocation[i].rest = lpPlayerQuick->rest;
								PlayerLocation[i].calcmove = lpPlayerQuick->calcmove;
								PlayerLocation[i].moved = lpPlayerQuick->moved;
							}
							break;
						case MSG_PLAYER:
							lpPlayer = (LPPLAYER)CommBuff;
							i = lpPlayer->PlayerNum;
							if (i == netplayernum && initplayer > 0) {

							} else {

								if (i == 1) {
									i = netplayernum;

								} else if (initplayer == 0 && i == netplayernum) {
									initplayer = 1;
									i = 1;
								}
								PlayerLocation[i].protection = lpPlayer->protection;

								PlayerLocation[i].stairsx = lpPlayer->stairsx;
								PlayerLocation[i].stairsy = lpPlayer->stairsy;
								PlayerLocation[i].justdied = lpPlayer->justdied;
								PlayerLocation[i].x = lpPlayer->x;
								PlayerLocation[i].y = lpPlayer->y;
								PlayerLocation[i].direction = lpPlayer->direction;
								PlayerLocation[i].mapx = lpPlayer->mapx;
								PlayerLocation[i].mapy = lpPlayer->mapy;
								PlayerLocation[i].level = lpPlayer->level;
								PlayerLocation[i].copx = lpPlayer->copx;
								PlayerLocation[i].copy = lpPlayer->copy;
								PlayerLocation[i].coplvl = lpPlayer->coplvl;
								PlayerLocation[i].asleep = lpPlayer->asleep;
								strcpy(PlayerLocation[i].name, lpPlayer->name);
								strcpy(PlayerLocation[i].type, lpPlayer->type);
								PlayerLocation[i].maxmove = lpPlayer->maxmove;
								strcpy(PlayerLocation[i].lastmove, lpPlayer->lastmove);
								PlayerLocation[i].dead = lpPlayer->dead;
								PlayerLocation[i].fx = lpPlayer->fx;
								PlayerLocation[i].fy = lpPlayer->fy;
								PlayerLocation[i].fx2 = lpPlayer->fx2;
								PlayerLocation[i].fy2 = lpPlayer->fy2;
								PlayerLocation[i].frame = lpPlayer->frame;
								PlayerLocation[i].limitspeed = lpPlayer->limitspeed;
								PlayerLocation[i].dx = lpPlayer->dx;
								PlayerLocation[i].dy = lpPlayer->dy;
								PlayerLocation[i].dl = lpPlayer->dl;
								PlayerLocation[i].maxspeed = lpPlayer->maxspeed;
								PlayerLocation[i].moved = lpPlayer->moved;
								PlayerLocation[i].speed = lpPlayer->speed;
								PlayerLocation[i].calcmove = lpPlayer->calcmove;
								PlayerLocation[i].image = lpPlayer->image;
								PlayerLocation[i].character = lpPlayer->character;
								PlayerLocation[i].stairs = lpPlayer->stairs;

								PlayerLocation[i].damage = lpPlayer->damage;
								PlayerLocation[i].hit = lpPlayer->hit;
								PlayerLocation[i].tolx = lpPlayer->tolx;
								PlayerLocation[i].toly = lpPlayer->toly;
								PlayerLocation[i].rest = lpPlayer->rest;
								PlayerLocation[i].staircounter = lpPlayer->staircounter;
								PlayerLocation[i].shoot = lpPlayer->shoot;
								PlayerLocation[i].track = lpPlayer->track;
								PlayerLocation[i].framehuman = lpPlayer->framehuman;
								PlayerLocation[i].lasttarget = lpPlayer->lasttarget;
								PlayerLocation[i].numshots = lpPlayer->numshots;
								PlayerLocation[i].protecttreasure = lpPlayer->protecttreasure;
								PlayerLocation[i].team = lpPlayer->team;

								PlayerLocation[i].intelligence = lpPlayer->intelligence;

								if (i != 1) {
									PlayerLocation[i].ability = lpPlayer->ability;
									PlayerLocation[i].maxmissle = lpPlayer->maxmissle;
									PlayerLocation[i].armour = lpPlayer->armour;
									PlayerLocation[i].ring = lpPlayer->ring;
									PlayerLocation[i].damagemax = lpPlayer->damagemax;
									PlayerLocation[i].damagemin = lpPlayer->damagemin;
									PlayerLocation[i].skill = lpPlayer->skill;
									PlayerLocation[i].firerate = lpPlayer->firerate;
									PlayerLocation[i].firecount = lpPlayer->firecount;
								}
								PlayerLocation[i].active = lpPlayer->active;
							}
							break;
						}
					}
				}
			}
		}
	}
}

void send_settimer(int slot) {
	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_STARTTIMER;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = 0;
	lpDungeon->x = 0;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);
	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_stoptimer(int slot) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_STOPTIMER;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = 0;
	lpDungeon->x = 0;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {

		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_treasure(int slot, int forsure) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_TREASURE;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = foundtreasure;
	lpDungeon->x = treasurex;
	lpDungeon->y = treasurey;
	lpDungeon->lvl = treasurelvl;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {
		m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                    DPSEND_GUARANTEE, (LPSTR)Comm, nBytes);

	} else {
		if (forsure) {
			m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                    DPSEND_GUARANTEE, (LPSTR)Comm, nBytes);

		} else {
			m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                    0, (LPSTR)Comm, nBytes);
		}
	}
}

void send_treasure2(int slot, int forsure) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_TREASURE;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 1;
	lpDungeon->mark = 0;
	lpDungeon->order = foundtreasure2;
	lpDungeon->x = treasurex2;
	lpDungeon->y = treasurey2;
	lpDungeon->lvl = treasurelvl2;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {
		m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                    DPSEND_GUARANTEE, (LPSTR)Comm, nBytes);
		send_settimer(slot);
		if (!istimerset) {
			showmission = 3;

			SetTimer(hWindow, 3, timerlength, TimerProc);
			receiveok = 1;
			istimerset = 1;
		}
	} else {
		if (forsure) {
			m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                    DPSEND_GUARANTEE, (LPSTR)Comm, nBytes);

		} else {
			m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                    0, (LPSTR)Comm, nBytes);
		}
	}
}

void send_numplayers(int slot) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;
	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_NUMPLAYERS;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = 0;
	lpDungeon->x = NumPlayers;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);
	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_playernum(int slot) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_PLAYERNUM;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = 0;
	lpDungeon->x = slot;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
	                        0, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}

void send_missle(int client, int misslex, int missley, int misslelvl, int frame, int image, int direction, int owner, int missledx, int missledy, int slot) {

	DWORD nBytes;
	LPMISSLE lpMissle;

	BYTE Comm[512]; // Communication buffer for messages

	lpMissle = (LPMISSLE)Comm;
	lpMissle->MsgCode = MSG_MISSLE;
	if (client == 1)
		lpMissle->owner = netplayernum;
	else
		lpMissle->owner = owner;

	lpMissle->slot = slot;
	lpMissle->misslex = misslex;
	lpMissle->missley = missley;
	lpMissle->misslelvl = misslelvl;
	lpMissle->frame = frame;
	lpMissle->image = image;
	lpMissle->direction = direction;
	lpMissle->missledx = missledx;
	lpMissle->missledy = missledy;
	nBytes = sizeof(MISSLEMSG);

	if (client == 1) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
		                        0, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        0, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_death(int player, int x, int y, int lvl, int image, int direction, int counter, int mapx, int mapy, int dead) {

	DWORD nBytes;
	LPDEATH lpDeath;

	BYTE Comm[512]; // Communication buffer for messages

	lpDeath = (LPDEATH)Comm;
	lpDeath->MsgCode = MSG_DEATH;

	lpDeath->player = player;
	lpDeath->x = x;
	//	debug_me("send_death x-----------","x=",x,0);
	lpDeath->y = y;
	lpDeath->lvl = lvl;
	lpDeath->image = image;
	lpDeath->direction = direction;
	lpDeath->counter = counter;
	lpDeath->mapx = mapx;
	lpDeath->mapy = mapy;
	lpDeath->dead = dead;

	nBytes = sizeof(DSCENE);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
	                        0, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}

int find_free_slot() {

	int i;
	int start = 1;

	if (networkserver)
		start = 2;
	for (i = start; i <= newwarrior; i++) {
		if (PlayerLocation[i].active == FALSE) {
			PlayerLocation[i].active = TRUE;
			PlayerLocation[i].dead = 65;

			break;
		}
	}

	return i;
}

void send_deleteme() {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_DELETEPLAYER;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = 0;
	lpDungeon->x = netplayernum;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
	                        0, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}

void clear_player(int id) {

	int randnum;

	PlayerLocation[id].mapx = PlayerLocation[id].copx;
	PlayerLocation[id].mapy = PlayerLocation[id].copy;
	PlayerLocation[id].level = PlayerLocation[id].coplvl;
	PlayerLocation[id].rest = 0;
	PlayerLocation[id].firerate = 2;
	PlayerLocation[id].damagemin = 1;
	PlayerLocation[id].damagemax = 1;
	PlayerLocation[id].maxmissle = 2;
	PlayerLocation[id].framehuman = 0;
	PlayerLocation[id].protection = 0;
	PlayerLocation[id].lasttarget = '-';
	PlayerLocation[id].track = 0;
	PlayerLocation[id].direction = 3;
	PlayerLocation[id].stairsx = 0;
	PlayerLocation[id].stairsy = 0;
	PlayerLocation[id].firecount = 0;
	PlayerLocation[id].numshots = 0;
	PlayerLocation[id].frame = 0;
	PlayerLocation[id].fx = 0;
	PlayerLocation[id].fy = 0;
	PlayerLocation[id].fx2 = 0;
	PlayerLocation[id].fy2 = 0;
	PlayerLocation[id].tolx = 3;
	PlayerLocation[id].toly = 4;
	PlayerLocation[id].dx = 0;
	PlayerLocation[id].protecttreasure = 0;
	PlayerLocation[id].dy = 0;
	PlayerLocation[id].dl = 0;
	PlayerLocation[id].moved = 0;
	PlayerLocation[id].stairs = 0;
	PlayerLocation[id].staircounter = 0;
	PlayerLocation[id].asleep = 0;
	//	PlayerLocation[id].ability = 0;
	randnum = random_num(20) + 1;
	PlayerLocation[id].ability = 20 + randnum + countlevels;
	PlayerLocation[id].skill = 0;
	PlayerLocation[id].ring = 0;
	PlayerLocation[id].armour = 0;

	if (id == 1) {
		advancelevel = 20;
		advanceattrib = 0;
	}
}

void showdialogs() {
	int result;
	int lvlnum, x, y;

	if (!networkserver)
		pDirDraw->FlipToGDISurface();

	//	if (m_pGame)
	//		delete m_pGame;

	kill_dplay();

	m_pGame = new CGame;

	m_pGame = NULL;

	doit = FALSE;

	DialogBox(NULL, (LPCTSTR)IDD_CHOOSEPROVIDER, hWindow,
	          (DLGPROC)DlgProcChooseProvider);

	if (!doit) {
		result = m_pGame->DPInit();
		if (result == FALSE) {
			debug_me("ShowDialogs", "DPInit Failed", 0, 0);
			strcat(networkresponse, "Direct Play Initialization failed...~");
			abortgame = 2;
			doit = TRUE;
			return;
		} else
			strcat(networkresponse, "Direct Play Initialized~");

		doit = FALSE;
		DialogBox(NULL, (LPCTSTR)IDD_Q_CREATE, hWindow,
		          (DLGPROC)DlgProcQCreate);

		ishost = 0;
		networkgame = 1;

		if (!doit) {
			if (dialogresponse == 1) {
				result = m_pGame->DPCreateSession();
				ishost = 1;
				if (result == FALSE) {
					strcat(networkresponse, "Create Session failed...");
					debug_me("ShowDialogs", "DPCreateSesion Failed", 0, 0);
					abortgame = 3;
					doit = TRUE;
					return;
				} else {
					strcat(networkresponse, "Create Session ok..~");
					doit = FALSE;
				}
			} else {
				ishost = 0;
				doit = FALSE;
				DialogBox(NULL, (LPCTSTR)IDD_SELSESSION, hWindow,
				          (DLGPROC)DlgProcSelSession);

				if (!doit) {
					result = m_pGame->DPConnectSession();

					if (result == FALSE) {
						if (abortgame != 5)
							abortgame = 4;
						debug_me("ShowDialogs", "DPConnectSession Failed", 0, 0);
						doit = TRUE;
					} else {
						strcat(networkresponse, "Connected to server...~");
						doit = FALSE;
					}
				}
			}

			if (!doit)
				strcat(networkresponse, "Receiving Dungeon");

			if (dialogresponse != 1 && !doit) {
				for (lvlnum = 1; lvlnum <= maxlevel; lvlnum++) {
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
			}
		}
	}
}

void reset_networkserver() {

	int i;
	BOOL flag;

	flag = FALSE;

	for (i = 2; i <= 5; i++) {

		if (PlayerLocation[i].active)
			flag = TRUE;
	}

	if (flag) {

		return;
	}

	debug_me("reset_networkserver", "Resetting Server", 0, 0);

	KillTimer(hWindow, 3);
	initplayer = 0;
	startgame = 1;
	movelevel = 0;
	istimerset = 0;
	countlevels = 0;
	levelcounter = 1;
	new_level(0);
}

void send_new_level() {
	int i;

	receiveok = 0;
	send_numplayers(0);
	send_dungeon_size(0);
	send_dungeon_info(0);

	for (i = 1; i <= NumPlayers; i++) {
		send_player(i, 0, 0);
	}
	send_treasure(0, 1);
	send_treasure2(0, 1);
	debug_me("sned_new_level", "Send and start timers", 0, 0);

	//	SetTimer(hWindow, 2, 4000, TimerProc);
	SetTimer(hWindow, 3, timerlength, TimerProc);
	receiveok = 1;
	send_settimer(0);
}

void send_dungeon_square(int x, int y, int lvl, int forsure) {
	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_DUNGEONSQUARE;
	lpDungeon->type = dungeon[x][y][lvl].type;
	lpDungeon->explored = dungeon[x][y][lvl].explored;
	lpDungeon->item = dungeon[x][y][lvl].item;
	lpDungeon->image = dungeon[x][y][lvl].image;
	lpDungeon->mark = dungeon[x][y][lvl].mark;
	lpDungeon->order = dungeon[x][y][lvl].order;
	lpDungeon->x = x;
	lpDungeon->y = y;
	lpDungeon->lvl = lvl;
	nBytes = sizeof(DUNGEONMSG);

	if (forsure) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        0, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_chat(int client) {
	DWORD nBytes;
	LPCHATMSG lpChat;

	BYTE Comm[512]; // Communication buffer for messages

	lpChat = (LPCHATMSG)Comm;
	lpChat->MsgCode = MSG_CHAT;
	strcpy(lpChat->say, networkresponse);

	nBytes = sizeof(CHATMSG);

	if (client == 1) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
		                        0, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        0, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void make_thread2() {
	createthread();
}

void ThreadFunction(LPVOID lpParam) {
	HANDLE hEvents[2];

	hEvents[0] = quitthread;
	hEvents[1] = globalthread;
	debug_me("ThreadFunction", "Start thread...", 0, 0);

	while (WaitForMultipleObjects(2, hEvents, FALSE, INFINITE)) {

		DPEventMsg2();
		ResetEvent(globalthread);
	}
	debug_me("ThreadFunction", "Quit thread...", 0, 0);

	ExitThread(0);
}

void createthread() {

	//	globalthread = CreateEvent(NULL,TRUE,FALSE,NULL);

	quitthread = CreateEvent(NULL, TRUE, FALSE, NULL);

	ResetEvent(globalthread);
	ResetEvent(quitthread);

	mainthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunction,
	                          globalthread, 0, &IDThread);
}

void send_dungeon_size(int slot) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_DUNGEONSIZE;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = gametype;
	lpDungeon->x = DUNGEONX;
	lpDungeon->y = DUNGEONY;
	lpDungeon->lvl = maxlevel;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {

		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_drop_treasure(int treasure) {

	DWORD nBytes;
	LPMSGINT lpMsgInt;

	BYTE Comm[512]; // Communication buffer for messages

	lpMsgInt = (LPMSGINT)Comm;
	lpMsgInt->MsgCode = MSG_DROPTREASURE;

	lpMsgInt->miscint1 = treasure;
	lpMsgInt->miscint2 = netplayernum;
	nBytes = sizeof(MSGINT);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
	                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}

void send_pinghost() {

	DWORD nBytes;
	LPMSGINT lpMsgInt;

	BYTE Comm[512]; // Communication buffer for messages

	lpMsgInt = (LPMSGINT)Comm;
	lpMsgInt->MsgCode = MSG_HOSTPING;

	lpMsgInt->miscint1 = netplayernum;
	lpMsgInt->miscint2 = 0;
	nBytes = sizeof(MSGINT);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], 0,
	                        0, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}
void send_pingserver() {

	DWORD nBytes;
	LPMSGINT lpMsgInt;

	BYTE Comm[512]; // Communication buffer for messages

	lpMsgInt = (LPMSGINT)Comm;
	lpMsgInt->MsgCode = MSG_HOSTPING;

	lpMsgInt->miscint1 = 0;
	lpMsgInt->miscint2 = 0;

	nBytes = sizeof(MSGINT);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
	                        0, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}

void serversettings(int go) {

	int result;
	int lvlnum, x, y;
	m_pGame = new CGame;

	//	lpGuid = (LPGUID) Globalid;

	if (go) {
		debug_me("serversettings", "Read server settings", 0, 0);
		x = read_server_settings();

		if (x == 0) {

			networktype = 0;

			if (networkserver) {
				//				ValidateRect(hWindow,NULL);
				UpdateWindow(hWindow);
			}

			debug_me("serversettings", "Error reading server settings", 0, 0);
			return;

		} else {
			if (x == 1) {
				networktype = 1;
				//				strcpy(serverscreen,"MURK TCP Server is running...");
			}
			if (x == 2) {
				networktype = 2;
				//				strcpy(serverscreen,"MURK IPX Server is running...");
			}

			if (networkserver) {

				InvalidateRect(hWindow, NULL, TRUE);

				//				ValidateRect(hWindow,NULL);
				UpdateWindow(hWindow);
			}
			find_tcp_ipx();
		}
	}

	if (!go) {
		DialogBox(NULL, (LPCTSTR)IDD_CHOOSEPROVIDER, hWindow,
		          (DLGPROC)DlgProcChooseProvider);
	}

	result = m_pGame->DPInit();
	if (result == FALSE) {
		debug_me("serversettings", "DPINIT Failed", 0, 0);
	}

	dialogresponse = 1;

	ishost = 0;
	networkgame = 1;
	if (dialogresponse == 1) {
		result = m_pGame->DPCreateSession();
		ishost = 1;
		if (result == FALSE) {
			debug_me("serversettings", "DPCreateSession Failed", 0, 0);
		}
	}

	if (dialogresponse != 1) {
		for (lvlnum = 1; lvlnum <= maxlevel; lvlnum++) {
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
	}
}

void send_play_sound(int sound) {

	DWORD nBytes;
	LPMSGINT lpMsgInt;

	BYTE Comm[512]; // Communication buffer for messages

	lpMsgInt = (LPMSGINT)Comm;
	lpMsgInt->MsgCode = MSG_PLAYSOUND;

	lpMsgInt->miscint1 = sound;
	lpMsgInt->miscint2 = 0;
	nBytes = sizeof(MSGINT);

	if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
	                        0, (LPSTR)Comm, nBytes) == DP_OK) {
	}
}

void send_pit(int slot, int forsure, int x, int y, int lvl, int frame) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_PIT;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = frame;
	lpDungeon->x = x;
	lpDungeon->y = y;
	lpDungeon->lvl = lvl;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {
		m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                    DPSEND_GUARANTEE, (LPSTR)Comm, nBytes);

	} else {
		if (forsure) {
			m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                    DPSEND_GUARANTEE, (LPSTR)Comm, nBytes);

		} else {
			m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
			                    0, (LPSTR)Comm, nBytes);
		}
	}
}

void send_client_error(int slot, int problem) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_ERROR;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = problem;
	lpDungeon->x = 0;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {

		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_version(int slot) {

	DWORD nBytes;
	LPDUNGEON lpDungeon;

	BYTE Comm[512]; // Communication buffer for messages

	lpDungeon = (LPDUNGEON)Comm;
	lpDungeon->MsgCode = MSG_VERSION;
	lpDungeon->type = 0;
	lpDungeon->explored = 0;
	lpDungeon->item = 0;
	lpDungeon->image = 0;
	lpDungeon->mark = 0;
	lpDungeon->order = 112;
	lpDungeon->x = 0;
	lpDungeon->y = 0;
	lpDungeon->lvl = 0;
	nBytes = sizeof(DUNGEONMSG);

	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {

		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void send_reward(int slot, int reward) {

	DWORD nBytes;
	LPMSGINT lpMsgInt;

	BYTE Comm[512]; // Communication buffer for messages

	lpMsgInt = (LPMSGINT)Comm;
	lpMsgInt->MsgCode = MSG_REWARD;

	lpMsgInt->miscint1 = reward;
	lpMsgInt->miscint2 = 0;

	nBytes = sizeof(MSGINT);

	if (slot != 0) {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], m_dpidPlayer[slot],
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	} else {
		if (m_pDirectPlay->Send(m_dpidPlayer[0], DPID_ALLPLAYERS,
		                        DPSEND_GUARANTEE, (LPSTR)Comm, nBytes) == DP_OK) {
		}
	}
}

void kill_dplay() {

	debug_me("kill_dplay", "", 0, 0);

	if (networkgame && m_pDirectPlay != NULL)
		DPShutDown();

	if (globalthread != NULL) {
		debug_me("kill_dplay", "Kill Thread", 0, 0);
		// wake up receive thread and wait for it to quit
		SetEvent(quitthread);
		WaitForSingleObject(quitthread, INFINITE);
		CloseHandle(quitthread);
		quitthread = NULL;
		CloseHandle(globalthread);
		globalthread = NULL;
	}

	debug_me("kill_dplay", "Finished kill_dplay", 0, 0);
}

void DPShutDown() {

	if (m_pDirectPlay->DestroyPlayer(m_dpidPlayer[0]) == DP_OK) {
		debug_me("DPShutDown", "DestroyPlayer Success...", 0, 0);
	}
	m_dpidPlayer[1] = 0;

	// Close the DP object
	if (m_pDirectPlay)
		m_pDirectPlay->Close();
	if (m_pDirectPlay) {
		debug_me("DPShutDown", "m_pDirectPlay Release...", 0, 0);
		m_pDirectPlay->Release();
		m_pDirectPlay = NULL;
	}

	if (m_pGame) {
		delete m_pGame;
		debug_me("DPShutDown", "Delete m_pGame...", 0, 0);
	}
}
