///////////////////////////////////////////////////////////
// CDIRSND.H -- Header file for the CDirSound class.
///////////////////////////////////////////////////////////

#ifndef __CDIRSND_H
#define __CDIRSND_H

#include <afxwin.h>
#include "cwave.h"
#include <dsound.h>

class CDirSound
{
protected:
    LPDIRECTSOUND m_pDirectSoundObj;
    HWND m_hWindow;
    UINT m_currentBufferNum;
    LPDIRECTSOUNDBUFFER m_bufferPointers[101];
    DWORD m_bufferSizes[101];

public:
    CDirSound(HWND hWnd);
    ~CDirSound();
    UINT CreateSoundBuffer(CWave* pWave);
    BOOL DirectSoundOK();
    BOOL CopyWaveToBuffer(CWave* pWave, UINT bufferNum);
    BOOL PlaySound(UINT bufferNum);
    BOOL StopSound(UINT bufferNum);

protected:
    void ReleaseAll();
};

#endif

