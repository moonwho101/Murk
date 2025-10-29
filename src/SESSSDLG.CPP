//-----------------------------------------------------------------
// Session Selection Dialog Object
// C++ Source - SessSDlg.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------
#include "SessSDlg.h"

//-----------------------------------------------------------------
// MFC Debugging Support
//-----------------------------------------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


//-----------------------------------------------------------------
// Externs
//-----------------------------------------------------------------
extern GUID TICTACTOE_10;

//-----------------------------------------------------------------
// CSessionSelDlg Message Map & Runtime Support
//-----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CSessionSelDlg, CDialog)
  ON_LBN_DBLCLK(IDC_LB_SESSIONS, OnDblClkList)
END_MESSAGE_MAP()

//-----------------------------------------------------------------
// CSessionSelDlg Public Constructor(s)/Destructor
//-----------------------------------------------------------------
CSessionSelDlg::CSessionSelDlg(LPDIRECTPLAY pDirectPlay, CWnd*
  pParent)
  : CDialog(CSessionSelDlg::IDD, pParent),
    m_pDirectPlay(pDirectPlay)
{
}

CSessionSelDlg::~CSessionSelDlg()
{
}

//-----------------------------------------------------------------
// CSessionSelDlg Protected Methods
//-----------------------------------------------------------------
BOOL
CSessionSelDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  // Enumerate DP sessions
  DPSESSIONDESC dpsdDesc;
  ::ZeroMemory(&dpsdDesc, sizeof(DPSESSIONDESC));
  dpsdDesc.dwSize = sizeof(DPSESSIONDESC);
  dpsdDesc.guidSession = TICTACTOE_10;
  if (m_pDirectPlay->EnumSessions(&dpsdDesc, 5000, EnumSessions,
    (LPVOID)this, DPENUMSESSIONS_ALL) == DP_OK)
    m_lbSessions.SetCurSel(0);
  else
    EndDialog(FALSE);

  return TRUE;
}

void
CSessionSelDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  // Initialize listbox member variable
  DDX_Control(pDX, IDC_LB_SESSIONS, m_lbSessions);
}

void
CSessionSelDlg::OnOK()
{
  // Get current listbox selection
  int iIndex = m_lbSessions.GetCurSel();
  if (iIndex != LB_ERR)
  {
    m_dwSelSession = m_lbSessions.GetItemData(iIndex);
    CDialog::OnOK();
  }
}

BOOL FAR PASCAL
CSessionSelDlg::EnumSessions(LPDPSESSIONDESC pdpsdDesc, LPVOID
  pData, LPDWORD pdwTimeOut, DWORD dwFlags)
{
  // Call non-static EnumSessions with session ID and name
  ASSERT((CSessionSelDlg*)pData);
  return ((CSessionSelDlg*)pData)->EnumSessions(*pdpsdDesc,
    dwFlags);
}

BOOL
CSessionSelDlg::EnumSessions(const DPSESSIONDESC& dpsdDesc, DWORD
  dwFlags)
{
  // Check to see if it has timed out
  if (dwFlags & DPESC_TIMEDOUT)
    return FALSE;

  // Add name to listbox and set item data to ID
  int iIndex = m_lbSessions.AddString(dpsdDesc.szSessionName);
  m_lbSessions.SetItemData(iIndex, dpsdDesc.dwSession);

  return TRUE;
}

//-----------------------------------------------------------------
// CSessionSelDlg Message Handler Methods
//-----------------------------------------------------------------
void
CSessionSelDlg::OnDblClkList()
{
  OnOK();
}
