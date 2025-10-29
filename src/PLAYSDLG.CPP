//-----------------------------------------------------------------
// Player Selection Dialog Object
// C++ Source - PlaySDlg.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include "StdAfx.h"
//-----------------------------------------------------------------
#include "PlaySDlg.h"

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
extern GUID RH_TICTACTOE_10;

//-----------------------------------------------------------------
// CPlayerSelDlg Message Map & Runtime Support
//-----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CPlayerSelDlg, CDialog)
  ON_LBN_DBLCLK(IDC_LB_PLAYERS, OnDblClkList)
END_MESSAGE_MAP()

//-----------------------------------------------------------------
// CPlayerSelDlg Public Constructor(s)/Destructor
//-----------------------------------------------------------------
CPlayerSelDlg::CPlayerSelDlg(LPDIRECTPLAY pDirectPlay, CWnd*
  pParent)
  : CDialog(CPlayerSelDlg::IDD, pParent),
    m_pDirectPlay(pDirectPlay), m_dpidSelPlayer(0)
{
}

CPlayerSelDlg::~CPlayerSelDlg()
{
}

//-----------------------------------------------------------------
// CPlayerSelDlg Protected Methods
//-----------------------------------------------------------------
BOOL
CPlayerSelDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  // Enumerate DP Players
  if (m_pDirectPlay->EnumPlayers(0, EnumPlayers, (LPVOID)this,
    DPENUMPLAYERS_SESSION) == DP_OK)
    m_lbPlayers.SetCurSel(0);
  else
    EndDialog(FALSE);

  return TRUE;
}

void
CPlayerSelDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  // Initialize listbox member variable
  DDX_Control(pDX, IDC_LB_PLAYERS, m_lbPlayers);
}

void
CPlayerSelDlg::OnOK()
{
  // Get current listbox selection
  int iIndex = m_lbPlayers.GetCurSel();
  if (iIndex != LB_ERR)
  {
    m_dpidSelPlayer = (DPID)m_lbPlayers.GetItemData(iIndex);
    CDialog::OnOK();
  }
}

BOOL FAR PASCAL
CPlayerSelDlg::EnumPlayers(DPID dpID, LPSTR pszFriendlyName,
  LPSTR pszFormalName, DWORD dwFlags, LPVOID pData)
{
  // Call non-static EnumPlayers with player ID and name
  ASSERT((CPlayerSelDlg*)pData);
  ((CPlayerSelDlg*)pData)->EnumPlayers((DWORD)dpID,
    pszFriendlyName, pszFormalName);

  return TRUE;
}

void
CPlayerSelDlg::EnumPlayers(DWORD dwID, const CString&
  sFriendlyName, const CString& sFormalName)
{
  // Add name to listbox and set item data to ID
  int iIndex = m_lbPlayers.AddString(sFriendlyName);
  m_lbPlayers.SetItemData(iIndex, dwID);
}

//-----------------------------------------------------------------
// CPlayerSelDlg Message Handler Methods
//-----------------------------------------------------------------
void
CPlayerSelDlg::OnDblClkList()
{
  OnOK();
}
