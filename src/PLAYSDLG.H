//-----------------------------------------------------------------
// Player Selection Dialog Object
// C++ Header - PlaySDlg.h
//-----------------------------------------------------------------

#ifndef __PLAYSDLG_H__
#define __PLAYSDLG_H__

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include <DPlay.h>
#include "Resource.h"

//-----------------------------------------------------------------
// CPlayerDlg Class - Player Selection Dialog Object
//-----------------------------------------------------------------
class CPlayerSelDlg : public CDialog
{
  // Public Types
public:
  enum
  {
    IDD = IDD_PLAYERSEL
  };

  // Public Constructor(s)/Destructor
public:
                          CPlayerSelDlg(LPDIRECTPLAY pDirectPlay,
                            CWnd* pParent = NULL);
  virtual                 ~CPlayerSelDlg();

  // Public Methods
public:
  DPID                    GetSelPlayer() const
                            { return m_dpidSelPlayer; }

  // Protected Methods
protected:
  virtual BOOL            OnInitDialog();
  virtual void            DoDataExchange(CDataExchange* pDX);
  virtual void            OnOK();
  static BOOL FAR PASCAL  EnumPlayers(DPID dpID, LPSTR
                            pszFriendlyName, LPSTR pszFormalName,
                            DWORD dwFlags, LPVOID lpData);
  void                    EnumPlayers(DWORD dwID, const CString&
                            sFriendlyName, const CString&
                            sFormalName);

  // Message Handler Methods
protected:
  afx_msg void            OnDblClkList();

  // Private Data
private:
  LPDIRECTPLAY  m_pDirectPlay;
  CListBox      m_lbPlayers;
  DPID          m_dpidSelPlayer;

  // Message Map & Runtime Support
protected:
  DECLARE_MESSAGE_MAP()
};

#endif
