/*==========================================================================
 *
 *  Based on ROIDS.H, which is 
 *  Copyright (C) 1995 Microsoft Corporation. All Rights Reserved.
 *
 *  File:       NetRoids.h
 *  Content:    Main include file
 *
 *  DirectPlay support added by Chris Howard, 
 *  Copyright (C) 1996 Chris Howard. All Rights Reserved.
 *
 ***************************************************************************/

#ifndef NETROIDS_INCLUDED
#define NETROIDS_INCLUDED

#define INITGUID
#define WIN32_LEAN_AND_MEAN




#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <dsound.h>
#include <dplay.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "resource.h"

/*

#include <windows.h>
#include "resource.h"
#include <io.h>

#include <dplay.h>
#include <ddraw.h>
#include <dsound.h>
#include <dwave.h>
*/

#define MAXPLAYERS      4
#define MAXOBJECTS      1000

#define MAX_BUFFER_SIZE 256

#define DEF_SHOW_DELAY  2000


// keyboard commands
#define KEY_STOP   0x00000001l
#define KEY_DOWN   0x00000002l
#define KEY_LEFT   0x00000004l
#define KEY_RIGHT  0x00000008l
#define KEY_UP     0x00000010l
#define KEY_FIRE   0x00000020l
#define KEY_THROW  0x00000040l
#define KEY_SHIELD 0x00000080l

// Objects
enum
{
    OBJ_DONUT = 0,
    OBJ_PYRAMID,
    OBJ_CUBE,
    OBJ_SPHERE,
    OBJ_SHIP,
    OBJ_BULLET,
    OBJ_EXPLODE
};


// Program states
enum
{
    PS_SPLASH,
    PS_ACTIVE,
    PS_BEGINREST,
    PS_REST
};

// Position maximums
#define     MAX_SCREEN_X      639
#define     MAX_SCREEN_Y      479
#define     MAX_DONUT_X       MAX_SCREEN_X - 64
#define     MAX_DONUT_Y       MAX_SCREEN_Y - 64
#define     MAX_DONUT_FRAME   30
#define     MAX_PYRAMID_X     MAX_SCREEN_X - 32
#define     MAX_PYRAMID_Y     MAX_SCREEN_Y - 32
#define     MAX_PYRAMID_FRAME 40
#define     MAX_SPHERE_X      MAX_SCREEN_X - 16
#define     MAX_SPHERE_Y      MAX_SCREEN_Y - 16
#define     MAX_SPHERE_FRAME  40
#define     MAX_CUBE_X        MAX_SCREEN_X - 16
#define     MAX_CUBE_Y        MAX_SCREEN_Y - 16
#define     MAX_CUBE_FRAME    40
#define     MAX_SHIP_X        MAX_SCREEN_X - 32
#define     MAX_SHIP_Y        MAX_SCREEN_Y - 32
#define     MAX_SHIP_FRAME    40
#define     MAX_BULLET_X      MAX_SCREEN_X - 3;
#define     MAX_BULLET_Y      MAX_SCREEN_Y - 3;
#define     MAX_BULLET_FRAME  400


// Offsets for the bullet bitmap
#define     BULLET_X    304
#define     BULLET_Y    0

// Structures

// DBLNODE - a node in a generic doubly-linked list
typedef struct _DBLNODE
{
    SHORT               type;       // type of object
    SHORT               index;      // index position in array
    BOOL                enabled;    // object enabled
    BOOL                shield;     // shield enabled
    DWORD               creator;    // object creator
    DWORD               timeStamp;  // time created or updated
    double              posx, posy; // actual x and y position
    double              velx, vely; // x and y velocity (pixels/millisecond)
    double              frame;      // current frame
    double              delay;      // frame/millisecond
    RECT                src, dst;   // source and destination rects
    LPDIRECTDRAWSURFACE surf;       // surface containing bitmap
} DBLNODE;
typedef DBLNODE FAR *LPDBLNODE;

//----------------------------------------------------------
// Communication packet structures
//----------------------------------------------------------

#define MSG_HEREIAM     0x11
#define MSG_INIT        0x22
#define MSG_UPDATE      0x33
#define MSG_ADDOBJ      0x44
#define MSG_DELOBJ      0x55
#define MSG_SCORE       0x66
#define MSG_LEVEL       0x77
#define MSG_DUNGEON       0x88

typedef struct _HEREIAMMSG
{
    BYTE        MsgCode;
    DWORD       ID;
} HEREIAMMSG, *LPHEREIAMMSG;

typedef struct _INITMSG
{
    BYTE        MsgCode;
    BYTE        YouAre;
    WORD        Level;
} INITMSG, *LPINITMSG;

typedef struct _UPDATEMSG
{
    BYTE        MsgCode;
    WORD        Index; 
    DBLNODE     Object;
} UPDATEMSG, *LPUPDATEMSG;

typedef struct _DELOBJMSG
{
    BYTE        MsgCode;
    WORD        Index; 
} DELOBJMSG, *LPDELOBJMSG;

typedef struct _SCOREMSG
{
    BYTE        MsgCode;
    short       Value;
} SCOREMSG, *LPSCOREMSG;

typedef struct _LEVELMSG
{
    BYTE        MsgCode;
    WORD        Level;
} LEVELMSG, *LPLEVELMSG;


typedef struct _DUNGEONMSG {

	BYTE MsgCode;
	char type;
	BOOL explored;
	char item;
	int image;
	int mark;
	int order;
	int x;
	int y;
	int lvl;

} DUNGEONMSG, *LPDUNGEON;






double      Initx[MAXPLAYERS] =
{
    272.0,
    336.0,
    272.0,
    336.0
};

double      Inity[MAXPLAYERS] =
{
    192.0,
    192.0,
    256.0,
    256.0
};

double      Dirx[40] =
{
    0.000000,
    0.156434,
    0.309017,
    0.453991,
    0.587785,
    0.707107,
    0.809017,
    0.891007,
    0.951057,
    0.987688,
    1.000000,
    0.987688,
    0.951057,
    0.891007,
    0.809017,
    0.707107,
    0.587785,
    0.453990,
    0.309017,
    0.156434,
    0.000000,
    -0.156435,
    -0.309017,
    -0.453991,
    -0.587785,
    -0.707107,
    -0.809017,
    -0.891007,
    -0.951057,
    -0.987688,
    -1.000000,
    -0.987688,
    -0.951056,
    -0.891006,
    -0.809017,
    -0.707107,
    -0.587785,
    -0.453990,
    -0.309017,
    -0.156434
};

double      Diry[40] =
{
    -1.000000,
    -0.987688,
    -0.951057,
    -0.891007,
    -0.809017,
    -0.707107,
    -0.587785,
    -0.453990,
    -0.309017,
    -0.156434,
    0.000000,
    0.156434,
    0.309017,
    0.453991,
    0.587785,
    0.707107,
    0.809017,
    0.891007,
    0.951057,
    0.987688,
    1.000000,
    0.987688,
    0.951057,
    0.891006,
    0.809017,
    0.707107,
    0.587785,
    0.453990,
    0.309017,
    0.156434,
    0.000000,
    -0.156435,
    -0.309017,
    -0.453991,
    -0.587785,
    -0.707107,
    -0.809017,
    -0.891007,
    -0.951057,
    -0.987688
};


// Function prototypes
void    DestroyGame( void );
BOOL    InitializeGame( void );
void    makeFontStuff( void );
void    UpdateFrame( void );
BOOL    CleanupAndExit( int err );
BOOL    RestoreSurfaces( void );
BOOL    isDisplayListEmpty( void );
void    initShip( BOOL delay );
void    initLevel( int level );
void    addObject( SHORT type, double x, double y, double vx, double vy );
void    UpdateDisplayList( void );
void    DrawDisplayList( void );
int     randInt( int low, int high );
double  randDouble( double low, double high );

void    CheckForHits( void );
void    bltScore( char *num, int x, int y );
void    DisplayFrameRate( void );
void    bltSplash( void );
void    EraseScreen( void );
void    FlipScreen( void );
void    DisplayLevel( void );
void    InitializeSound( void );
void    DestroySound( void );

// Functions added for DirectPlay support
void    DisplayWhoIAm(void);
BOOL RemoteCreate(GUID pGuid, LPSTR FullName, LPSTR NickName);
int     GetProvider(void);
int     CreateGame(void);
int     GetGame(void);
BOOL CALLBACK DlgProcChooseProvider(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcQCreate (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DlgProcSelSession (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL EnumSP(LPGUID lpGuid, LPSTR lpDesc, DWORD dwJamorVersion,
                       DWORD dwMinorVersion, LPVOID lpv);
BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID pContext,
                            LPDWORD lpdwTimeOut, DWORD dwFlags);
void    EvaluateMessage(DWORD len);
void    ReceiveGameMessages(void);
void    SendGameMessage(BYTE msg, DWORD to, short obj);

#endif
