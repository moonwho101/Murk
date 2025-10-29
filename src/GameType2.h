#if !defined(AFX_GAMETYPE2_H__EDBEDD44_2D78_11D2_B41E_0080C86A8838__INCLUDED_)
#define AFX_GAMETYPE2_H__EDBEDD44_2D78_11D2_B41E_0080C86A8838__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GameType2.h : header file
//
#include "Resource.h"
/////////////////////////////////////////////////////////////////////////////
// CGameType2 dialog

class CGameType2 : public CDialog
{
// Construction
public:
	CGameType2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data

	enum { IDD = IDD_GAMETYPE };


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameType2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGameType2)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMETYPE2_H__EDBEDD44_2D78_11D2_B41E_0080C86A8838__INCLUDED_)
