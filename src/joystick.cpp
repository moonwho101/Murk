//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.

//-----------------------------------------------------------------
// Main Frame Window Object
// C++ Source - MainFrm.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
#include "Resource.h"
#include "joystick.h"
#include "murkcommon.h"
#include "murkdecl.h"

int joystickhit;

int xmin;
int xmax;
int ymin;
int ymax;

extern int usejstick;
extern void send_drop_treasure(int treasure);
//-----------------------------------------------------------------
// CMainFrame Message Map & Runtime Support
//-----------------------------------------------------------------
/*
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
  // Message Handlers
  ON_WM_CREATE()
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_MESSAGE(MM_JOY1MOVE, OnJoyMove)
  ON_MESSAGE(MM_JOY2MOVE, OnJoyMove)
  ON_MESSAGE(MM_JOY1BUTTONDOWN, OnJoyButtonDown)
  ON_MESSAGE(MM_JOY2BUTTONDOWN, OnJoyButtonDown)
END_MESSAGE_MAP()
*/

//  : m_uiThreshold(125), m_bJoyActive(FALSE)

//-----------------------------------------------------------------
// Protected Methods
//-----------------------------------------------------------------
BOOL InitJoystick() {
	JOYINFO jiInfo;
	UINT uiNumDevs;
	BOOL bDev1Attached, bDev2Attached;

	m_uiThreshold = 125;
	m_bJoyActive = FALSE;

	// See if joystick driver is present
	if ((uiNumDevs = ::joyGetNumDevs()) == 0) {
		debug_me("InitJoystick", "joyNumDev failed", 0, 0);
		return FALSE;
	}

	// See if joystick(s) are attached
	bDev1Attached = (::joyGetPos(JOYSTICKID1, &jiInfo) !=
	                 JOYERR_UNPLUGGED);
	bDev2Attached = (::joyGetPos(JOYSTICKID2, &jiInfo) !=
	                 JOYERR_UNPLUGGED);

	// Decide which joystick to use
	if (bDev1Attached || bDev2Attached)
		m_uiDevID = bDev1Attached ? JOYSTICKID1 : JOYSTICKID2;
	else {
		debug_me("InitJoystick", "no joysticks", 0, 0);
		return FALSE;
	}

	// Set the joystick threshold
	VERIFY(::joySetThreshold(m_uiDevID, m_uiThreshold) ==
	       JOYERR_NOERROR);

	debug_me("InitJoystick", "Found a joystick", 0, 0);

	if (usejstick)
		joySetCapture(hWindow, m_uiDevID, NULL, TRUE);

	JoyToClient();
	return TRUE;
}

BOOL JoyToClient() {
	// Get the joystick capabilities
	JOYCAPS jcCaps;
	if (::joyGetDevCaps(m_uiDevID, &jcCaps, sizeof(JOYCAPS)) !=
	    JOYERR_NOERROR)
		return FALSE;

	xmin = jcCaps.wXmin;
	xmax = jcCaps.wXmax;
	ymin = jcCaps.wYmin;
	ymax = jcCaps.wYmax;

	debug_me("JOY", "wXmin wXmax", jcCaps.wXmin, jcCaps.wXmax);
	debug_me("JOY", "wYmin wYmax", jcCaps.wYmin, jcCaps.wYmax);
	// Get the client rect
	//  CRect rcClient;
	//  GetClientRect(&rcClient);

	// Convert joystick x position to client coordinates
	//  pptJoyPos->x = ((LONG)rcClient.Width() * (pptJoyPos->x -
	//    jcCaps.wXmin) / (jcCaps.wXmax - jcCaps.wXmin));

	// Convert joystick y position to client coordinates
	//  pptJoyPos->y = ((LONG)rcClient.Height() * (pptJoyPos->y -
	//    jcCaps.wYmin) / (jcCaps.wYmax - jcCaps.wYmin));

	return TRUE;
}

//  if (!InitJoystick())
//  return -1;

//  return 0;

/*
void
OnSetFocus(CWnd* pOldWnd)
{
  // Capture the joystick
  ::joySetCapture(GetSafeHwnd(), m_uiDevID, NULL, TRUE);
}

void
CMainFrame::OnKillFocus(CWnd* pNewWnd)
{
  // Release the joystick
  ::joyReleaseCapture(m_uiDevID);
}

*/

LONG OnJoyMove(UINT wParam, LONG lParam) {
	//  if (m_bJoyActive)
	//  {
	CPoint ptJoyPos(LOWORD(lParam), HIWORD(lParam));

	// Convert joystick position to client coordinates
	//    if (JoyToClient(&ptJoyPos))
	//    {

	// Convert client joystick position to screen coordinates
	//      ClientToScreen(&ptJoyPos);

	// Move the ghost cursor
	//      ::SetCursorPos(ptJoyPos.x, ptJoyPos.y);
	//    }
	//  }

	if (ptJoyPos.x == 0 && ptJoyPos.y == 0) {
		//		mousehit=0;
		joystickhit = 0;

		return TRUE;
	}

	if (ptJoyPos.x < (xmax / 2) + 5000 && ptJoyPos.x > (xmax / 2) - 5000 && ptJoyPos.y < ymin + 10000) {
		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 2;
		}
		return TRUE;
	}

	if (ptJoyPos.x < (xmax / 2) + 5000 && ptJoyPos.x > (xmax / 2) - 5000 && ptJoyPos.y > ymax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 3;
		}
		return TRUE;
	}

	if (ptJoyPos.y < (ymax / 2) + 5000 && ptJoyPos.y > (xmax / 2) - 5000 && ptJoyPos.x < xmin + 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 1;
		}
		return TRUE;
	}
	if (ptJoyPos.y < (ymax / 2) + 5000 && ptJoyPos.y > (xmax / 2) - 5000 && ptJoyPos.x > xmax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 4;
		}
		return TRUE;
	}

	if (ptJoyPos.x < xmin + 10000 && ptJoyPos.y < ymin + 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 1;
		}
		return TRUE;
	}
	if (ptJoyPos.x > xmax - 10000 && ptJoyPos.y < ymin + 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 2;
		}
		return TRUE;
	}

	if (ptJoyPos.x < xmin + 10000 && ptJoyPos.y > ymax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 3;
		}
		return TRUE;
	}

	if (ptJoyPos.x > xmax - 10000 && ptJoyPos.y > ymax - 10000) {

		if (hitkeyboard == 0 && PlayerLocation[1].stairs == 0 && PlayerLocation[1].dead == 0) {
			mousehit = 1;
			joystickhit = 1;
			newmove = 4;
		}
		return TRUE;
	}

	if (ptJoyPos.x < (xmax / 2) + 10000 && ptJoyPos.x > (xmax / 2) - 10000 &&
	    ptJoyPos.y < (ymax / 2) + 10000 && ptJoyPos.x > (ymax / 2) - 10000) {

		if (joystickhit == 1)
			mousehit = 0;
		joystickhit = 0;
		return TRUE;
	}

	joystickhit = 0;

	return TRUE;
}

LONG OnJoyButtonDown(UINT wParam, LONG lParam) {
	if (wParam & JOY_BUTTON1) {
		if (PlayerLocation[1].firecount == 0) {
			if (PlayerLocation[1].firecount == 0) {
				PlayerLocation[1].firecount = PlayerLocation[1].firerate;
			}

			start_missle(PlayerLocation[1].mapx, PlayerLocation[1].mapy, PlayerLocation[1].direction,
			             PlayerLocation[1].level,
			             PlayerLocation[1].x, PlayerLocation[1].y, 1);
		}

	} else if (wParam & JOY_BUTTON2) {
		handle_stairs();
	}

	else if (wParam & JOY_BUTTON3) {
		if (foundtreasure == 1) {
			treasurex = PlayerLocation[1].mapx;
			treasurey = PlayerLocation[1].mapy;
			treasurelvl = PlayerLocation[1].level;
			foundtreasure = 0;
			PlayerLocation[1].hit = 1;
			dropt = 0;
			if (networkgame)
				send_drop_treasure(1);
		}

		if (foundtreasure2 == 1) {
			treasurex2 = PlayerLocation[1].mapx;
			treasurey2 = PlayerLocation[1].mapy;
			treasurelvl2 = PlayerLocation[1].level;
			foundtreasure2 = 0;
			PlayerLocation[1].hit = 1;
			dropt = 0;
			if (networkgame)
				send_drop_treasure(2);
		}

	} else if (wParam & JOY_BUTTON4) {
		if (PlayerLocation[1].protection == 0 && PlayerLocation[1].ring != 0) {
			if (m_directSoundOK)
				m_pDirSound->PlaySound(m_bufferTadaCleric);

			PlayerLocation[1].protection = 70;
			PlayerLocation[1].ring--;
		}
	}

	return TRUE;
}
