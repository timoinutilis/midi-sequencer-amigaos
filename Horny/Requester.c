#include <string.h>

#include <intuition/intuition.h>
#include <libraries/asl.h>
#include <dos/dos.h>

#include <proto/intuition.h>
#include <proto/asl.h>
#include <proto/dos.h>

struct FileRequester *projreq=NULL;
char projdatei[300]="";

extern struct Window *hfenster;
extern struct Window *aktfenster;


LONG Frage(STRPTR text, STRPTR knopf) {
	struct EasyStruct struk={
		sizeof(struct EasyStruct),
		0,
		"Meldung", text, knopf,
	};
	return(EasyRequest(aktfenster, &struk, NULL, NULL));
}

void Meldung(STRPTR text) {
	struct EasyStruct struk={
		sizeof(struct EasyStruct),
		0,
		"Meldung", text, "Okay",
	};
	EasyRequest(aktfenster, &struk, NULL, NULL);
}


void ErstelleAslReqs(void) {
	projreq=(struct FileRequester *)AllocAslRequestTags(ASL_FileRequest,
		ASLFR_Window, hfenster,
		ASLFR_SleepWindow, TRUE,
		ASLFR_InitialPattern, "#?.horny",
		ASLFR_DoPatterns, TRUE,
		ASLFR_RejectIcons, TRUE,
		TAG_DONE);
}

void EntferneAslReqs(void) {
	FreeAslRequest((APTR)projreq);
}

void EndungAnfuegen(STRPTR datei, STRPTR end) {
	STRPTR pp;
	
	if (pp=strrchr(datei, '.')) {
		if (strcmp(pp, end)!=0) strncat(datei, end, 299);
	} else {
		strncat(datei, end, 299);
	};
}

BOOL AslProjLaden(void) {
	if (AslRequestTags((APTR)projreq,
		ASLFR_TitleText, "Horny Projekt laden",
		ASLFR_DoSaveMode, FALSE,
		TAG_DONE)) {
		
		strcpy(projdatei, projreq->fr_Drawer);
		AddPart(projdatei, projreq->fr_File, 300);
		
	} else {
		return(FALSE);
	};
}

BOOL AslProjSpeichern(void) {
	BPTR file;
	
	if (AslRequestTags((APTR)projreq,
		ASLFR_TitleText, "Horny Projekt speichern",
		ASLFR_DoSaveMode, TRUE,
		TAG_DONE)) {
		
		strcpy(projdatei, projreq->fr_Drawer);
		AddPart(projdatei, projreq->fr_File, 300);
		
		EndungAnfuegen(projdatei, ".horny");

		if (file=Open(projdatei, MODE_OLDFILE)) {
			Close(file);
			if (!Frage("Datei mit diesem Namen existiert schon", "Überschreiben|Abbruch")) return(FALSE);
		};
		return(TRUE);
	} else {
		return(FALSE);
	};
}
