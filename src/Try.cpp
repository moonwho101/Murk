/*==========================================================================
 *
 *  Based on ROIDS.C, which is 
 *  Copyright (C) 1995 Microsoft Corporation. All Rights Reserved.
 *
 *  File:       NetRoids.c
 *  Content:    Multiplayer Shoot-em-up game
 *
 *  DirectPlay support added by Chris Howard, 
 *  Copyright (C) 1996 Chris Howard. All Rights Reserved.
 *
 ***************************************************************************/




#include "netroid.h"
// NetRoids a64e8e40-5750-11cf-a4ac-0000c0ec0b9f
DEFINE_GUID(NETROIDS_GUID,0xa64e8e40,0x5750,0x11cf,0xa4,0xac,0x00,0x00,0xc0,0xec,0x0b,0x9f);




LPDIRECTPLAY            lpIDC = NULL;     // DirectPlay Object
DPID                    dcoID = 0;        // our DirectPlay ID
LPGUID                  g_lpGuid;         // Global GUID
HANDLE                  dphEvent = NULL;  // Disable event callbacks

BOOL                    IsHost;           // Whether we are the host
BOOL                    HaveHostInit;     // Whether we been initialized by the host
BYTE                    WhoIAm;           // Our player number and index entry

BOOL                    NewLevel = FALSE; // Signals clients to start a new level

LPDBLNODE               MyShip;           // Convenient pointer to our array entry
BYTE                    CommBuff[256];    // Communication buffer for messages

BOOL                    bNoCollision = FALSE;  // Hidden debugging flag to turn
                                               // collision detection off

// All variables after this were in the original game

LPDIRECTDRAWSURFACE     lpFrontBuffer;
LPDIRECTDRAWSURFACE     lpBackBuffer;
LPDIRECTDRAWSURFACE     lpDonut;
LPDIRECTDRAWSURFACE     lpPyramid;
LPDIRECTDRAWSURFACE     lpCube;
LPDIRECTDRAWSURFACE     lpSphere;
LPDIRECTDRAWSURFACE     lpShip;
LPDIRECTDRAWSURFACE     lpNum;
LPDIRECTDRAW            lpDD;
LPDIRECTSOUND           lpDS;
LPDIRECTDRAWPALETTE     lpArtPalette;
LPDIRECTDRAWPALETTE     lpSplashPalette;
BOOL                    bSoundEnabled = FALSE;
BOOL                    bPlayIdle = FALSE;
BOOL                    bPlayBuzz = FALSE;
BOOL                    bPlayRev = FALSE;
DWORD                   lastInput = 0;
BOOL                    lastThrust = FALSE;
int                     showDelay = 0;
HWND                    hWndMain;
BOOL                    bShowFrameCount=TRUE;
BOOL                    bIsActive;
DWORD                   dwFrameCount;
DWORD                   dwFrameTime;
DWORD                   dwFrames;
DWORD                   dwFramesLast;
BOOL                    bUseEmulation;
BOOL                    bTest=FALSE;
DWORD                   dwTransType;
RGBQUAD                 SPalette[256];
DWORD                   lastTickCount;
int                     score;
int                     ProgramState;
int                     level;
int                     restCount;
DWORD                   dwFillColor;
BOOL                    bSpecialEffects = FALSE;
DWORD                   ShowLevelCount = 3000;

#ifdef DEBUG
char                    DebugBuf[256];
BOOL                    bHELBlt = FALSE;
#endif

DBLNODE                 DL[MAXOBJECTS];   // Display List


BOOL RemoteCreate(GUID pGuid, LPSTR FullName, LPSTR NickName)
{
    HRESULT hr;
    DPSESSIONDESC dpDesc;

    // Be sure we aren't already initialized
    if (lpIDC != NULL)
    {
        return( FALSE );
    }

    // Get a list of DirectPlay service providers, and let the player choose one
    GetProvider();

    // If one was not chosen, exit now
    if (lpIDC == NULL)
        return(FALSE);

    // The CreateGame dialog gox determines whether the player wants to create a
    // new game, or connect to an existing one.
    switch(CreateGame())
    {
    case 1:
        // Create a new game, so we're the host
        IsHost = TRUE;
        // Initialize session description structure
        memset(&dpDesc, 0x00, sizeof(DPSESSIONDESC));
        dpDesc.dwSize = sizeof(dpDesc);
        dpDesc.dwMaxPlayers = MAXPLAYERS;
        dpDesc.dwFlags = DPOPEN_CREATESESSION;
        dpDesc.guidSession = pGuid;
        strcpy(dpDesc.szSessionName, FullName);
        // Try to open the session




        if ((hr = lpIDC->lpVtbl->Open(lpIDC, &dpDesc)) != DP_OK)
        {
            // We failed
            lpIDC->lpVtbl->Release(lpIDC);
            lpIDC = NULL;
            return(FALSE);
        }
        break;

    case 2:             
        // Connect to an existing game, so we're NOT the host
        IsHost = FALSE;
        // Set the global GUID
        g_lpGuid = (LPGUID) &pGuid;
        // Try and find an existing game
        GetGame();
        // Was a game found, and selected?
        if (lpIDC == NULL)
            return(FALSE);
        break;

    default:
        return(FALSE);
    }

    // Either way, we have to create a player
    if ((hr = lpIDC->lpVtbl->CreatePlayer(lpIDC, &dcoID, NickName,
                                  "NetRoids Player", &dphEvent)) != DP_OK)
    {
        // We failed
        lpIDC->lpVtbl->Close(lpIDC);
        lpIDC->lpVtbl->Release(lpIDC);
        lpIDC = NULL;
        return(FALSE);
    }

	
//	SetTimer(hWndMain, 1, 200, TimerProc);		    
    // Sweet success
    return(TRUE);

}

//**********

int GetProvider()
{
  return(DialogBox (NULL, (LPCTSTR) IDD_CHOOSEPROVIDER, hWndMain,
                    (DLGPROC) DlgProcChooseProvider));
}

//**********

int CreateGame()
{
  return(DialogBox (NULL, (LPCTSTR) IDD_Q_CREATE, hWndMain, 
                    (DLGPROC) DlgProcQCreate));
}

//**********

int GetGame()
{
  return(DialogBox (NULL, (LPCTSTR) IDD_SELSESSION, hWndMain, 
                    (DLGPROC) DlgProcSelSession));
}

//**********

BOOL CALLBACK DlgProcChooseProvider(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPGUID  lpGuid;
    static  LONG iIndex;
    static  HWND hWndCtl;
    
    switch (msg)
    {
    case WM_INITDIALOG:
        // Get the list control
        hWndCtl = GetDlgItem(hDlg, IDC_LIST1);
        if (hWndCtl == NULL)
        {
            EndDialog(hDlg, TRUE);
            return(TRUE);
        }
        // Fill the list with DirectPlay service providers
        DirectPlayEnumerate(EnumSP, (LPVOID) hWndCtl);
        // Highlight the first one
        SetFocus(hWndCtl);
        SendMessage(hWndCtl, LB_SETCURSEL, 0, 0);
        return(FALSE);

    case WM_COMMAND:
        switch( HIWORD(wParam))
        {
        case LBN_SELCHANGE:
            // Move cursor
            iIndex = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
            hWndCtl = (HWND) lParam;
            return(FALSE);

        case LBN_DBLCLK:
            // They've selected one, so get it
            iIndex = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
            if (iIndex != LB_ERR)
            {
                lpGuid = (LPGUID) SendMessage((HWND) lParam, LB_GETITEMDATA, iIndex, 0);
                // Create a DirectPlay object for the chosen provider
                DirectPlayCreate(lpGuid, &lpIDC, NULL);
                EndDialog(hDlg, TRUE);
                return(TRUE);
            }
            break;

        case 0:
            // Was enter pressed?
            if (LOWORD(wParam) == IDOK)
            {
                // Is there anything in the listbox?
                if (iIndex != LB_ERR)
                {
                    // Get the selected provider
                    lpGuid = (LPGUID) SendMessage(hWndCtl, LB_GETITEMDATA, iIndex, 0);
                    if (lpGuid)
                    {
                        // Create a DirectPlay object for the chosen provider
                        DirectPlayCreate(lpGuid, &lpIDC, NULL);
                        EndDialog(hDlg, TRUE);
                    }
                    else
                        EndDialog(hDlg, FALSE);
                    return(TRUE);
                }
            }
            else if (LOWORD(wParam) == IDCANCEL)
            {
                // Escape was pressed
                EndDialog(hDlg, FALSE);
                return(TRUE);
            }
            break;

        }
    }
    return (FALSE);
}

//**********

BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID lpContext, 
                            LPDWORD lpdwTimeOut, DWORD dwFlags)
{
    LONG iIndex;
    HWND hWnd = (HWND) lpContext;

    // Have we timed out?
    if( dwFlags & DPESC_TIMEDOUT )
    {
        return FALSE;       // don't try again
    }

    // Add the session to the listbox
    iIndex = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) lpDPGameDesc->szSessionName);
    if (iIndex != LB_ERR)
        SendMessage(hWnd, LB_SETITEMDATA, iIndex, (LPARAM) lpDPGameDesc->dwSession);

    SetFocus(hWnd);
    SendMessage(hWnd, LB_SETCURSEL, 0, 0);
    return(TRUE);

}

//**********

BOOL FAR PASCAL EnumSP(LPGUID lpGuid, LPSTR lpDesc, DWORD dwMajorVersion,
                       DWORD dwMinorVersion, LPVOID lpv)
{
    LONG iIndex;
    HWND hWnd = (HWND) lpv;

    // Add the service provider to the listbox
    iIndex = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) lpDesc);
    if (iIndex != LB_ERR)
        SendMessage(hWnd, LB_SETITEMDATA, iIndex, (LPARAM) lpGuid);

    SetFocus(hWnd);
    SendMessage(hWnd, LB_SETCURSEL, 0, 0);
    return(TRUE);
}

//**********

BOOL CALLBACK DlgProcQCreate (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // This handles the messages from the Create/Connect dialog box
    switch (msg)
    {
    case WM_COMMAND:
        switch(wParam)
        {
        case IDC_CREATE:
            EndDialog(hDlg, 1);
            return(TRUE);

        case IDC_CONNECT:
            EndDialog(hDlg, 2);
            return(TRUE);

        case IDCANCEL:
            EndDialog(hDlg, -1);
            return(TRUE);
        }
        break;

    }
    return(FALSE);
}

//**********

BOOL CALLBACK DlgProcSelSession (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static  LONG    iIndex;
    static  HWND hWndCtl;
    DPSESSIONDESC dpDesc;
    HRESULT hr = DP_OK + 10;

    switch (msg)
    {
    case WM_INITDIALOG:
        // Get the listbox
        hWndCtl = GetDlgItem(hDlg, IDC_LB_SESSION);
        if (hWndCtl == NULL)
        {
            EndDialog(hDlg, TRUE);
            return(TRUE);
        }
        // Initialize the session description structure
        memset(&dpDesc, 0x00, sizeof(DPSESSIONDESC));
        dpDesc.dwSize = sizeof(dpDesc);
        dpDesc.guidSession = *g_lpGuid;
        // Enum sessions with 5 second timeout
        lpIDC->lpVtbl->EnumSessions(lpIDC, &dpDesc, (DWORD)5000, EnumSession, (LPVOID) hWndCtl, (DWORD)NULL);
        SetFocus(hWndCtl);
        return(FALSE);

    case WM_COMMAND:

        switch( HIWORD(wParam))
        {
        case LBN_SELCHANGE:
            // Change selection
            iIndex = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
            hWndCtl = (HWND) lParam;
            return(FALSE);

        case 0:
            if (LOWORD(wParam) == IDCANCEL)
            {
                // Player pressed escape
                lpIDC->lpVtbl->Close(lpIDC);
                lpIDC->lpVtbl->Release(lpIDC);
                lpIDC = NULL;
                EndDialog(hDlg, FALSE);
                return(TRUE);
            }
            //
            // Fall Through.
            //
        case LBN_DBLCLK:
            if (HIWORD(wParam) == LBN_DBLCLK)
            {
                // They selected an entry, so get it
                hWndCtl = (HWND) lParam;
                iIndex = SendMessage(hWndCtl, LB_GETCURSEL, 0, 0);
            }

            // Do we have an entry?
            if (iIndex != LB_ERR)
            {
                // Yes, so initialize session description struc to open it
                memset(&dpDesc, 0x00, sizeof(DPSESSIONDESC));
                dpDesc.dwSize       = sizeof(dpDesc);
                dpDesc.guidSession  = *g_lpGuid;
                dpDesc.dwFlags      = DPOPEN_OPENSESSION;
                dpDesc.dwSession    = SendMessage((HWND) hWndCtl, LB_GETITEMDATA, iIndex, 0);
                hr = lpIDC->lpVtbl->Open(lpIDC, &dpDesc);

                // Successful?
                if (hr != DP_OK)
                {
                    // Failed, so clean up
                    lpIDC->lpVtbl->Close(lpIDC);
                    lpIDC->lpVtbl->Release(lpIDC);
                    lpIDC = NULL;
                    EndDialog(hDlg, FALSE);
                }

                EndDialog(hDlg, TRUE);
                return(TRUE);

            }
        }
    }
    return (FALSE);
}

//**********

void ReceiveGameMessages(void)
{
    DPID    fromID, dcoReceiveID;
    DWORD   nBytes;

    // Do we have an ID?
    if (lpIDC)
    {
        // read all messages in queue
        while(1)
        {
            HRESULT status;
            nBytes = MAX_BUFFER_SIZE;
            status = lpIDC->lpVtbl->Receive(lpIDC, &fromID, &dcoReceiveID, DPRECEIVE_ALL, CommBuff, &nBytes);
            switch(status)
            {
            case DP_OK:
                if (fromID == 0)
                {
                    // ignore system messages
                }
                else
                {
                    EvaluateMessage(nBytes);
                }
                break;

            default:

                // Error condition of some kind - we just stop
                // checking for now
                return;
            }
        }
    }
}

//**********

void EvaluateMessage(DWORD len)
{
    LPHEREIAMMSG    lpHereIAm;
    LPINITMSG       lpInit;
    LPUPDATEMSG     lpUpdate;
    LPDELOBJMSG     lpDelObj;
    LPSCOREMSG      lpScore; 
    LPLEVELMSG      lpLevel; 
   	LPDUNGEON		lpDungeon;
    int             shipID;
    int             i;

    char            dBuf[256];



    // The first byte is the message ID
    switch(CommBuff[0])
    {



    case MSG_DUNGEON:

            lpDungeon = (LPDUNGEON)CommBuff;




		break;

    case MSG_HEREIAM:
        // Someone wants to play
        if (IsHost)
        {
            // I'm the host, so find out who is here
            lpHereIAm = (LPHEREIAMMSG)CommBuff;
            shipID = lpHereIAm->ID;
            // Initialize them
            SendGameMessage(MSG_INIT, shipID, 0);
        }
        break;

    case MSG_INIT:
        // The host answered me, so find out who I am
        lpInit = (LPINITMSG)CommBuff;
        WhoIAm = lpInit->YouAre;
        MyShip = &DL[WhoIAm];
        // and what level we are on
        level  = lpInit->Level;
//        initLevel(level);
        // We're all set
        HaveHostInit = TRUE;
        break;

    case MSG_ADDOBJ:
    case MSG_UPDATE:
        // Add or update an object's state
        lpUpdate = (LPUPDATEMSG)CommBuff;
        i = lpUpdate->Index;
        // The host processes players and added objects, clients process everything
        if ((i < MAXPLAYERS) || (lpUpdate->MsgCode == MSG_ADDOBJ) || (!IsHost))
        {
            // Don't update me
            if (i != WhoIAm) {
                // Update the object
                DL[i] = lpUpdate->Object;
                // IMPORTANT! We have to specify our own surfaces
                switch (DL[i].type)
                {
                    case OBJ_DONUT:
                        DL[i].surf = lpDonut;
                        break;
                    case OBJ_PYRAMID:
                        DL[i].surf = lpPyramid;
                        break;
                    case OBJ_CUBE:
                        DL[i].surf = lpCube;
                        break;
                    case OBJ_SPHERE:
                        DL[i].surf = lpSphere;
                        break;
                    case OBJ_BULLET:
                    case OBJ_EXPLODE:
                        DL[i].surf = lpNum;
                        break;
                    case OBJ_SHIP:
                        DL[i].surf = lpShip;
                        break;
                }
            }
        }
        break;
        
    case MSG_DELOBJ:
        // Delete an object
        lpDelObj = (LPDELOBJMSG)CommBuff;
        i = lpDelObj->Index;

		/*
        // Figure out what sound to play
        switch (DL[i].type)
        {
            case OBJ_DONUT:
                hSO = hsoDonutExplode;
                break;
            case OBJ_PYRAMID:
                hSO = hsoPyramidExplode;
                break;
            case OBJ_CUBE:
                hSO = hsoCubeExplode;
                break;
            case OBJ_SPHERE:
                hSO = hsoSphereExplode;
                break;
            case OBJ_SHIP:
                hSO = hsoShipExplode;
                break;
            default:
                hSO = NULL;
        }
        // If there is a sound, play it
        if (hSO != NULL)
            playPanned(hSO, &DL[i]);

		*/
        // Delete the object
//        DeleteFromList(&DL[i]);
        // If this is my ship, re-initialize it
//        if (i == WhoIAm)
  //          initShip(TRUE);
        break;

    case MSG_SCORE:
        // Update a player's score
        lpScore = (LPSCOREMSG)CommBuff;
        score += lpScore->Value;
        if (score < 0)
            score = 0;
        break;
        
    case MSG_LEVEL:
        // Start a new level, or end a game
        lpLevel = (LPLEVELMSG)CommBuff;
        if (lpLevel->Level > 0)
        {
            // New level (client will bump level number, so -1)
            level = lpLevel->Level-1;
            NewLevel = TRUE;
        }
        else 
        {
            // Host has left, so end the game
            PostMessage(hWndMain, WM_CLOSE, 0, 0);
        }
        break;
        
    default:
        // Whoa! What happened?
        wsprintf(dBuf, "Unknown message: %d\n", CommBuff[0]);
        OutputDebugString(dBuf);
        break;
    }

}

//**********

void SendGameMessage(BYTE msg, DWORD to, short obj)
{
    LPHEREIAMMSG    lpHereIAm;
    LPINITMSG       lpInit;
    LPUPDATEMSG     lpUpdate;
    LPDELOBJMSG     lpDelObj;
    LPSCOREMSG      lpScore; 
    LPLEVELMSG      lpLevel; 
	LPDUNGEON		lpDungeon;
    int             nBytes;
    int             i;
    DWORD           send_to = 0;
    
    // What message should we send?
    switch(msg)
    {
    case MSG_DUNGEON:
        // Tell host we are here
        lpDungeon = (LPDUNGEON)CommBuff;
        lpDungeon->MsgCode = msg;

        lpDungeon->type = 'f';
        lpDungeon->explored = 1;
        lpDungeon->item = 0;
        lpDungeon->image=0;
        lpDungeon->mark=0;
        lpDungeon->order=0;

        nBytes = sizeof(LPDUNGEON);
        break;

    case MSG_HEREIAM:
        // Tell host we are here
        lpHereIAm = (LPHEREIAMMSG)CommBuff;
        lpHereIAm->MsgCode = msg;
        lpHereIAm->ID = (DWORD)dcoID;
        nBytes = sizeof(HEREIAMMSG);
        break;

    case MSG_INIT:
        // Initialize a client
        lpInit = (LPINITMSG)CommBuff;
        lpInit->MsgCode = msg;
        // Only send to specific player
        send_to = to;
        // Find a player slot
        for(i=1; i<MAXPLAYERS; i++)
        {
            // If this slot is not enabled and has no creator, we're done
            if (!DL[i].enabled && (DL[i].creator == 0))
                break;
        }
        // Did we find any room?
        if( i == MAXPLAYERS )
        {
            // No room for more players
            return;
        }
        // Tell them who they are
        lpInit->YouAre = i;
        // And what level we are on
        lpInit->Level = (WORD) level;
        nBytes = sizeof(INITMSG);
        break;
        
    case MSG_ADDOBJ:
    case MSG_UPDATE:
        // Add and update messages are the same, really
        lpUpdate = (LPUPDATEMSG)CommBuff;
        lpUpdate->MsgCode = msg;
        // Add/update this object
        lpUpdate->Index  = obj;
        lpUpdate->Object = DL[obj];
        nBytes = sizeof(UPDATEMSG);
        break;

    case MSG_DELOBJ:
        // Delete this object
        lpDelObj = (LPDELOBJMSG)CommBuff;
        lpDelObj->MsgCode = msg;
        lpDelObj->Index  = obj;
        nBytes = sizeof(DELOBJMSG);
        break;

    case MSG_SCORE:
        // If score is for the host, don't send, just add it to host score
        if (to == dcoID)
        {
            score += obj;
            if (score < 0)
                score = 0;
            return;
        }
        // Score is for another player
        lpScore = (LPSCOREMSG)CommBuff;
        lpScore->MsgCode = msg;
        // Send to this player only
        send_to = to;
        lpScore->Value = obj;
        nBytes = sizeof(SCOREMSG);
        break;

    case MSG_LEVEL:
        // Start a new level. The clients won't always "see" an empty
        // list, so this makes sure the level gets displayed
        lpLevel = (LPLEVELMSG)CommBuff;
        lpLevel->MsgCode = msg;
        lpLevel->Level = obj;
        nBytes = sizeof(LEVELMSG);
        break;

    }

    // Send the message
    lpIDC->lpVtbl->Send(lpIDC, dcoID, send_to, 0, (LPSTR) CommBuff, nBytes);       

}
        

/*
void CALLBACK EXPORT TimerProc(HWND hWnd,UINT nMsg,UINT nIDEvent,DWORD dwTime) {
	
	if (nIDEvent == 1) {
        SendGameMessage(MSG_DUNGEON, 1, 0);		
	}


}

  *//////////////////////////////////////////////////////////////////////////////
// Try dialog


Try::Try(CWnd* pParent /*=NULL*/)
	: CDialog(Try::IDD, pParent)
{
	//{{AFX_DATA_INIT(Try)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void Try::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Try)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Try, CDialog)
	//{{AFX_MSG_MAP(Try)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Try message handlers
