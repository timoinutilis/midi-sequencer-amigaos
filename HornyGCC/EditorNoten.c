#include <stdio.h>

#include <proto/intuition.h>
#include <proto/exec.h>

#include <intuition/gadgetclass.h>
#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Gui.h"
#include "Gui2.h"
#include "Fenster.h"
#include "EditorNotenGui.h"
#include "EditorContrGui.h"
#include "Midi.h"
#include "Projekt.h"
#include "Sequenzen.h"
#include "MidiEdit.h"
#include "Marker.h"
#include "Requester.h"
#include "Menu.h"
#include "Instrumente.h"
#include "Undo.h"

#ifndef __amigaos4__
#include "newmouse.h"
#endif

#define QUALIFIER_SHIFT 0x03
#define QUALIFIER_ALT 0x30
#define QUALIFIER_CTRL 0x08

extern struct Screen *hschirm;
extern struct Window *hfenster;
extern struct Window *aktfenster;

extern struct Window *edfenster;
extern struct Gadget *edgad[];

extern struct LIED lied;
extern struct SPUR spur[];
extern struct SPURTEMP sp[];
extern int8 contrspur[];

extern int32 takt;
extern int32 tick;
extern int32 edittakt;
int8 editnote = -1;
int8 playnote = -1;

struct SEQUENZ *edseq = NULL;
struct EVENT *wahlnote = NULL;
extern struct INSTRCONTR *edinstrcontr;

extern struct EDGUI edgui;
extern struct UMGEBUNG umgebung;

int8 ganz2halb[77];
int8 halb2ganz[132];


int32 RasterTakt(int32 t)
{
	int32 faktor;
	int32 rtakt = t;
	int32 rest;

	if (edgui.raster > 0) {
		if (edgui.tripled) { //Triolen
			faktor = 1 << (edgui.raster + 1);
			rtakt = (t / faktor * faktor);

			rest = t - rtakt;
			faktor = (1 << edgui.raster) * 2 / 3;
			rtakt += (rest / faktor * faktor);
		} else {
			faktor = 1 << edgui.raster;
			rtakt = (t / faktor * faktor);
		}
	}
	return rtakt;
}

void InitUmrechnungstabellen(void) {
	int16 o;
	
	for (o = 0; o<11; o++) {
		ganz2halb[0 + (o*7)] = 0 + (o*12);
		ganz2halb[1 + (o*7)] = 2 + (o*12);
		ganz2halb[2 + (o*7)] = 4 + (o*12);
		ganz2halb[3 + (o*7)] = 5 + (o*12);
		ganz2halb[4 + (o*7)] = 7 + (o*12);
		ganz2halb[5 + (o*7)] = 9 + (o*12);
		ganz2halb[6 + (o*7)] = 11+ (o*12);
		
		halb2ganz[0 + (o*12)] = 0 + (o*7);
		halb2ganz[1 + (o*12)] = 0 + (o*7);
		halb2ganz[2 + (o*12)] = 1 + (o*7);
		halb2ganz[3 + (o*12)] = 1 + (o*7);
		halb2ganz[4 + (o*12)] = 2 + (o*7);
		halb2ganz[5 + (o*12)] = 3 + (o*7);
		halb2ganz[6 + (o*12)] = 3 + (o*7);
		halb2ganz[7 + (o*12)] = 4 + (o*7);
		halb2ganz[8 + (o*12)] = 4 + (o*7);
		halb2ganz[9 + (o*12)] = 5 + (o*7);
		halb2ganz[10+ (o*12)] = 5 + (o*7);
		halb2ganz[11+ (o*12)] = 6 + (o*7);
	}
}

int8 MittlereNotenhoehe(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl;
	int16 evnum;
	uint8 h, n, a;

	evbl = seq->eventblock; evnum = 0; h = 0; n = 127;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
		
		if ((evbl->event[evnum].status & MS_StatBits) == MS_NoteOn) {
			a = evbl->event[evnum].data1;
			if (a>h) h = a;
			if (a<n) n = a;
		}

		evnum++; if (evnum == EVENTS) {evnum = 0; evbl = evbl->next;}
	}
	return((h+n)/2);
}

void ZeichneFeld(BOOL anders) {
	if (edgui.modus == EDMODUS_NOTEN) ZeichneNotenfeld(TRUE, FALSE, anders);
	else ZeichneControllerFeld(anders);
}

void NotenSequenzEditor(struct SEQUENZ *seq) {
	int16 s;
	int32 a;
	
	if (seq->aliasorig) {
		a = Frage(CAT(MSG_0595, "Aliases cannot be edited. Do you want to...\n...edit the original, or...\n...change alias to real sequence?"), CAT(MSG_0596, "Original|Alias to real|Cancel"));
		if (a == 1) edseq = seq->aliasorig;
		else if (a == 2) {
			s = seq->spur;
			sp[s].neuseq = NeueKopie(seq);
			SequenzAusSpurEntfernen(seq);
			SequenzEntfernen(seq);
			edseq = NeueSequenzEinordnen(s);
			ZeichneSequenzen(s, TRUE);
		} else edseq = NULL;
	} else {
		edseq = seq;
	}

	if (edseq) {
		if (!edfenster) {
			ErstelleEditorNotenFenster();
		} else {
			EntferneAlleEdUndo();
			IIntuition->ActivateWindow(edfenster);
			IIntuition->WindowToFront(edfenster);
		}
		
		if (edfenster) {
			if (edseq == seq) edgui.takt = edittakt; else edgui.takt = edseq->start + (edittakt - seq->start);
			edgui.takt = ((edgui.takt - (edgui.taktsicht<<5)) & (0xFFFFFFFF << (VIERTEL - 2)));
			if (edgui.takt < 0) edgui.takt = 0;
			
			edgui.taste = halb2ganz[MittlereNotenhoehe(edseq)] - (edgui.tastsicht/2);
			if (edgui.taste<0) edgui.taste = 0;
		
			wahlnote = NULL;
		
			InitController();
			KeineEventsMarkieren(edseq);

			AddEdUndo(edseq, (STRPTR)"");

			EdFensterTitel();
			ZeichneEdZeitleiste();
			if (edgui.modus == EDMODUS_NOTEN) {
				ZeichneTastatur();
				ZeichneNotenfeld(TRUE, FALSE, FALSE);
			} else {
				ContrSpurenEinpassen();
				ZeichneControllerSpalten();
				ZeichneControllerFeld(FALSE);
			}
			ZeichneEdInfobox();
			AktualisiereEdGadgets();
			AktualisiereEdRasterGadgets();
			AktualisiereEdNeuLenGadgets();
			AktualisiereEdModus();
		}
	}
}

void EdSchleifeGadgets(struct Gadget *g, uint16 code, BOOL final) {
	int16 id = g->GadgetID;
	
	if ((id >= 5) && (id <= 10)) {
		if (id<10) edgui.raster = VIERTEL+5-id; else edgui.raster = 0;
		AktualisiereEdRasterGadgets();
	} else if ((id >= 11) && (id <= 16)) {
		edgui.neulen = id - 11;
		AktualisiereEdNeuLenGadgets();
	} else {
		switch (id) {
			case 0:
			KeineEventsMarkieren(edseq);
			if (edgui.modus == EDMODUS_CONTR) {
				ZeichneTastatur();
				ZeichneNotenfeld(TRUE, FALSE, FALSE);
				edgui.modus = EDMODUS_NOTEN;
			} else {
				ContrSpurenEinpassen();
				ZeichneControllerSpalten();
				ZeichneControllerFeld(FALSE);
				edgui.modus = EDMODUS_CONTR;
			}
			AktualisiereEdGadgets();
			AktualisiereEdModus();
			break;
			
			case 1:
			if (edgui.takt != (code << (VIERTEL - 2))) {
				edgui.takt = code << (VIERTEL - 2);
				ZeichneEdZeitleiste();
				ZeichneFeld(FALSE);
			}
			break;
	
			case 2:
			if (edgui.modus == EDMODUS_NOTEN) {
				if (edgui.taste != (75 - code - edgui.tastsicht)) {
					edgui.taste = 75 - code - edgui.tastsicht;
					if (edgui.taste<0) edgui.taste = 0;
					ZeichneTastatur(); ZeichneNotenfeld(TRUE, FALSE, FALSE);
				}
			} else {
				if (edgui.contr != code) {
					edgui.contr = code;
					ZeichneControllerSpalten(); ZeichneControllerFeld(FALSE);
				}
			}
			break;
			
			case 3:
			if (edgui.taktb != code) {
				edgui.taktb = code;
				ZeichneEdZeitleiste();
				ZeichneFeld(FALSE);
				if (final) AktualisiereEdGadgets();
			}
			break;
	
			case 4:
			if (edgui.modus == EDMODUS_NOTEN) {
				if (edgui.tasth != code) {
					edgui.tasth = code;
					ZeichneTastatur();
					ZeichneNotenfeld(TRUE, FALSE, FALSE);
				}
			} else {
				edgui.contrh = code;
				if (final) ContrSpurenEinpassen();
				ZeichneControllerSpalten();
				ZeichneControllerFeld(FALSE);
			}
			if (final) AktualisiereEdGadgets();
			break;

			case 17:
			edgui.tripled = code;
			break;
		}
	}
}

void EdNeuLen(int32 len) {
	int32 neulen;
	
	neulen = 5;
	if (len * 4 >= 3 * VIERTELWERT / 4) neulen = 4; // 1/16
	if (len * 4 >= 3 * VIERTELWERT / 2) neulen = 3; // 1/8
	if (len * 4 >= 3 * VIERTELWERT) neulen = 2;     // 1/4
	if (len * 4 >= 3 * 2 * VIERTELWERT) neulen = 1; // 2/4
	if (len * 4 >= 3 * 4 * VIERTELWERT) neulen = 0; // 4/4

	if (neulen != edgui.neulen) {
		edgui.neulen = neulen;
		AktualisiereEdNeuLenGadgets();
	}
}

void EdSchleifeNoten(int16 mousex, int16 mousey, uint16 qualifier) {
	struct IntuiMessage *mes;
	int32 alttakt;
	int32 neutakt;
	int8 neunote;
	BOOL verlassen = FALSE;
	BOOL bewegt = FALSE;
	int16 mx2 = 0, my2 = 0;
	int8 greifp = 0;
	struct EVENT ev1;
	struct EVENT ev2;

	editnote = PunktTaste(mousey);
	neutakt = EdPunktPosition(mousex);
	edittakt = RasterTakt(neutakt);

	wahlnote = TaktNote(neutakt, editnote, edseq, &greifp);
	if (wahlnote) {
		SendeEvent(edseq->spur, wahlnote->status, playnote = wahlnote->data1, wahlnote->data2);
		if (!wahlnote->markiert && !(qualifier & QUALIFIER_SHIFT)) KeineEventsMarkieren(edseq);
		wahlnote->markiert = TRUE;
		NotenEndenMarkieren(edseq);
		ZeichneNotenfeld(FALSE, FALSE, FALSE);

		if (qualifier & QUALIFIER_CTRL) {
			// Velocities ändern...
			KeinePosition();
			ZeichneNotenVelos();
			my2 = mousey;
			do {
				IExec->WaitPort(edfenster->UserPort);
				while ((mes = (struct IntuiMessage *)IExec->GetMsg(edfenster->UserPort))) {
					switch (mes->Class) {
						case IDCMP_MOUSEMOVE:
						bewegt = TRUE;
						MarkEventsDynamik(edseq, 0, 0, (int8)(my2 - mes->MouseY)); my2 = mes->MouseY;
						SendeEvent(edseq->spur, MS_NoteOff, playnote, 0);
						SendeEvent(edseq->spur, wahlnote->status, playnote, wahlnote->data2);
						ZeichneNotenVelos(); ZeichneEdInfobox();
						break;
						
						case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
					}
					IExec->ReplyMsg((struct Message *)mes);
				}
			} while (!verlassen);
			if (bewegt) AddEdUndo(edseq, CAT(MSG_0598, "Edit Dynamics"));
			ZeichneNotenfeld(TRUE, FALSE, FALSE);
		} else {
			// Noten bearbeiten...
			if (qualifier & QUALIFIER_ALT) {
				MarkEventsKopieren(edseq);
				greifp = 0;
				bewegt = TRUE;
			}
			if (greifp == 0) alttakt = edittakt; else alttakt = neutakt;
			do {
				IExec->WaitPort(edfenster->UserPort);
				while ((mes = (struct IntuiMessage *)IExec->GetMsg(edfenster->UserPort))) {
					switch (mes->Class) {
						case IDCMP_MOUSEMOVE:
						if (greifp == 0) neutakt = RasterTakt(EdPunktPosition(mes->MouseX));
						else neutakt = EdPunktPosition(mes->MouseX);
						neunote = PunktTaste(mes->MouseY);
						if ((alttakt != neutakt) || (editnote != neunote)) {
							
							if (editnote != neunote) {
								SendeEvent(edseq->spur, MS_NoteOff, playnote, 0);
								SendeEvent(edseq->spur, wahlnote->status, playnote = neunote, wahlnote->data2);
							}
							if (bewegt) ZeichneNotenfeld(FALSE, TRUE, FALSE);
							bewegt = TRUE;
							if (greifp == 0) {
								MarkNotenVerschieben(edseq, neutakt - alttakt, neunote - editnote);
							} else {
								MarkNotenEndenVerschieben(edseq, neutakt-alttakt);
							}
							alttakt = neutakt;
							editnote = neunote;
							ZeichneNotenfeld(FALSE, TRUE, FALSE);
							ZeichneNotenanzeige();
							if (greifp == 0) ZeichneEdInfobox();
						}
						break;
						
						case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
					}
					IExec->ReplyMsg((struct Message *)mes);
				}
			} while (!verlassen);
			if (bewegt) {
				OrdneEvents(edseq);
				AddEdUndo(edseq, CAT(MSG_0599, "Move Notes"));
				ZeichneNotenfeld(TRUE, FALSE, TRUE);
			}
		}
		SendeEvent(edseq->spur, MS_NoteOff, playnote, 0); playnote = -1;
	} else {
		if (!(qualifier & QUALIFIER_SHIFT)) {
			KeineEventsMarkieren(edseq);
			ZeichneNotenfeld(FALSE, FALSE, FALSE);
		}
		if (qualifier & QUALIFIER_ALT) {
			// Neue Note erzeugen...
			wahlnote = &ev1;
			ev1.zeit = edittakt - edseq->start;
			ev1.status = MS_NoteOn;
			ev1.data1 = editnote;
			ev1.data2 = 100;
			ev2.zeit = ev1.zeit + ((VIERTELWERT << 2) >> edgui.neulen) - (VIERTELWERT >> 5);
			ev2.status = MS_NoteOff;
			ev2.data1 = editnote;
			alttakt = neutakt;

			ZeichneNote(&ev1, &ev2, TRUE);
			SendeEvent(edseq->spur, MS_NoteOn, playnote = ev1.data1, ev1.data2);
			do {
				IExec->WaitPort(edfenster->UserPort);
				while ((mes = (struct IntuiMessage *)IExec->GetMsg(edfenster->UserPort))) {
					switch (mes->Class) {
						case IDCMP_MOUSEMOVE:
						neutakt = EdPunktPosition(mes->MouseX);
						neunote = PunktTaste(mes->MouseY);
						if ((alttakt != neutakt) || (editnote != neunote)) {
							ZeichneNote(&ev1, &ev2, TRUE);
							if (alttakt != neutakt) {
								ev2.zeit += neutakt - alttakt;
								EdNeuLen(ev2.zeit - ev1.zeit);
							}
							if (editnote != neunote) {
								ev1.data1 = neunote;
								ev2.data1 = neunote;
								SendeEvent(edseq->spur, MS_NoteOff, playnote, 0);
								SendeEvent(edseq->spur, MS_NoteOn, playnote = neunote, 100);
							}
							if (ev2.zeit < ev1.zeit+(VIERTELWERT>>4)) ev2.zeit = ev1.zeit + (VIERTELWERT >> 4);
							ZeichneNote(&ev1, &ev2, TRUE);
							ZeichneNotenanzeige();
							if (editnote != neunote) ZeichneEdInfobox();
							alttakt = neutakt;
							editnote = neunote;
						}
						break;
						
						case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
					}
					IExec->ReplyMsg((struct Message *)mes);
				}
			} while (!verlassen);
			wahlnote = EventEinfuegen(edseq, ev1.zeit, MS_NoteOn, editnote, 100, TRUE);
			EventEinfuegen(edseq, ev2.zeit, MS_NoteOff, editnote, 100, TRUE);
			SendeEvent(edseq->spur, MS_NoteOff, playnote, 0); playnote = -1;

			AddEdUndo(edseq, CAT(MSG_0600, "Create Note"));

			KeinePosition();
			ZeichneNote(&ev1, &ev2, FALSE);
			ZeichneSequenzen(edseq->spur, TRUE);
			ZeichnePosition(TRUE);
		} else {
			aktfenster = edfenster;
			if (LassoWahl(mousex, mousey, &mx2, &my2)) {
				NotenBereichMarkieren(edseq, PunktTaste(mousey), PunktTaste(my2), EdPunktPosition(mousex), EdPunktPosition(mx2));
				ZeichneNotenfeld(FALSE, FALSE, FALSE);
			}
		}
	}
	ZeichneEdInfobox();
}

void EdSchleifeContr(int16 mousex, int16 mousey, uint16 qualifier) {
	struct IntuiMessage *mes;
	BOOL verlassen = FALSE;
	int16 cs;
	int8 contr;
	int32 neutakt;
	struct EVENT *fcev;
	BOOL mitte, onoff;
	uint8 neustatus;
	int8 neudata1;
	BOOL erzeugt = FALSE;
	BOOL bewegt = FALSE;

	cs = PunktContrSpur(mousey);
	if (cs < edgui.contranz) {
		contr = contrspur[cs];
		if (contr >= 0) {
			neustatus = MS_Ctrl;
			neudata1 = contr;
			mitte = edinstrcontr->flags[contr] & CONTR_MITTE;
			onoff = edinstrcontr->flags[contr] & CONTR_ONOFF;
		} else {
			neustatus = MS_PolyPress + ((contr + 5) << 4);
			neudata1 = 0;
			mitte = (contr == -1);
			onoff = FALSE;
		}
		
		neutakt = EdPunktPosition(mousex);
		edittakt = RasterTakt(neutakt);

		KeinePosition();
		if (qualifier & QUALIFIER_SHIFT) {
			// Markieren...
			wahlnote = TaktContr(neutakt, contr, edseq, &fcev);
			if (wahlnote) {
				wahlnote->markiert = TRUE;
				ZeichneContr(cs, wahlnote, fcev, mitte, TRUE);
			}
			do {
				IExec->WaitPort(edfenster->UserPort);
				while ((mes = (struct IntuiMessage *)IExec->GetMsg(edfenster->UserPort))) {
					switch (mes->Class) {
						case IDCMP_MOUSEMOVE:
						neutakt = EdPunktPosition(mes->MouseX);
						wahlnote = TaktContr(neutakt, contr, edseq, &fcev);
						if (wahlnote) {
							wahlnote->markiert = TRUE;
							ZeichneContr(cs, wahlnote, fcev, mitte, TRUE);
							ZeichneEdContrInfobox();
						}
						break;
	
						case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
					}
					IExec->ReplyMsg((struct Message *)mes);
				}
			} while (!verlassen);

		} else {
			// Wert ändern oder neu erzeugen...
			wahlnote = TaktContr(neutakt, contr, edseq, &fcev);
			if (wahlnote) {
				if ((qualifier & QUALIFIER_ALT) && (wahlnote->zeit + edseq->start != edittakt)) wahlnote = NULL;
			}
			if (wahlnote) {
				wahlnote->data2 = PunktContrWert(mousey, cs, onoff);
				ZeichneContr(cs, wahlnote, fcev, mitte, TRUE);
				bewegt = TRUE;
			} else if (qualifier & QUALIFIER_ALT) {
				wahlnote = EventEinfuegen(edseq, edittakt - edseq->start, neustatus, neudata1, PunktContrWert(mousey, cs, onoff), FALSE);
				if (wahlnote) ZeichneContrVorschau(cs, wahlnote, mitte);
				erzeugt = TRUE;
			}
			do {
				IExec->WaitPort(edfenster->UserPort);
				while ((mes = (struct IntuiMessage *)IExec->GetMsg(edfenster->UserPort))) {
					switch (mes->Class) {
						case IDCMP_MOUSEMOVE:
						neutakt = EdPunktPosition(mes->MouseX);
						edittakt = RasterTakt(neutakt);
	
						wahlnote = TaktContr(neutakt, contr, edseq, &fcev);
						if (wahlnote) {
							if ((qualifier & QUALIFIER_ALT) && (wahlnote->zeit + edseq->start != edittakt)) wahlnote = NULL;
						}
						if (wahlnote) {
							wahlnote->data2 = PunktContrWert(mes->MouseY, cs, onoff);
							ZeichneContr(cs, wahlnote, fcev, mitte, TRUE);
							bewegt = TRUE;
						} else if (qualifier & QUALIFIER_ALT) {
							wahlnote = EventEinfuegen(edseq, edittakt - edseq->start, neustatus, neudata1, PunktContrWert(mes->MouseY, cs, onoff), FALSE);
							if (wahlnote) ZeichneContrVorschau(cs, wahlnote, mitte);
							erzeugt = TRUE;
						}
						ZeichneEdContrInfobox();
						break;
	
						case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
					}
					IExec->ReplyMsg((struct Message *)mes);
				}
			} while (!verlassen);
			if (erzeugt) {
				ZeichneControllerSpur(cs);
				AddEdUndo(edseq, CAT(MSG_0601, "Create Controller"));
			}
			if (bewegt && !erzeugt) AddEdUndo(edseq, CAT(MSG_0602, "Move Controller"));
			ZeichneSequenzen(edseq->spur, TRUE);
		}
		ZeichnePosition(TRUE);
		ZeichneEdContrInfobox();
	}
}

void NeuerController(void) {
	int8 c;
	
	c = InstrControllerFenster(edfenster, spur[edseq->spur].channel, spur[edseq->spur].port, -128);
	if (c > -128) {
		contrspur[edgui.contranz] = c;
		edgui.contranz++;
		ZeichneControllerSpalten();
		KeinePosition();
		ZeichneControllerSpur(edgui.contranz - 1);
		ZeichnePosition(TRUE);
		AktualisiereEdGadgets();
	}
}


void KontrolleEditorNotenFenster(void) {
	struct IntuiMessage *mes;
	struct IntuiMessage mescpy;
	BOOL schliessen = FALSE;
	int8 note;
	uint32 b;
	int8 u, v, w;
	int16 n;
	int16 updateid = -1, updatecode = -1;

	while ((mes = (struct IntuiMessage *)IExec->GetMsg(edfenster->UserPort))) {
		if (mes->Class == IDCMP_IDCMPUPDATE) SchleifeUpdate((struct TagItem *)mes->IAddress, &updateid, &updatecode);
		mescpy.Class = mes->Class;
		mescpy.Code = mes->Code;
		mescpy.IAddress = mes->IAddress;
		mescpy.Qualifier = mes->Qualifier;
		mescpy.MouseX = mes->MouseX;
		mescpy.MouseY = mes->MouseY;
		IExec->ReplyMsg((struct Message *)mes);
		
		switch (mescpy.Class) {
			case IDCMP_CLOSEWINDOW: schliessen = TRUE; break;

			case IDCMP_NEWSIZE:
			aktfenster = edfenster; BildFrei();
			ZeichneEdZeitleiste();
			if (edgui.modus == EDMODUS_NOTEN) {
				ZeichneTastatur();
				ZeichneNotenfeld(TRUE, FALSE, FALSE);
			} else {
				ContrSpurenEinpassen();
				ZeichneControllerSpalten();
				ZeichneControllerFeld(FALSE);
			}
			ZeichneEdInfobox();
			PositioniereEdGadgets();
			AktualisiereEdGadgets();
			break;

			case IDCMP_GADGETUP:
			EdSchleifeGadgets((struct Gadget *)mescpy.IAddress, mescpy.Code, TRUE);
			break;
			
			case IDCMP_MENUPICK:
			b = EdMenuPunkt(mescpy.Code);
			MinMenuKontrolle(b);
			switch (b) {
				case MENU_EDIT_UNDO:
				if (EdUndo(edseq)) ZeichneFeld(TRUE);
				break;

				case MENU_EDIT_REDO:
				if (EdRedo(edseq)) ZeichneFeld(TRUE);
				break;
				
				case MENU_EV_CONTRHINZU:
				NeuerController();
				break;
				
				case MENU_EV_LOESCHEN:
				MarkEventsEntfernen(edseq);
				AddEdUndo(edseq, CAT(MSG_0603, "Delete"));
				ZeichneFeld(TRUE);
				break;
				
				case MENU_EV_MARK_ALLE:
				NotenMarkieren(edseq, 0, 0);
				ZeichneNotenfeld(FALSE, FALSE, FALSE);
				break;
				
				case MENU_EV_MARK_NICHTS:
				KeineEventsMarkieren(edseq);
				ZeichneControllerFeld(FALSE);
				break;

				case MENU_EV_MARK_HOEHERE:
				if (wahlnote) {
					NotenMarkieren(edseq, 2, wahlnote->data1);
					ZeichneNotenfeld(FALSE, FALSE, FALSE);
				} else Meldung(CAT(MSG_0606, "A reference note must be marked"));
				break;

				case MENU_EV_MARK_TIEFERE:
				if (wahlnote) {
					NotenMarkieren(edseq, 3, wahlnote->data1);
					ZeichneNotenfeld(FALSE, FALSE, FALSE);
				} else Meldung(CAT(MSG_0606, "A reference note must be marked"));
				break;

				case MENU_EV_MARK_ANSCHLAEGE:
				if (wahlnote) {
					NotenMarkieren(edseq, 4, wahlnote->data2);
					ZeichneNotenfeld(FALSE, FALSE, FALSE);
				} else Meldung(CAT(MSG_0606, "A reference note must be marked"));
				break;

				case MENU_EV_MARK_KANAL:
				if (wahlnote) {
					NotenMarkieren(edseq, 5, wahlnote->status & MS_ChanBits);
					ZeichneNotenfeld(FALSE, FALSE, FALSE);
				} else Meldung(CAT(MSG_0606, "A reference note must be marked"));
				break;
				
				case MENU_EV_DYNAMIK:
				u = 64; v = 0; w = 0;
				if (DynamikFenster(edfenster, &u, &v, &w)) {
					MarkEventsDynamik(edseq, u, v, w);
					AddEdUndo(edseq, CAT(MSG_0609, "Edit Dynamics"));
					if (edgui.modus == EDMODUS_CONTR) ZeichneControllerFeld(FALSE);
					ZeichneEdInfobox();
				}
				break;
				
				case MENU_EV_QUANT_ON:
				if (edgui.raster > 0) {
					MarkNotenQuantisieren(edseq, edgui.raster, 0, edgui.tripled);
					AddEdUndo(edseq, CAT(MSG_0610, "Quantize"));
					ZeichneNotenfeld(TRUE, FALSE, TRUE);
					ZeichneEdInfobox();
				} else Meldung(CAT(MSG_0611, "Quantizing on free grid is useless"));
				break;

				case MENU_EV_QUANT_OFF:
				if (edgui.raster > 0) {
					MarkNotenQuantisieren(edseq, edgui.raster, 1, edgui.tripled);
					AddEdUndo(edseq, CAT(MSG_0612, "Quantize (Ends)"));
					ZeichneNotenfeld(TRUE, FALSE, FALSE);
					ZeichneEdInfobox();
				} else Meldung(CAT(MSG_0611, "Quantizing on free grid is useless"));
				break;

				case MENU_EV_QUANT_ONNEAR:
				if (edgui.raster > 0) {
					MarkNotenQuantisieren(edseq, edgui.raster, 2, edgui.tripled);
					AddEdUndo(edseq, CAT(MSG_0614, "Quantize (Near)"));
					ZeichneNotenfeld(TRUE, FALSE, TRUE);
					ZeichneEdInfobox();
				} else Meldung(CAT(MSG_0611, "Quantizing on free grid is useless"));
				break;
				
				case MENU_EV_REPARIEREN:
				RepariereNoten(edseq);
				AddEdUndo(edseq, CAT(MSG_0616, "Repair"));
				ZeichneNotenfeld(TRUE, FALSE, TRUE);
				break;
				
				case MENU_EV_REDUZIEREN:
				n = QuantisierungsFenster(edfenster, TRUE);
				if (n) {
					MarkContrReduzieren(edseq, n);
					AddEdUndo(edseq, CAT(MSG_0617, "Reduce"));
					ZeichneControllerFeld(TRUE);
				}
				break;
				
				case MENU_EV_GLAETTEN:
				MarkContrGlaetten(edseq);
				AddEdUndo(edseq, CAT(MSG_0618, "Smooth"));
				ZeichneControllerFeld(TRUE);
				break;
			}
			break;

			case IDCMP_RAWKEY: {

#ifndef __amigaos4__
				//os3 (newmouse) mouse wheel
				int16 vscroll = 0;
				int16 hscroll = 0;

				if (!umgebung.mausradtauschen) {
					switch (mescpy.Code) {
						case NM_WHEEL_UP: vscroll = -1;	break;
						case NM_WHEEL_DOWN: vscroll = 1; break;
						case NM_WHEEL_LEFT: hscroll = -1; break;
						case NM_WHEEL_RIGHT: hscroll = 1; break;
					}
				} else {
					switch (mescpy.Code) {
						case NM_WHEEL_UP: hscroll = -1;	break;
						case NM_WHEEL_DOWN: hscroll = 1; break;
						case NM_WHEEL_LEFT: vscroll = -1; break;
						case NM_WHEEL_RIGHT: vscroll = 1; break;
					}
				}

				if (hscroll != 0) {
					edgui.takt += hscroll * (edgui.taktsicht / 6 * (VIERTELWERT >> 2));
					if (edgui.takt < 0) edgui.takt = 0;
					ZeichneEdZeitleiste(); ZeichneFeld(FALSE); AktualisiereEdGadgets();
				}
				if (vscroll != 0) {
					if (edgui.modus == EDMODUS_NOTEN) {
						edgui.taste -= vscroll * 3;
						if (edgui.taste + edgui.tastsicht > 75) edgui.taste = 75 - edgui.tastsicht;
						if (edgui.taste < 0) edgui.taste = 0;
						ZeichneTastatur();
					} else {
						edgui.contr += vscroll;
						if (edgui.contr + edgui.contrsicht > edgui.contranz + 1) edgui.contr = edgui.contranz - edgui.contrsicht + 1;
						if (edgui.contr < 0) edgui.contr = 0;
						ZeichneControllerSpalten();
					}
					ZeichneFeld(FALSE); AktualisiereEdGadgets();
				}

#endif


				switch (mescpy.Code) {
					case 76:
					if (edgui.modus == EDMODUS_NOTEN) {
						if (mescpy.Qualifier & QUALIFIER_SHIFT) {
							edgui.tasth += 1;
						} else {
							edgui.taste += 7;
							if (edgui.taste + edgui.tastsicht > 75) edgui.taste = 75 - edgui.tastsicht;
						}
						ZeichneTastatur();
					} else {
						if (edgui.contr > 0) edgui.contr--;
						ZeichneControllerSpalten();
					}
					ZeichneFeld(FALSE); AktualisiereEdGadgets();
					break;
					
					case 77:
					if (edgui.modus == EDMODUS_NOTEN) {
						if (mescpy.Qualifier & QUALIFIER_SHIFT) {
							if (edgui.tasth > 6) edgui.tasth -= 1;
						} else {
							edgui.taste -= 7;
							if (edgui.taste<0) edgui.taste = 0;
						}
						ZeichneTastatur();
					} else {
						if (edgui.contr + edgui.contrsicht < edgui.contranz + 1) edgui.contr++;
						ZeichneControllerSpalten();
					}
					ZeichneFeld(FALSE); AktualisiereEdGadgets();
					break;
					
					case 78:
					if (mescpy.Qualifier & QUALIFIER_SHIFT) edgui.taktb += 1;
					else edgui.takt += (edgui.taktsicht / 3 * (VIERTELWERT >> 2));
					ZeichneEdZeitleiste(); ZeichneFeld(FALSE); AktualisiereEdGadgets();
					break;
					
					case 79:
					if (mescpy.Qualifier & QUALIFIER_SHIFT) {
						if (edgui.taktb > 2) edgui.taktb -= 1;
					} else {
						edgui.takt -= (edgui.taktsicht / 3 * (VIERTELWERT >> 2));
						if (edgui.takt < 0) edgui.takt = 0;
					}
					ZeichneEdZeitleiste(); ZeichneFeld(FALSE); AktualisiereEdGadgets();
					break;
				}
				TransportKontrolleRaw(mescpy.Code);
			}
			break;

			case IDCMP_VANILLAKEY:
			TransportKontrolle(mescpy.Code);
			switch (mescpy.Code) {
				case 127:
				MarkEventsEntfernen(edseq);
				AddEdUndo(edseq, CAT(MSG_0619, "Delete"));
				ZeichneFeld(TRUE);
				break;
			}
			break;

			case IDCMP_MOUSEMOVE:
			b = TesteEdPunktBereich(mescpy.MouseX, mescpy.MouseY);
			if ((b == AREA_SEQUENZEN) || (b == AREA_SPUREN)) {
				if (edgui.modus == EDMODUS_NOTEN) {
					note = PunktTaste(mescpy.MouseY);
					if ((playnote >= 0) && (playnote != note)) {
						SendeEvent(edseq->spur, MS_NoteOff, playnote, 0);
						SendeEvent(edseq->spur, MS_NoteOn, note, 100);
						playnote = note;
						if (mescpy.Qualifier & QUALIFIER_SHIFT) {
							NotenMarkieren(edseq, 1, playnote);
							ZeichneNotenfeld(FALSE, FALSE, FALSE);
						}
					}
					if (note != editnote) {
						editnote = note; ZeichneNotenanzeige();
					}
				}
				edittakt = RasterTakt(EdPunktPosition(mescpy.MouseX));
				ZeichnePosition(TRUE);
			}
			break;

			case IDCMP_EXTENDEDMOUSE:
				if (mescpy.Code == IMSGCODE_INTUIWHEELDATA) {
					struct IntuiWheelData *data = (struct IntuiWheelData *)mescpy.IAddress;
					int16 vscroll = 0, hscroll = 0;
					BOOL tauschen = umgebung.mausradtauschen;
					if (mescpy.Qualifier & QUALIFIER_ALT) {
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
						edgui.takt += hscroll * (edgui.taktsicht / 6 * (VIERTELWERT >> 2));
						if (edgui.takt < 0) edgui.takt = 0;
						ZeichneEdZeitleiste(); ZeichneFeld(FALSE); AktualisiereEdGadgets();
					}
					if (vscroll != 0) {
						if (edgui.modus == EDMODUS_NOTEN) {
							edgui.taste -= vscroll * 3;
							if (edgui.taste + edgui.tastsicht > 75) edgui.taste = 75 - edgui.tastsicht;
							if (edgui.taste < 0) edgui.taste = 0;
							ZeichneTastatur();
						} else {
							edgui.contr += vscroll;
							if (edgui.contr + edgui.contrsicht > edgui.contranz + 1) edgui.contr = edgui.contranz - edgui.contrsicht + 1;
							if (edgui.contr < 0) edgui.contr = 0;
							ZeichneControllerSpalten();
						}
						ZeichneFeld(FALSE); AktualisiereEdGadgets();
					}
				}
			break;
			
			case IDCMP_MOUSEBUTTONS:
			if (mescpy.Code == 104) {
				switch (TesteEdPunktBereich(mescpy.MouseX, mescpy.MouseY)) {
					case AREA_ZEIT:
					takt = RasterTakt(EdPunktPosition(mescpy.MouseX));
					tick = TaktSmpteTicks(takt);
					ZeichnePosition(FALSE);
					ZeichneAnzeigen(FALSE);
					break;
					
					case AREA_SPUREN:
					if (edgui.modus == EDMODUS_NOTEN) {
						playnote = PunktTaste(mescpy.MouseY);
						SendeEvent(edseq->spur, MS_NoteOn, playnote, 100);
						if (mescpy.Qualifier & QUALIFIER_SHIFT) {
							NotenMarkieren(edseq, 1, playnote);
							ZeichneNotenfeld(FALSE, FALSE, FALSE);
						}
					} else {
						n = PunktContrSpur(mescpy.MouseY);
						if (n < edgui.contranz) {
							ControllerMarkieren(edseq, contrspur[n]);
							ZeichneControllerFeld(FALSE);
						} else {
							if (mescpy.Qualifier & QUALIFIER_ALT) NeuerController();
						}
					}
					break;
					
					case AREA_SEQUENZEN:
					if (edgui.modus == EDMODUS_NOTEN) EdSchleifeNoten(mescpy.MouseX, mescpy.MouseY, mescpy.Qualifier);
					else EdSchleifeContr(mescpy.MouseX, mescpy.MouseY, mescpy.Qualifier);
					break;
				}
			}
			if ((mescpy.Code == 232) && (edgui.modus == EDMODUS_NOTEN) && (playnote >= 0)) {
				SendeEvent(edseq->spur, MS_NoteOff, playnote, 0); playnote = -1;
			}
			break;
		}
		if (!edfenster) break;
	}
	if (updateid != -1) EdSchleifeGadgets((struct Gadget *)edgad[updateid], updatecode, FALSE);
	
	if (schliessen) EntferneEditorNotenFenster();
}
