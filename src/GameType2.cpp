// GameType2.cpp : implementation file
//

#include "stdafx.h"

#include "GameType2.h"
#include "Resource.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGameType2 dialog


CGameType2::CGameType2(CWnd* pParent /*=NULL*/)
	: CDialog(CGameType2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGameType2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGameType2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGameType2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGameType2, CDialog)
	//{{AFX_MSG_MAP(CGameType2)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGameType2 message handlers
