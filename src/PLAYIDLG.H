//-----------------------------------------------------------------
// Player Information Dialog Object
// C++ Header - PlayIDlg.h
//-----------------------------------------------------------------

#ifndef __PLAYIDLG_H__
#define __PLAYIDLG_H__

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include <DPlay.h>
#include "Resource.h"

//-----------------------------------------------------------------
// CPlayerDlg Class - Player Information Dialog Object
//-----------------------------------------------------------------
class CPlayerInfoDlg : public CDialog
{
  // Public Types
public:
  enum
  {
    IDD = IDD_PLAYERINFO
  };

  // Public Constructor(s)/Destructor
public:
                CPlayerInfoDlg(CWnd* pParent = NULL);
  virtual       ~CPlayerInfoDlg();

  // Public Methods
public:
  char*         GetFriendlyName() { return
                  m_sFriendlyName.GetBuffer(0); }
  char*         GetFormalName() { return
                  m_sFormalName.GetBuffer(0); }

  // Protected Methods
protected:
  virtual void  DoDataExchange(CDataExchange* pDX);

  // Private Data
private:
  CString m_sFriendlyName,
          m_sFormalName;

  // Message Map & Runtime Support
protected:
  DECLARE_MESSAGE_MAP()
};

#endif
