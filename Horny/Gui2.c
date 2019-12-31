#include <stdio.h>
#include <string.h>

#include <intuition/intuition.h>
#include <midi/mididefs.h>

#include "Strukturen.h"
#include "Gui.h"

extern struct Window *hfenster=NULL;
extern struct Window *aktfenster=NULL;

struct GUI gui={0, 10, 32, 0, 20, 33, 160};
WORD guibox=60;
extern WORD randlr;
extern WORD randou;
extern WORD bpm;

extern struct SPUR spur[12];
extern struct LIED lied;
extern STRPTR midiname[16];
extern struct LOOP loop;

void InitGuiWerte(void) {
	gui.spur=0;
	gui.sph=32;
	gui.takt=0;
	gui.tab=33;
	gui.spalte=160;
}

void ZeichneSequenzen(WORD s) {
	struct SEQUENZ *seq;
	WORD y;
	WORD xs, xe, xn, altx;
	WORD rr;
	struct EVENTBLOCK *evbl;
	WORD evnum;

	if ((s>=gui.spur) && (s<(gui.spur+gui.spsicht))) {
		rr=hfenster->Width - randlr - 21;
		y=20+((s-gui.spur)*gui.sph);
		altx=gui.spalte+2;
		seq=spur[s].seq;
		while (seq) {
			xs=gui.spalte+2+((((seq->start-gui.takt)>>6)*gui.tab)>>6);
			xe=gui.spalte+2+((((seq->ende-gui.takt)>>4)*gui.tab)>>8);
			if (xe-xs<1) xe=xs+1;
			if (!((xs<gui.spalte+2) && (xe<gui.spalte+2)) && !((xs>rr) && (xe>rr))) {
				if (xs<gui.spalte+2) xs=gui.spalte+2;
				if (xe>rr) xe=rr;
				
				if (xs>altx) Balken(0, altx, y+1, xs-1, y+gui.sph-2);
				if (xe>altx) altx=xe+1;
				
				if (seq->markiert) {
					RahmenEin(9+spur[s].channel, xs, y+1, xe, y+gui.sph-2);
				} else {
					RahmenAus(9+spur[s].channel, xs, y+1, xe, y+gui.sph-2);
				};

				//Noten zeichnen...
				if ((xe-xs>10) && (gui.sph>30)) {
					evbl=seq->eventblock; evnum=0; Farbe(2);
					do {
						if ((evbl->event[evnum].status & MS_StatBits)==MS_NoteOn) {
							xn=gui.spalte+2+((((seq->start - gui.takt + evbl->event[evnum].zeit)>>4)*gui.tab)>>8);
							if ((xn>xs+1) && (xn<xe-1)) Punkt(xn, y+gui.sph-(((WORD)evbl->event[evnum].data1*gui.sph)>>7));
						};
						evnum++;
						if (evnum==EVENTS) {evnum=0; evbl=evbl->next};
					} while (evbl && evbl->event[evnum].status);
				};
				//...Ende


				if (seq->aliasorig) {
					Fett(FALSE);
					Schreibe(1, xs+2, y+12, seq->aliasorig->name, xe-2);
				} else {
					Fett(TRUE);
					Schreibe(1, xs+2, y+12, seq->name, xe-2);
				};
				if ((gui.sph>26) && seq->trans) SchreibeZahl(1, xs+2, y+24, seq->trans);
				
			};
			seq=seq->next;
		};
		if (altx<rr) Balken(0, altx, y+1, rr, y+gui.sph-2);
	};
	Fett(FALSE);
}

void ZeichneSpuren(WORD snum, BOOL spa, BOOL seq) {
	WORD s;
	UBYTE f;
	WORD n;
	WORD y;

	gui.spsicht=(hfenster->Height - randou - 75 - guibox)/gui.sph;
	if (seq) Balken(0, 2, 20+(gui.spsicht*gui.sph), hfenster->Width - randlr - 20, hfenster->Height - randou - 56 - guibox);
	for(n=0; n<gui.spsicht; n++) {
		s=n+gui.spur;
		y=20+(n*gui.sph);
		if (s<lied.spuranz) {
			if (spa) {
				RahmenAus(9+spur[s].channel, 2, y, 25, y+gui.sph-1);
				SchreibeZahl(1, 4, y+10, s+1);
				if (s!=snum) f=5 else {f=6; Fett(TRUE)};
				RahmenAus(f, 26, y, gui.spalte, y+gui.sph-1);
				Schreibe(1, 50, y+10, spur[s].name, gui.spalte);
				if (spur[s].mute) RahmenEin(8, 30, y+2, 46, y+11) else RahmenAus(7, 30, y+2, 46, y+11);
			};
			if (seq) {
				RahmenEin(0, gui.spalte+1, y, hfenster->Width - randlr - 20, y+gui.sph-1);
				ZeichneSequenzen(s);
			};
		} else {
			if (spa) RahmenAus(0, 2, y, gui.spalte, y+gui.sph-1);
			if (seq) RahmenEin(0, gui.spalte+1, y, hfenster->Width - randlr - 20, y+gui.sph-1);
		};
		Fett(FALSE);
	};
}

WORD PunktSpur(WORD y) {
	WORD s;

	s=((y-hfenster->BorderTop-20)/gui.sph)+gui.spur;
	if (s<0) s=0;
	return(s);
}

void ZeichneZeitleiste(void) {
	WORD xs, xe;
	WORD lr, rr;
	FLOAT x;
	LONG t;
	BYTE a;
	BYTE f;

	lr=gui.spalte+2;
	rr=hfenster->Width - randlr - 20;

	Balken(2, lr, 2, rr, 16);

	xs=gui.spalte+2+((((loop.start-gui.takt)>>10)*gui.tab)>>2);
	xe=gui.spalte+2+((((loop.ende-gui.takt)>>10)*gui.tab)>>2);
	if (xs<xe) {
		if (!((xs<lr) && (xe<lr)) && !((xs>rr) && (xe>rr))) {
			if (xs<lr) xs=lr;
			if (xe>rr) xe=rr;
			Balken(4, xs, 2, xe, 16);
		};
	};


	gui.tasicht=0;
	x=lr; t=gui.takt;
	a=128;
	if (gui.tab>=2) a=32;
	if (gui.tab>=4) a=16;
	if (gui.tab>=8) a=8;
	if (gui.tab>=14) a=4;
	if (gui.tab>=22) a=2;
	if (gui.tab>=30) a=1;
	while ((WORD)x < rr-10) {
		if ((t>>10)%a==0) {
			if ((t>>10)%4==0) {
				if ((t>=loop.start) && (t<loop.ende)) f=2 else f=3;
				SchreibeZahl(f, (WORD)x+1, 11, (t>>12)+1);
				Linie(1, (WORD)x, 4, (WORD)x, 16);
			} else {
				Linie(1, (WORD)x, 12, (WORD)x, 16);
			};
		};
		x=x+((FLOAT)gui.tab/4); t=t+(1<<10); gui.tasicht++;
	};
}

void ZeichneInfobox(WORD snum, struct SEQUENZ *seq) {
	WORD y;

	y=hfenster->Height - hfenster->BorderBottom - 52 - guibox;
	RahmenEin(4, 2, y, hfenster->Width - randlr - 2, y+guibox-1);

	Schreibe(2, 6, y+12, "Titel:", 200); Schreibe(2, 50, y+12, lied.name, 200);
	Schreibe(1, 6, y+24, "Tempo:", 200); SchreibeZahl(1, 50, y+24, bpm);
	Schreibe(1, 6, y+36, "Länge:", 200); SchreibeZahl(1, 50, y+36, lied.taktanz);

	Schreibe(2, 200, y+12, "Spur:", 400); Schreibe(2, 250, y+12, spur[snum].name, 400);
	Schreibe(1, 200, y+24, "Kanal:", 400); SchreibeZahl(1, 250, y+24, spur[snum].channel+1);
	Schreibe(1, 272, y+24, midiname[spur[snum].port], 400);
	Schreibe(1, 200, y+36, "Instr.:", 400);
	if (spur[snum].bank>=0) {
		SchreibeZahl(1, 250, y+36, spur[snum].prog+1);
		SchreibeZahl(1, 272, y+36, spur[snum].bank+1);
	} else {
		Schreibe(1, 250, y+36, "nicht gewählt", 400);
	};
	Schreibe(1, 200, y+48, "Versch.:", 400); SchreibeZahl(1, 250, y+48, spur[snum].shift);

	if (seq) {
		if (!seq->aliasorig) {
			Schreibe(2, 400, y+12, "Sequenz:", 600); Schreibe(2, 450, y+12, seq->name, 600);
			Schreibe(1, 400, y+36, "Aliase:", 600); SchreibeZahl(1, 450, y+36, seq->aliasanz);
		} else {
			Schreibe(2, 400, y+12, "Alias:", 600); Schreibe(2, 450, y+12, seq->aliasorig->name, 600);
		};
		Schreibe(1, 400, y+24, "Trans.:", 600); SchreibeZahl(1, 450, y+24, seq->trans);
	};
}

BYTE TestePunktInfo(WORD x, WORD y) {
	BYTE b;

	x=x - hfenster->BorderLeft - 2;
	y=y - hfenster->BorderTop -(hfenster->Height - hfenster->BorderBottom - 48 - guibox);
	b=(x/200)*10+(y/12);
	return(b);
}

BYTE TestePunktBereich(WORD x, WORD y) {
	UBYTE b=0;
	WORD uy, rx;

	x=x-hfenster->BorderLeft;
	y=y-hfenster->BorderTop;
	uy=hfenster->Height - randou;
	rx=hfenster->Width - randlr;

	if ((y<20) && (x>gui.spalte+1) && (x<rx-20)) b=1;
	if ((y>20) && (y<uy-54-guibox)) {
		if (x<gui.spalte+2) b=2;
		if ((x>gui.spalte+2) && (x<rx-20)) b=3;
	};
	if ((y>uy-67-guibox) && (y<uy-36)) b=4;
	return(b);
}
