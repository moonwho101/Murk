//-----------------------------------------------------------------
// Session Selection Dialog Object
// C++ Header - SessSDlg.h
//-----------------------------------------------------------------

#ifndef __SESSSDLG_H__
#define __SESSSDLG_H__

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include <DPlay.h>
#include "Resource.h"

//-----------------------------------------------------------------
// CSessionDlg Class - Session Selection Dialog Object
//-----------------------------------------------------------------
class CSessionSelDlg : public CDialog {
	// Public Types
  public:
	enum {
		IDD = IDD_SESSIONSEL
	};

	// Public Constructor(s)/Destructor
  public:
	CSessionSelDlg(LPDIRECTPLAY pDirectPlay,
	               CWnd *pParent = NULL);
	virtual ~CSessionSelDlg();

	// Public Methods
  public:
	DWORD GetSelSession() const {
		return m_dwSelSession;
	}

	// Protected Methods
  protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual void OnOK();
	static BOOL FAR PASCAL EnumSessions(LPDPSESSIONDESC pdpsdDesc,
	                                    LPVOID pData, LPDWORD pdwTimeOut, DWORD dwFlags);
	BOOL EnumSessions(const DPSESSIONDESC &
	                      dpsdDesc,
	                  DWORD dwFlags);

	// Message Handler Methods
  protected:
	afx_msg void OnDblClkList();

	// Private Data
  private:
	LPDIRECTPLAY m_pDirectPlay;
	CListBox m_lbSessions;
	DWORD m_dwSelSession;

	// Message Map & Runtime Support
  protected:
	DECLARE_MESSAGE_MAP()
};

#endif
