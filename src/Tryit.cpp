// Tryit.cpp : implementation file
//

#include "stdafx.h"
#include "aztec.h"
#include "Tryit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTryit dialog


CTryit::CTryit(CWnd* pParent /*=NULL*/)
	: CDialog(CTryit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTryit)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTryit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTryit)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTryit, CDialog)
	//{{AFX_MSG_MAP(CTryit)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTryit message handlers
