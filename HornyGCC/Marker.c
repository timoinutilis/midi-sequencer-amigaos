#include <stdio.h>
#include <string.h>

#include <proto/exec.h>

#include "locale.h"

#include "Strukturen.h"
#include "Requester.h"

struct MARKER *rootmark = NULL;

extern struct MARKER *wahlmark[3];
extern struct MARKER *ltmark;
extern struct MARKER *lkmark;

extern struct SMPTE smpte;

// Allgemeine Marker

struct MARKER *NeuerMarker(BYTE typ, LONG t, WORD d1, WORD d2) {
	struct MARKER *akt;
	struct MARKER *neu;
	WORD gr;
	
	gr = sizeof(struct MARKER);
	if (typ == M_TEXT) gr += 128;
	
	
	neu = AllocVec(gr, MEMF_ANY);
	if (neu) {
		neu->takt = t;
		neu->typ = typ;
		neu->d1 = d1;
		neu->d2 = d2;
		neu->ticks = 0;
		neu->text = 0;
		if (rootmark) {
			akt = rootmark;
			while (akt->next) {
				if (akt->next->takt > t) break;
				akt = akt->next;
			}
			neu->prev = akt;
			neu->next = akt->next;
			akt->next = neu;
			if (neu->next) neu->next->prev = neu;
		} else {
			rootmark = neu;
			neu->prev = NULL;
			neu->next = NULL;
		}
	} else Meldung(CAT(MSG_0048, "Not enough memory for marker\n<Marker.c>"));
	return(neu);
}

void EntferneMarker(struct MARKER *mark) {
	if (mark != rootmark) {
		mark->prev->next = mark->next;
		if (mark->next) mark->next->prev = mark->prev;
	} else {
		if (mark->next) mark->next->prev = NULL;
		rootmark = mark->next;
	}
	FreeVec(mark);
}

void EntferneAlleMarker(void) {
	struct MARKER *akt;
	struct MARKER *next;
	
	akt = rootmark;
	while (akt) {
		next = akt->next;
		FreeVec(akt);
		akt = next;
	}
	rootmark = NULL;
}

struct MARKER *TaktDirektMarker(LONG t) {
	struct MARKER *akt;
	
	akt = rootmark;
	while (akt) {
		if (akt->takt == t) return(akt);
		akt = akt->next;
	}
	return(NULL);
}

struct MARKER *TaktMarker(struct MARKER *start, BYTE typ, LONG t) {
	struct MARKER *akt;
	struct MARKER *mark;
	
	if (start) {
		if (start->typ == typ) mark = start; else mark = NULL;
		akt = start;
	} else {
		mark = NULL;
		akt = rootmark;
	}
	while (akt) {
		if (akt->takt > t) break;
		if (akt->typ == typ) mark = akt;
		akt = akt->next;
	}
	return(mark);
}

struct MARKER *NextMarker(struct MARKER *akt) {
	BYTE typ;
	
	typ = akt->typ;
	akt = akt->next;
	while (akt) {
		if (akt->typ == typ) break;
		akt = akt->next;
	}
	return(akt);
}

struct MARKER *PrevMarker(struct MARKER *akt) {
	BYTE typ;
	
	typ = akt->typ;
	akt = akt->prev;
	while (akt) {
		if (akt->typ == typ) break;
		akt = akt->prev;
	}
	return(akt);
}

void MarkerTauschen(struct MARKER *mark1, struct MARKER *mark2) {
	struct MARKER *vmark;
	struct MARKER *nmark;

	vmark = mark1->prev;
	nmark = mark2->next;
	
	vmark->next = mark2;
	mark2->next = mark1;
	mark1->next = nmark;
	mark2->prev = vmark;
	mark1->prev = mark2;
	if (nmark) nmark->prev = mark1;
}

void MarkerSortieren(void) {
	struct MARKER *mark;
	BOOL getauscht;
	
	do {
		mark = rootmark;
		getauscht = FALSE;
		while (mark) {
			if (mark->next) {
				if (mark->takt > mark->next->takt) {
					MarkerTauschen(mark, mark->next);
					getauscht = TRUE;
					break;
				}
			}
			mark = mark->next;
		}
	} while (getauscht);
}

void ErstelleGrundMarker(void) {
	wahlmark[M_TAKT] = NeuerMarker(M_TAKT, 0, 1, 4);
	wahlmark[M_TEMPO] = NeuerMarker(M_TEMPO, 0, 120, 0);
	wahlmark[M_TEXT] = NeuerMarker(M_TEXT, 0, 0, 0);
	if (wahlmark[M_TEXT]) strncpy(&wahlmark[M_TEXT]->text, CAT(MSG_0049, "Beginning"), 128);
}

void TaktWahlMark(LONG t) {
	wahlmark[M_TEMPO] = TaktMarker(NULL, M_TEMPO, t);
	wahlmark[M_TAKT] = TaktMarker(NULL, M_TAKT, t);
	wahlmark[M_TEXT] = TaktMarker(NULL, M_TEXT, t);
}


// Tempomarker

LONG TaktZeit(LONG t) {
	struct MARKER *akt;
	LONG z;
	LONG tr;
	
	z = 0; tr = t;

	akt = rootmark;
	if (akt) {
		while (akt->typ != M_TEMPO) {
			akt = akt->next;
			if (!akt) break;
		}
		
		if (akt) {
			while (akt->next) {
				if (akt->next->takt > t) break;
				if (akt->typ == M_TEMPO) {
					z = z + ((((akt->next->takt - akt->takt) * 1125) >> (VIERTEL - 6)) / akt->m_bpm);
					tr = tr - (akt->next->takt - akt->takt);
				}
				akt = akt->next;
			}
			z = z + (((tr * 1125) >> (VIERTEL - 6)) / akt->m_bpm);
		}
	}
	return(z);
}

LONG TaktSmpteTicks(LONG t) {
	LONG ticks;
	struct MARKER *akt;
	struct MARKER *next;
	
	akt = TaktMarker(NULL, M_TEMPO, 0);
	if (akt) {
		ticks = akt->ticks;
		
		next = NextMarker(akt);
		while (next) {
			if (next->takt >= t) break;
			ticks += (next->ticks - akt->ticks);
			akt = next;
			next = NextMarker(akt);
		}
		ticks += ((t - akt->takt) * 60 / akt->m_bpm * 600) >> VIERTEL;
		return(ticks);
	} else return(0);
}

LONG SmpteTicksTakt(LONG ticks) {
	LONG t;
	struct MARKER *akt;
	struct MARKER *next;
	
	akt = TaktMarker(NULL, M_TEMPO, 0);
	if (akt) {
		t = akt->takt;
		
		if (ticks < akt->ticks) return(0);
		
		next = NextMarker(akt);
		while (next) {
			if (next->ticks >= ticks) break;
			t += (next->takt - akt->takt);
			akt = next;
			next = NextMarker(akt);
		}
		t += (ticks - akt->ticks) / 60 * (akt->m_bpm << VIERTEL) / 600;
		return(t);
	} else return(0);
}

void SmpteTicksAktualisieren(void) {
	struct MARKER *akt;
	struct MARKER *next;
	
	akt = TaktMarker(NULL, M_TEMPO, 0);
	if (akt) {
		akt->ticks = smpte.startticks;

		next = NextMarker(akt);
		while (next) {
			next->ticks = akt->ticks + (((next->takt - akt->takt) >> VIERTEL) * 60 * 600 / akt->m_bpm);
			
			akt = next;
			next = NextMarker(akt);
		}
	}
}



// Taktmarker


void TakteAktualisieren(void) {
	struct MARKER *akt;
	WORD prevtaktnum;
	WORD prevzaehler;
	LONG prevtakt;
	
	akt = rootmark;
	if (akt) {
		while (akt->typ != M_TAKT) {
			akt = akt->next;
			if (!akt) break;
		}
		if (akt) {
			prevtaktnum = akt->m_taktnum;
			prevzaehler = akt->m_zaehler;
			prevtakt = akt->takt;
	
			while (akt) {
				if (akt->typ == M_TAKT) {
					akt->m_taktnum = prevtaktnum + ((((akt->takt - prevtakt) >> VIERTEL) + prevzaehler - 1) / prevzaehler);
					prevtaktnum = akt->m_taktnum;
					prevzaehler = akt->m_zaehler;
					prevtakt = akt->takt;
				}
				akt = akt->next;
			}
		}
	}
}


// Textmarker

LONG NextXMarkerTakt(LONG t) {
	struct MARKER *mark;

	mark = TaktMarker(NULL, M_TEXT, t);
	mark = NextMarker(mark);
	if (mark) return(mark->takt);
	else return(t);
}

LONG PrevXMarkerTakt(LONG t) {
	struct MARKER *mark;

	mark = TaktMarker(NULL, M_TEXT, t);
	if ((mark->takt == t) && (mark->takt > 0)) {
		mark = PrevMarker(mark);
	}
	return(mark->takt);
}
