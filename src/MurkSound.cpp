
//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

void play_sound() {

	int t;
	char filename[80];
	char junk[80];

	m_directSoundOK = FALSE;

	m_pDirSound = NULL;
	m_pWave = NULL;

	m_pDirSound = new CDirSound(hWindow);

	if (!m_pDirSound->DirectSoundOK())
		return;

	m_pWave = new CWave("../sound/walk.wav");
	if (!m_pWave->WaveOK())
		return;
	m_bufferNum = m_pDirSound->CreateSoundBuffer(m_pWave);
	if (!m_bufferNum)
		return;

	// Load sound data into the new sound buffer.
	BOOL result =
	    m_pDirSound->CopyWaveToBuffer(m_pWave, m_bufferNum);
	if (!result)
		return;

	srand((unsigned)time(NULL));

	t = random_num(14) + 1;
	sprintf(junk, "%d", t);
	strcpy(filename, "locmus");
	strcat(filename, junk);
	strcat(filename, ".wav");
	m_pMusic = new CWave("../sound/back.wav");
	if (!m_pMusic->WaveOK())
		return;
	m_bufferMusic = m_pDirSound->CreateSoundBuffer(m_pMusic);
	if (!m_bufferMusic)
		return;

	result = m_pDirSound->CopyWaveToBuffer(m_pMusic, m_bufferMusic);

	if (!result)
		return;
	m_pDrip = new CWave("../sound/drip2.wav");
	if (!m_pDrip->WaveOK())
		return;

	// Create a secondary sound buffer.
	m_bufferDrip = m_pDirSound->CreateSoundBuffer(m_pDrip);
	if (!m_bufferDrip)
		return;
	// Load sound data into the new sound buffer.
	result = m_pDirSound->CopyWaveToBuffer(m_pDrip, m_bufferDrip);
	if (!result)
		return;

	m_pScream = new CWave("../sound/orb.wav");
	if (!m_pScream->WaveOK())
		return;

	// Create a secondary sound buffer.
	m_bufferScream = m_pDirSound->CreateSoundBuffer(m_pScream);
	if (!m_bufferScream)
		return;

	result =
	    m_pDirSound->CopyWaveToBuffer(m_pScream, m_bufferScream);
	if (!result)
		return;

	m_pTreasure = new CWave("../sound/Treasure.wav");
	//	m_pTreasure = new CWave("treasure.wav");
	m_bufferTreasure = m_pDirSound->CreateSoundBuffer(m_pTreasure);
	m_pDirSound->CopyWaveToBuffer(m_pTreasure, m_bufferTreasure);

	m_pDead = new CWave("../sound/dead.wav");
	m_bufferDead = m_pDirSound->CreateSoundBuffer(m_pDead);
	m_pDirSound->CopyWaveToBuffer(m_pDead, m_bufferDead);

	m_pWin = new CWave("../sound/win.wav");
	m_bufferWin = m_pDirSound->CreateSoundBuffer(m_pWin);
	m_pDirSound->CopyWaveToBuffer(m_pWin, m_bufferWin);

	m_pReward = new CWave("../sound/item.wav");
	m_bufferReward = m_pDirSound->CreateSoundBuffer(m_pReward);
	m_pDirSound->CopyWaveToBuffer(m_pReward, m_bufferReward);

	m_pBow = new CWave("../sound/throw.wav");
	m_bufferBow = m_pDirSound->CreateSoundBuffer(m_pBow);
	m_pDirSound->CopyWaveToBuffer(m_pBow, m_bufferBow);

	m_pSword = new CWave("../sound/throw2.wav");
	m_bufferSword = m_pDirSound->CreateSoundBuffer(m_pSword);
	m_pDirSound->CopyWaveToBuffer(m_pSword, m_bufferSword);

	m_pHit = new CWave("../sound/impact.wav");
	m_bufferHit = m_pDirSound->CreateSoundBuffer(m_pHit);
	m_pDirSound->CopyWaveToBuffer(m_pHit, m_bufferHit);

	m_pYell = new CWave("../sound/spawn.wav");
	m_bufferYell = m_pDirSound->CreateSoundBuffer(m_pYell);
	m_pDirSound->CopyWaveToBuffer(m_pYell, m_bufferYell);

	m_pMonsterdie = new CWave("../sound/die.wav");
	m_bufferMonsterdie = m_pDirSound->CreateSoundBuffer(m_pMonsterdie);
	m_pDirSound->CopyWaveToBuffer(m_pMonsterdie, m_bufferMonsterdie);

	m_pMumble = new CWave("../sound/mouther.wav");
	m_bufferMumble = m_pDirSound->CreateSoundBuffer(m_pMumble);
	m_pDirSound->CopyWaveToBuffer(m_pMumble, m_bufferMumble);

	m_pDragon = new CWave("../sound/Dragon.wav");
	m_bufferDragon = m_pDirSound->CreateSoundBuffer(m_pDragon);
	m_pDirSound->CopyWaveToBuffer(m_pDragon, m_bufferDragon);

	m_pRegen = new CWave("../sound/alive.wav");
	m_bufferRegen = m_pDirSound->CreateSoundBuffer(m_pRegen);
	m_pDirSound->CopyWaveToBuffer(m_pRegen, m_bufferRegen);

	m_pSkeleton = new CWave("../sound/skeleton.wav");
	m_bufferSkeleton = m_pDirSound->CreateSoundBuffer(m_pSkeleton);
	m_pDirSound->CopyWaveToBuffer(m_pSkeleton, m_bufferSkeleton);

	m_pDorb = new CWave("../sound/deathorb.wav");
	m_bufferDorb = m_pDirSound->CreateSoundBuffer(m_pDorb);
	m_pDirSound->CopyWaveToBuffer(m_pDorb, m_bufferDorb);

	m_pFireball = new CWave("../sound/fireball.wav");
	m_bufferFireball = m_pDirSound->CreateSoundBuffer(m_pFireball);
	m_pDirSound->CopyWaveToBuffer(m_pFireball, m_bufferFireball);

	m_pOuch = new CWave("../sound/ouch.wav");
	m_bufferOuch = m_pDirSound->CreateSoundBuffer(m_pOuch);
	m_pDirSound->CopyWaveToBuffer(m_pOuch, m_bufferOuch);

	m_pButton = new CWave("../sound/button.wav");
	m_bufferButton = m_pDirSound->CreateSoundBuffer(m_pButton);
	m_pDirSound->CopyWaveToBuffer(m_pButton, m_bufferButton);

	m_pSwordThief = new CWave("../sound/tsword.wav");
	m_bufferSwordThief = m_pDirSound->CreateSoundBuffer(m_pSwordThief);
	m_pDirSound->CopyWaveToBuffer(m_pSwordThief, m_bufferSwordThief);

	m_pSwordBarb = new CWave("../sound/bsword.wav");
	m_bufferSwordBarb = m_pDirSound->CreateSoundBuffer(m_pSwordBarb);
	m_pDirSound->CopyWaveToBuffer(m_pSwordBarb, m_bufferSwordBarb);

	m_pEyeswizard = new CWave("../sound/ewizard.wav");
	m_bufferEyeswizard = m_pDirSound->CreateSoundBuffer(m_pEyeswizard);
	m_pDirSound->CopyWaveToBuffer(m_pEyeswizard, m_bufferEyeswizard);

	m_pTadaCleric = new CWave("../sound/tcleric.wav");
	m_bufferTadaCleric = m_pDirSound->CreateSoundBuffer(m_pTadaCleric);
	m_pDirSound->CopyWaveToBuffer(m_pTadaCleric, m_bufferTadaCleric);

	m_pProtection = new CWave("../sound/protection.wav");
	m_bufferProtection = m_pDirSound->CreateSoundBuffer(m_pProtection);
	m_pDirSound->CopyWaveToBuffer(m_pProtection, m_bufferProtection);

	m_directSoundOK = TRUE;
}
