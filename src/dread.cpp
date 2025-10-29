///////////////////////////////////////////////////////////
// AZTEC.CPP: Main source code for Aztec Adventure.
///////////////////////////////////////////////////////////

#include <afx.h>
#include <windows.h>
#include "resource.h"
#include "cdirdraw.h"

// Prototype for window procedure.
LRESULT APIENTRY WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);

// Function prototypes.
BOOL InitGraphics();
void OnKeyDown(WPARAM wParam);
void OnDestroy();
void LoadFloorDibs(UINT floor);
void LoadDudeDibs(UINT floor);
void ShowPlayer (void);
void animate_player(int startpos);
void delay_s(unsigned char seconds);
void OnTimer(void);


// Global instance handle.
HINSTANCE hInstance;

// Global variables.
CDirDraw* pDirDraw;
BOOL ddok;
BOOL setupOK = FALSE;
HWND hWindow;
UINT partsASurfaceNum;


int playerx,playery,direction;

int moveleft,moveright;

int frame_num;


///////////////////////////////////////////////////////////
// WinMain
///////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hCurrentInst,
    HINSTANCE hPreviousInst, LPSTR /*lpszCmdLine */,
    int nCmdShow)
{
    WNDCLASS wndClass;
    HWND hWnd;
    MSG msg;

    // Define and register the window class.
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(hCurrentInst,
        MAKEINTRESOURCE(IDI_ICON1));
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = NULL;
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = "Aztec";

    RegisterClass(&wndClass);

    hInstance = hCurrentInst;

    // Create a window of the previously defined class.
    hWnd = CreateWindow(
               "Aztec",          // Window class's name.
               "Aztec Adventure",// Title bar text.
               WS_POPUP,         // The window's style.
               0,                // X position.
               0,                // Y position.
               640,              // Width.
               480,              // Height.
               NULL,             // Parent window's handle.
               NULL,             // Menu handle.
               hCurrentInst,     // Instance handle.
               NULL);            // No additional data.

    // Display the window on the screen.
    ShowWindow(hWnd, nCmdShow);

    // Force the window to repaint itself.
    UpdateWindow(hWnd);

    // Save the window handle.
    hWindow = hWnd;

	playerx =100;
	playery=100;
	moveleft = 100;
	moveright =100 ;
	direction = 1;




    // Set up DirectDraw.
    setupOK = InitGraphics();
    if (!setupOK)
        return 0;




    ShowPlayer();

	
	// Start the message loop.
    while (GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

///////////////////////////////////////////////////////////
// WndProc()
//
// This is the main window procedure, which is called by
// Windows.
///////////////////////////////////////////////////////////
LRESULT APIENTRY WndProc(HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    // Handle the messages to which the application
    // must respond.
    switch(message)
    {
        case WM_KEYDOWN:
            OnKeyDown(wParam);
            return 0;

        case WM_DESTROY:
            OnDestroy();
            PostQuitMessage(0);
            return 0;

		case WM_TIMER:
            OnTimer();
            return 0;
	
	
	}

    // Make sure all unhandled messages get returned to Windows.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

///////////////////////////////////////////////////////////
// InitGraphics()
//
// This function sets up DirectDraw and the program's
// graphics.
///////////////////////////////////////////////////////////
BOOL InitGraphics()
{
    
	BOOL resultOK;
    int i;

	// Create the main DirectDraw object.
    pDirDraw = new CDirDraw(hWindow);
    ddok = pDirDraw->DirectDrawOK();
    if (!ddok)
        return FALSE;

    // Create an off-screen surface for the bitmap.
    partsASurfaceNum =
        pDirDraw->CreateOffScreenSurface(640, 480);
    if (partsASurfaceNum == 0)
        return FALSE;

    // Load the bitmaps for the current dungeon floor.

/*

    LoadFloorDibs(1);

    // Blit an image to the back buffer.
     resultOK = pDirDraw->BlitImage(&CPoint(1, 1),
        partsASurfaceNum, &CRect(0, 0, 600, 400));
    if (!resultOK)
        return FALSE;

    // Show the game screen.
    resultOK = pDirDraw->FlipSurfaces();

    // Clear the back buffer.
    pDirDraw->ClearSurface(BACKBUFFERNUM);

*/

	LoadDudeDibs(1);


    ShowPlayer();




    // Blit an image to the back buffer.
    
      
	
	

    // Show the game screen.
    resultOK = pDirDraw->FlipSurfaces();

    // Clear the back buffer.
    /*pDirDraw->ClearSurface(BACKBUFFERNUM);*/




    return TRUE;
}

///////////////////////////////////////////////////////////
// OnKeyDown()
//
// This function handles key presses received by the
// program in the form of WM_KEYDOWN messages.
///////////////////////////////////////////////////////////
void OnKeyDown(WPARAM wParam)
{
    if (wParam == VK_ESCAPE)
    {
        PostMessage(hWindow, WM_CLOSE, 0, 0L);
    }



	switch (wParam) {
	case VK_LEFT : 

		/*direction++;*/

		switch (direction) {
		case 1:
			direction=3;
			break;
		case 2:
			direction = 1;
		break;
		case 3:
			direction = 4;
		break;
		case 4:
			direction = 2;

break;
		}


if (direction > 4 )
			direction = 1;


		ShowPlayer();
		
		break;
	case VK_RIGHT: 

		
				switch (direction) {
		case 1:
			direction=2;
			break;
		case 2:
			direction = 4;
		break;
		case 3:
			direction = 1;
		break;
		case 4:
			direction = 3;

break;
		}

		
		
		ShowPlayer();
		break;
	case VK_UP :

	SetTimer(hWindow, 1, 200, 0);		
		/*animate_player(1) ;		*/
		



		break;
	case VK_DOWN :
		ShowPlayer();
		break;

	}



}

///////////////////////////////////////////////////////////
// OnDestroy()
//
// This function cleans up after the program when the
// application receives a WM_DESTROY message from Windows.
///////////////////////////////////////////////////////////
void OnDestroy()

{

    delete pDirDraw;
}

///////////////////////////////////////////////////////////
// LoadFloorDibs()
//
// This function loads the bitmaps associated with a given
// floor number. These bitmaps include the dungeon parts,
// as well as the three types of monster images for the
// floor.
///////////////////////////////////////////////////////////
void LoadFloorDibs(UINT floor)
{
    // Convert the floor number to a string.
    char s[5];
    wsprintf(s, "%d", floor);

    // Create a file-name CString object.
    CString floorFileName("LEVELXXA.BMP");

    // Add the floor number to the file name.
    if (floor < 10)
    {
        floorFileName.SetAt(5, '0');
        floorFileName.SetAt(6, s[0]);
    }
    else
    {
        floorFileName.SetAt(5, s[0]);
        floorFileName.SetAt(6, s[1]);
    }

    // Construct a CDib object from the "Parts A" bitmap.
    CDib* pDib = new CDib(floorFileName);

    // Set the DirectDraw palette.
    BOOL resultOK = pDirDraw->SetPalette(pDib);

    // Copy the bitmap to the off-screen surface.
    pDirDraw->CopyDibToSurface(partsASurfaceNum,
        &CPoint(0, 0), pDib, &CRect(0, 0, 600, 400));
    delete pDib;
}

void LoadDudeDibs(UINT floor)
{


    // Convert the floor number to a string.
    char s[5];
    wsprintf(s, "%d", floor);

    // Create a file-name CString object.
    CString floorFileName("p256.BMP");

    // Construct a CDib object from the "Parts A" bitmap.
    CDib* pDib = new CDib(floorFileName);

    // Set the DirectDraw palette.
    BOOL resultOK = pDirDraw->SetPalette(pDib);

    // Copy the bitmap to the off-screen surface.
    pDirDraw->CopyDibToSurface(partsASurfaceNum,
        &CPoint(0, 0), pDib, &CRect(0, 0, 640,128 ));
    delete pDib;

}

void ShowPlayer () {



BOOL resultOK;
int pos,framey,framex;
int frame2y,frame2x;



	  BOOL result  = pDirDraw->SetTransparentColors(partsASurfaceNum,255,255);


		 
		 switch (direction) {
		case 1:
			framex = 0;
			framey = 0;
			frame2x = 63;
			frame2y = 63;
			break;
		case 2:
			framex = 256;
			framey = 0;
			frame2x = 319;
			frame2y = 63;
			break;
		case 3:
			framex = 0;
			framey = 64;
			frame2x = 63;
			frame2y = 127;
			break;
		case 4:
			framex = 256;
			framey = 64;
			frame2x = 319;
			frame2y = 127;
			break;
	 }





     
resultOK = pDirDraw->BlitImage(&CPoint(playerx, playery),partsASurfaceNum, 
		 &CRect(framex, framey, frame2x, frame2y));



    

     resultOK = pDirDraw->FlipSurfaces();

    pDirDraw->ClearSurface(BACKBUFFERNUM);


}


void animate_player(int startpos) {

BOOL resultOK;
int pos,framey,framex,i,j;
int frame2y,frame2x,adjust;

int move1,move2;


move1 =0 ;
move2=0;

framex=0;
framey=0;
frame2x =0;
frame2y =0;


adjust =0;	
	




	  BOOL result  = pDirDraw->SetTransparentColors(partsASurfaceNum,255,255);


		if (frame_num == 1 ) {
			adjust =64;
		}
		if (frame_num == 2 ) {
			adjust =128;
		}

		if (frame_num == 3 ) {
			adjust =192;
		}

		if (frame_num == 4 ) {
			adjust =0;
		}



	 switch (direction) {
		case 1:
			framex = 0+adjust;
			framey = 0;
			frame2x = 63+adjust;
			frame2y = 63;

			move1 = -1;
			move2=-1;
			break;
		case 2:
			framex = 256+adjust;
			framey = 0;
			frame2x = 319+adjust;
			frame2y = 63;

		move1 = +1;
			move2=-1;

			break;
		case 3:
			framex = 0+adjust;
			framey = 64;
			frame2x = 63+adjust;
			frame2y = 127;
			move1 = -1;
			move2=+1;
			break;
		case 4:
			framex = 256+adjust;
			framey = 64;
			frame2x = 319+adjust;
			frame2y = 127;
			move1 = +1;
			move2=+1;

			break;
	 }




	playerx =playerx + move1*6;
	playery = playery + move2*6;

	
 
resultOK = pDirDraw->BlitImage(&CPoint(playerx, playery),partsASurfaceNum, 
		 &CRect(framex, framey, frame2x, frame2y));
     resultOK = pDirDraw->FlipSurfaces();



pDirDraw->ClearSurface(BACKBUFFERNUM);





	



}

void delay_s(unsigned char seconds) {

    time_t tstart, tend;
    time( &tstart );
    time( &tend );


    while (difftime (tend, tstart) < (double) seconds) {
       time( &tend );
    }

}


void OnTimer() {


	
    animate_player(1);
	frame_num++;
	if (frame_num > 4 ) {
	frame_num = 1;

	KillTimer(hWindow, 1);
	}



}