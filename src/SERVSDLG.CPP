//-----------------------------------------------------------------
// Server Selection Dialog Object
// C++ Source - ServSDlg.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------
#include "ServSDlg.h"

//-----------------------------------------------------------------
// MFC Debugging Support
//-----------------------------------------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------
// CServerSelDlg Message Map & Runtime Support
//-----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CServerSelDlg, CDialog)
  ON_LBN_DBLCLK(IDC_LB_SERVERS, OnDblClkList)
END_MESSAGE_MAP()

//-----------------------------------------------------------------
// CServerSelDlg Public Constructor(s)/Destructor
//-----------------------------------------------------------------
CServerSelDlg::CServerSelDlg(CWnd* pParent)
  : CDialog(CServerSelDlg::IDD, pParent), m_pguidSelServer(NULL)
{
}

CServerSelDlg::~CServerSelDlg()
{
}

//-----------------------------------------------------------------
// CServerSelDlg Protected Methods
//-----------------------------------------------------------------
BOOL
CServerSelDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  // Enumerate DP servers
  if (::DirectPlayEnumerate(EnumServers, (LPVOID)this) == DP_OK)
    m_lbServers.SetCurSel(0);
  else
    EndDialog(FALSE);

  return TRUE;
}

void
CServerSelDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  // Initialize listbox member variable
  DDX_Control(pDX, IDC_LB_SERVERS, m_lbServers);
}

void
CServerSelDlg::OnOK()
{
  // Get current listbox selection
  int iIndex = m_lbServers.GetCurSel();
  if (iIndex != LB_ERR)
  {
    m_pguidSelServer = (LPGUID)m_lbServers.GetItemData(iIndex);
    CDialog::OnOK();
  }
}

BOOL FAR PASCAL
CServerSelDlg::EnumServers(LPGUID pguidID, LPSTR pszName, DWORD
  dwMajorVersion, DWORD dwMinorVersion, LPVOID pData)
{
  // Call non-static EnumServers with server ID and name
  ASSERT((CServerSelDlg*)pData);
  ((CServerSelDlg*)pData)->EnumServers((DWORD)pguidID, pszName);

  return TRUE;
}

void
CServerSelDlg::EnumServers(DWORD dwID, const CString& sName)
{
  // Add name to listbox and set item data to ID
  int iIndex = m_lbServers.AddString(sName);
  m_lbServers.SetItemData(iIndex, dwID);
}

//-----------------------------------------------------------------
// CServerSelDlg Message Handler Methods
//-----------------------------------------------------------------
void
CServerSelDlg::OnDblClkList()
{
  OnOK();
}
