
//  Murk Ver 1.12 By Mark Longo
//  Copyright 1999  , All Rights Reserver.

#include "murkcommon.h"
#include "murkdecl.h"
#include "midistrm.h"

extern void play_random_music();
extern void stop_song();
BOOL InitMID();

void CleanupMIDI();

CMIDIStream *m_pmidiStream;

CMIDIStream *m_pSong1;
CMIDIStream *m_pSong2;
CMIDIStream *m_pSong3;
CMIDIStream *m_pSong4;
int songinplay;

BOOL InitMIDI() {

	// Create and initialize MIDI stream
	m_pSong1 = new CMIDIStream(IDR_MIDS1);
	m_pSong2 = new CMIDIStream(IDR_MIDS2);
	m_pSong3 = new CMIDIStream(IDR_MIDS3);
	m_pSong4 = new CMIDIStream(IDR_MIDS4);
	// Play the stream looped
	//  m_pmidiStream->Play(TRUE);

	//	play_random_music();
	return TRUE;
}

void CleanupMIDI() {
	// Cleanup MIDI stream
	if (m_pSong1) {
		delete m_pSong1;
		m_pSong1 = NULL;
	}
	if (m_pSong2) {
		delete m_pSong2;
		m_pSong2 = NULL;
	}
	if (m_pSong3) {
		delete m_pSong3;
		m_pSong3 = NULL;
	}
	if (m_pSong4) {
		delete m_pSong4;
		m_pSong4 = NULL;
	}
}

void play_random_music() {

	return;
	int pick;

	if (!m_directSoundOK || songinplay != 0)
		return;
	songinplay = 0;

	if (countlevels == 0)
		pick = random_num(4) + 1;
	else
		pick = random_num(5) + 1;

	switch (pick) {
	case 1:
		m_pSong1->Play(TRUE);
		songinplay = 1;
		break;
	case 2:
		m_pSong2->Play(TRUE);
		songinplay = 2;
		break;
	case 3:
		m_pSong3->Play(TRUE);
		songinplay = 3;
		break;
	case 4:
		m_pSong4->Play(TRUE);
		songinplay = 4;
		break;
	case 5:
		songinplay = 0;
		break;
	}
}

void stop_song() {

	return;

	if (!m_directSoundOK || songinplay == 0)
		return;

	switch (songinplay) {
	case 1:
		m_pSong1->Stop();
		break;
	case 2:
		m_pSong2->Stop();
		break;
	case 3:
		m_pSong3->Stop();
		break;
	case 4:
		m_pSong4->Stop();
		break;
	}

	songinplay = 0;
}