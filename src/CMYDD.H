///////////////////////////////////////////////////////////
// CMyDD.H: The header file to the CMyDirDraw class.
///////////////////////////////////////////////////////////

#ifndef __CMYDD_H
#define __CMYDD_H

#include "cdirdraw.h"

class CMyDirDraw : public CDirDraw
{
public:
	CMyDirDraw(HWND hWnd);
	~CMyDirDraw();
	BOOL FlipToGDISurface();

protected:
	virtual void RepaintOffScreenSurfaces();
};

#endif
