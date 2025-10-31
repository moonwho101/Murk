#if !defined(AFX_DIALOG_H__7EA2E775_2D81_11D2_B41E_0080C86A8838__INCLUDED_)
#define AFX_DIALOG_H__7EA2E775_2D81_11D2_B41E_0080C86A8838__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Dialog.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Dialog dialog

class Dialog : public CDialog {
	// Construction
  public:
	Dialog(CWnd *pParent = NULL); // standard constructor

	// Dialog Data

	enum {
		IDD = IDD_DIALOG1
	};

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Dialog)
  protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV support
	                                                 //}}AFX_VIRTUAL

	// Implementation
  protected:
	// Generated message map functions
	//{{AFX_MSG(Dialog)
	// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOG_H__7EA2E775_2D81_11D2_B41E_0080C86A8838__INCLUDED_)
