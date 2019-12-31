#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Sequenzen.h"
#include "Requester.h"
#include "Undo.h"

extern struct SPUR spur[];
extern struct SPURTEMP sp[];
extern struct LIED lied;

struct EVENT *stev[128];
extern char oktnote[12][3];

void AliaseAnpassen(struct SEQUENZ *original, WORD d) {
	struct SEQUENZ *seq;
	WORD s;

	if (original->aliasanz) {
		for (s = 0; s < lied.spuranz; s++) {
			seq = spur[s].seq;
			while (seq) {
				if (seq->aliasorig == original) {
					seq->start = seq->start + d;
					seq->ende = seq->start + (original->ende - original->start);
					seq->eventblock = original->eventblock;
					sp[s].anders = 2;
				}
				seq = seq->next;
			}
		}
	}
}

void EventsVerschieben(struct SEQUENZ *seq, WORD d) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	
	if (d) {
		evbl = seq->eventblock; evnum = 0;
		while (evbl) {
			if (!evbl->event[evnum].status) break;

			evbl->event[evnum].zeit += d;
			
			evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
		}
	}
}

void OrdneEvents(struct SEQUENZ *seq) {
	struct EVENT *ev1;
	struct EVENT *ev2;
	struct EVENT evk;
	struct EVENTBLOCK *evbl;
	WORD evnum;
	
	evbl = seq->eventblock; evnum = 0;
	if (evbl) {
		if (evbl->event[0].status) {
			do {
				ev1 = &evbl->event[evnum];
				evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
				if (!evbl) break;
				ev2 = &evbl->event[evnum];
				if (!ev2->status) break;
				
				if (ev1->zeit > ev2->zeit) {
					memcpy(&evk, ev1, sizeof(struct EVENT));
					memcpy(ev1, ev2, sizeof(struct EVENT));
					memcpy(ev2, &evk, sizeof(struct EVENT));
					evnum = evnum-2;
					if (evnum < 0) {evbl = evbl->prev; evnum = evnum + EVENTS;}
					if (!evbl) {evbl = seq->eventblock; evnum = 0;}
				}
		
			} while (TRUE);
		}
	}
}

void NotenEndenMarkieren(struct SEQUENZ *seq) {
	UBYTE n;
	struct EVENTBLOCK *evbl;
	WORD evnum;
	WORD p;
	
	for (n = 0; n < 128; n++) stev[n] = NULL;

	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
	
		p = evbl->event[evnum].data1;
		if ((evbl->event[evnum].status & MS_StatBits) == MS_NoteOn) {
			stev[p] = &evbl->event[evnum];
		}
		if ((evbl->event[evnum].status & MS_StatBits) == MS_NoteOff) {
			if (stev[p]) {
				if (stev[p]->markiert) {
					evbl->event[evnum].markiert = TRUE;
				} else {
					if (evbl->event[evnum].markiert) stev[p]->markiert = TRUE;
				}
				stev[p] = NULL;
			}
		}

		evnum++; if (evnum == EVENTS) {evnum = 0; evbl = evbl->next;}
	}
}

void QuantisiereSequenz(struct SEQUENZ *seq, WORD quant) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	LONG z;
	LONG nstart;
	LONG d;
	
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
		
		if ((evbl->event[evnum].status & MS_StatBits) == MS_NoteOn) {
			z = evbl->event[evnum].zeit;

			z = z >> (quant - 1);
			if ((z & 0x00000001) == 1) z++;
			z = z >> 1;
			z = z << quant;

			evbl->event[evnum].zeit = z;
		}
			
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}

	nstart = (seq->start + seq->eventblock->event[0].zeit) & VIERTELMASKE;
	d = seq->start - nstart;
	EventsVerschieben(seq, d);
	seq->start = nstart;
	
	OrdneEvents(seq);
	AliaseAnpassen(seq, -d);
}

void MarkSequenzenQuantisieren(WORD quant) {
	struct SEQUENZ *seq;
	WORD s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (!seq->aliasorig) {
					QuantisiereSequenz(seq, quant);
				} else {
					QuantisiereSequenz(seq->aliasorig, quant);
				}
				sp[s].anders = 2;
			}
			seq = seq->next;
		}
	}
}

void MarkEventsEntfernen(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl1;
	struct EVENTBLOCK *evbl2;
	WORD evnum1, evnum2;
	
	evbl1 = seq->eventblock; evnum1 = 0;
	evbl2 = seq->eventblock; evnum2 = 0;
	while (evbl2) {
		if (!evbl2->event[evnum2].markiert) {
			memcpy(&evbl1->event[evnum1], &evbl2->event[evnum2], sizeof(struct EVENT));
			evnum1++; if (evnum1 == EVENTS) {evbl1 = evbl1->next; evnum1 = 0;}
		}

		if (!evbl2->event[evnum2].status) break;
		evnum2++; if (evnum2 == EVENTS) {evbl2 = evbl2->next; evnum2 = 0;}
	}

	EvblsAbschneiden(evbl1);
}

struct EVENT *EventEinfuegen(struct SEQUENZ *seq, LONG t, BYTE status, BYTE data1, BYTE data2, BOOL markieren) {
	struct EVENTBLOCK *evbl1;
	struct EVENTBLOCK *evbl2;
	WORD evnum1;
	WORD evnum2;
	struct EVENT *ev = NULL;
		
	evbl1 = seq->eventblock;
	if (evbl1) {
		while (evbl1->next) evbl1 = evbl1->next;
		evnum1 = 0;
		while (evbl1->event[evnum1].status) evnum1++;
		
		evbl2 = evbl1; evnum2 = evnum1;
		evnum1++;
		if (evnum1 == EVENTS) {
			if (AddEvbl(evbl1)) {evnum1 = 0; evbl1 = evbl1->next;} else evbl1 = NULL;
		}
		if (evbl1) {
			while ((t < evbl2->event[evnum2].zeit) || !evbl2->event[evnum2].status) {
				memcpy(&evbl1->event[evnum1], &evbl2->event[evnum2], sizeof(struct EVENT));
				evnum1--; if (evnum1 < 0) {evbl1 = evbl1->prev; evnum1 = EVENTS - 1;}
				evnum2--; if (evnum2 < 0) {evbl2 = evbl2->prev; evnum2 = EVENTS - 1;}
				if (!evbl2) break;
			}
			ev = &evbl1->event[evnum1];
			ev->zeit = t;
			if (spur[seq->spur].channel < 16) status = status | spur[seq->spur].channel;
			ev->status = status;
			ev->data1 = data1;
			ev->data2 = data2;
			ev->markiert = markieren;
		}
	}
	return(ev);
}

void MarkEventsKopieren(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	
	evbl = seq->eventblock;
	if (evbl) {
		while (evbl->next) evbl = evbl->next;
		evnum = 0;
		while (evbl->event[evnum].status) evnum++;
		
		while (TRUE) {
			evnum--;
			if (evnum < 0) {evbl = evbl->prev; evnum = EVENTS - 1;}
			if (!evbl) break;
			ev = &evbl->event[evnum];
			if (ev->markiert) {
				ev->markiert = FALSE;
				EventEinfuegen(seq, ev->zeit, ev->status, ev->data1, ev->data2, TRUE);
			}
		}
	}
}

void MarkNotenVerschieben(struct SEQUENZ *seq, LONG d, WORD h) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
		
		ev = &evbl->event[evnum];
		if (((ev->status & MS_StatBits) <= MS_NoteOn) && ev->markiert) {
			ev->zeit += d;
			if (ev->data1 + h < 0) {
				ev->data1 = 0;
			} else {
				if (ev->data1 + h > 127) ev->data1 = 127; else ev->data1 += h;
			}
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
}

void MarkNotenEndenVerschieben(struct SEQUENZ *seq, LONG d) {
	UBYTE n;
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	
	for (n = 0; n < 128; n++) stev[n] = NULL;

	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
		
		ev = &evbl->event[evnum];
		if ((ev->status & MS_StatBits) == MS_NoteOn) stev[ev->data1] = ev;
		if ((ev->status & MS_StatBits) == MS_NoteOff) {
			if (stev[ev->data1]) {
				if (stev[ev->data1]->markiert) {
					if (ev->zeit + d >= stev[ev->data1]->zeit + (VIERTELWERT >> 5)) {
						ev->zeit = ev->zeit + d;
					}
				}
				stev[ev->data1] = NULL;
			}
		}

		evnum++; if (evnum == EVENTS) {evnum = 0; evbl = evbl->next;}
	}
}

void MarkEventsDynamik(struct SEQUENZ *seq, BYTE thresh, BYTE ratio, BYTE gain) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	WORD u, w;
	
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if (((ev->status & MS_StatBits) != MS_NoteOff) && ev->markiert) {
			u = (WORD)ev->data2;
			if ((ratio > 0) && (u > thresh)) u = ((u - thresh) / (1 + ratio)) + thresh;
			if ((ratio < 0) && (u < thresh)) u = ((u - thresh) * (1 - ratio)) + thresh;
			w = u + gain;
			if (w < 0) w = 0;
			if (w > 127) w = 127;
			ev->data2 = w;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
}

void MarkNotenQuantisieren(struct SEQUENZ *seq, WORD quant, BYTE modus, BOOL tripled) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	UBYTE status;
	LONG z;
	LONG faktor;
	LONG ttakt;
	LONG rest;
	
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if (ev->markiert) {
			status = (ev->status & MS_StatBits);
			// modus 0: Notenanfänge quantisieren
			// modus 1: Notenenden quantisieren
			// modus 2: Notenänfänge nähern
			if (((modus != 1) && (status == MS_NoteOn)) || ((modus == 1) && (status == MS_NoteOff))) {
				z = ev->zeit;

				if (quant > 0) {
					if (tripled) { //Triolen
						faktor = 1 << (quant + 1);
						ttakt = (z / faktor * faktor);

						rest = z - ttakt;
						faktor = (1 << quant) * 2 / 3;
						rest = rest / (faktor >> 1);
						if ((rest & 0x00000001) == 1) rest++;
						rest = rest >> 1;
						rest = rest * faktor;
						z = ttakt + rest;
					} else {
						faktor = 1 << quant;
						z = z / (faktor >> 1);
						if ((z & 0x00000001) == 1) z++;
						z = z >> 1;
						z = z * faktor;
					}
				}

				if (modus == 2) ev->zeit = (z + ev->zeit) / 2;
				else ev->zeit = z;
			}
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	OrdneEvents(seq);
}

struct EVENT *TaktContr(LONG t, BYTE contr, struct SEQUENZ *seq, struct EVENT **fcev) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	struct EVENT *lcev;
	UBYTE status;
	
	lcev = NULL;
	evbl = seq->eventblock; evnum = 0;
	// Ziel-Event suchen...
	while (evbl) {
		ev = &evbl->event[evnum];
		status = (ev->status & MS_StatBits);
		if (!status || (seq->start + ev->zeit > t)) break;
		
		if (contr >= 0) {
			if ((status == MS_Ctrl) && (ev->data1 == contr)) lcev = ev;
		} else {
			if ((status - MS_PolyPress) >> 4 == contr + 5) lcev = ev;
		}

		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	if (fcev) {
		*fcev = NULL;
		// Folgendes Event suchen...
		while (evbl) {
			ev = &evbl->event[evnum];
			status = (ev->status & MS_StatBits);
			if (!status) break;
			
			if (contr >= 0) {
				if ((status == MS_Ctrl) && (ev->data1 == contr)) {
					*fcev = ev; break;
				}
			} else {
				if ((status - MS_PolyPress) >> 4 == contr + 5) {
					*fcev = ev; break;
				}
			}
	
			evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
		}
	}
	return(lcev);
}


struct EVENT *TaktNote(LONG t, UBYTE note, struct SEQUENZ *seq, BYTE *p) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	struct EVENT *tn = NULL;
	struct EVENT *tnu = NULL;
	BOOL schwarz;
	
	//Notenanfang finden
	schwarz = (oktnote[note%12][1] == '#');
	t = t - seq->start;
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if (ev->zeit > t) break;
		if ((ev->status & MS_StatBits) == MS_NoteOn) {
			if (ev->data1 == note) tn = ev;
			if (ev->data1 == note - 1) tnu = ev;
		}
		if ((ev->status & MS_StatBits) == MS_NoteOff) {
			if (ev->data1 == note) tn = NULL;
			if (ev->data1 == note - 1) tnu = NULL;
		}

		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	if (!tn && tnu && schwarz) tn = tnu;
	
	//Notenende finden
	if (tn) {
		while (evbl) {
			if (!evbl->event[evnum].status) break;
			
			ev = &evbl->event[evnum];
			if ((ev->status & MS_StatBits) == MS_NoteOff) {
				if (ev->data1 == tn->data1) break;
			}
			
			evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
		}
		if (t >= ev->zeit - ((ev->zeit - tn->zeit) / 3)) *p = 1; else *p = 0;
	}
	return(tn);		
}

void KeineEventsMarkieren(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
		evbl->event[evnum].markiert = FALSE;
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
}

void NotenMarkieren(struct SEQUENZ *seq, BYTE modus, UBYTE referenz) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	
	if (modus == 4) KeineEventsMarkieren(seq);
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if ((ev->status & MS_StatBits) == MS_NoteOn) {
			switch (modus) {
				case 0: ev->markiert = TRUE; break;
				case 1: if (ev->data1 == referenz) ev->markiert = TRUE; break;
				case 2: if (ev->data1 >= referenz) ev->markiert = TRUE; break;
				case 3: if (ev->data1 <= referenz) ev->markiert = TRUE; break;
				case 4: if (ev->data2 < referenz) ev->markiert = TRUE; break;
				case 5: if ((ev->status & MS_ChanBits) == referenz) ev->markiert = TRUE; break;
			}
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	NotenEndenMarkieren(seq);
}

void NotenBereichMarkieren(struct SEQUENZ *seq, WORD vtast, WORD btast, LONG vt, LONG bt) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	LONG z;
	
	if (vtast > btast) {z = vtast; vtast = btast; btast = z;}
	if (vt > bt) {z = vt; vt = bt; bt = z;}

	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;
		
		ev = &evbl->event[evnum];
		z= ev->zeit + seq->start;
		if (z > bt) break;
		if ((ev->status & MS_StatBits) <= MS_NoteOn) {
			if ((z >= vt) && (z <= bt) && (ev->data1 >= vtast) && (ev->data1 <= btast)) ev->markiert = !ev->markiert;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	NotenEndenMarkieren(seq);
}

void ControllerMarkieren(struct SEQUENZ *seq, BYTE contr) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	UBYTE zstatus;
	
	if (contr >= 0) zstatus = MS_Ctrl; else zstatus = ((contr + 5) << 4) + MS_PolyPress;
		
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		ev->markiert = FALSE;
		if ((ev->status & MS_StatBits) == zstatus) {
			if ((contr >= 0) && (ev->data1 == contr)) ev->markiert = TRUE;
			if (contr < 0) ev->markiert = TRUE;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
}

void MarkContrReduzieren(struct SEQUENZ *seq, WORD quant) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	struct EVENT *altev;
	LONG z;
	
	evbl = seq->eventblock;
	evnum = 0;
	altev = NULL;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if (ev->markiert && ((ev->status & MS_StatBits) >= MS_PolyPress)) {
			z = ev->zeit;
			z = z >> (quant - 1);
			if ((z & 0x00000001) == 1) z++;
			z = z >> 1;
			z = z << quant;
			ev->zeit = z;
			ev->markiert = FALSE;
			if (altev) {
				if (altev->zeit == ev->zeit) altev->markiert = TRUE;
				if (altev->data2 == ev->data2) ev->markiert = TRUE;
			}
			altev = ev;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	MarkEventsEntfernen(seq);
	OrdneEvents(seq);
}

void MarkContrGlaetten(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	struct EVENT *altev = NULL;
	BYTE altdata2 = -1;
	BYTE altaltdata2 = -1;
	
	evbl = seq->eventblock;
	evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if (ev->markiert && ((ev->status & MS_StatBits) >= MS_PolyPress)) {
			if (altev) {
				altdata2 = altev->data2;
				if (altaltdata2 > -1) {
					altev->data2 = (BYTE)(((WORD)altaltdata2 + (WORD)altev->data2 + (WORD)ev->data2) / 3);
				}
				altaltdata2 = altdata2;
			}
			altev = ev;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
}

void RepariereNoten(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *ev;
	BOOL noteon[128];
	WORD n;
	struct NOTEOFF {
		LONG takt;
		BYTE note;
		struct NOTEOFF *next;
	};
	struct NOTEOFF *rootnoteoff = NULL;
	struct NOTEOFF *akt;
	struct NOTEOFF *next;
	struct EVENT *noteonev[128];
	BOOL anders = FALSE;
	
	KeineEventsMarkieren(seq);
	
	// Doppelanfänge sammeln... Doppelenden markieren...
	for (n = 0; n < 128; n++) noteon[n] = FALSE;
	evbl = seq->eventblock;
	evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if ((ev->status & MS_StatBits) == MS_NoteOn) {
			if (noteon[ev->data1]) {
				akt = AllocVec(sizeof(struct NOTEOFF), 0);
				if (akt) {
					akt->takt = ev->zeit - 1;
					akt->note = ev->data1;
					akt->next = rootnoteoff;
					rootnoteoff = akt;
				}
			} else noteon[ev->data1] = TRUE;
		} else
		if ((ev->status & MS_StatBits) == MS_NoteOff) {
			if (noteon[ev->data1]) noteon[ev->data1] = FALSE;
			else {
				ev->markiert = TRUE;
				anders = TRUE;
			}
		}

		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	
	// NoteOffs einfügen...
	if (rootnoteoff) anders = TRUE;
	akt = rootnoteoff;
	while (akt) {
		EventEinfuegen(seq, akt->takt, MS_NoteOff, akt->note, 0, FALSE);
		next = akt->next;
		FreeVec(akt);
		akt = next;
	}
	
	// Offene NoteOns markieren...
	for (n = 0; n < 128; n++) noteonev[n] = NULL;

	evbl = seq->eventblock;
	evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		if ((ev->status & MS_StatBits) == MS_NoteOn) noteonev[ev->data1] = ev;
		else if ((ev->status & MS_StatBits) == MS_NoteOff) noteonev[ev->data1] = NULL;

		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	for (n = 0; n < 128; n++) {
		if (noteonev[n]) {
			noteonev[n]->markiert = TRUE;
			anders = TRUE;
		}
	}

	// Doppelenden und offene NoteOns löschen...
	MarkEventsEntfernen(seq);
	
	if (anders) Meldung(CAT(MSG_0206, "Corrections were done"));
	else Meldung(CAT(MSG_0207, "All notes are okay"));
}

BYTE ZerschneideSequenzNoten(struct SEQUENZ *seq, LONG t, BYTE trennart) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	struct EVENT *noteon[128];
	WORD n;
	struct EVENT *ev;
	BOOL bruch;
	
	for (n = 0; n < 128; n++) noteon[n] = NULL;
	
	t -= seq->start;
	
	// Offene Noten an Trennstelle suchen...
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status || (ev->zeit >= t)) break;
		
		if ((ev->status & MS_StatBits) == MS_NoteOn) noteon[ev->data1] = ev;
		else if ((ev->status & MS_StatBits) == MS_NoteOff) noteon[ev->data1] = NULL;
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	
	bruch = FALSE;
	for (n = 0; n < 128; n++) {
		if (noteon[n]) {bruch = TRUE; break;}
	}
	
	if (bruch) {
		if (trennart == 0) {
			if (Frage(CAT(MSG_0208, "Notes are lying on split point"), CAT(MSG_0209, "Split|Shorten"))) trennart = 1;
			else trennart = 2;
		}
		if (trennart == 1) {
			// Noten zerschneiden...
			for (n = 0; n < 128; n++) {
				if (noteon[n]) {
					EventEinfuegen(seq, t - 1, MS_NoteOff, n, 0, FALSE);
					EventEinfuegen(seq, t, MS_NoteOn, n, noteon[n]->data2, FALSE);
				}
			}
			
		} else {
			// Noten kürzen...
			while (evbl) {
				ev = &evbl->event[evnum];
				if (!ev->status) break;
				
				if ((ev->status & MS_StatBits) == MS_NoteOff) {
					if (noteon[ev->data1]) {
						ev->zeit = t - 1;
						noteon[ev->data1] = NULL;
					}
				}
				
				evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
			}
			OrdneEvents(seq);
			sp[seq->spur].anders = 2;
		}
	}
	return(trennart);
}
