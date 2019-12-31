#include <stdio.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <exec/exec.h>

#include <proto/intuition.h>
#include <proto/exec.h>

#include "Strukturen.h"
#include "Midi.h"
#include "Sequenzen.h"
#include "MidiEdit.h"
#include "Gui.h"
#include "Gui2.h"
#include "Menu.h"
#include "Fenster.h"
#include "Disk.h"
#include "Requester.h"


#define QUALIFIER_SHIFT 0x03
#define QUALIFIER_ALT 0x30
#define QUALIFIER_CTRL 0x08

WORD snum=0;

struct SEQUENZ *wahlseq=NULL;
struct LIED lied;
struct LOOP loop={0, 0, FALSE};

extern struct Window *hfenster;
extern struct Gadget *gadget[12];
extern BYTE midisig;

extern LONG takt;
extern WORD bpm;
extern struct SPUR spur[128];
extern struct SPURTEMP sp[128];
extern struct METRONOM metro;

extern GUI gui;

extern char projdatei[300];

void Frisch(void) {
	WORD s;
	KeinePosition();
	for (s=0; s<lied.spuranz; s++) {
		if (sp[s].anders) {
			ZeichneSequenzen(s);
			sp[s].anders=FALSE;
		};
	};
	ZeichnePosition(takt);
}

void Aufnehmen() {
	BOOL stopp=FALSE;
	WORD s;
	struct IntuiMessage *mes;

	for(s=0; s<lied.spuranz; s++) AbspielenVorbereiten(s);
	SetGadgetAttrs(gadget[3], hfenster, NULL, GA_Selected, TRUE, TAG_DONE);
	ResetteZeit();
	do {
		AktualisiereTakt();
		if (metro.rec) SpieleMetronom();
		for(s=0; s<lied.spuranz; s++) {
			if (s!=snum) SpurAbspielen(s);
		};
		SpurAufnehmen(snum);
		ZeichnePosition(takt);
		if (mes=(struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
			switch (mes->Class) {
				case IDCMP_VANILLAKEY:
				if (mes->Code==32) stopp=TRUE;
				break;
				
				case IDCMP_GADGETUP:
				stopp=TRUE;
				break;
			};
			ReplyMsg((struct Message *)mes);
		};
	} while (!stopp);
	SetGadgetAttrs(gadget[3], hfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	SetGadgetAttrs(gadget[2], hfenster, NULL, GA_Selected, FALSE, TAG_DONE);

	for (s=0; s<lied.spuranz; s++) SpurAbklingen(s);
	wahlseq=NeueSequenzEinordnen(snum);
	NichtsMarkieren();
	wahlseq->markiert=TRUE;
	Frisch();
	ZeichneInfobox(snum, wahlseq);
	AktualisiereGadgets();
}

void Abspielen() {
	BOOL stopp=FALSE;
	WORD s;
	struct IntuiMessage *mes;
	
	Balken(0, 101, 18, 600, 19);
	for(s=0; s<lied.spuranz; s++) AbspielenVorbereiten(s);
	if (loop.aktiv) {
		for(s=0; s<lied.spuranz; s++) LoopVorbereiten(s);
	};
	SetGadgetAttrs(gadget[2], hfenster, NULL, GA_Selected, TRUE, TAG_DONE);
	ResetteZeit();
	do {
		AktualisiereTakt();
		if (loop.aktiv && (takt>=loop.ende)) {
			takt=loop.start;
			for (s=0; s<lied.spuranz; s++) {
				SpurAbklingen(s);
				spur[s].aktseq=sp[s].loopseq;
				spur[s].aktevbl=sp[s].loopevbl;
				spur[s].aktevnum=sp[s].loopevnum;
			};
		};
		if (metro.play) SpieleMetronom();
		for(s=0; s<lied.spuranz; s++) SpurAbspielen(s);
		TesteMidiThru(snum);
		ZeichnePosition(takt);
		if (mes=(struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
			switch (mes->Class) {
				case IDCMP_VANILLAKEY:
				if (mes->Code==32) stopp=TRUE;
				break;
				
				case IDCMP_GADGETUP:
				stopp=TRUE;
				break;
			};
			ReplyMsg((struct Message *)mes);
		};
	} while (!stopp);
	SetGadgetAttrs(gadget[2], hfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	SetGadgetAttrs(gadget[3], hfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	for(s=0; s<lied.spuranz; s++) SpurAbklingen(s);
}

void AktualisiereEditFeld(BOOL zl) {
	KeinePosition();
	ZeichneSpuren(snum, TRUE, TRUE);
	if (zl) ZeichneZeitleiste();
	ZeichnePosition(takt);
	AktualisiereGadgets();
}

void haupt(void) {
	BOOL beendet=FALSE;
	BOOL verlassen=FALSE;
	WORD s;
	WORD neus;
	WORD n;
	struct Gadget *g;
	struct IntuiMessage *mes;
	LONG alttakt;
	LONG neutakt;
	ULONG var;
	BYTE greifp;

	ErstelleHauptfenster();
	ErstelleCamdRealTime();
	ErstelleLinks();
	ErstelleAslReqs();
	
	InitLied();
	
	ZeichneSpuren(snum, TRUE, TRUE);
	ZeichnePosition(takt);
	ZeichneZeitleiste();
	ZeichneInfobox(snum, NULL);
	ErstelleGadgets();
	ErstelleMenu();
	
	for (s=0; s<128; s++) sp[s].anders=FALSE;
	
	do {
		Wait((1L << hfenster->UserPort->mp_SigBit) | (1L<<midisig));
		
		TesteMidiThru(snum);

		while (mes=(struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
			switch (mes->Class) {
				case IDCMP_CLOSEWINDOW:
				if (Frage("Programm wirklich beenden?", "Ja|Nein")) beendet=TRUE;
				break;
				
				case IDCMP_SIZEVERIFY:
				KeinePosition();
				EntferneGadgets();
				break;
				
				case IDCMP_NEWSIZE:
				BildFrei();
				ZeichneSpuren(snum, TRUE, TRUE);
				ZeichnePosition(takt);
				ZeichneZeitleiste();
				ZeichneInfobox(snum, wahlseq);
				ErstelleGadgets();
				break;
				
				case IDCMP_GADGETUP:
				g=(struct Gadget *)mes->IAddress;
				switch (g->GadgetID) {
					case 0: takt=0; ZeichnePosition(0); break;
					case 1: takt=loop.start; ZeichnePosition(takt); break;
					case 2: Abspielen(); break;
					case 3: Aufnehmen(); break;
					
					case 4:
					gui.takt=mes->Code<<10;
					AktualisiereEditFeld(TRUE);
					break;
					
					case 5:
					gui.spur=mes->Code;
					KeinePosition(); ZeichneSpuren(snum, TRUE, TRUE);
					ZeichnePosition(takt);
					break;
					
					case 6: if (gui.tab>4) {gui.tab-=4; AktualisiereEditFeld(TRUE)}; break;
					case 7: gui.tab+=4; AktualisiereEditFeld(TRUE); break;
					case 8: if (gui.sph>16) {gui.sph-=4; AktualisiereEditFeld(FALSE)}; break;
					case 9: gui.sph+=4; AktualisiereEditFeld(FALSE); break;
					case 10: GetAttr(GA_Selected, g, &var); metro.rec=(BOOL)var; break;
					case 11: GetAttr(GA_Selected, g, &var); metro.play=(BOOL)var; break;
					case 12: GetAttr(GA_Selected, g, &var); loop.aktiv=(BOOL)var; break;
				}
				break;

				case IDCMP_MOUSEBUTTONS:
				if (mes->Code==104) {
					switch (TestePunktBereich(mes->MouseX, mes->MouseY)) {
						case 1:
						takt=PunktPosition(mes->MouseX);
						ZeichnePosition(takt);
						verlassen=FALSE;
						alttakt=takt;
						ReplyMsg((struct Message *)mes);
						do {
							WaitPort(hfenster->UserPort);
							while (mes=(struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
								switch (mes->Class) {
									case IDCMP_MOUSEMOVE:
									neutakt=PunktPosition(mes->MouseX);
									if (alttakt!=neutakt) {
										if (neutakt>takt) {
											loop.start=takt; loop.ende=neutakt;
											loop.aktiv=TRUE;
										} else {
											loop.start=0; loop.ende=0;
											loop.aktiv=FALSE;
										};
										ZeichneZeitleiste();
										alttakt=neutakt;
									};
									break;
									
									case IDCMP_MOUSEBUTTONS: verlassen=TRUE; break;
								};
								ReplyMsg((struct Message *)mes);
							};
						} while (!verlassen);
						mes=NULL;
						AktualisiereGadgets();
						break;
						
						case 2:
						snum=PunktSpur(mes->MouseY);
						if ((snum>=lied.spuranz) && (lied.spuranz<128)) {
							lied.spuranz++; snum=lied.spuranz-1;
							spur[snum].channel=spur[snum-1].channel+1;
							if (spur[snum].channel>15) spur[snum].channel=0;
							KeinePosition();
							ZeichneSequenzen(snum);
							ZeichnePosition(takt);
						};
						if ((mes->MouseX-hfenster->BorderLeft>=30) && (mes->MouseX-hfenster->BorderLeft<=46)) {
							if (mes->Qualifier & QUALIFIER_ALT) {
								for (s=0; s<lied.spuranz; s++) spur[s].mute=FALSE;
							} else {
								if (mes->Qualifier & QUALIFIER_SHIFT) {
									for (s=0; s<lied.spuranz; s++) spur[s].mute=TRUE;
								};
								spur[snum].mute= !spur[snum].mute;
							};
						};
						ZeichneSpuren(snum, TRUE, FALSE);
						ZeichneInfobox(snum, wahlseq);
						AktualisiereGadgets();
						SendeInstrument(snum);
						verlassen=FALSE;
						s=snum; if (s>=lied.spuranz) s=lied.spuranz-1;
						ReplyMsg((struct Message *)mes);
						KeinePosition();
						do {
							WaitPort(hfenster->UserPort);
							while (mes=(struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
								switch (mes->Class) {
									case IDCMP_MOUSEMOVE:
									snum=PunktSpur(mes->MouseY); if (snum>=lied.spuranz) snum=lied.spuranz-1;
									if (snum!=s) {
										SpurenTauschen(s, snum);
										ZeichneSpuren(snum, TRUE, FALSE);
										ZeichneSequenzen(snum);
										ZeichneSequenzen(s);
										s=snum;
									};
									break;
									
									case IDCMP_MOUSEBUTTONS: verlassen=TRUE; break;
								};
								ReplyMsg((struct Message *)mes);
							};
						} while (!verlassen);
						ZeichnePosition(takt);
						mes=NULL;
						break;
						
						case 3:
						s=PunktSpur(mes->MouseY);
						takt=PunktPosition(mes->MouseX);
						if (wahlseq=TaktSequenz(s, takt, &greifp)) {
							if (gui.tab<8) greifp=0;
							if (!wahlseq->markiert) {
								if (!(mes->Qualifier & QUALIFIER_SHIFT)) NichtsMarkieren();
							};
							wahlseq->markiert=TRUE; sp[s].anders=TRUE;
							if (mes->Qualifier & QUALIFIER_ALT) {
								greifp=0;
								if (mes->Qualifier & QUALIFIER_SHIFT) MarkSequenzenKopieren() else MarkSequenzenAlias();
							};
							Frisch();
							verlassen=FALSE;
							alttakt=PunktPosition(mes->MouseX);
							ReplyMsg((struct Message *)mes);
							do {
								WaitPort(hfenster->UserPort);
								while (mes=(struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
									switch (mes->Class) {
										case IDCMP_MOUSEMOVE:
										neutakt=PunktPosition(mes->MouseX);
										if (greifp==0) { //Sequenz verschieben
											neus=PunktSpur(mes->MouseY);
											if (neus>=lied.spuranz) neus=lied.spuranz-1;
											if (s==neus) {
												for (n=0; n<lied.spuranz; n++) MarkSequenzenVerschieben(n, 0, neutakt-alttakt);
											} else if (s>neus) {
												for (n=0; n<lied.spuranz; n++) {
													MarkSequenzenVerschieben(n, neus-s, neutakt-alttakt);
												};
											} else if (s<neus) {
												for (n=lied.spuranz-1; n>=0; n--) {
													MarkSequenzenVerschieben(n, neus-s, neutakt-alttakt);
												};
											};
										};
										if (greifp==1) MarkSequenzenStartVerschieben(neutakt-alttakt);
										if (greifp==2) MarkSequenzenEndeVerschieben(neutakt-alttakt);
										if ((alttakt!=neutakt) || (s!=neus)) Frisch();
										alttakt=neutakt;
										s=neus;
										break;
										
										case IDCMP_MOUSEBUTTONS: verlassen=TRUE; break;
									};
									ReplyMsg((struct Message *)mes);
								};
							} while (!verlassen);
							mes=NULL;
							AktualisiereGadgets();
						} else {
							if (!(mes->Qualifier & QUALIFIER_SHIFT)) {NichtsMarkieren(); Frisch()};
						};
						ZeichneInfobox(snum, wahlseq);
						break;
						
						case 4:
						n=TestePunktInfo(mes->MouseX, mes->MouseY);
						if (n==0) StringFenster(hfenster, lied.name, 60);
						if (n==1) bpm=IntegerFenster(hfenster, bpm, 60, 300);
						if (n==2) {
							lied.taktanz=IntegerFenster(hfenster, lied.taktanz, 40, 32000);
							AktualisiereGadgets();
						};
						if (n==10) {
							StringFenster(hfenster, spur[snum].name, 30);
							ZeichneSpuren(snum, TRUE, FALSE);
						};
						if (n==11) {
							ChannelPortFenster(hfenster, &spur[snum].channel, &spur[snum].port);
							ZeichneSpuren(snum, TRUE, FALSE);
							KeinePosition();
							ZeichneSequenzen(snum);
							ZeichnePosition(takt);
						};
						if (n==12) {
							InstrumentenFenster(hfenster, &spur[snum].bank, &spur[snum].prog);
							SendeInstrument(snum);
						};
						if (n==13) spur[snum].shift=IntegerFenster(hfenster, spur[snum].shift, -1024, 1024);
						if (wahlseq) {
							if (n==20) {
								if (!wahlseq->aliasorig) {
									StringFenster(hfenster, wahlseq->name, 30);
								} else {
									StringFenster(hfenster, wahlseq->aliasorig->name, 30);
								};
								KeinePosition(); ZeichneSpuren(snum, FALSE, TRUE); ZeichnePosition(takt);
							};
							if (n==21) {
								wahlseq->trans=IntegerFenster(hfenster, wahlseq->trans, -128, 127);
								KeinePosition(); ZeichneSequenzen(wahlseq->spur); ZeichnePosition(takt);
							};
						};
						ZeichneInfobox(snum, wahlseq);
						break;
					};
				};
				break;
			
				case IDCMP_RAWKEY:
				//printf("RKey: %ld\n", mes->Code);
				switch (mes->Code) {
					case 89: PrintEvents(wahlseq); break;
				};
				break;

				case IDCMP_VANILLAKEY:
				//printf("VKey: %ld\n", mes->Code);
				switch (mes->Code) {
					case 48: takt=0; ZeichnePosition(0); break;
					case 32: Abspielen(); break;
					case 13: Aufnehmen(); break;
	
					case 127:
					MarkSequenzenEntfernen(); Frisch(); wahlseq=NULL; ZeichneInfobox(snum, NULL);
					break;
				};
				break;
				
				case IDCMP_MENUPICK:
				switch (MenuPunkt(mes->Code)) {
					case MENU_UEBER:
					Meldung("Horny Version 0.1\nMidi-Sequencer\n\n©2003 Inutilis Software\nEntwickelt von Timo Kloss\n\nhttp://www.inutilis.de/horny/\neMail: Timo@inutilis.de");
					break;
					
					case MENU_ENDE:
					if (Frage("Programm wirklich beenden?", "Ja|Nein")) beendet=TRUE;
					break;
					
					case MENU_NEU:
					if (Frage("Wirklich neues Projekt beginnen?", "Ja|Nein")) {
						for(s=0; s<128; s++) SpurSequenzenEntfernen(s);
						projdatei[0]=0;
						InitLied(); InitGuiWerte();
						snum=0; wahlseq=NULL;
						AktualisiereEditFeld(TRUE);
						ZeichneInfobox(snum, NULL);
					};
					break;
					
					case MENU_LADEN:
					if (AslProjLaden()) {
						SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
						Laden(projdatei);
						snum=0; wahlseq=NULL; bpm=lied.bpm;
						AktualisiereEditFeld(TRUE);
						AktualisiereGadgets();
						ZeichneInfobox(snum, NULL);
						for (s=0; s<lied.spuranz; s++) SendeInstrument(s);
						ClearPointer(hfenster);
					};
					break;
					
					case MENU_SPEICHERN:
					lied.bpm=bpm;
					SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
					if (projdatei[0]) {
						Speichern(projdatei);
					} else {
						if (AslProjSpeichern()) {
							Speichern(projdatei);
						};
					};
					ClearPointer(hfenster);
					break;

					case MENU_SPEICHERNALS:
					if (AslProjSpeichern()) {
						SetWindowPointer(hfenster, WA_BusyPointer, TRUE, TAG_DONE);
						Speichern(projdatei);
						ClearPointer(hfenster);
					};
					break;
					
					case MENU_SPR_LOESCHEN:
					if (Frage("Spur wirklich löschen?", "Ja|Nein")) {
						if (SpurLoeschen(snum)) {
							if (snum>=lied.spuranz) snum=lied.spuranz-1;
							KeinePosition();
							ZeichneSpuren(snum, TRUE, TRUE);
							ZeichnePosition(takt);
							wahlseq=NULL; ZeichneInfobox(snum, NULL);
						};
					};
					break;

					case MENU_SEQ_LOESCHEN:
					MarkSequenzenEntfernen(); Frisch(); wahlseq=NULL; ZeichneInfobox(snum, NULL);
					break;

					case MENU_SEQ_QUANT:
					if (n=QuantisierungsFenster(hfenster)) {
						MarkSequenzenQuantisieren(n); Frisch();
					};
					break;
					
					case MENU_SEQ_SCHNEIDEN:
					MarkSequenzenZerschneiden(takt); Frisch();
					break;
					
					case MENU_SEQ_VERBINDEN:
					MarkSequenzenVerbinden(); Frisch();
					break;
				}
				break;
			};
			if (mes) ReplyMsg((struct Message *)mes);
		};
	} while (!beendet);
	
	for(s=0; s<128; s++) SpurSequenzenEntfernen(s);

	EntferneAslReqs();
	EntferneLinks();
	EntferneCamdRealTime();
	EntferneGadgets();
	EntferneMenu();
	EntferneHauptfenster();
}

