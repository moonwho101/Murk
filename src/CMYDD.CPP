///////////////////////////////////////////////////////////
// CMyDD.CPP: The implementation file for the CMyDirDraw
//            class.
///////////////////////////////////////////////////////////

#include "cmydd.h"

CMyDirDraw::CMyDirDraw(HWND hWnd) : CDirDraw(hWnd) {
}

CMyDirDraw::~CMyDirDraw() {
}

BOOL CMyDirDraw::FlipToGDISurface() {
	HRESULT result = m_pDirectDrawObj->FlipToGDISurface();
	if (result != DD_OK)
		return FALSE;

	return TRUE;
}

void CMyDirDraw::RepaintOffScreenSurfaces() {
	ClearSurface(BACKBUFFERNUM);
	FlipSurfaces();
	ClearSurface(BACKBUFFERNUM);
}
