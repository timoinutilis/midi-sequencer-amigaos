#include <stdio.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <intuition/gadgetclass.h>
#include <dos/dostags.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Midi.h"
#include "Spuren.h"
#include "Sequenzen.h"
#include "MidiEdit.h"
#include "Marker.h"
#include "Gui.h"
#include "Gui2.h"
#include "Menu.h"
#include "Fenster.h"
#include "DiskSMF.h"
#include "DiskHorny.h"
#include "Requester.h"
#include "Einstellungen.h"
#include "Umgebung.h"
#include "SysEx.h"
#include "EditorNoten.h"
#include "Clipboard.h"
#include "Mischpult.h"
#include "DTGrafik.h"
#include "Automation.h"
#include "CtrlWandler.h"
#include "Instrumente.h"
#include "Dynamic_Strings.h"

void AktualisiereEditFeld(BOOL spalte, BOOL zl);

extern int16 snum;

extern struct SYSEXUNIT *rootsexunit;
extern struct SYSEXUNIT *wahlsexunit;
extern struct SEQUENZ *wahlseq;
extern struct LIED lied;
extern struct LOOP loop;
extern struct MARKER *wahlmark[];
extern struct MARKER *ltmark;
extern struct MARKER *lkmark;
extern struct GUI gui;
extern struct UMGEBUNG umgebung;
struct MARKER *sprung[20];

extern struct Process *playproc;
extern struct Window *hfenster;
extern struct Window *aktfenster;
extern struct Gadget *gadget[];
extern int8 playerprocsig;

extern struct Window *setfenster;
extern struct Window *envfenster;
extern struct Window *sexfenster;
extern struct Window *mpfenster;
extern struct Window *ccfenster;

extern int32 takt;
extern int32 tick;
extern int8 hornystatus;
extern struct SPUR spur[];
extern struct SPURTEMP sp[];
extern struct KANALTEMP kt[OUTPORTS][16];
extern char projdatei[];
extern char smfdatei[];
extern struct UMGEBUNG umgebung;

extern BOOL beendet;

extern Object *bmo[];


void EntferneLied(void) {
	int16 s;
	int16 p, c;
	
	ClipboardAliasZuReal();
	EntferneAlleMarker();
	EntferneAlleSysExUnits();
	for (s = 0; s < verSPUREN; s++) SpurSequenzenEntfernen(s);
	for (p = 0; p < verOUTPORTS; p++) {
		for (c = 0; c < 16; c++) {
			for (s = 0; s < 8; s++) EntferneAlleAutoPunkte(p, c, s);
		}
	}
	EntferneAlleChangeCtrl();

	lied.phonolithprojekt[0] = 0;
}

void InitLied(void) {
	int16 s;
	uint8 p;
	uint8 n;

	for (s = 0; s < verSPUREN; s++) {
		InitSpur(s);
		sp[s].anders = 0;
		sp[s].neuseq = NULL;
	}

	for (p = 0; p < verOUTPORTS; p++) {
		for (s = 0; s < 16; s++) {
			kt[p][s].aktbank0 = -1;
			kt[p][s].aktbank32 = -1;
			kt[p][s].aktprog = -1;
			for (n = 0; n < 128; n++) kt[p][s].note[n] = FALSE;
		}
	}
	
	strncpy(lied.name, CAT(MSG_0210, "Unnamed"), 128);
	lied.spuranz = 1;
	lied.taktanz = 100;
	lied.phonolithprojekt[0] = 0;
	
	loop.start = 0;
	loop.ende = 16 << VIERTEL;
	
	wahlmark[0] = NULL;
	wahlmark[1] = NULL;
	wahlmark[2] = NULL;
	
	InitMPKanaele();
	InitAutokanaele();
	
	DeaktiviereExtreamSync();
}

void NeuesLied(void) {
	InitLied();
	ErstelleGrundMarker();
	ResetteLMarker();
	InitSequenzInfo();
	wahlsexunit = NeueSysExUnit(CAT(MSG_0211, "Standard"));
	snum = 0;
}



void Stoppen(void) {
	int16 s;
	
	if (hornystatus == STATUS_REC) {
		NichtsMarkieren();
		for(s = 0; s < lied.spuranz; s++) {
			if (sp[s].neuseq) {
				wahlseq = NeueSequenzEinordnen(s);
				wahlseq->markiert = TRUE;
			}
		}
		AktualisiereSpuren(FALSE);
		ZeichneInfobox(8);
		AktualisiereGadgets();
	}
	hornystatus = STATUS_STOP;

	StopExtreamSync();
	
	IIntuition->SetGadgetAttrs(gadget[GAD_T_PLAY], hfenster, NULL, GA_Image, bmo[IMG_PLAY], TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_T_REC], hfenster, NULL, GA_Image, bmo[IMG_REC], TAG_DONE);
	MenuDeaktivieren(0, 4, FALSE);
	MenuItemDeaktivieren(0, 0xF802, FALSE);
	MenuItemDeaktivieren(0, 0xF822, FALSE);
}

void StoppenZero(void) {
	if (hornystatus != STATUS_STOP) {
		Stoppen();
	} else {
		if ((takt > loop.start) && (loop.start >= 0)) takt = loop.start;
		else takt = 0;
		tick = TaktSmpteTicks(takt);
		ResetteLMarker();
		ZeichnePosition(FALSE);
		ZeichneAnzeigen(TRUE);
		
		LocateExtreamSync();
	}
}

void WiedergabeStarten(BOOL remote) {
	if (hornystatus == STATUS_STOP) {
		MenuItemDeaktivieren(0, 0xF802, TRUE);
		MenuItemDeaktivieren(0, 0xF822, TRUE);
		MenuDeaktivieren(0, 4, TRUE);
		
		if (!remote) StartExtreamSync();
		hornystatus = STATUS_PLAY;
		IExec->Signal(&playproc->pr_Task, 1L << playerprocsig);

		IIntuition->SetGadgetAttrs(gadget[GAD_T_PLAY], hfenster, NULL, GA_Image, bmo[IMG_PLAY_A], TAG_DONE);
	}
}

void AufnahmeStarten(void) {
	if (hornystatus == STATUS_STOP) {
		MenuItemDeaktivieren(0, 0xF802, TRUE);
		MenuItemDeaktivieren(0, 0xF822, TRUE);
		MenuDeaktivieren(0, 4, TRUE);
		
		StartExtreamSync();
		hornystatus = STATUS_REC;
		IExec->Signal(&playproc->pr_Task, 1L << playerprocsig);
		
		IIntuition->SetGadgetAttrs(gadget[GAD_T_REC], hfenster, NULL, GA_Image, bmo[IMG_REC_A], TAG_DONE);
	}
}

void SpringeTakt(int32 t) {
	if (t < 0) t = 0;
	if (hornystatus == STATUS_STOP) {
		takt = t;
		tick = TaktSmpteTicks(t);
		ResetteLMarker();
		ZeichneAnzeigen(TRUE);
		
		LocateExtreamSync();
	}
	if ((t < gui.takt) || (t >= gui.takt + ((gui.tasicht - 10) << VIERTEL))) {
		gui.takt = (t - ((gui.tasicht << VIERTEL)/3)) & VIERTELMASKE;
		if (gui.takt < 0) gui.takt = 0;
		AktualisiereEditFeld(FALSE, TRUE);
	} else {
		ZeichnePosition(FALSE);
	}
}

void Springe(int8 n) {
	if (sprung[n]) {
		SpringeTakt(sprung[n]->takt);
	}
}

void TransportKontrolleRaw(uint8 taste) {
	if (umgebung.tastatur == TASTATUR_PC) {
		switch (taste) {
			case 72: // PgUp
			SpringeTakt(NextXMarkerTakt(takt));
			break;
			
			case 73: // PgDown
			SpringeTakt(PrevXMarkerTakt(takt));
			break;
		}
	}
}

void TransportKontrolle(uint8 taste) {
	if ((taste >= '1') && (taste <= '9')) {
		Springe(taste - '1');
	} else {
		if (umgebung.tastatur == TASTATUR_AMIGA) {
			switch (taste) {
				case '+': SpringeTakt(NextXMarkerTakt(takt)); break;
				case '-': SpringeTakt(PrevXMarkerTakt(takt)); break;
				case '[': SpringeTakt(takt - (4 << VIERTEL)); break;
				case ']': SpringeTakt(takt + (4 << VIERTEL)); break;
				case '*':
				if (hornystatus == STATUS_STOP) AufnahmeStarten();
				break;
			}
		} else { // TASTATUR_PC
			switch (taste) {
				case '/': SpringeTakt(takt - (4 << VIERTEL)); break;
				case '*': SpringeTakt(takt + (4 << VIERTEL)); break;
				case '-':
				if (hornystatus == STATUS_STOP) AufnahmeStarten();
				break;
			}
		}
		switch (taste) {
			case '0': StoppenZero(); break;
			
			case ' ':
			if (hornystatus != STATUS_STOP) Stoppen();
			else if (hornystatus == STATUS_STOP) WiedergabeStarten(FALSE);
			break;
			
			case 13: // <RETURN>
			if (hornystatus == STATUS_STOP) WiedergabeStarten(FALSE);
			break;
		}
	}
}

void StartProjekt(STRPTR startdatei) {
	int16 s;
	STRPTR datei;
	
	if (startdatei) datei = startdatei;
	else if (umgebung.startaktiv) datei = umgebung.startproj;
	else datei = NULL;
	
	if (datei) {
		if (hfenster) IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
		if (LadenHorny(datei)) {
			if (startdatei) strncpy(projdatei, startdatei, 1024);
			wahlsexunit = rootsexunit;
			TaktWahlMark(0);
			ResetteLMarker();				

			for (s = 0; s < lied.spuranz; s++) SendeInstrument(s);
			SammleMPKanaele();
			SendeMischpult();
		}
		if (hfenster) IIntuition->ClearPointer(hfenster);
	}
}

void FensterFrontActive(struct Window *fen) {
	IIntuition->WindowToFront(fen);
	IIntuition->ActivateWindow(fen);
}

void ProjektLadenKomplett(void) {
	int16 s;
	
	if (hornystatus > STATUS_STOP) Stoppen();
	IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
	if (!LadenHorny(projdatei)) projdatei[0] = 0;
	FensterTitel(projdatei);
	wahlsexunit = rootsexunit;
	TaktWahlMark(0);
	ResetteLMarker();
	InitSequenzInfo();

	LoescheLinksOben();
	ZeichneAnzeigen(TRUE);
	AktualisiereEditFeld(TRUE, TRUE);
	ZeichneInfobox(0);
	AktualisiereSprungliste();
	AktualisiereSysExGruppenListe();
	AktualisiereSysExMsgListe();
	AktualisiereFunctGadgets();
	AktualisiereMischpult();

    for (s = 0; s < lied.spuranz; s++) SendeInstrument(s);
	SendeMischpult();

	TesteInstrumente();

	IIntuition->ClearPointer(hfenster);
}

void MinMenuKontrolle(uint32 item) {
	int16 s;
	STRPTR strpuf;

	switch (item) {
		case MENU_UEBER:
		OeffneTitel();
		AboutTitel();
		SchliesseTitel();
		break;
		
		case MENU_UMGEBUNG:
		if (!envfenster) ErstelleUmgebungsfenster(); else FensterFrontActive(envfenster);
		break;
		
		case MENU_FENSTERFIX:
		HoleAlleFensterpos();
		SpeichereFensterPos();
		break;
		
		case MENU_ENDE:
		aktfenster = hfenster;
		if (Frage(CAT(MSG_0212, "Really quit program?"), CAT(MSG_0213, "Yes|No"))) beendet = TRUE;
		break;
		
		case MENU_NEU:
		if (Frage(CAT(MSG_0214, "Really start new project?"), CAT(MSG_0213, "Yes|No"))) {
			if (hornystatus > STATUS_STOP) Stoppen();
			EntferneLied();
			NeuesLied();
			InitGuiWerte();
			StartProjekt(NULL);
			projdatei[0] = 0;
			FensterTitel(projdatei);
			AktualisiereEditFeld(TRUE, TRUE);
			ZeichneInfobox(0);
			AktualisiereSprungliste();
			AktualisiereSysExGruppenListe();
			AktualisiereSysExMsgListe();
			AktualisiereFunctGadgets();
			AktualisiereMischpult();
			SendeMischpult();
		}
		break;
		
		case MENU_PROJEINST:
		if (!setfenster) ErstelleEinstellungsfenster(); else FensterFrontActive(setfenster);
		break;
		
		case MENU_SYSEX:
		if (!sexfenster) ErstelleSysExFenster(); else FensterFrontActive(sexfenster);
		break;
		
		case MENU_MISCHPULT:
		if (!mpfenster) ErstelleMPFenster(); else FensterFrontActive(mpfenster);
		break;

		case MENU_CTRLCHANGE:
		if (!ccfenster) ErstelleChangeCtrlFenster(); else FensterFrontActive(ccfenster);
		break;
		
		case MENU_PROGSENDEN:
		for (s = 0; s < lied.spuranz; s++) SendeInstrument(s);
		break;
		
		case MENU_SYSEXSENDEN:
		if (Frage(CAT(MSG_0216, "Should really all SysEx messages\n(except locked ones) be sent?"), CAT(MSG_0213, "Yes|No"))) SendeAlleSysEx();
		break;
		
		case MENU_MIXSENDEN:
		SendeMischpult();
		break;
		
		case MENU_ALLOFFSENDEN:
		Panik();
		break;
		
		case MENU_LADEN:
		if (AslProjLaden()) ProjektLadenKomplett();
		break;
		
		case MENU_SPEICHERN:
		IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
		ClipboardAliaseRechnen(-1);
		SammleMPKanaele();
		if (projdatei[0]) {
			SpeichernHorny(projdatei);
		} else {
			if (AslProjSpeichern()) {
				SpeichernHorny(projdatei);
				FensterTitel(projdatei);
			}
		}
		ClipboardAliaseRechnen(+1);
		IIntuition->ClearPointer(hfenster);
		break;

		case MENU_SPEICHERNALS:
		if (AslProjSpeichern()) {
			IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
			ClipboardAliaseRechnen(-1);
			SammleMPKanaele();
			SpeichernHorny(projdatei);
			ClipboardAliaseRechnen(+1);
			FensterTitel(projdatei);
			IIntuition->ClearPointer(hfenster);
		}
		break;

		case MENU_SPEICHERNALSAUTOLOAD:
		strpuf = String_Copy(NULL, CAT(MSG_0217, "Save as '"));
		strpuf = String_Cat(strpuf, umgebung.startproj);
		strpuf = String_Cat(strpuf, (STRPTR)"'?");
		if (Frage(strpuf, CAT(MSG_0213, "Yes|No"))) {
			IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
			ClipboardAliaseRechnen(-1);
			SammleMPKanaele();
			SpeichernHorny(umgebung.startproj);
			ClipboardAliaseRechnen(+1);
			IIntuition->ClearPointer(hfenster);
		}
		String_Free(strpuf);
		break;
		
		case MENU_IMPORTSMF:
		if (AslSMFLaden()) {
			if (hornystatus > STATUS_STOP) Stoppen();
			IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
			ImportSMF(smfdatei);
			projdatei[0] = 0;
			FensterTitel(smfdatei);
			TaktWahlMark(0);
			ResetteLMarker();
			AktualisiereEditFeld(TRUE, TRUE);
			ZeichneInfobox(0);
			AktualisiereSprungliste();
			AktualisiereSysExGruppenListe();
			AktualisiereSysExMsgListe();
			AktualisiereFunctGadgets();
			for (s = 0; s < lied.spuranz; s++) SendeInstrument(s);
			SendeMischpult();
			AktualisiereMischpult();
			IIntuition->ClearPointer(hfenster);
		}
		break;

		case MENU_EXPORTSMF:
		if (AslSMFSpeichern()) {
			IIntuition->SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
			ExportSMF(smfdatei);
			FensterTitel(smfdatei);
			IIntuition->ClearPointer(hfenster);
		}
		break;
	}
}
