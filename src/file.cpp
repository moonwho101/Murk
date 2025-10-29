//  Murk Ver 1.12 By Mark Longo 
//  Copyright 1999  , All Rights Reserver.
#include "stdafx.h"

#include "murkdecl.h"
#include "murkcommon.h"
#include "registry.h"
extern void RegCrypt(char* user, char* key, char* result);
int save_server_settings(int network);
int read_server_settings();
int save_game();
int load_game();
int is_shareware();


int usejstick;
int usesound;


int save_server_settings(int network) {
	FILE* fp;

	if ((fp = fopen("server.cfg", "wb+")) == NULL) {
	}

	fwrite(&network, sizeof(int), 1, fp);
	fclose(fp);
	return 1;
}


int read_server_settings() {
	FILE* fp;
	int network;

	if ((fp = fopen("server.cfg", "rb")) == NULL) {
		return 0;
	}
	fread(&network, sizeof(int), 1, fp);


	fclose(fp);
	return network;

}


int save_game() {
	FILE* fp;

	if ((fp = fopen("murk.sav", "wb+")) == NULL) {
		return 0;
	}


	fwrite(&countlevels, sizeof(int), 1, fp);
	fwrite(&levelcounter, sizeof(int), 1, fp);
	fwrite(&advancelevel, sizeof(int), 1, fp);
	fwrite(&advanceattrib, sizeof(int), 1, fp);

	fwrite(&PlayerLocation[1], sizeof(struct PlayerDat), 1, fp);
	fclose(fp);


	return 1;
}

int load_game() {
	FILE* fp;

	if ((fp = fopen("murk.sav", "rb")) == NULL) {
		return 0;
	}
	fread(&countlevels, sizeof(int), 1, fp);
	fread(&levelcounter, sizeof(int), 1, fp);
	fread(&advancelevel, sizeof(int), 1, fp);
	fread(&advanceattrib, sizeof(int), 1, fp);
	//   levelcounter--;
	fread(&PlayerLocation[1], sizeof(struct PlayerDat), 1, fp);



	fclose(fp);


	return 1;
}



int is_shareware() {


	CRegistry Registry;
	CString Path;
	CString DefaultTitle;
	char* p;
	char keyresult[100];

	char buffer[100];
	char buffer2[100];
	char work[80];
	int result;



	Path = "SOFTWARE\\Murk\\";

	if (!Registry.VerifyKey(HKEY_LOCAL_MACHINE, Path))  // Check to see if the key exists
		Registry.CreateKey(HKEY_LOCAL_MACHINE, Path);  // if it doesn’t then create it

	Registry.Open(HKEY_LOCAL_MACHINE, Path);  // Open the Registry
	Registry.Read2("Name", DefaultTitle); // Read the default title
	sprintf(buffer, "%s", DefaultTitle);


	//			if (strcmp(buffer,"Jerry Cooper")==0) {

	//				UINT resultclick = MessageBox(hWindow,"You are using a PIRATE key.\nPlease REGISTER MURK!!!","Murk Shareware",MB_OK);
	//				PostMessage(hWindow, WM_CLOSE, 0, 0L);		

	//			}

	Registry.Read2("Key", DefaultTitle); // Read the default title





	sprintf(buffer2, "%s", DefaultTitle);


	Registry.Read2("Sound", DefaultTitle); // Read the default title
	sprintf(work, "%s", DefaultTitle);


	if (strcmp(work, "1") == 0) {
		m_directSoundOK = TRUE;
		usesound = 1;
	}
	else {
		usesound = 0;
		m_directSoundOK = FALSE;
	}

	Registry.Read2("Joystick", DefaultTitle); // Read the default title
	sprintf(work, "%s", DefaultTitle);


	if (strcmp(work, "1") == 0) {
		usejstick = 1;
	}
	else {
		usejstick = 0;
	}


	Registry.Close();


	p = &keyresult[0];
	RegCrypt(buffer, "Murk", p);

	//			debug_me("KEY",keyresult,0,0);


	if (strcmp(buffer2, keyresult) == 0) {

		return 0;
	}

	return 1;
}