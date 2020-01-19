#include <stdio.h>
#include <string.h>

#include <intuition/intuition.h>
#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Gui.h"
#include "Marker.h"
#include "Smpte.h"
#include "Instrumente.h"
#include "DTGrafik.h"
#include "Versionen.h"

extern struct Window *hfenster;
extern struct Window *aktfenster;

struct GUI gui = {
	0, //spur
	10, //spsicht
	32, //sph
	0, //takt
	20, //tasicht
	32, //tab
	160, //spalte
	FALSE //folgen
};
int16 guibox = 56;
int16 guileiste = 58;
extern int16 randlr;
extern int16 randou;

extern struct SPUR spur[];
extern struct LIED lied;
extern struct OUTPORT outport[];
extern struct LOOP loop;
extern struct MARKER *rootmark;
extern struct SEQUENZINFO seqinfo;

extern int16 snum;
extern int32 takt;
extern int32 tick;
extern int8 mtyp;
extern struct SEQUENZ *wahlseq;
extern struct MARKER *wahlmark[];
extern struct MARKER *ltmark;
extern struct MARKER *lkmark;
extern struct MARKER *lxmark;

extern struct MPKANAL mpkanal[OUTPORTS][16];
extern struct AUTOKANAL autokanal[OUTPORTS][16];


void InitGuiWerte(void) {
	gui.spur = 0;
	gui.sph = 32;
	gui.takt = 0;
	gui.tab = 32;
	gui.spalte = 160;
	gui.folgen = FALSE;
}

void LoescheLinksOben(void) {
	aktfenster = hfenster;
	Balken(0, 60, 0, gui.spalte + 2, guileiste + 16);
}

void ZeichneUebersicht(void) {
	int16 x, y, xr, yu;
	FLOAT faktbr, faktho;
	struct SEQUENZ *seq;
	int16 s;
	int16 rxs, rxe;
	int16 zxs, zxe, zy;
	
	x = 418;
	y = hfenster->Height - randou - 33;
	xr = hfenster->Width - randlr - 4;
	yu = y + 27;
	
	faktbr = (FLOAT)(xr - x) / (FLOAT)lied.taktanz;
	faktho = (FLOAT)(yu - y) / (FLOAT)lied.spuranz;
	if (faktho > 3) faktho = 3;
	
	aktfenster = hfenster;
	RahmenEin(1, 0, x - 1, y - 1, xr + 1, yu + 1);
	RahmenRundungEin(x - 2, y - 2, xr + 2, yu + 2);
	rxs = (int16)(x + (FLOAT)(gui.takt >> VIERTEL) * faktbr);
	rxe = (int16)(x + (FLOAT)((gui.takt >> VIERTEL) + gui.tasicht) * faktbr);
	if (rxs > xr) rxs = xr;
	if (rxe > xr) rxe = xr;
	Gradient(5, STIL_DN, rxs, y, rxe, yu);
	
	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			zxs = (int16)(x + (FLOAT)(seq->start >> VIERTEL) * faktbr);
			while (seq->next) {
				if (seq->ende < seq->next->start) break;
				seq = seq->next;
			}
			zxe = (int16)(x + (FLOAT)(seq->ende  >> VIERTEL) * faktbr);
			zy = (int16)(y + (FLOAT)s * faktho);
			if (zxs > xr) zxs = xr;
			if (zxe > xr) zxe = xr;
			Balken(2, zxs, zy, zxe, zy + 1);
			seq = seq->next;
		}
	}
	Linie(6, rxs, y, rxs, yu);
	Linie(6, rxe, y, rxe, yu);
}

int32 TestePunktUebersicht(int16 x) {
	int32 ergtakt;
	int16 br;
	FLOAT faktbr;
	
	x = x - hfenster->BorderLeft - 418;
	br = hfenster->Width - randlr - 418;
	faktbr = (FLOAT)br / (FLOAT)lied.taktanz;
	ergtakt = ((int32)((FLOAT)x / faktbr) - (gui.tasicht / 2)) << VIERTEL;
	if (ergtakt < 0) ergtakt = 0;
	return(ergtakt);
}

void ZeichneSpurAutomation(int16 s) {
	int8 p, c;
	int16 lr, rr;
	int16 yu;
	int16 x, y;
	int16 altx, alty;
	struct AUTOPUNKT *akt;
	
	aktfenster = hfenster;
	if ((s >= gui.spur) && (s < gui.spur + gui.spsicht)) {
		lr = gui.spalte + 2;
		rr = hfenster->Width - randlr - 21;
		yu = guileiste + 16 + ((s - gui.spur + 1) * gui.sph);
		altx = ~0;

		akt = autokanal[spur[s].port][spur[s].channel].liste[spur[s].autostatus - 1];
		
		if (spur[s].autostatus == 2) {
			y = yu - (MCCenter * (gui.sph - 1) / 128);
			PunktLinie(4, lr, y, rr, y);
		}
		
		// Normale Linie
		if (akt) {
			if (akt->takt > gui.takt) {
				if (akt->takt < gui.takt + (gui.tasicht << VIERTEL))
					x = lr + ((((akt->takt - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6) - 3;
				else x = rr;
			} else x = -1;
		} else x = rr;
		if (x != -1) { // Linie sichtbar
			p = spur[s].port;
			c = spur[s].channel;
			switch (spur[s].autostatus) {
				case 1: y = yu - ((int16)mpkanal[p][c].fader * (gui.sph - 1) / 128); break;
				case 2: y = yu - ((int16)mpkanal[p][c].pan * (gui.sph - 1) / 128); break;
				default: y = yu - ((int16)mpkanal[p][c].contrwert[spur[s].autostatus - 3] * (gui.sph - 1) / 128);
			}
			Linie(4, lr, y, x, y);
		}
			
		// Automations-Linie
		while (akt) {
			x = lr + ((((akt->takt - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
			y = yu - ((int16)akt->wert * (gui.sph - 1) / 128);
			
			if (x >= lr) {
				if (x <= rr) Balken(1, x - 2, y - 2, x + 2, y + 2);
				if (altx != ~0) {
					if (altx < lr) {
						alty = (int16)((((int32)y * ((int32)lr - (int32)altx)) + ((int32)alty * ((int32)x - (int32)lr))) / ((int32)x - (int32)altx));
						altx = lr;
					}
					if (x > rr) {
						y = (int16)((((int32)alty * ((int32)x - (int32)rr)) + ((int32)y * ((int32)rr - (int32)altx))) / ((int32)x - (int32)altx));
						x = rr;
					}
					Linie(1, altx, alty, x, y);
				}
			}
			altx = x; alty = y;
			if (akt->takt > gui.takt + (gui.tasicht << VIERTEL)) break;
			akt = akt->next;
		}
		if (!akt && (altx != ~0)) {
			altx += 3;
			if (altx <= rr) {
				if (altx < lr) altx = lr;
				Linie(4, altx, alty, rr, alty);
			}
		}
	}
}

void ZeichneSequenzen(int16 s, BOOL events) {
	struct SEQUENZ *seq;
	int16 y, yn;
	int16 xs, xe, xn, lx, altx;
	int16 rr;
	struct EVENTBLOCK *evbl;
	int16 evnum;
	BOOL randl, randr;
	uint8 stat;
	int16 lfarbe, hfarbe, tfarbe;
	int16 seqfarbe;
	
	aktfenster = hfenster;
	if ((s >= gui.spur) && (s < gui.spur + gui.spsicht)) {
		events = TRUE;
		rr = hfenster->Width - randlr - 21;
		y = guileiste + 18 + ((s - gui.spur) * gui.sph);
		altx = gui.spalte + 2;
		lx = 0;

		if (spur[s].autostatus == 0) {
			tfarbe = 1;
			if (s % 2) hfarbe = 28; else hfarbe = 29;
			lfarbe = hfarbe;
		} else {
			tfarbe = 4;
			hfarbe = 25;
			lfarbe = 1;
		}

		Linie(hfarbe, gui.spalte + 2, y, rr, y);
		Linie(lfarbe, gui.spalte + 2, y + gui.sph - 1, rr, y + gui.sph - 1);
		
		seq = spur[s].seq;
		while (seq) {
			xs = gui.spalte + 2 + ((((seq->start - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
			xe = gui.spalte + 1 + ((((seq->ende - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
			if (xe - xs < 1) xe = xs + 1;
			if (!((xs < gui.spalte + 2) && (xe < gui.spalte + 2)) && !((xs > rr) && (xe > rr))) {
				if (xs < gui.spalte + 2) xs = gui.spalte + 2;
				if (xe > rr) xe = rr;
				
				if (xs > altx) Balken(hfarbe, altx, y + 1, xs - 1, y + gui.sph - 2);
				if (xe > altx) altx = xe + 1;
				
				if (spur[s].autostatus == 0) {
					if (seq->mute)
						seqfarbe = 0;
					else
						seqfarbe = 9 + spur[s].channel;
						
					if (seq->markiert)
						RahmenEin(seqfarbe, STIL_DH, xs, y + 1, xe, y + gui.sph - 2);
					else
						RahmenAus(seqfarbe, STIL_DH, xs, y + 1, xe, y + gui.sph - 2);
				} else {
					Balken(0, xs, y + 1, xe - 1, y + gui.sph - 2);
					Linie(25, xe, y + 1, xe, y + gui.sph - 2);
				}

				//Events zeichnen...
				if ((spur[s].autostatus == 0) && events && (xe - xs > 10) && (gui.sph >= 16)) {
					randl = FALSE; randr = FALSE;
					evbl = seq->eventblock; evnum = 0; Farbe(2);
					while (evbl) {
						if (!evbl->event[evnum].status) break;
						
						xn = gui.spalte + 2 + ((((seq->start - gui.takt + evbl->event[evnum].zeit) >> (VIERTEL - 6)) * gui.tab) >> 8);
						if (xn < xs) randl = TRUE;
						if (xn > xe + 1) {randr = TRUE; break;}
						if ((xn >= xs) && (xn < xe - 1)) {
							if (xn == xs) xn++;
							stat = (evbl->event[evnum].status & MS_StatBits);
							if (stat == MS_NoteOn) {
								yn = y + gui.sph - 3 - (((int16)evbl->event[evnum].data1 * (gui.sph-4 )) >> 7);
								if (gui.sph < 35) Punkt(xn, yn);
								else Balken(2, xn, yn, xn + 1, yn + 1);
							} else if (stat != MS_NoteOff) {
								xn &= 0xFFFE;
								if (xn != lx) {
									if (xn == xs) xn++;
									Linie(2, xn, y + gui.sph - 3 - (((int16)evbl->event[evnum].data2 * (gui.sph - 4 )) >> 7), xn, y + gui.sph - 3);
									lx = xn;
								}
							}
						}
						
						evnum++; if (evnum == EVENTS) {evnum = 0; evbl = evbl->next;}
					}
					if (randl) {
						Linie(1, xs + 7, y + gui.sph - 12, xs + 3, y + gui.sph - 8);
						Linie(1, xs + 3, y + gui.sph - 8, xs + 7, y + gui.sph - 4);
					}
					if (randr) {
						Linie(1, xe - 6, y + gui.sph - 12, xe - 2, y + gui.sph - 8);
						Linie(1, xe - 2, y + gui.sph - 8, xe - 6, y + gui.sph - 4);
					}
				}
				//...Events Ende

				if (gui.sph >= 16) {
					if (seq->aliasorig) {
						Fett(FALSE);
						Schreibe(tfarbe, xs + 2, y + 11, seq->aliasorig->name, xe - 2);
					} else {
						Fett(TRUE);
						Schreibe(tfarbe, xs + 2, y + 11, seq->name, xe - 2);
					}
					if ((spur[s].autostatus == 0) && (gui.sph > 26) && (xs + 19 < xe) && seq->trans) SchreibeZahl(1, xs + 2, y + 23, seq->trans);
				}
			}
			seq = seq->next;
		}
		if (altx < rr) Balken(hfarbe, altx, y + 1 , rr, y + gui.sph - 2);
		if (spur[s].autostatus) ZeichneSpurAutomation(s);
	}
	Fett(FALSE);
}

void ZeichneSequenzRahmen(int16 s) {
	struct SEQUENZ *seq;
	int16 y;
	int16 xs, xe;
	int16 rr;

	aktfenster = hfenster;
	if ((s >= gui.spur) && (s < gui.spur + gui.spsicht)) {
		if (spur[s].autostatus == 0) {
			rr = hfenster->Width - randlr - 21;
			y = guileiste + 18 + ((s - gui.spur) * gui.sph);
			seq = spur[s].seq;
			while (seq) {
				xs = gui.spalte + 2 + ((((seq->start - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
				xe = gui.spalte + 1 + ((((seq->ende - gui.takt) >> (VIERTEL - 4))*gui.tab) >> 6);
				if (xe - xs < 1) xe = xs + 1;
				if (!((xs < gui.spalte + 2) && (xe < gui.spalte + 2)) && !((xs > rr) && (xe > rr))) {
					if (xs < gui.spalte + 2) xs = gui.spalte + 2;
					if (xe > rr) xe = rr;
					
					if (seq->markiert) RahmenEin(-1, 0, xs, y + 1, xe, y + gui.sph - 2);
					else RahmenAus(-1, 0, xs, y + 1, xe, y + gui.sph - 2);
	
				}
				seq = seq->next;
			}
		}
	}
}

void ZeichneSpurSpalte(int16 s, BOOL aktiv) {
	int16 y;
	int16 n;
	int8 contr;
	int8 p, c;
	int8 num;
	BOOL autov = FALSE;
	struct INSTRUMENT *instr;
	
	aktfenster = hfenster;
	if ((s >= gui.spur) && (s < gui.spur + gui.spsicht)) {
		p = spur[s].port;
		c = spur[s].channel;
		n = s - gui.spur;
		y = guileiste + 18 + (n * gui.sph);

		Fett(TRUE);
		RahmenAus(9 + spur[s].channel, 0, 2, y, 25, y + gui.sph - 1);
		if (aktiv)
			RahmenAus(6, STIL_DH, 26, y, gui.spalte, y + gui.sph - 1);
		else
			RahmenAus(5, STIL_ND, 26, y, gui.spalte, y + gui.sph - 1);

		if (gui.sph >= 14) {
			SchreibeZahl(1, 4, y + 10, s + 1);
			Schreibe(1, 50, y + 10, spur[s].name, gui.spalte);
			
			if (spur[s].mute) BlitteBitMap(BMAP_MUTE_ON, 0, 0, 30, y + 2, 17, 10); // Mute
			else BlitteBitMap(BMAP_MUTE_OFF, 0, 0, 30, y + 2, 17, 10);
			
			if (gui.spalte > 80) { // Automation
				for (num = 0; num < 8; num++) if (autokanal[p][c].liste[num]) autov = TRUE;
				if (autov) BlitteBitMap(BMAP_AUTO, 0, 0, gui.spalte - 18, y + 2, 16, 10);
			}
		}
		Fett(FALSE);
		if (gui.sph >= 26) {
			if (spur[s].autostatus == 1) Schreibe(1, 50, y + 22, CAT(MSG_0218, "Volume"), gui.spalte);
			if (spur[s].autostatus == 2) Schreibe(1, 50, y + 22, CAT(MSG_0219, "Panorama"), gui.spalte);
			if (spur[s].autostatus >= 3) {
				contr = mpkanal[p][c].contr[spur[s].autostatus - 3];
				if (contr >= 0) {
					instr = SucheChannelInstrument(p, c);
					Schreibe(1, 50, y + 22, instr->contr->name[contr], gui.spalte);
				} else Schreibe(1, 50, y + 22, (STRPTR)"---", gui.spalte);
			}
		}
	}
}

void ZeichneSpuren(BOOL spalte, BOOL events) {
	int16 s;
	int16 n;
	int16 y;
	uint8 hfarbe;

	aktfenster = hfenster;
	gui.spsicht = (hfenster->Height - randou - (74 + guileiste) - guibox) / gui.sph;
	Balken(0, 2, guileiste + 18 + (gui.spsicht * gui.sph), hfenster->Width - randlr - 20, hfenster->Height - randou - 56 - guibox);
	
	RahmenEin(-1, 0, gui.spalte + 1, guileiste + 17, hfenster->Width - randlr - 20, guileiste + 18 + (gui.spsicht * gui.sph));
	
	for(n = 0; n < gui.spsicht; n++) {
		s = n + gui.spur;
		y = guileiste + 18 + (n * gui.sph);
		if (s < lied.spuranz) {
			if (spalte) ZeichneSpurSpalte(s, s == snum);
			ZeichneSequenzen(s, events);
		} else {
			if (spalte) RahmenAus(0, STIL_ND, 2, y, gui.spalte, y + gui.sph - 1);
			if (s % 2) hfarbe = 28; else hfarbe = 29;
			Balken(hfarbe, gui.spalte + 2, y, hfenster->Width - randlr - 21, y + gui.sph - 1);
		}
		Fett(FALSE);
	}
}

void SpurenEinpassen(void) {
	int16 sicht;
	int16 pixel;
	
	pixel = hfenster->Height - randou - (74 + guileiste) - guibox;
	if (pixel % gui.sph > gui.sph / 2) sicht = pixel / gui.sph + 1;
	else sicht = pixel / gui.sph;
	if (sicht > 0) gui.sph = pixel / sicht;
	else gui.sph = pixel;
	
	if (gui.sph > 100) gui.sph = pixel / (sicht + 1);
}

int16 PunktSpur(int16 y) {
	int16 s;

	s = ((y - hfenster->BorderTop - (guileiste + 18)) / gui.sph) + gui.spur;
	if (s < 0) s = 0;
	return(s);
}

int32 PunktPosition(int16 x) {
	int32 p;

	x = x - hfenster->BorderLeft - gui.spalte - 2;
	p = ((x * 4 / gui.tab) << VIERTEL) + gui.takt;
	return(p);
}

int8 PunktAutoWert(int16 spur, int16 y) {
	int16 wert;
	
	y = y - hfenster->BorderTop  - (guileiste + 18);
	y = gui.sph - (y - ((spur - gui.spur) * gui.sph)) - 2;
	wert = y * 127 / (gui.sph - 2);
	if (wert < 0) wert = 0;
	if (wert > 127) wert = 127;
	return((int8)wert);
}

void ZeichneZeitleiste(BOOL zahlen) {
	int16 xs, xe;
	int16 lr, rr;
	int16 x;
	int32 t;
	int8 a;
	int16 viertel;
	struct MARKER *akt;
	int8 farbe;

	aktfenster = hfenster;
	lr = gui.spalte + 2;
	rr = hfenster->Width - randlr - 20;

	Balken(2, lr, guileiste, rr, guileiste + 14);

	xs = gui.spalte + 2 + ((((loop.start - gui.takt) >> VIERTEL) * gui.tab) >> 2);
	xe = gui.spalte + 2 + ((((loop.ende - gui.takt) >> VIERTEL) * gui.tab) >> 2);
	if (xs < xe) {
		if (!((xs < lr) && (xe < lr)) && !((xs > rr) && (xe > rr))) {
			if (xs < lr) xs = lr;
			if (xe > rr) xe = rr;
			Balken(6, xs, guileiste + 10, xe, guileiste + 14);
		}
	}


	gui.tasicht = 0;
	x = lr; t = gui.takt;
	a = 1;
	if (gui.tab < 28) a = 2;
	if (gui.tab < 20) a = 4;
	if (gui.tab < 14) a = 8;
	if (gui.tab < 8) a = 16;
	akt = TaktMarker(NULL, M_TAKT, t);

	while (TRUE) {
		x = lr + ((((t - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
		if (x > rr) break;
		if (x > rr - 20) zahlen = FALSE;

		akt = TaktMarker(akt, M_TAKT, t);
		
		viertel = (t - akt->takt) >> VIERTEL;

		if (t > (lied.taktanz << VIERTEL)) farbe = 0; else farbe = 1;

		if (viertel % akt->m_zaehler == 0) {
			if ((viertel / akt->m_zaehler) % a == 0) {
				if (zahlen) SchreibeZahl(farbe, x + 2, guileiste + 8, akt->m_taktnum + (viertel / akt->m_zaehler));
				Linie(farbe, x, guileiste + 2, x, guileiste + 14);
			} else {
				if (gui.tab >= 6) Linie(farbe, x, guileiste + 10, x, guileiste + 14);
			}
		} else {
			if (gui.tab >= 16) Linie(farbe, x, guileiste + 12, x, guileiste + 14);
		}
		
		t += VIERTELWERT; gui.tasicht++;
	}
}

void ZeichneSmpteLeiste(void) {
	int16 lr, rr;
	int16 x;
	char puf[12];
	int32 t;
	int32 ticks;

	aktfenster = hfenster;
	lr = gui.spalte + 2;
	rr = hfenster->Width - randlr - 20;

	Balken(2, lr, guileiste - 16, rr, guileiste - 2);
	
	for (x = lr; x < rr; x += 70) {
		t = (((int32)(x - gui.spalte - 2) * 4) << VIERTEL) / gui.tab + gui.takt; // PunktPosition
		ticks = TaktSmpteTicks(t);
		sprintf(puf, "%ld:%ld:%ld:%ld", Ticks2hh(ticks), Ticks2mm(ticks), Ticks2ss(ticks), (int32)Ticks2ff(ticks));
		Linie(1, x, guileiste - 14, x, guileiste - 2);
		Schreibe(1, x + 2, guileiste - 8, puf, rr);
	}
}

void ZeichneMarkerleiste(int8 typ) {
	int16 lr, rr;
	int16 x, nx;
	struct MARKER *akt;
	struct MARKER *next;
	char puf[128];
	int8 f;
	int16 y;
	
	aktfenster = hfenster;
	switch (typ) {
		case M_TEXT: y = 2; f = 9; break;
		case M_TEMPO: y = 15; f = 15; break;
		case M_TAKT: y = 28; f = 20; break;
	}

	lr = gui.spalte + 2;
	rr = hfenster->Width - randlr - 20;

	Balken(f, lr, y, rr, y + 11);
	
	akt = rootmark;
	next = NULL;
	while (akt) {
		if (akt->typ == typ) {
			x = gui.spalte + 2 + ((((akt->takt - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
			if (x > rr) break;
			if ((x >= lr) && (x < rr)) {
				if (akt == wahlmark[typ]) f = 2; else f = 1;
				Linie(f, x, y, x, y + 11);
				next = NextMarker(akt);
				if (next) {
					nx = gui.spalte + 2 + ((((next->takt - gui.takt) >> (VIERTEL - 4)) * gui.tab) >> 6);
					if (nx > rr) nx = rr;
				} else {
					nx = rr;
				}
				
				if (typ == M_TEXT) strncpy(puf, &akt->text, 128);
				if (typ == M_TAKT) sprintf(puf, "%d/4", akt->m_zaehler);
				if (typ == M_TEMPO) sprintf(puf, "%d bpm", akt->m_bpm);
				
				Schreibe(f, x + 2, y + 8, puf, nx);
			}
		}
		akt = akt->next;
	}

}

void ZeichneMarkerleisten(void) {
	ZeichneMarkerleiste(M_TEXT);
	ZeichneMarkerleiste(M_TEMPO);
	ZeichneMarkerleiste(M_TAKT);
}

void ZeichneInfobox(uint8 sparten) {
	int16 y;
	int16 x, x2, xr;
	char sign[6];

	aktfenster = hfenster;
	y = hfenster->Height - randou - 36 - guibox;
	if (sparten) {

		if (sparten & 1) Gradient(4, STIL_DN, 3, y + 1, 144, y + guibox - 6);
		if (sparten & 2) Gradient(4, STIL_DN, 147, y + 1, 294, y + guibox - 6);
		if (sparten & 4) Gradient(4, STIL_DN, 297, y + 1, 444, y + guibox - 6);
		if (sparten & 8) Gradient(4, STIL_DN, 447, y + 1, 594, y + guibox - 6);
	} else {
		Gradient(4, STIL_DN, 0, y - 1, hfenster->Width - randlr - 1, y + guibox - 3);
		Linie(1, 0, y, hfenster->Width - randlr - 1, y);
		Linie(2, 0, y + guibox - 2, hfenster->Width - randlr - 1, y + guibox - 2);
		Linie(8, 0, y + guibox - 1, hfenster->Width - randlr - 1, y + guibox - 1);
		RahmenEin(-1, 0, 145, y + 4, 146, y + guibox - 5);
		RahmenEin(-1, 0, 295, y + 4, 296, y + guibox - 5);
		RahmenEin(-1, 0, 445, y + 4, 446, y + guibox - 5);
		RahmenEin(-1, 0, 595, y + 4, 596, y + guibox - 5);
		sparten = 0xFF;
	}

	if (sparten & 1) {
		x = 6; x2 = 50; xr = 144;
		Schreibe(2, x, y + 12, CAT(MSG_0224, "Title:"), xr); Schreibe(2, x2, y + 12, lied.name, xr);
		Schreibe(1, x, y + 24, CAT(MSG_0225, "Length:"), xr); SchreibeZahl(1, x2, y + 24, lied.taktanz);
	}

	if (sparten & 2) {
		x = 150; x2 = 200; xr = 294;
		Schreibe(2, x, y + 12, CAT(MSG_0226, "Edit Position"), xr);
		Schreibe(1, x, y + 24, CAT(MSG_0227, "Tempo:"), xr); SchreibeZahl(1, x2, y + 24, wahlmark[M_TEMPO]->m_bpm);
		Schreibe(1, x, y + 36, CAT(MSG_0228, "Signature:"), xr);
		sprintf(sign, "%d/4", wahlmark[M_TAKT]->m_zaehler);
		Schreibe(1, x2, y + 36, sign, xr);
		if (wahlmark[M_TEXT]) {
			Schreibe(1, x, y + 48, CAT(MSG_0229, "Text:"), xr); Schreibe(1, x2, y + 48, &wahlmark[M_TEXT]->text, xr);
		}
	}

	if (sparten & 4) {
		x = 300; x2 = 350; xr = 445;
		Schreibe(2, x, y + 12, CAT(MSG_0230, "Track:"), xr); Schreibe(2, x2, y + 12, spur[snum].name, xr);
		Schreibe(1, x, y + 24, CAT(MSG_0231, "Channel:"), xr);
		if (spur[snum].channel < 16) {
			SchreibeZahl(1, x2, y + 24, spur[snum].channel + 1);
		} else {
			Schreibe(1, x2, y + 24, CAT(MSG_0232, "Thru"), xr);
		}
		Schreibe(1, x2 + 22, y + 24, outport[spur[snum].port].name, xr);
		Schreibe(1, x, y + 36, CAT(MSG_0233, "Program:"), xr);
		if (spur[snum].prog >= 0) {
			SchreibeZahl(1, x2, y + 36, spur[snum].prog);
			if (spur[snum].bank0 >= 0) SchreibeZahl(1, x2 + 22, y + 36, spur[snum].bank0);
			else Schreibe(1, x2 + 22, y + 36, (STRPTR)"**", xr);
			if (spur[snum].bank32 >= 0) SchreibeZahl(1, x2 + 44, y + 36, spur[snum].bank32);
			else Schreibe(1, x2 + 44, y + 36, (STRPTR)"**", xr);
		} else {
			Schreibe(1, x2, y + 36, CAT(MSG_0236, "not chosen"), xr);
		}
		Schreibe(1, x, y + 48, CAT(MSG_0237, "Shift:"), xr); SchreibeZahl(1, x2, y + 48, spur[snum].shift);
	}

	if (seqinfo.benutzt && (sparten & 8)) {
		x = 450; x2 = 500; xr = 595;
		Schreibe(2, x, y + 12, CAT(MSG_0238, "Sequence:"), xr);
			if (seqinfo.namemulti) Schreibe(2, x2, y + 12, (STRPTR)"***", xr);
			else Schreibe(2, x2, y + 12, seqinfo.name, xr);
		Schreibe(1, x, y + 24, CAT(MSG_0240, "Transpose:"), xr);
			if (seqinfo.transmulti) Schreibe(1, x2, y + 24, (STRPTR)"***", xr);
			else SchreibeZahl(1, x2, y + 24, seqinfo.trans);
		Schreibe(1, x, y + 36, CAT(MSG_0241, "Mute:"), xr);
			if (seqinfo.mutemulti) Schreibe(1, x2, y + 36, (STRPTR)"***", xr);
			else if (seqinfo.mute) Schreibe(1, x2, y + 36, CAT(MSG_0241A, "ON"), xr);
			else Schreibe(1, x2, y + 36, CAT(MSG_0241B, "OFF"), xr);
		Schreibe(1, x, y + 48, CAT(MSG_0242, "Aliases:"), xr); SchreibeZahl(1, x2, y + 48, seqinfo.aliasanz);
	}
}

int8 TestePunktInfo(int16 x, int16 y) {
	int8 b;

	x = x - hfenster->BorderLeft - 2;
	y = y - hfenster->BorderTop -(hfenster->Height - hfenster->BorderTop - hfenster->BorderBottom - 32 - guibox);
	b = (x / 150) * 10 + (y / 12);
	return(b);
}

void ZeichneAnzeigen(BOOL tt) {
	char puf[20];
	uint16 atakt;
	uint16 viertel;
	
	aktfenster = hfenster;
	Balken(2, 2, guileiste - 16, gui.spalte - 2, guileiste - 2); // SMPTE
	Balken(2, 2, guileiste, gui.spalte - 2, guileiste + 14);    // Takt
	if (tt) {
		Balken(9, 23, 2, gui.spalte - 2, 13);
		Balken(15, 2, 15, gui.spalte - 2, 26);
		Balken(20, 2, 28, gui.spalte - 2, 39);
	}

	if (lkmark && ltmark) {
		sprintf(puf, "%ld:%ld:%ld:%ld", Ticks2hh(tick), Ticks2mm(tick), Ticks2ss(tick), (int32)Ticks2ff(tick));
		Schreibe(1, 6, guileiste - 6, puf, gui.spalte - 2);
	
		viertel = ((uint32)takt - lkmark->takt) >> VIERTEL;
		atakt = lkmark->m_taktnum + (viertel / lkmark->m_zaehler);
		sprintf(puf, "%ld | %ld | %ld", (int32)atakt, (int32)(viertel % lkmark->m_zaehler) + 1, ((takt & ~VIERTELMASKE) >> (VIERTEL - 2)) + 1);
		Schreibe(1, 6, guileiste + 10, puf, gui.spalte - 2);

		if (tt && lxmark) {
			Schreibe(2, 25, 10, &lxmark->text, gui.spalte - 2);
			sprintf(puf, "%d bpm", ltmark->m_bpm);
			Schreibe(2, 4, 23, puf, gui.spalte - 2);
			sprintf(puf, "%d/4", lkmark->m_zaehler);
			Schreibe(2, 4, 36, puf, gui.spalte - 2);
		}
	}
}

int8 TestePunktBereich(int16 x, int16 y) {
	uint8 b = 0;
	int16 uy, rx;

	x = x - hfenster->BorderLeft;
	y = y - hfenster->BorderTop;
	uy = hfenster->Height - randou;
	rx = hfenster->Width - randlr;

	if ((x > gui.spalte + 1) && (x < rx - 20)) {
		if ((y > 1) && (y < 14)) {b = AREA_MARKER; mtyp = M_TEXT;}
		if ((y > 14) && (y < 27)) {b = AREA_MARKER; mtyp = M_TEMPO;}
		if ((y > 27) && (y < 40)) {b = AREA_MARKER; mtyp = M_TAKT;}
		if ((y > guileiste) && (y < guileiste + 14)) b = AREA_ZEIT;
	}
	if ((y > guileiste + 14) && (y < uy - 54 - guibox)) {
		if (x < gui.spalte) b = AREA_SPUREN;
		if ((x > gui.spalte - 3) && (x < gui.spalte + 2)) b = AREA_GUISPALTE;
		if ((x > gui.spalte + 2) && (x < rx - 20)) b = AREA_SEQUENZEN;
	}
	if ((y > uy - 36 - guibox) && (y < uy - 36)) b = AREA_INFOBOX;
	if ((y > uy - 33) && (x > 418)) b = AREA_UEBERSICHT;
	return(b);
}
