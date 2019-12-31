#include <stdio.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <intuition/gadgetclass.h>
#include <dos/dostags.h>

#ifdef __amigaos4__
#include <proto/application.h>
#include <libraries/application.h>
#endif

#include "locale.h"

#include "Strukturen.h"
#include "Midi.h"
#include "Projekt.h"
#include "Spuren.h"
#include "Sequenzen.h"
#include "MidiEdit.h"
#include "Marker.h"
#include "Gui.h"
#include "Gui2.h"
#include "Menu.h"
#include "Fenster.h"
#include "Requester.h"
#include "Einstellungen.h"
#include "Umgebung.h"
#include "SysEx.h"
#include "EditorNoten.h"
#include "Instrumente.h"
#include "Clipboard.h"
#include "Mischpult.h"
#include "DTGrafik.h"
#include "Automation.h"
#include "AppWindow.h"
#include "CtrlWandler.h"
#include "Versionen.h"

#ifndef __amigaos4__
#include "newmouse.h"
#endif

void TesteKey(void); // Versionen.c
#ifdef __amigaos4__
void checkPhonolithProject(); //Start.c
#endif


#define QUALIFIER_SHIFT 0x03
#define QUALIFIER_ALT 0x30
#define QUALIFIER_CTRL 0x08

#ifdef __amigaos4__
extern unsigned long start_appID;
extern struct MsgPort *start_appPort;
extern unsigned long start_appSigMask;
#endif


WORD snum = 0;
LONG edittakt = 0;
BYTE mtyp = 0;

struct SEQUENZ *wahlseq = NULL;
struct MARKER *wahlmark[3] = {NULL, NULL, NULL};
struct LIED lied = {"", 0, 0, FALSE, NULL};
struct LOOP loop = {0, 4096, FALSE};

struct Process *playproc = NULL;
struct Process *thruproc = NULL;
extern struct Window *hfenster;
extern struct Screen *hschirm;
extern struct Window *aktfenster;
extern struct Gadget *gadget[];
extern BYTE playsig;
extern BYTE playerprocsig;
extern BYTE thruprocsig;
extern struct MsgPort *syncport;

extern struct Window *setfenster;
extern struct Window *envfenster;
extern struct Window *sexfenster;
extern struct Window *edfenster;
extern struct Window *mpfenster;
extern struct Window *ccfenster;

extern LONG takt;
extern LONG tick;
extern BYTE hornystatus;
extern BOOL tmarkwechsel;
extern struct OUTPORT outport[];
extern struct SPUR spur[];
extern struct SPURTEMP sp[];
extern struct METRONOM metro;
extern struct MARKER *rootmark;
extern struct MARKER *ltmark;
extern struct SEQUENZINFO seqinfo;
extern UBYTE playsigaktion;
extern struct UMGEBUNG umgebung;


extern struct GUI gui;
extern WORD guileiste;

extern char projdatei[];
extern char smfdatei[];

BOOL beendet = FALSE;


void AktualisiereEditFeld(BOOL spalte, BOOL zl) {
	KeinePosition();
	ZeichneSpuren(spalte, TRUE);
	if (zl) {
		ZeichneZeitleiste(TRUE);
		ZeichneSmpteLeiste();
		ZeichneMarkerleisten();
	}
	ZeichnePosition(TRUE);
	ZeichneUebersicht();
	AktualisiereGadgets();
}

void GgfFolgenAbschalten(void) {
	if (gui.folgen && (hornystatus != STATUS_STOP)) {
		if ((takt > gui.takt + (gui.tasicht << VIERTEL)) || (takt < gui.takt)) {
			gui.folgen = FALSE;
			AktualisiereFunctGadgets();
		}
	}
}

//Funktionen der Hauptschleife

void SchleifeGadgets(struct Gadget *g, UWORD code) {
	ULONG var;
	
	switch (g->GadgetID) {
		case GAD_T_PREV: SpringeTakt(PrevXMarkerTakt(takt)); break;
		case GAD_T_NEXT: SpringeTakt(NextXMarkerTakt(takt)); break;
		
		case GAD_T_STOP: StoppenZero(); break;
		case GAD_T_PLAY: WiedergabeStarten(FALSE); break;
		case GAD_T_REC: AufnahmeStarten(); break;
		case GAD_T_MARKER: Springe(code); break;
		
		case GAD_S_H:
		gui.takt = code << VIERTEL;
		GgfFolgenAbschalten();
		AktualisiereEditFeld(FALSE, TRUE);
		break;
		
		case GAD_S_V:
		gui.spur = code;
		AktualisiereGadgets();
		KeinePosition(); ZeichneSpuren(TRUE, TRUE);
		ZeichnePosition(TRUE);
		break;
		
		case GAD_Z_H: gui.tab = code; AktualisiereEditFeld(FALSE, TRUE); break;
		case GAD_Z_V: gui.sph = code; SpurenEinpassen(); AktualisiereEditFeld(TRUE, FALSE); break;
		case GAD_F_MREC: GetAttr(GA_Selected, g, &var); metro.rec = (BOOL)var; break;
		case GAD_F_MPLAY: GetAttr(GA_Selected, g, &var); metro.play = (BOOL)var; break;
		case GAD_F_LOOP: GetAttr(GA_Selected, g, &var); loop.aktiv = (BOOL)var; break;
		case GAD_F_FOLLOW: GetAttr(GA_Selected, g, &var); gui.folgen = (BOOL)var; break;
		case GAD_F_THRU: GetAttr(GA_Selected, g, &var); outport[spur[snum].port].thru = (BOOL)var; break;
		case GAD_F_SYNC:
			if (hornystatus != STATUS_STOP) {
				Meldung(CAT(MSG_0159A, "Cannot change sync status while playback"));
				AktualisiereFunctGadgets();
			} else {
				GetAttr(GA_Selected, g, &var);
				if (var == TRUE) {
					AktiviereExtreamSync();
					if (!IstExtreamSyncAktiv()) {
						AktualisiereFunctGadgets();
					}
				} else {
					DeaktiviereExtreamSync();
				}
			}
		break;
	}
}

void UpdateAusfuehren(WORD id, WORD code) {
	if (id != -1) {
		switch (id) {
			case GAD_S_H:
			if (gui.takt != (code << VIERTEL)) {
				gui.takt = code << VIERTEL;
				GgfFolgenAbschalten();
				KeinePosition(); ZeichneSpuren(FALSE, FALSE);
				ZeichneZeitleiste(TRUE); ZeichneSmpteLeiste(); ZeichneMarkerleisten();
			}
			break;
			
			case GAD_S_V:
			if (gui.spur != code) {
				gui.spur = code;
				KeinePosition(); ZeichneSpuren(TRUE, FALSE);
			}
			break;
			
			case GAD_Z_H:
			if (gui.tab != code) {
				gui.tab = code;
				KeinePosition(); ZeichneSpuren(FALSE, FALSE);
				ZeichneZeitleiste(TRUE); ZeichneSmpteLeiste(); ZeichneMarkerleisten();
			}
			break;

			case GAD_Z_V:
			if (gui.sph != code) {
				gui.sph = code;
				KeinePosition(); ZeichneSpuren(TRUE, FALSE);
			}
			break;
		}
	}
}

void SchleifeGuispalte(void) {
	struct IntuiMessage *mes;
	BOOL verlassen = FALSE;

	KeinePosition();
	do {
		WaitPort(hfenster->UserPort);
		while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
			switch (mes->Class) {
				case IDCMP_MOUSEMOVE:
				gui.spalte = mes->MouseX - hfenster->BorderLeft;
				if (gui.spalte < 60) gui.spalte = 60;
				if (gui.spalte > 350) gui.spalte = 350;
				ZeichneSpuren(TRUE, FALSE);
				break;
				
				case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
			}
			ReplyMsg((struct Message *)mes);
		}
	} while (!verlassen);
	LoescheLinksOben();
	ZeichneSpuren(TRUE, TRUE);
	ZeichneMarkerleisten();
	ZeichneSmpteLeiste();
	ZeichneZeitleiste(TRUE);
	ZeichneAnzeigen(TRUE);
	ZeichnePosition(TRUE);
}

void SchleifeMarker(WORD mousex, UWORD qualifier) {
	BOOL schieben;
	BOOL verlassen = FALSE;
	BOOL kill = FALSE;
	struct IntuiMessage *mes;
	LONG neutakt;
	struct MARKER *erstmark;
	struct MARKER *aktmark;
	LONG d;
	BOOL taktanders = FALSE;
	BOOL tempanders = FALSE;

	if (hornystatus == STATUS_STOP) {
		edittakt = PunktPosition(mousex);
		TaktWahlMark(edittakt);
		ZeichneMarkerleisten();
		ZeichnePosition(TRUE);
		if (wahlmark[mtyp]) {
			schieben = (edittakt == wahlmark[mtyp]->takt);
		} else {
			schieben = FALSE;
		}
		if (schieben) {
			if (wahlmark[mtyp]->takt != 0) {
				erstmark = TaktDirektMarker(edittakt); // nur für Multiverschiebung
				do {
					WaitPort(hfenster->UserPort);
					while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
						switch (mes->Class) {
							case IDCMP_MOUSEMOVE:
							neutakt = PunktPosition(mes->MouseX);
							if (neutakt < VIERTELWERT) neutakt = VIERTELWERT;
							if (neutakt != wahlmark[mtyp]->takt) {
							
								if (qualifier & QUALIFIER_SHIFT) { // Alle ab X verschieben
									d = neutakt - erstmark->takt;
									aktmark = erstmark;
									while (aktmark) {
										aktmark->takt += d;
										aktmark = aktmark->next;
									}
									ZeichneMarkerleisten();
									taktanders = TRUE;
									tempanders = TRUE;
								} else { // Gewählten verschieben
									wahlmark[mtyp]->takt = neutakt;
									while (wahlmark[mtyp]->next) {
										if (wahlmark[mtyp]->takt > wahlmark[mtyp]->next->takt) {
											MarkerTauschen(wahlmark[mtyp], wahlmark[mtyp]->next);
										} else break;
									}
									while (wahlmark[mtyp]->takt < wahlmark[mtyp]->prev->takt) MarkerTauschen(wahlmark[mtyp]->prev, wahlmark[mtyp]);
									ZeichneMarkerleiste(mtyp);
									if (mtyp == M_TAKT) {
										ZeichneZeitleiste(FALSE);
										taktanders = TRUE;
									}
									if (mtyp == M_TEMPO) tempanders = TRUE;
								}
								
							}
							kill = (mes->MouseY - hfenster->BorderTop < 2);
							break;
								
							case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
						}
						ReplyMsg((struct Message *)mes);
					}
				} while (!verlassen);
				if (kill) {
					EntferneMarker(wahlmark[mtyp]); wahlmark[mtyp] = TaktMarker(NULL, mtyp, edittakt);
				}
			}
		} else {
			if (qualifier & QUALIFIER_ALT) {
				switch (mtyp) {
					case M_TEXT:
					wahlmark[M_TEXT] = NeuerMarker(M_TEXT, edittakt, 0, 0);
					strcpy(&wahlmark[M_TEXT]->text, CAT(MSG_0159, "Unnamed"));
					StringFenster(hfenster, &wahlmark[M_TEXT]->text, 127);
					break;
					
					case M_TEMPO:
					wahlmark[M_TEMPO] = NeuerMarker(M_TEMPO, edittakt, wahlmark[M_TEMPO]->d1, wahlmark[M_TEMPO]->d2);
					wahlmark[M_TEMPO]->m_bpm = IntegerFenster(hfenster, wahlmark[M_TEMPO]->m_bpm, 40, 240);
					tempanders = TRUE;
					break;
	
					case M_TAKT:
					wahlmark[M_TAKT] = NeuerMarker(M_TAKT, edittakt, wahlmark[M_TAKT]->d1, wahlmark[M_TAKT]->d2);
					wahlmark[M_TAKT]->m_zaehler = IntegerFenster(hfenster, wahlmark[M_TAKT]->m_zaehler, 2, 18);
					taktanders = TRUE;
					break;
				}
			}
		}
		if (qualifier & QUALIFIER_SHIFT) {
			MarkerSortieren();
			ZeichneMarkerleisten();
		} else {
			ZeichneMarkerleiste(mtyp);
		}
		if (taktanders) {
			TakteAktualisieren();
			ZeichneZeitleiste(TRUE);
		}
		if (tempanders) {
			SmpteTicksAktualisieren();
			ZeichneSmpteLeiste();
		}
		
		ZeichneInfobox(2);
		AktualisiereSprungliste();
	}
}

void SchleifeZeit(WORD mousex, UWORD qualifier) {
	BOOL verlassen = FALSE;
	LONG alttakt;
	LONG neutakt;
	BYTE greifp = 0;
	struct IntuiMessage *mes;

	if (qualifier & QUALIFIER_ALT) {
		lied.taktanz = PunktPosition(mousex) >> VIERTEL;
		if (lied.taktanz < 40) lied.taktanz = 40;
		ZeichneZeitleiste(TRUE);
		ZeichneInfobox(1);
		ZeichneUebersicht();
	} else if (hornystatus == STATUS_STOP) {
		takt = PunktPosition(mousex);
		ZeichnePosition(FALSE);
		ltmark = TaktMarker(NULL, M_TEMPO, takt);
		alttakt = takt;
		if (loop.start < loop.ende + VIERTELWERT) {
			if (takt == loop.start) greifp = 1; // Startpunkt
			if (takt == loop.ende - VIERTELWERT) greifp = 2; // Endpunkt
			if ((takt > loop.start) && (takt < loop.ende - VIERTELWERT)) greifp = 3; // Mitte
		}
		do {
			WaitPort(hfenster->UserPort);
			while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
				switch (mes->Class) {
					case IDCMP_MOUSEMOVE:
					neutakt = PunktPosition(mes->MouseX);
					if (alttakt != neutakt) {
						switch (greifp) {
							case 0:
							if (neutakt > takt) {
								loop.start = takt; loop.ende = neutakt;
								loop.aktiv = TRUE;
								ZeichneZeitleiste(TRUE);
							}
							break;
							
							case 1:
							if (neutakt < loop.ende) {
								loop.start = neutakt;
								loop.aktiv = TRUE;
								ZeichneZeitleiste(TRUE);
							}
							break;
							
							case 2:
							if (neutakt > loop.start) {
								loop.ende = neutakt + VIERTELWERT;
								loop.aktiv = TRUE;
								ZeichneZeitleiste(TRUE);
							}
							break;
							
							case 3:
							if (loop.start + (neutakt - alttakt) >= 0) {
								loop.start += neutakt - alttakt;
								loop.ende += neutakt - alttakt;
								loop.aktiv = TRUE;
								ZeichneZeitleiste(TRUE);
							}
							break;
						}
						alttakt = neutakt;
					}
					break;
					
					case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
				}
				ReplyMsg((struct Message *)mes);
			}
		} while (!verlassen);
		AktualisiereFunctGadgets();
		ResetteLMarker();
		tick = TaktSmpteTicks(takt);
		ZeichneAnzeigen(TRUE);
		
		LocateExtreamSync();
	}
}

void SchleifeSpuren(WORD mousex, WORD mousey, UWORD qualifier) {
	BOOL verlassen = FALSE;
	WORD s;
	struct IntuiMessage *mes;

	s = PunktSpur(mousey);
	if (s >= lied.spuranz) {
		if (qualifier & QUALIFIER_ALT) NeueSpur(); // Neue Spur erzeugen
	} else {
		mousex -= hfenster->BorderLeft;
		if ((mousex >= 30) && (mousex <= 46)) { // Mute Krams
			if (qualifier & QUALIFIER_ALT) {
				SpurenMutesAus();
			} else {
				if (qualifier & QUALIFIER_SHIFT) SpurSolo(s); else SpurMuteSchalter(s);
			}
		} else {
			if (qualifier & QUALIFIER_SHIFT) { // Sequenzen markieren
				SequenzenInSpurMarkieren(s);
				AktualisiereSpuren(FALSE);
				MarkSequenzInfo();
				ZeichneInfobox(8);
				
			} else if (qualifier & QUALIFIER_ALT) { // Spur duplizieren
				ZeichneSpurSpalte(snum, FALSE);
				SpurDuplizieren(s);

			} else { // Spur aktivieren
				if (s != snum) SpurAktivieren(s);
	
				if (hornystatus == STATUS_STOP) { // Drag & Drop
					do {
						WaitPort(hfenster->UserPort);
						while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
							switch (mes->Class) {
								case IDCMP_MOUSEMOVE:
								snum = PunktSpur(mes->MouseY); if (snum >= lied.spuranz) snum = lied.spuranz-1;
								if (snum != s) {
									SpurVerschieben(s, snum);
									s = snum;
								}
								break;
								
								case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
							}
							ReplyMsg((struct Message *)mes);
						}
					} while (!verlassen);
				}
			}
		}
	}
}

void SchleifeSequenzen(WORD mousex, WORD mousey, UWORD qualifier, BOOL doubleclick) {
	WORD s;
	WORD neus;
	BYTE greifp;
	BOOL verlassen = FALSE;
	LONG alttakt;
	LONG neutakt;
	struct IntuiMessage *mes;
	WORD mx2, my2;
	WORD n;
	struct SEQUENZ *altseq;
	WORD difspur;
	LONG diftakt;
	LONG altguitakt;
	BYTE p, c, num;
	struct AUTOPUNKT *autopunkt = NULL;
	BOOL autoanders = FALSE;

	s = PunktSpur(mousey);
	edittakt = PunktPosition(mousex);
	TaktWahlMark(edittakt);
	ZeichneMarkerleisten();
	if (spur[s].autostatus == 0) {
		// Sequenzspur...
		
		altseq = wahlseq;
		if ((wahlseq = TaktSequenz(s, edittakt, &greifp)) && !(qualifier & QUALIFIER_CTRL)) {
			if (gui.tab < 20) greifp = 0;
			// Markieren...
			if ((qualifier & QUALIFIER_SHIFT) && !(qualifier & QUALIFIER_ALT)) {
				wahlseq->markiert = !wahlseq->markiert;
			} else {
				if (!wahlseq->markiert) NichtsMarkieren();
				wahlseq->markiert = TRUE;
			}
			sp[s].anders = 1;
	
			if (doubleclick && (altseq == wahlseq)) {
				// Editor...
				NotenSequenzEditor(wahlseq);
			} else {
	
				if (qualifier & QUALIFIER_ALT) {
					// Kopieren...
					greifp = 0;
					if (qualifier & QUALIFIER_SHIFT) wahlseq = MarkSequenzenKopieren(); else wahlseq = MarkSequenzenAlias();
				}
				AktualisiereSpuren(FALSE);
				if (hornystatus == STATUS_STOP) {
					alttakt = PunktPosition(mousex);
					do {
						WaitPort(hfenster->UserPort);
						while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
							switch (mes->Class) {
								case IDCMP_MOUSEMOVE:
								neutakt = PunktPosition(mes->MouseX);
	
								// Bild scrollen?
								diftakt = 0;
								if (neutakt < gui.takt) diftakt = neutakt - gui.takt;
								if (neutakt > gui.takt + (gui.tasicht << VIERTEL)) diftakt = neutakt - (gui.takt + (gui.tasicht << VIERTEL));
								if (diftakt != 0) {
									KeineProjektion();
									altguitakt = gui.takt;
									gui.takt += diftakt;
									if (gui.takt < 0) gui.takt = 0;
									if (gui.takt != altguitakt) AktualisiereEditFeld(FALSE, TRUE);
								}
								
								difspur = 0;
								diftakt = neutakt - alttakt;
								// Sequenzen verschieben...
								if (greifp == 0) {
									neus = PunktSpur(mes->MouseY);
									difspur = neus - s;
									MarkSequenzenVerschiebenTest(&difspur, &diftakt);
									if (difspur <= 0) {
										for (n = 0; n < lied.spuranz; n++) {
											MarkSequenzenVerschieben(n, difspur, diftakt);
										}
									} else {
										for (n = lied.spuranz - 1; n >= 0; n--) {
											MarkSequenzenVerschieben(n, difspur, diftakt);
										}
									}
								}
								// Größe ändern...
								if (greifp == 1) MarkSequenzenStartVerschieben(diftakt);
								if (greifp == 2) MarkSequenzenEndeVerschieben(diftakt);
								if (diftakt || difspur) AktualisiereSpuren(FALSE);
								ZeichneProjektion(wahlseq->start, wahlseq->ende);
								alttakt += diftakt;
								s += difspur;
								break;
								
								case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
							}
							ReplyMsg((struct Message *)mes);
						}
					} while (!verlassen);
					KeineProjektion();
					AktualisiereGadgets();
					ZeichneUebersicht();
				}
			}
		} else {
			if (!(qualifier & QUALIFIER_SHIFT)) {NichtsMarkieren(); AktualisiereSpuren(FALSE);}
			if ((qualifier & QUALIFIER_ALT) && (s < lied.spuranz)) {
				if (hornystatus == STATUS_STOP) {
					// Neue Sequenz erzeugen...
					sp[s].neuseq = ErstelleSequenz(s, edittakt, TRUE);
					if (sp[s].neuseq) {
						wahlseq = NeueSequenzEinordnen(s);
						wahlseq->markiert = TRUE;
						AktualisiereSpuren(FALSE);
						ZeichneUebersicht();
					}
				}
			} else {
				// Lassowahl...
				aktfenster = hfenster;
				if (LassoWahl(mousex, mousey, &mx2, &my2)) {
					BereichMarkieren(PunktSpur(mousey), PunktSpur(my2), PunktPosition(mousex), PunktPosition(mx2));
					AktualisiereSpuren(FALSE);
				}
			}
		}
		MarkSequenzInfo();
		ZeichneInfobox(2 | 8);
	} else {
		// Automationsspur...
		
		p = spur[s].port;
		c = spur[s].channel;
		num = spur[s].autostatus - 1;
		n = PunktAutoWert(s, mousey);
		if (qualifier & QUALIFIER_ALT) {
			autopunkt = NeuerAutoPunkt(p, c, num, edittakt, n);
			autoanders = TRUE;
		} else {
			autopunkt = TaktAutoPunkt(p, c, num, edittakt);
			if (autopunkt) autopunkt->wert = n;
		}
		KeinePosition(); ZeichneSequenzen(s, FALSE); ZeichnePosition(TRUE);
		if (autopunkt) {
			do {
				WaitPort(hfenster->UserPort);
				while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
					switch (mes->Class) {
						case IDCMP_MOUSEMOVE:
						autopunkt->takt = PunktPosition(mes->MouseX);
						if (autopunkt->takt < 0) autopunkt->takt = 0;
						if (autopunkt->prev) if (autopunkt->prev->takt > autopunkt->takt) autopunkt->takt = autopunkt->prev->takt;
						if (autopunkt->next) if (autopunkt->next->takt < autopunkt->takt) autopunkt->takt = autopunkt->next->takt;
						autopunkt->wert = PunktAutoWert(s, mes->MouseY);
						autoanders = TRUE;
						KeinePosition(); ZeichneSequenzen(s, FALSE); ZeichnePosition(TRUE);
						break;
						
						case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
					}
					ReplyMsg((struct Message *)mes);
				}
			} while (!verlassen);
			if (!autoanders) EntferneAutoPunkt(p, c, num, autopunkt);
		}
		KanalSpurenBearbeitet(p, c);
		KeinePosition(); AktualisiereSpuren(TRUE); ZeichnePosition(TRUE);
	}
}

void SchleifeInfobox(WORD mousex, WORD mousey) {
	BYTE n;
	
	n = TestePunktInfo(mousex, mousey);
	// Lied
	if (n == 0) {
		StringFenster(hfenster, lied.name, 127);
		ZeichneInfobox(1);
	}
	if (n == 1) {
		lied.taktanz = IntegerFenster(hfenster, lied.taktanz, 40, 4000);
		ZeichneInfobox(1);
		AktualisiereGadgets();
		ZeichneUebersicht();
	}
	// Editposition
	if (n / 10 == 1) {
		if (n == 11) {
			wahlmark[M_TEMPO]->m_bpm = IntegerFenster(hfenster, wahlmark[M_TEMPO]->m_bpm, 40, 240);
			ZeichneMarkerleiste(M_TEMPO);
			SmpteTicksAktualisieren();
			ZeichneSmpteLeiste();
		}
		if (n == 12) {
			wahlmark[M_TAKT]->m_zaehler = IntegerFenster(hfenster, wahlmark[M_TAKT]->m_zaehler, 2, 18);
			TakteAktualisieren();
			ZeichneMarkerleiste(M_TAKT);
			ZeichneZeitleiste(TRUE);
		}
		if ((n == 13) && wahlmark[M_TEXT]) {
			StringFenster(hfenster, &wahlmark[M_TEXT]->text, 127);
			ZeichneMarkerleiste(M_TEXT);
			AktualisiereSprungliste();
		}
		ZeichneInfobox(2);
	}
	// Spur
	if (n / 10 == 2) {
		if (n == 20) {
			StringFenster(hfenster, spur[snum].name, 127);
			ZeichneSpurSpalte(snum, TRUE);
		}
		if (n == 21) {
			ChannelPortFenster(hfenster, &spur[snum].channel, &spur[snum].port);
			ZeichneSpurSpalte(snum, TRUE);
			KeinePosition();
			ZeichneSequenzen(snum, TRUE);
			ZeichnePosition(TRUE);
			AktualisiereFunctGadgets();
		}
		if (n == 22) {
			if (spur[snum].channel < 16) {
				InstrumentenFenster2(hfenster, spur[snum].channel, spur[snum].port, &spur[snum].bank0, &spur[snum].bank32, &spur[snum].prog);
				SendeInstrument(snum);
			}
		}
		if (n == 23) spur[snum].shift = IntegerFenster(hfenster, spur[snum].shift, -4 * VIERTELWERT, 4 * VIERTELWERT);
		ZeichneInfobox(4);
	}
	// Sequenz
	if ((n / 10 == 3) && seqinfo.benutzt) {
		if (n == 30) {
			StringFenster(hfenster, seqinfo.name, 127);
			MarkSequenzenSetzeName(seqinfo.name);
			KeinePosition(); ZeichneSpuren(FALSE, TRUE); ZeichnePosition(TRUE);
		}
		if (n == 31) {
			seqinfo.trans = IntegerFenster(hfenster, seqinfo.trans, -24, 24);
			MarkSequenzenSetzeTrans(seqinfo.trans);
			AktualisiereSpuren(FALSE);
		}
		if (n == 32) {
			seqinfo.mute = !seqinfo.mute;
			MarkSequenzenSetzeMute(seqinfo.mute);
			AktualisiereSpuren(FALSE);
		}
		MarkSequenzInfo();
		ZeichneInfobox(8);
	}
}

void SchleifeRawKey(UWORD code, UWORD qualifier) {
	switch (code) {
		case 76:
		if (qualifier & QUALIFIER_SHIFT) {
			if (gui.sph > 15) {
				gui.sph -= 4;
				AktualisiereGadgets();
				SpurenEinpassen();
				AktualisiereEditFeld(TRUE, TRUE);
			}
		} else {
			if (snum > 0) SpurAktivieren(snum - 1);
		}
		break;
		
		case 77:
		if (qualifier & QUALIFIER_SHIFT) {
			gui.sph += 4;
			AktualisiereGadgets();
			SpurenEinpassen();
			AktualisiereEditFeld(TRUE, TRUE);
		} else {
			if (snum < lied.spuranz - 1) SpurAktivieren(snum + 1);
		}
		break;
		
		case 78:
		if (qualifier & QUALIFIER_SHIFT) {
			gui.tab += 3;
			AktualisiereGadgets();
		} else {
			gui.takt += (gui.tasicht / 3 * VIERTELWERT);
			GgfFolgenAbschalten();
		}
		AktualisiereEditFeld(FALSE, TRUE);
		break;
		
		case 79:
		if (qualifier & QUALIFIER_SHIFT) {
			if (gui.tab > 4) {
				gui.tab -= 3;
				AktualisiereGadgets();
			}
		} else {
			gui.takt -= (gui.tasicht / 3 * VIERTELWERT); if (gui.takt < 0) gui.takt = 0;
			GgfFolgenAbschalten();
		}
		AktualisiereEditFeld(FALSE, TRUE);
		break;
	}
}

void SchleifeMenuPick(UWORD code, UWORD qualifier) {
	BOOL kill;
	WORD n;
	LONG start, ende;
	
	//printf("%X\n", code);
	MinMenuKontrolle(MenuPunkt(code));
	switch (MenuPunkt(code)) {

		case MENU_SPR_HINZU:
		NeueSpur();
		break;
		
		case MENU_SPR_DUPLIZIEREN:
		SpurDuplizieren(snum);
		break;

		case MENU_SPR_LOESCHEN:
		if (spur[snum].seq) kill = Frage(CAT(MSG_0160, "Really delete track?"), CAT(MSG_0161, "Yes|No")); else kill = TRUE;
		if (kill) {
			if (wahlseq) {
				if (wahlseq->spur == snum) wahlseq = NULL;
			}
			SpurLoeschen(snum);
		}
		break;
		
		case MENU_SPR_MUTEN:
		SpurMuteSchalter(snum);
		break;
		
		case MENU_SPR_SOLO:
		SpurSolo(snum);
		break;
		
		case MENU_SPR_MUTESAUS:
		SpurenMutesAus();
		break;
		
		case MENU_SPR_AUTO_VOL:
		case MENU_SPR_AUTO_PAN:
		case MENU_SPR_AUTO_CTRL1:
		case MENU_SPR_AUTO_CTRL2:
		case MENU_SPR_AUTO_CTRL3:
		case MENU_SPR_AUTO_CTRL4:
		case MENU_SPR_AUTO_CTRL5:
		case MENU_SPR_AUTO_CTRL6:
		KeinePosition();
		if (qualifier & QUALIFIER_SHIFT) {
			for (n = 0; n < lied.spuranz; n++) {
				spur[n].autostatus = MenuPunkt(code) - MENU_SPR_AUTO_VOL + 1;
				ZeichneSequenzen(n, TRUE);
				ZeichneSpurSpalte(n, n == snum);
			}
		} else {
			spur[snum].autostatus = MenuPunkt(code) - MENU_SPR_AUTO_VOL + 1;
			ZeichneSequenzen(snum, TRUE);
			ZeichneSpurSpalte(snum, TRUE);
		}
		ZeichnePosition(TRUE);
		break;
		
		case MENU_SPR_AUTO_HIDE:
		KeinePosition();
		if (qualifier & QUALIFIER_SHIFT) {
			for (n = 0; n < lied.spuranz; n++) {
				spur[n].autostatus = 0;
				ZeichneSequenzen(n, TRUE);
				ZeichneSpurSpalte(n, n == snum);
			}
		} else {
			spur[snum].autostatus = 0;
			ZeichneSequenzen(snum, TRUE);
			ZeichneSpurSpalte(snum, TRUE);
		}
		ZeichnePosition(TRUE);
		break;
		
		case MENU_SPR_AUTO_DEL:
		if (spur[snum].autostatus > 0) {
			EntferneAlleAutoPunkte(spur[snum].port, spur[snum].channel, spur[snum].autostatus - 1);
			KanalSpurenBearbeitet(spur[snum].port, spur[snum].channel);
			KeinePosition(); AktualisiereSpuren(TRUE); ZeichnePosition(TRUE);
		}
		break;
		
		case MENU_SPR_AUTO_COPY:
		AutomationKopieren(spur[snum].port, spur[snum].channel, spur[snum].autostatus - 1);
		break;
		
		case MENU_SPR_AUTO_PASTE:
		AutomationEinfuegen(spur[snum].port, spur[snum].channel, spur[snum].autostatus - 1);
		KanalSpurenBearbeitet(spur[snum].port, spur[snum].channel);
		KeinePosition(); AktualisiereSpuren(TRUE); ZeichnePosition(TRUE);
		break;
		
		case MENU_SPR_AUTO_C2A:
		if (hornystatus == STATUS_STOP) {
			KonvertiereContrZuAuto(snum);
			KanalSpurenBearbeitet(spur[snum].port, spur[snum].channel);
			KeinePosition(); AktualisiereSpuren(TRUE); ZeichnePosition(TRUE);
		}
		break;

		case MENU_SPR_AUTO_A2C:
		if (hornystatus == STATUS_STOP) {
			KonvertiereAutoZuContr(snum);
			KeinePosition(); AktualisiereSpuren(TRUE); ZeichnePosition(TRUE);
		}
		break;
		
		case MENU_SEQ_MARK_ALLELIED:
		for(n = 0; n < lied.spuranz; n++) SequenzenInSpurMarkieren(n);
		AktualisiereSpuren(FALSE);
		MarkSequenzInfo(); ZeichneInfobox(8);
		break;
		
		case MENU_SEQ_MARK_ALLESPUR:
		SequenzenInSpurMarkieren(snum);
		AktualisiereSpuren(FALSE);
		MarkSequenzInfo(); ZeichneInfobox(8);
		break;
		
		case MENU_SEQ_MARK_EDITPOS:
		SequenzenAbXMarkieren(edittakt);
		AktualisiereSpuren(FALSE);
		MarkSequenzInfo(); ZeichneInfobox(8);
		break;
		
		case MENU_SEQ_SETZELOOP:
		if (HoleMarkSequenzenRahmen(&start, &ende)) {
			loop.start = start;
			loop.ende = ende;
			loop.aktiv = TRUE;
			ZeichneZeitleiste(TRUE);
			AktualisiereFunctGadgets();
		}
		break;
		
		case MENU_SEQ_COPY:
		ClipboardKopieren(); AktualisiereSpuren(FALSE);
		InitSequenzInfo(); ZeichneInfobox(8);
		break;
		
		case MENU_SEQ_PASTE:
		NichtsMarkieren();
		edittakt = ClipboardEinfuegen(edittakt); AktualisiereSpuren(FALSE);
		MarkSequenzInfo(); ZeichneInfobox(8);
		break;
		
		case MENU_SEQ_LOESCHEN:
		MarkSequenzenEntfernen(); AktualisiereSpuren(FALSE); InitSequenzInfo(); ZeichneInfobox(8);
		break;

		case MENU_SEQ_QUANT:
		n = QuantisierungsFenster(hfenster, FALSE);
		if (n) {
			MarkSequenzenQuantisieren(n); AktualisiereSpuren(FALSE);
		}
		break;
		
		case MENU_SEQ_SCHNEIDEN:
		MarkSequenzenZerschneiden(edittakt); AktualisiereSpuren(FALSE);
		break;
		
		case MENU_SEQ_UNTERTEILEN:
		SequenzUnterteilen(wahlseq, edittakt); AktualisiereSpuren(FALSE);
		break;
		
		case MENU_SEQ_VERBINDEN:
		MarkSequenzenVerbinden(); AktualisiereSpuren(FALSE);
		break;

		case MENU_SEQ_ALIASREAL:
		MarkSequenzenAliasZuReal(); AktualisiereSpuren(FALSE);
		break;
	}
}

void HFensterNeu(void) {
	ErstelleHauptfenster();
	BildFrei();
	FensterTitel(projdatei);
	SpurenEinpassen();
	ZeichneSpuren(TRUE, TRUE);
	ZeichneZeitleiste(TRUE);
	ZeichneSmpteLeiste();
	ZeichneMarkerleisten();
	ZeichneAnzeigen(TRUE);
	ZeichnePosition(TRUE);
	ZeichneInfobox(0);
	ZeichneUebersicht();
	ErstelleGadgets();
	ErstelleMenu();
	ErstelleAslReqs();
	AktualisiereFunctGadgets();
	AppWindowAnmelden();
}

void HFensterWeg(void) {
	EntferneAslReqs();
	EntferneEditorNotenFenster();
	EntferneEinstellungsfenster();
	EntferneUmgebungsfenster();
	EntferneSysExFenster();
	EntferneChangeCtrlFenster();
	EntferneMPFenster();
	EntferneGadgets();
	EntferneMenu();
	AppWindowAbmelden();
	EntferneHauptfenster();
}

void haupt(STRPTR startdatei) {
	struct IntuiMessage *mes;
	ULONG signals;
	ULONG waitsigs;
	ULONG altsec = 0, altmic = 0;
	BOOL doubleclick;
	WORD updateid, updatecode;
	WORD vscroll, hscroll;

	TesteKey();
	OeffneFont();
	OeffneTitel();

	InitPfadStrings();
	LadeUmgebung();
	LadeFensterPos();

	playproc = (struct Process *)CreateNewProcTags(
		NP_Entry, &PlayerProcess,
		NP_StackSize, 1024,
		NP_Name, "HornyPlayer",
		NP_Priority, umgebung.playerPri,
		TAG_DONE);
	if (!playproc) Meldung(CAT(MSG_0163, "Could not create player process"));
	
	thruproc = (struct Process *)CreateNewProcTags(
		NP_Entry, &ThruProcess,
		NP_StackSize, 1024,
		NP_Name, "HornyThru",
		NP_Priority, umgebung.thruPri,
		TAG_DONE);
	if (!thruproc) Meldung("Could not create thru process");
	

	ErstelleCamd();
	hornystatus = STATUS_STOP;

	WarteStart(2);

	ErstelleMenuLibs();
	ErstelleGuiClasses();
	ErstelleBildschirm();
	OeffneAlleGfx();
	NeuesLied();
	ErstelleLinks();
	InitUmrechnungstabellen();
	LadeAlleInstrumente();
	StartProjekt(startdatei);

	WarteEnde();

	HFensterNeu();
	AktualisiereSprungliste();
	ZeigeBildschirm();

	SchliesseTitel();

	if (verLITE) Meldung("This is the unregistered Lite version!\n\nThis means:\n- Only 16 tracks (full version: 128)\n- Only 1 midi port (full version: 16)\n- Only 3 controller in mixer unit (full version: 6)\n\nCheck www.inutilis.de/horny/ for registering!");
	
	do {
		waitsigs = (1L << hfenster->UserPort->mp_SigBit) | (1L << playsig);
		if (setfenster) waitsigs |= (1L << setfenster->UserPort->mp_SigBit);
		if (envfenster) waitsigs |= (1L << envfenster->UserPort->mp_SigBit);
		if (sexfenster) waitsigs |= (1L << sexfenster->UserPort->mp_SigBit);
		if (edfenster)  waitsigs |= (1L << edfenster->UserPort->mp_SigBit);
		if (mpfenster)  waitsigs |= (1L << mpfenster->UserPort->mp_SigBit);
		if (ccfenster)  waitsigs |= (1L << ccfenster->UserPort->mp_SigBit);
		if (syncport)   waitsigs |= (1L << syncport->mp_SigBit);
		#ifdef __amigaos4__
		waitsigs |= start_appSigMask;
		#endif
		signals = Wait(waitsigs);

		#ifdef __amigaos4__
		if (signals & start_appSigMask)
		{
			struct ApplicationMsg *msg;
			while (msg = (struct ApplicationMsg *) GetMsg(start_appPort))
			{
			    switch (msg->type)
			    {
			        case APPLIBMT_Quit:
						aktfenster = hfenster;
						if (Frage(CAT(MSG_0164, "Really quit program?"), CAT(MSG_0161, "Yes|No"))) beendet = TRUE;
					break;

					case APPLIBMT_ForceQuit:
						beendet = TRUE;
					break;

					case APPLIBMT_ToFront:
						if (hschirm) {
							ScreenToFront(hschirm);
						} else {
							if (hfenster) {
								WindowToFront(hfenster);
								ActivateWindow(hfenster);
							}
							if (edfenster) WindowToFront(edfenster);
							if (mpfenster) WindowToFront(mpfenster);
						}
					break;

					case APPLIBMT_OpenDoc: {
						struct ApplicationOpenPrintDocMsg *openMsg = (struct ApplicationOpenPrintDocMsg *) msg;
						strncpy(projdatei, openMsg->fileName, 1024);
                        ProjektLadenKomplett();
					} break;

					case APPLIBMT_AppRegister: {
						ULONG id = msg->senderAppID;
						STRPTR url = NULL;
						STRPTR name = NULL;
						if (GetApplicationAttrs(id,
								APPATTR_Name, &name,
								APPATTR_URLIdentifier, &url,
								TAG_DONE))
						{
							if (strcmp(name, "Phonolith") == 0 && strcmp(url, "inutilis.de") == 0)
							{
								checkPhonolithProject();
							}
						}
					} break;
			    }

				ReplyMsg((struct Message *)msg);
			}
		}
		#endif

		if (syncport) {
			if (signals & (1L << syncport->mp_SigBit)) KontrolleExtreamSync();
		}
		
		if (setfenster) {
			if (signals & (1L << setfenster->UserPort->mp_SigBit)) KontrolleEinstellungsfenster();
		}
		if (envfenster) {
			if (signals & (1L << envfenster->UserPort->mp_SigBit)) {
				if (KontrolleUmgebungsfenster()) { //Screenmode geändert?
					KeinePosition();
					HFensterWeg();
					SchliesseAlleGfx();
					EntferneBildschirm();
					ErstelleBildschirm();
					ZeigeBildschirm();
					OeffneAlleGfx();
					HFensterNeu();
				}
			}
		}
		if (edfenster) {
			if (signals & (1L << edfenster->UserPort->mp_SigBit)) KontrolleEditorNotenFenster();
		}
		if (sexfenster) {
			if (signals & (1L << sexfenster->UserPort->mp_SigBit)) KontrolleSysExFenster();
		}
		if (mpfenster) {
			if (signals & (1L << mpfenster->UserPort->mp_SigBit)) KontrolleMischpultFenster();
		}
		if (ccfenster) {
			if (signals & (1L << ccfenster->UserPort->mp_SigBit)) KontrolleChangeCtrlFenster();
		}
		
		if (signals & (1L << playsig)) {
			if (playsigaktion == 1) { // SysEx empfangen
				if (sexfenster) {
					AktualisiereSysExMsgListe();
					AktualisiereSysExGruppenListe();
				}
				playsigaktion = 0;
			} else { // Position
				ZeichnePosition(FALSE);
				ZeichneAnzeigen(tmarkwechsel);
				if (tmarkwechsel) tmarkwechsel = FALSE;
				if (gui.folgen) {
					if ((takt > gui.takt + ((gui.tasicht - 4) * VIERTELWERT)) || (takt < gui.takt)) {
						gui.takt = takt & VIERTELMASKE;
						AktualisiereEditFeld(FALSE, TRUE);
					}
				}
				if (mpfenster) {
					ErniedrigeMeter(5);
					ZeichneMeter();
					AutoUpdateMischpult();
				}
			}
		}

		if (signals & (1L << hfenster->UserPort->mp_SigBit)) {
			updateid = -1; updatecode = 0;
			while (mes = (struct IntuiMessage *)GetMsg(hfenster->UserPort)) {
				switch (mes->Class) {
					case IDCMP_CLOSEWINDOW:
					aktfenster = hfenster;
					if (Frage(CAT(MSG_0164, "Really quit program?"), CAT(MSG_0161, "Yes|No"))) beendet = TRUE;
					break;
					
					case IDCMP_NEWSIZE:
					KeinePosition();
					aktfenster = hfenster; BildFrei();
					PositioniereGadgets();
					ZeichneSpuren(TRUE, TRUE);
					ZeichneZeitleiste(TRUE);
					ZeichneSmpteLeiste();
					ZeichneMarkerleisten();
					ZeichneAnzeigen(TRUE);
					ZeichnePosition(TRUE);
					ZeichneInfobox(0);
					ZeichneUebersicht();
					AktualisiereGadgets();
					break;
					
					case IDCMP_GADGETUP:
					SchleifeGadgets((struct Gadget *)mes->IAddress, mes->Code);
					break;
					
					case IDCMP_IDCMPUPDATE:
					SchleifeUpdate((struct TagItem *)mes->IAddress, &updateid, &updatecode);
					break;

#ifdef __amigaos4__
					case IDCMP_EXTENDEDMOUSE: {
						if (mes->Code == IMSGCODE_INTUIWHEELDATA) {
							struct IntuiWheelData *data = (struct IntuiWheelData *)mes->IAddress;
							BOOL tauschen = umgebung.mausradtauschen;
							if (mes->Qualifier & QUALIFIER_ALT) {
								tauschen = !tauschen;
							}
							if (!tauschen) {
								hscroll = data->WheelX;
								vscroll = data->WheelY;
							} else {
								hscroll = data->WheelY;
								vscroll = data->WheelX;
							}
							if (hscroll != 0) {
								gui.takt += hscroll * (gui.tasicht / 6 * VIERTELWERT);
								if (gui.takt < 0) gui.takt = 0;
								GgfFolgenAbschalten();
								AktualisiereEditFeld(FALSE, TRUE);
							}
							if (vscroll != 0) {
								SpurScroll(vscroll);
							}
						}
					} break;
#endif

					case IDCMP_MOUSEBUTTONS:
					if (mes->Code == 104) {
						doubleclick = DoubleClick(altsec, altmic, mes->Seconds, mes->Micros);
						if (doubleclick) altsec = 0;
						else {
							altsec = mes->Seconds; altmic = mes->Micros;
						}

						switch (TestePunktBereich(mes->MouseX, mes->MouseY)) {
							case AREA_GUISPALTE:
							SchleifeGuispalte();
							break;
							
							case AREA_MARKER:
							SchleifeMarker(mes->MouseX, mes->Qualifier);
							break;

							case AREA_ZEIT:
							SchleifeZeit(mes->MouseX, mes->Qualifier);
							break;
							
							case AREA_SPUREN:
							SchleifeSpuren(mes->MouseX, mes->MouseY, mes->Qualifier);
							break;
							
							case AREA_SEQUENZEN:
							SchleifeSequenzen(mes->MouseX, mes->MouseY, mes->Qualifier, doubleclick);
							break;
							
							case AREA_INFOBOX:
							SchleifeInfobox(mes->MouseX, mes->MouseY);
							break;
							
							case AREA_UEBERSICHT:
							gui.takt = TestePunktUebersicht(mes->MouseX);
							GgfFolgenAbschalten();
							AktualisiereEditFeld(FALSE, TRUE);
							break;
						}
					}
					break;
				
					case IDCMP_RAWKEY: {
						//printf("RKey: %ld\n", mes->Code);
						SchleifeRawKey(mes->Code, mes->Qualifier);
						TransportKontrolleRaw(mes->Code);

#ifndef __amigaos4__
						//os3 (newmouse) mouse wheel
						vscroll = 0;
						hscroll = 0;

						if (!umgebung.mausradtauschen) {
							switch (mes->Code) {
								case NM_WHEEL_UP: vscroll = -1;	break;
								case NM_WHEEL_DOWN: vscroll = 1; break;
								case NM_WHEEL_LEFT: hscroll = -1; break;
								case NM_WHEEL_RIGHT: hscroll = 1; break;
							}
						} else {
							switch (mes->Code) {
								case NM_WHEEL_UP: hscroll = -1;	break;
								case NM_WHEEL_DOWN: hscroll = 1; break;
								case NM_WHEEL_LEFT: vscroll = -1; break;
								case NM_WHEEL_RIGHT: vscroll = 1; break;
							}
						}

						if (hscroll != 0) {
							gui.takt += hscroll * (gui.tasicht / 6 * VIERTELWERT);
							if (gui.takt < 0) gui.takt = 0;
							GgfFolgenAbschalten();
							AktualisiereEditFeld(FALSE, TRUE);
						}
						if (vscroll != 0) {
							SpurScroll(vscroll);
						}
#endif
					} break;
	
					case IDCMP_VANILLAKEY:
					//printf("VKey: %ld\n", mes->Code);
					TransportKontrolle(mes->Code);
					switch (mes->Code) {
						case 127:
						if (hornystatus == STATUS_STOP) {
							MarkSequenzenEntfernen();
							AktualisiereSpuren(FALSE); InitSequenzInfo(); ZeichneInfobox(8);
						}
						break;
					}
					break;
					
					case IDCMP_MENUPICK:
					SchleifeMenuPick(mes->Code, mes->Qualifier);
					break;
					
					case 0x00070000:
					if (Frage(CAT(MSG_0162, "Really open project?"), CAT(MSG_0161, "Yes|No"))) {
						strncpy(projdatei, HoleAppMessageDatei(mes), 1024);
						ProjektLadenKomplett();
					}
					break;
				}
				if (mes) ReplyMsg((struct Message *)mes);
			}
			UpdateAusfuehren(updateid, updatecode);
		}
	} while (!beendet);
	
	hornystatus = STATUS_ENDE;
	Signal(&playproc->pr_Task, 1L << playerprocsig);
	Signal(&thruproc->pr_Task, 1L << thruprocsig);
	
	ClipboardLoeschen();
	EntferneLied();
	EntferneAutomationsKopie();

	EntferneSprungliste();
	EntferneLinks();
	EntferneCamd();
	HFensterWeg();
	SchliesseAlleGfx();
	EntferneBildschirm();
	EntferneAlleInstrumente();
	EntferneGuiClasses();
	EntferneMenuLibs();
	EntferneFont();
	EntfernePfadStrings();

}
