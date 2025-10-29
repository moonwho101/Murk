//-----------------------------------------------------------------
// Main Frame Window Object
// C++ Source - MainFrm.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------
#include "MainFrm.h"

#include "Resource.h"
#include "Gametype2.h"
#include <StdLib.h>

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
#define GRIDBORDER  20
#define GRIDLINE    4
#define TILEOFFSET  2

//-----------------------------------------------------------------
// MFC Debugging Support
//-----------------------------------------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------
// CMainFrame Message Map & Runtime Support
//-----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
  // Messages
  ON_WM_CREATE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()

  // Command Responses
  ON_COMMAND(IDM_GAME_CREATE,   OnGameCreate)
  ON_COMMAND(IDM_GAME_CONNECT,  OnGameConnect)
  ON_COMMAND(IDM_GAME_END,      OnGameEnd)
  ON_COMMAND(IDM_GAME_EXIT,     OnGameExit)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

//-----------------------------------------------------------------
// CMainFrame Public Constructor(s)/Destructor
//-----------------------------------------------------------------
CMainFrame::CMainFrame()
  : m_pGame(NULL)
{
  // Load bitmaps
  VERIFY(m_bmGrid.LoadBitmap(IDB_GRID));
  VERIFY(m_bmTile[CGame::tsEMPTY].LoadBitmap(IDB_EMPTY));
  VERIFY(m_bmTile[CGame::tsX].LoadBitmap(IDB_X));
  VERIFY(m_bmTile[CGame::tsO].LoadBitmap(IDB_O));
}

CMainFrame::~CMainFrame()
{
  // Cleanup bitmaps
  VERIFY(m_bmGrid.DeleteObject());
  VERIFY(m_bmTile[CGame::tsEMPTY].DeleteObject());
  VERIFY(m_bmTile[CGame::tsX].DeleteObject());
  VERIFY(m_bmTile[CGame::tsO].DeleteObject());

  // Cleanup game
  if (m_pGame)
    delete m_pGame;
}

//-----------------------------------------------------------------
// Public Methods
//-----------------------------------------------------------------
BOOL
CMainFrame::Create(const CString& sTitle)
{
  CString sClassName;

  sClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
    LoadCursor(NULL, IDC_ARROW),
    (HBRUSH)(COLOR_WINDOW + 1),
    LoadIcon(AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDI_TICTACTOE)));

  return CFrameWnd::Create(sClassName, sTitle, WS_CAPTION |
    WS_SYSMENU | WS_MINIMIZEBOX, rectDefault, NULL,
    MAKEINTRESOURCE(IDM_MAIN));
}

//-----------------------------------------------------------------
// Protected Methods
//-----------------------------------------------------------------
void
CMainFrame::DrawGrid(CDC* pdc)
{
  CDC       dcMem;
  CBitmap*  pbmOld;

  // Setup memory DC
  ASSERT(pdc);
  dcMem.CreateCompatibleDC(pdc);
  VERIFY(pbmOld = dcMem.SelectObject(&m_bmGrid));

  // Draw tile images on memory grid image
  DrawGridTiles(&dcMem);

  // Get size of grid image
  BITMAP  bmInfo;
  m_bmGrid.GetObject(sizeof(BITMAP), &bmInfo);

  // Draw grid image
  pdc->BitBlt(GRIDBORDER, GRIDBORDER, bmInfo.bmWidth,
    bmInfo.bmHeight, &dcMem, 0, 0, SRCCOPY);

  // Cleanup memory DC
  VERIFY(dcMem.SelectObject(pbmOld));
}

void
CMainFrame::DrawGridTiles(CDC* pdc)
{
  // Draw individual grid tiles (Xs and Os)
  if (m_pGame)
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
      {
        CDC       dcMem;
        CBitmap*  pbmOld;

        // Setup memory DC
        ASSERT(pdc);
        dcMem.CreateCompatibleDC(pdc);
        VERIFY(pbmOld = dcMem.SelectObject(&m_bmTile
          [m_pGame->GetTileState(i, j)]));

        // Get size of tile image
        BITMAP  bmInfo;
        m_bmTile[m_pGame->GetTileState(i, j)].GetObject(sizeof
          (BITMAP), &bmInfo);

        // Calculate coordinates of tile image
        int iXTile = TILEOFFSET + i * (bmInfo.bmWidth + GRIDLINE);
        int iYTile = TILEOFFSET + j * (bmInfo.bmHeight + GRIDLINE);

        // Draw tile image
        pdc->BitBlt(iXTile, iYTile, bmInfo.bmWidth,
          bmInfo.bmHeight, &dcMem, 0, 0, SRCCOPY);

        // Cleanup memory DC
        VERIFY(dcMem.SelectObject(pbmOld));
      }
}

//-----------------------------------------------------------------
// CMainFrame Message Handler Methods
//-----------------------------------------------------------------
int
CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  // Resize window around grid image
  BITMAP  bmInfo;
  CRect   rcWnd, rcClient;
  m_bmGrid.GetObject(sizeof(BITMAP), &bmInfo);
  GetWindowRect(&rcWnd);
  GetClientRect(&rcClient);
  rcWnd.right = rcWnd.left + rcWnd.Width() - rcClient.Width() +
    bmInfo.bmWidth + 2 * GRIDBORDER;
  rcWnd.bottom = rcWnd.top + rcWnd.Height() - rcClient.Height() +
    bmInfo.bmHeight + 2 * GRIDBORDER;
  MoveWindow(&rcWnd);

  return 0;
}

void
CMainFrame::OnPaint()
{
  // Get the DC
  CPaintDC dc(this);

  // Draw the grid
  DrawGrid(&dc);
}

void
CMainFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
  if (m_pGame && m_pGame->DPIsValid())
  {
    if (m_pGame->GetMyTurn())
    {
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
          // Calculate tile rectangle
          CRect   rcTile;
          BITMAP  bmInfo;
          m_bmTile[CGame::tsX].GetObject(sizeof(BITMAP), &bmInfo);
          rcTile.left = GRIDBORDER + TILEOFFSET + i *
            (bmInfo.bmWidth + GRIDLINE);
          rcTile.right = rcTile.left + bmInfo.bmWidth;
          rcTile.top = GRIDBORDER + TILEOFFSET + j *
            (bmInfo.bmHeight + GRIDLINE);
          rcTile.bottom = rcTile.top + bmInfo.bmHeight;

          // See if click is in the tile
//          if (rcTile.PtInRect(point))
  //          m_pGame->SetTileState(i, j);
        }
    }
    else
    {
      // Play the tile error wave and notify user
      CWave wavError(IDW_ERROR);
      wavError.Play();
      MessageBox("Please wait your turn.", AfxGetAppName());
    }
  }
}

//------------------------------------------------------------------------------
// CMainFrame Command Response Methods
//------------------------------------------------------------------------------
void
CMainFrame::OnGameCreate()
{
  // Cleanup game if necessary

	    CGameType2 dlgGameType;
    if (dlgGameType.DoModal() == IDOK)
    {
	if (m_pGame)
    delete m_pGame;

  // Create new game and DP session
  m_pGame = new CGame;
  if (m_pGame)
    m_pGame->DPCreateSession();

	
	}

	
	

	
	
}

void
CMainFrame::OnGameConnect()
{
  // Cleanup game if necessary
  if (m_pGame)
    delete m_pGame;

  // Create new game and select a DP session
  m_pGame = new CGame;
  if (m_pGame)
    m_pGame->DPConnectSession();
}

void
CMainFrame::OnGameEnd()
{
  // End the DP session
  if (m_pGame)
    m_pGame->DPEndSession();
}

void
CMainFrame::OnGameExit()
{
  PostMessage(WM_CLOSE, 0, 0L);
}
