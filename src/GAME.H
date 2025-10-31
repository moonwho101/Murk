#ifndef __GAME_H__
#define __GAME_H__

#include "DPlay.h"

//-----------------------------------------------------------------
// CGame Class - Game Object
//-----------------------------------------------------------------
class CGame : public CObject {
	// Public Types
  public:
	enum TileState {
		tsEMPTY,
		tsX,
		tsO
	};

  public:
	CGame();
	virtual ~CGame();

  public:
	BOOL DPInit();
	void DPCleanup();
	BOOL DPCreateSession();
	BOOL DPConnectSession();
	void DPEndSession();
	void DPEventMsg();

  protected:
	BOOL DPCreateLocalPlayer();
	BOOL DPDestroyLocalPlayer();
	BOOL DPSendTurnMsg(CPoint &ptPoint);
	BOOL DPReceiveTurnMsg(CPoint &ptPoint);
	void DPCreateEventThread();
	static UINT DPEventMsgStart(LPVOID pData);
	BOOL CheckLine(int i, int j, int ii, int jj);
	BOOL IsWinner();
	BOOL IsDraw();

  private:
};

#endif
