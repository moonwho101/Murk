// Dialog.cpp : implementation file
//

#include "stdafx.h"

#include "Dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialog dialog

Dialog::Dialog(CWnd *pParent /*=NULL*/)
    : CDialog(Dialog::IDD, pParent) {
	//{{AFX_DATA_INIT(Dialog)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void Dialog::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Dialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(Dialog, CDialog)
//{{AFX_MSG_MAP(Dialog)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Dialog message handlers
