#include <stdio.h>
#include <string.h>

#include <intuition/intuition.h>
#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Gui.h"
#include "Gui2.h"
#include "Instrumente.h"

extern struct Window *aktfenster;
extern struct Window *edfenster;

extern struct SPUR spur[];
extern struct OUTPORT outport[];

extern int16 edlr;
extern int16 edou;
extern int16 edguibox;
extern struct EDGUI edgui;

extern struct SEQUENZ *edseq;
extern struct EVENT *wahlnote;

int8 contrspur[134];
struct INSTRCONTR *edinstrcontr = NULL;
STRPTR statname[] = {(STRPTR)"PolyPress", (STRPTR)"Ctrl", (STRPTR)"Prog", (STRPTR)"ChanPress", (STRPTR)"PitchBend"};

void InitController(void) {
	BOOL sammlungctrl[128];
	BOOL sammlungstat[5];
	struct EVENTBLOCK *evbl;
	int16 evnum;
	int16 n;
	uint8 status;
	int16 p;
	struct INSTRUMENT *instr;
	
	for (n = 0; n < 5; n++) sammlungstat[n] = FALSE;
	for (n = 0; n < 128; n++) sammlungctrl[n] = FALSE;
	
	// Controller sammeln...
	evbl = edseq->eventblock;
	evnum = 0;
	while (evbl) {
		status = evbl->event[evnum].status & MS_StatBits;
		if (!status) break;
		
		if (status >= MS_PolyPress) {
			if (status == MS_Ctrl) sammlungctrl[evbl->event[evnum].data1] = TRUE;
			else sammlungstat[(status - MS_PolyPress) >> 4] = TRUE;
		}

		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	
	// Controller-Spuren initialisieren...
	p = 0;
	for (n = 0; n < 5; n++) {
		if (sammlungstat[n]) {
			contrspur[p] = n - 5;
			p++;
		}
	}
	for (n = 0; n < 128; n++) {
		if (sammlungctrl[n]) {
			contrspur[p] = n;
			p++;
		}
	}
	edgui.contranz = p;
	for (n = p; n < 134; n++) contrspur[n] = -128;
	
	instr = SucheChannelInstrument(spur[edseq->spur].port, spur[edseq->spur].channel);
	edinstrcontr = instr->contr;
}

void ZeichneControllerSpalte(int16 n) {
	int16 y;
	STRPTR text;
	
	aktfenster = edfenster;
	if ((n >= edgui.contr) && (n < edgui.contr + edgui.contrsicht)) {
		y = 43 + ((n - edgui.contr) * (edgui.contrh + 1));
		if (contrspur[n] > -128) {
			RahmenAus(5, STIL_ND, 2, y, 69, y + edgui.contrh);

			if (contrspur[n] >= 0) text = edinstrcontr->name[contrspur[n]];
			else text = statname[contrspur[n] + 5];
			
			Schreibe(1, 4, y + 10, text, 68);

		} else {
			RahmenAus(0, STIL_ND, 2, y, 69, y + edgui.contrh);
		}
	}
}

void ZeichneControllerSpalten(void) {
	int16 n;
	int16 uy;
	
	uy = edfenster->Height - edou - edguibox - 58;
	edgui.contrsicht = (uy - 43) / (edgui.contrh + 1);
	for(n = 0; n < edgui.contrsicht; n++) ZeichneControllerSpalte(n + edgui.contr);
}

void ZeichneContr(int16 n, struct EVENT *lcev, struct EVENT *fcev, BOOL mitte, BOOL del) {
	int16 uspury, lcy, offsety;
	int16 fcx, lcx, liniex;
	int16 lr, rr;
	uint8 tfarbe, ffarbe;
	
	if (lcev) {
		lr = 72; rr = edfenster->Width - edlr - 21;
		
		// letztes Event
		lcx = lr + (((edseq->start + lcev->zeit - edgui.takt) * edgui.taktb) >> (VIERTEL-2));
		if ((lcx >= lr) && (lcx <= rr)) liniex = lcx; else liniex = -1;
		if (lcx < lr) lcx = lr;
		
		// folgendes Event
		if (fcev) {
			fcx = lr + (((edseq->start + fcev->zeit - edgui.takt) * edgui.taktb) >> (VIERTEL-2)) - 1;
			if (fcx > rr) fcx = rr;
		} else fcx = rr;
	
		if (fcx >= lr) {
			uspury = 41 + ((n - edgui.contr + 1) * (edgui.contrh + 1));
			lcy = uspury - (lcev->data2 * (edgui.contrh - 2) / 127);

			if (mitte) offsety = uspury - (64 * (edgui.contrh - 2) / 127);
			else offsety = uspury;

			if (del) Balken(0, lcx, uspury - edgui.contrh + 2, fcx, uspury);

			if (lcev->markiert) {
				tfarbe = 2;
				ffarbe = 19;
			} else {
				tfarbe = 1;
				ffarbe = 21;
			}
			if (liniex != -1) Linie(tfarbe, liniex, offsety, liniex, lcy);
			
			if (!mitte || (lcy < offsety)) Gradient(ffarbe, STIL_HD, lcx + 1, lcy, fcx, offsety);
			else Gradient(ffarbe, STIL_DH, lcx + 1, offsety, fcx, lcy);
			if (fcx - lcx > 20) {
				if (mitte)
					SchreibeZahl(tfarbe, lcx + 3, uspury - 3, lcev->data2 - 64);
				else
					SchreibeZahl(tfarbe, lcx + 3, uspury - 3, lcev->data2);
			}
		}
	}
}

void ZeichneContrVorschau(int16 n, struct EVENT *ev, BOOL mitte) {
	int16 lr, rr;
	int16 x, y;
	int16 uspury, offsety;

	lr = 72; rr = edfenster->Width - edlr - 21;
	x = lr + (((edseq->start + ev->zeit - edgui.takt) * edgui.taktb) >> (VIERTEL-2));
	if ((x >= lr) && (x <= rr)) {
		uspury = 41 + ((n - edgui.contr + 1) * (edgui.contrh + 1));
		y = uspury - (ev->data2 * (edgui.contrh - 2) / 127);

		if (mitte) offsety = uspury - (64 * (edgui.contrh - 2) / 127);
		else offsety = uspury;

		Linie(0, x, uspury - edgui.contrh + 2, x, uspury);
		Linie(1, x, offsety, x, y);
	}
}

void ZeichneControllerSpur(int16 n) {
	int16 y, my;
	struct EVENTBLOCK *evbl;
	int16 evnum;
	struct EVENT *ev;
	struct EVENT *lev;
	uint8 status;
	BOOL mitte;
	
	aktfenster = edfenster;
	if ((n >= edgui.contr) && (n < edgui.contr + edgui.contrsicht)) {
		y = 43 + ((n - edgui.contr) * (edgui.contrh + 1));
		RahmenAus(0, 0, 71, y, edfenster->Width - edlr - 20, y + edgui.contrh);
	
		if (contrspur[n] > -128) {
			if (contrspur[n] >= 0) {
				mitte = edinstrcontr->flags[contrspur[n]] & CONTR_MITTE;
			} else {
				mitte = (contrspur[n] == -1);
			}
			
			if (mitte) {
				my = y - 1 + edgui.contrh - (64 * (edgui.contrh - 2) / 127);
				Linie(21, 72, my, edfenster->Width - edlr - 21, my);
			} else my = 0;
			evbl = edseq->eventblock;
			evnum = 0;
			lev = NULL;
			while (evbl) {
				ev = &evbl->event[evnum];
				status = ev->status & MS_StatBits;
				if (!status || (edseq->start + ev->zeit > edgui.takt + (edgui.taktsicht << (VIERTEL - 2)))) break;
				
				if (status >= MS_PolyPress) {
					if (status == MS_Ctrl) {
						if (ev->data1 == contrspur[n]) {
							ZeichneContr(n, lev, ev, mitte, FALSE); lev = ev;
						}
					} else {
						if ((status - MS_PolyPress) >> 4 == contrspur[n] + 5) {
							ZeichneContr(n, lev, ev, mitte, FALSE); lev = ev;
						}
					}
				}
		
				evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
			}
			ZeichneContr(n, lev, NULL, mitte, FALSE);
		}
	}
}

void ZeichneControllerFeld(BOOL anders) {
	int16 n;
	int16 uy;

	uy = edfenster->Height - edou - edguibox - 58;

	KeinePosition();
	Balken(0, 2, 43 + (edgui.contrsicht * (edgui.contrh + 1)), edfenster->Width - edlr - 20, uy);
	for(n = 0; n < edgui.contrsicht; n++) {
		ZeichneControllerSpur(n + edgui.contr);
	}
	if (anders) ZeichneSequenzen(edseq->spur, TRUE);
	ZeichnePosition(TRUE);
}

void ContrSpurenEinpassen(void) {
	int16 sicht;
	int16 pixel;
	
	pixel = edfenster->Height - edou - edguibox - 58 - 43;
	if (pixel % (edgui.contrh + 1) > edgui.contrh / 2) sicht = pixel / (edgui.contrh + 1) + 1;
	else sicht = pixel / (edgui.contrh + 1);
	if (sicht > 0) edgui.contrh = (pixel / sicht) - 1;
	else edgui.contrh = pixel - 1;

	if (edgui.contrh > 160) edgui.contrh = (pixel / (sicht + 1)) - 1;
}

void ZeichneEdContrInfobox(void) {
	int16 y;
	char puf[30];
	int32 z;

	aktfenster = edfenster;
	y = edfenster->Height - edou - 37 - edguibox;
	Gradient(4, STIL_DN, 0, y - 1, edfenster->Width - edlr - 1, y + edguibox - 1);
	Linie(1, 0, y, edfenster->Width - edlr - 1, y);
	Linie(2, 0, y + edguibox - 2, edfenster->Width - edlr - 1, y + edguibox - 2);
	Linie(8, 0, y + edguibox - 1, edfenster->Width - edlr - 1, y + edguibox - 1);


	if (wahlnote) {
		z = wahlnote->zeit;
		sprintf(puf, "%ld | %ld | %ld", z >> VIERTEL, (z & ~VIERTELMASKE) >> (VIERTEL - 2), (z & ~VIERTELMASKE) % (VIERTELWERT >> 2));
		Schreibe(2, 6, y + 12, CAT(MSG_0545, "Rel Pos:"), 48); Schreibe(1, 50, y + 12, puf, 144);
		Schreibe(2, 130, y + 12, CAT(MSG_0546, "Control.:"), 178); SchreibeZahl(1, 180, y + 12, wahlnote->data1);
		Schreibe(2, 230, y + 12, CAT(MSG_0547, "Value:"), 278); SchreibeZahl(1, 280, y + 12, wahlnote->data2);
		Schreibe(2, 330, y + 12, CAT(MSG_0548, "Channel:"), 378); SchreibeZahl(1, 380, y + 12, (wahlnote->status & MS_ChanBits) + 1);
	}
}


int16 PunktContrSpur(int16 my) {
	my = my - edfenster->BorderTop - 43;
	return(edgui.contr + (my / (edgui.contrh + 1)));
}

int8 PunktContrWert(int16 my, int16 cs, BOOL onoff) {
	int8 erg;
	
	my = my - edfenster->BorderTop - 43;
	my = my - ((edgui.contrh + 1) * (cs - edgui.contr));
	if (my < 0) my = 0;
	if (my > edgui.contrh) my = edgui.contrh;
	erg = 127 - (my * 127 / edgui.contrh);
	if (onoff) {
		if (erg < 64) return(0);
		else return(127);
	} else return(erg);
}
