//-----------------------------------------------------------------
// Main Frame Window Object
// C++ Header - MainFrm.h
//-----------------------------------------------------------------

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

//-----------------------------------------------------------------
// Inclusions
//-----------------------------------------------------------------
#include <DPlay.h>
#include "Game.h"

//-----------------------------------------------------------------
// CMainFrame Class - Main Frame Window Object
//-----------------------------------------------------------------
class CMainFrame : public CFrameWnd
{
  // Public Constructor(s)/Destructor
public:
                      CMainFrame();
  virtual             ~CMainFrame();

  // Public Methods
public:
  BOOL                Create(const CString& sTitle);

  // Protected Methods
public:
  void                DrawGrid(CDC* pdc);
  void                DrawGridTiles(CDC* pdc);

  // Message Handler Methods
protected:
  afx_msg int         OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void        OnPaint();
  afx_msg void        OnLButtonDown(UINT nFlags, CPoint point);

  // Command Response Methods
protected:
  afx_msg void        OnGameCreate();
  afx_msg void        OnGameConnect();
  afx_msg void        OnGameEnd();
  afx_msg void        OnGameExit();

  // Private Data
private:
  CBitmap       m_bmGrid,
                m_bmTile[3];
  CGame*        m_pGame;

  // Message Map & Runtime Support
protected:
  DECLARE_MESSAGE_MAP()
  DECLARE_DYNCREATE(CMainFrame)
};

#endif
