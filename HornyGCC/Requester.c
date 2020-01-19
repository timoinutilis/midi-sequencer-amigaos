#include <string.h>
#include <stdio.h>

#include <proto/intuition.h>
#include <proto/asl.h>
#include <proto/dos.h>

#include "locale.h"

#include "Strukturen.h"

extern struct UMGEBUNG umgebung;

struct FileRequester *projreq = NULL;
char projdatei[1024] = "";
struct FileRequester *smfreq = NULL;
char smfdatei[1024] = "";
struct FileRequester *sysexreq = NULL;
char sysexdatei[1024] = "";

extern struct Window *hfenster;
extern struct Window *aktfenster;


int32 Frage(STRPTR text, STRPTR knopf) {
	struct EasyStruct struk;
	int32 answ;

	struk.es_StructSize = sizeof(struct EasyStruct);
	struk.es_Flags = 0;
	struk.es_Title = CAT(MSG_0444, "Message");
	struk.es_TextFormat = text;
	struk.es_GadgetFormat = knopf;
	answ = IIntuition->EasyRequestArgs(aktfenster, &struk, NULL, NULL);
	return(answ);
}

void Meldung(STRPTR text) {
	struct EasyStruct struk;

	struk.es_StructSize = sizeof(struct EasyStruct);
	struk.es_Flags = 0;
	struk.es_Title = CAT(MSG_0444, "Message");
	struk.es_TextFormat = text;
	struk.es_GadgetFormat = CAT(MSG_0446, "Okay");
	IIntuition->EasyRequestArgs(aktfenster, &struk, NULL, NULL);
}


void ErstelleAslReqs(void) {
	projreq = (struct FileRequester *)IAsl->AllocAslRequestTags(ASL_FileRequest,
		ASLFR_Window, hfenster,
		ASLFR_SleepWindow, TRUE,
		ASLFR_InitialPattern, "#?.horny",
		ASLFR_DoPatterns, TRUE,
		ASLFR_RejectIcons, TRUE,
		ASLFR_InitialDrawer, umgebung.pfadproj,
		TAG_DONE);
	smfreq = (struct FileRequester *)IAsl->AllocAslRequestTags(ASL_FileRequest,
		ASLFR_Window, hfenster,
		ASLFR_SleepWindow, TRUE,
		ASLFR_InitialPattern, "#?.(mid|midi)",
		ASLFR_DoPatterns, TRUE,
		ASLFR_RejectIcons, TRUE,
		ASLFR_InitialDrawer, umgebung.pfadsmf,
		TAG_DONE);
	sysexreq = (struct FileRequester *)IAsl->AllocAslRequestTags(ASL_FileRequest,
		ASLFR_Window, hfenster,
		ASLFR_SleepWindow, TRUE,
		ASLFR_InitialPattern, "#?.(sysex|syx)",
		ASLFR_DoPatterns, TRUE,
		ASLFR_RejectIcons, TRUE,
		ASLFR_InitialDrawer, umgebung.pfadsysex,
		TAG_DONE);
}

void EntferneAslReqs(void) {
	IAsl->FreeAslRequest((APTR)projreq);
	IAsl->FreeAslRequest((APTR)smfreq);
	IAsl->FreeAslRequest((APTR)sysexreq);
}

void EndungAnfuegen(STRPTR datei, STRPTR end) {
	STRPTR pp;
	
	pp = strrchr(datei, '.');
	if (pp) {
		if (strcmp(pp, end) != 0) strncat(datei, end, 1024);
	} else {
		strncat(datei, end, 1024);
	}
}

BOOL AslProjLaden(void) {
	if (IAsl->AslRequestTags((APTR)projreq,
		ASLFR_TitleText, CAT(MSG_0449, "Load Horny Project"),
		ASLFR_DoSaveMode, FALSE,
		TAG_DONE)) {
		
		strncpy(projdatei, projreq->fr_Drawer, 1024);
		IDOS->AddPart(projdatei, projreq->fr_File, 1024);
		return(TRUE);
	} else {
		return(FALSE);
	}
}

BOOL AslProjSpeichern(void) {
	BPTR file;
	
	if (IAsl->AslRequestTags((APTR)projreq,
		ASLFR_TitleText, CAT(MSG_0450, "Save Horny Project"),
		ASLFR_DoSaveMode, TRUE,
		TAG_DONE)) {
		
		strncpy(projdatei, projreq->fr_Drawer, 1024);
		IDOS->AddPart(projdatei, projreq->fr_File, 1024);
		
		EndungAnfuegen(projdatei, (STRPTR)".horny");

		file = IDOS->Open(projdatei, MODE_OLDFILE);
		if (file) {
			IDOS->Close(file);
			if (!Frage(CAT(MSG_0452, "File with this name already exists"), CAT(MSG_0453, "Overwrite|Cancel"))) return(FALSE);
		}
		return(TRUE);
	} else {
		return(FALSE);
	}
}

BOOL AslSMFLaden(void) {
	if (IAsl->AslRequestTags((APTR)smfreq,
		ASLFR_TitleText, CAT(MSG_0454, "Load MIDI Project"),
		ASLFR_DoSaveMode, FALSE,
		TAG_DONE)) {
		
		strncpy(smfdatei, smfreq->fr_Drawer, 1024);
		IDOS->AddPart(smfdatei, smfreq->fr_File, 1024);
		return(TRUE);
	} else {
		return(FALSE);
	}
}

BOOL AslSMFSpeichern(void) {
	BPTR file;
	
	if (IAsl->AslRequestTags((APTR)smfreq,
		ASLFR_TitleText, CAT(MSG_0455, "Save MIDI Project"),
		ASLFR_DoSaveMode, TRUE,
		TAG_DONE)) {
		
		strncpy(smfdatei, smfreq->fr_Drawer, 1024);
		IDOS->AddPart(smfdatei, smfreq->fr_File, 1024);
		
		EndungAnfuegen(smfdatei, (STRPTR)".mid");

		file = IDOS->Open(smfdatei, MODE_OLDFILE);
		if (file) {
			IDOS->Close(file);
			if (!Frage(CAT(MSG_0452, "File with this name already exists"), CAT(MSG_0453, "Overwrite|Cancel"))) return(FALSE);
		}
		return(TRUE);
	} else {
		return(FALSE);
	}
}

BOOL AslSysExLaden(void) {
	if (IAsl->AslRequestTags((APTR)sysexreq,
		ASLFR_TitleText, CAT(MSG_0456, "Load SysEx File"),
		ASLFR_DoSaveMode, FALSE,
		TAG_DONE)) {
		
		strncpy(sysexdatei, sysexreq->fr_Drawer, 1024);
		IDOS->AddPart(sysexdatei, sysexreq->fr_File, 1024);
		return(TRUE);
	} else {
		return(FALSE);
	}
}

BOOL AslSysExSpeichern(void) {
	BPTR file;
	
	if (IAsl->AslRequestTags((APTR)sysexreq,
		ASLFR_TitleText, CAT(MSG_0457, "Save SysEx File"),
		ASLFR_DoSaveMode, TRUE,
		TAG_DONE)) {
		
		strncpy(sysexdatei, sysexreq->fr_Drawer, 1024);
		IDOS->AddPart(sysexdatei, sysexreq->fr_File, 1024);
		
		EndungAnfuegen(sysexdatei, (STRPTR)".syx");

		file = IDOS->Open(sysexdatei, MODE_OLDFILE);
		if (file) {
			IDOS->Close(file);
			if (!Frage(CAT(MSG_0452, "File with this name already exists"), CAT(MSG_0453, "Overwrite|Cancel"))) return(FALSE);
		}
		return(TRUE);
	} else {
		return(FALSE);
	}
}
