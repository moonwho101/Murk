#include <MMSystem.h>

UINT m_uiThreshold;
UINT m_uiDevID;
BOOL m_bJoyActive;

BOOL InitJoystick();
BOOL JoyToClient();

int OnCreate(LPCREATESTRUCT lpCreateStruct);
void OnSetFocus(CWnd *pOldWnd);
void OnKillFocus(CWnd *pNewWnd);
LONG OnJoyMove(UINT wParam, LONG lParam);
LONG OnJoyButtonDown(UINT wParam, LONG lParam);
