#if !defined(AFX_TRYIT_H__7EA2E773_2D81_11D2_B41E_0080C86A8838__INCLUDED_)
#define AFX_TRYIT_H__7EA2E773_2D81_11D2_B41E_0080C86A8838__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Tryit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTryit dialog

class CTryit : public CDialog
{
// Construction
public:
	CTryit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTryit)
	enum { IDD = IDD_GAMETYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTryit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTryit)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRYIT_H__7EA2E773_2D81_11D2_B41E_0080C86A8838__INCLUDED_)
