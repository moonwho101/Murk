//-----------------------------------------------------------------
// Player Information Dialog Object
// C++ Source - PlayIDlg.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------
#include "PlayIDlg.h"

//-----------------------------------------------------------------
// MFC Debugging Support
//-----------------------------------------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------
// CPlayerInfoDlg Message Map & Runtime Support
//-----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CPlayerInfoDlg, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------
// CPlayerInfoDlg Public Constructor(s)/Destructor
//-----------------------------------------------------------------
CPlayerInfoDlg::CPlayerInfoDlg(CWnd *pParent)
    : CDialog(CPlayerInfoDlg::IDD, pParent) {
}

CPlayerInfoDlg::~CPlayerInfoDlg() {
}

//-----------------------------------------------------------------
// CPlayerInfoDlg Protected Methods
//-----------------------------------------------------------------
void CPlayerInfoDlg::DoDataExchange(CDataExchange *pDX) {
	CDialog::DoDataExchange(pDX);

	// Establish data exchange between text boxes and string members
	DDX_Text(pDX, IDC_EB_FRIENDLYNAME, m_sFriendlyName);
	DDX_Text(pDX, IDC_EB_FORMALNAME, m_sFormalName);
}
