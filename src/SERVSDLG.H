//-----------------------------------------------------------------
// Server Selection Dialog Object
// C++ Header - ServSDlg.h
//-----------------------------------------------------------------

#ifndef __SERVSDLG_H__
#define __SERVSDLG_H__

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include <DPlay.h>
#include "Resource.h"

//-----------------------------------------------------------------
// CServerSelDlg Class - Server Selection Dialog Object
//-----------------------------------------------------------------
class CServerSelDlg : public CDialog {
	// Public Types
  public:
	enum {
		IDD = IDD_SERVERSEL
	};

	// Public Constructor(s)/Destructor
  public:
	CServerSelDlg(CWnd *pParent = NULL);
	virtual ~CServerSelDlg();

	// Public Methods
  public:
	LPGUID GetSelServer() const {
		return m_pguidSelServer;
	}

	// Protected Methods
  protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual void OnOK();
	static BOOL FAR PASCAL EnumServers(LPGUID pguidID, LPSTR pszName, DWORD dwMajorVersion, DWORD dwMinorVersion, LPVOID pData);
	void EnumServers(DWORD dwID, const CString &
	                                 sName);

	// Message Handler Methods
  protected:
	afx_msg void OnDblClkList();

	// Private Data
  private:
	CListBox m_lbServers;
	LPGUID m_pguidSelServer;

	// Message Map & Runtime Support
  protected:
	DECLARE_MESSAGE_MAP()
};

#endif
