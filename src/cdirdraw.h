///////////////////////////////////////////////////////////
// CDIRDRAW.H -- Header file for the CDirDraw class.
///////////////////////////////////////////////////////////

#ifndef __CDIRDRAW_H
#define __CDIRDRAW_H

#include <afxwin.h>
#include "cdib.h"
#include "ddraw.h"

const int BACKBUFFERNUM = 0;

class CDirDraw {
  protected:
	LPDIRECTDRAW m_pDirectDrawObj;
	LPDIRECTDRAWPALETTE m_pDirectDrawPal;
	LPDIRECTDRAWSURFACE m_pPrimarySurface;
	LPDIRECTDRAWSURFACE m_pBackBuffer;
	HWND m_hWindow;
	UINT m_currentSurfaceNum;
	LPDIRECTDRAWSURFACE m_surfacePointers[101];
	CDib *m_surfaceDibPointers[101];
	BOOL m_colorKeySet[101];

  public:
	CDirDraw(HWND hWnd);
	~CDirDraw();
	UINT CreateOffScreenSurface(UINT width, UINT height);
	BOOL SetPalette(CDib *pDib);
	BOOL DirectDrawOK();
	BOOL ClearSurface(UINT surfaceNum);
	BOOL FlipSurfaces();
	BOOL CopyDibToSurface(UINT surfaceNum,
	                      CPoint *dstPoint, CDib *pDib, CRect *srcRect);
	BOOL SetTransparentColors(UINT surfaceNum,
	                          UINT lowColor, UINT hiColor);
	BOOL BlitImage(CPoint *point, UINT surfaceNum, CRect *rect);

  protected:
	BOOL InitDirectDraw();
	BOOL CreateMainSurfaces();
	void ReleaseAll();
	void RestoreOffScreenSurfaces();
	virtual void RepaintOffScreenSurfaces();
};

#endif
