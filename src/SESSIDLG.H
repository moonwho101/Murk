//-----------------------------------------------------------------
// Session Information Dialog Object
// C++ Header - SessIDlg.h
//-----------------------------------------------------------------

#ifndef __SESSIDLG_H__
#define __SESSIDLG_H__

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include <DPlay.h>
#include "Resource.h"

//-----------------------------------------------------------------
// CSessionDlg Class - Session Information Dialog Object
//-----------------------------------------------------------------
class CSessionInfoDlg : public CDialog {
	// Public Types
  public:
	enum {
		IDD = IDD_SESSIONINFO
	};

	// Public Constructor(s)/Destructor
  public:
	CSessionInfoDlg(CWnd *pParent = NULL);
	virtual ~CSessionInfoDlg();

	// Public Methods
  public:
	char *GetName() {
		return m_sName.GetBuffer(0);
	}

	// Protected Methods
  protected:
	virtual void DoDataExchange(CDataExchange *pDX);

	// Private Data
  private:
	CString m_sName;

	// Message Map & Runtime Support
  protected:
	DECLARE_MESSAGE_MAP()
};

#endif
