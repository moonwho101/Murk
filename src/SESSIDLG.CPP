//-----------------------------------------------------------------
// Session Information Dialog Object
// C++ Source - SessIDlg.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------
#include "SessIDlg.h"

//-----------------------------------------------------------------
// MFC Debugging Support
//-----------------------------------------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------
// CSessionInfoDlg Message Map & Runtime Support
//-----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CSessionInfoDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------
// CSessionInfoDlg Public Constructor(s)/Destructor
//-----------------------------------------------------------------
CSessionInfoDlg::CSessionInfoDlg(CWnd *pParent)
    : CDialog(CSessionInfoDlg::IDD, pParent) {
}

CSessionInfoDlg::~CSessionInfoDlg() {
}

//-----------------------------------------------------------------
// CSessionInfoDlg Protected Methods
//-----------------------------------------------------------------
void CSessionInfoDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);

	// Establish data exchange between text box and string member
	DDX_Text(pDX, IDC_EB_NAME, m_sName);
}
