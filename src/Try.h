#if !defined(AFX_TRY_H__7EA2E774_2D81_11D2_B41E_0080C86A8838__INCLUDED_)
#define AFX_TRY_H__7EA2E774_2D81_11D2_B41E_0080C86A8838__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Try.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Try dialog

class Try : public CDialog
{
// Construction
public:
	Try(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(Try)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Try)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Try)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRY_H__7EA2E774_2D81_11D2_B41E_0080C86A8838__INCLUDED_)
