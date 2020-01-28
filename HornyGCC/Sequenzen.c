#include <stdio.h>
#include <string.h>

#include <proto/exec.h>

#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "MidiEdit.h"
#include "Requester.h"
#include "EditorNotenGui.h"

extern struct SPUR spur[];
extern struct LIED lied;
extern struct SPURTEMP sp[];
extern struct SEQUENZ *edseq;

struct SEQUENZINFO seqinfo = {
	FALSE, // benutzt
	{0}, // name
	FALSE, // namemulti
	0, //trans
	FALSE, // transmulti
	FALSE, // mute
	FALSE, //mutemulti
	0 // aliasanz
};


struct SEQUENZ *ErstelleSequenz(int16 s, int32 t, BOOL allocevents) {
	struct SEQUENZ *seq;

	seq = IExec->AllocVecTags(sizeof(struct SEQUENZ), TAG_END);
	if (seq) {
		strncpy(seq->name, spur[s].name, 128);
		seq->start = t & VIERTELMASKE;
		seq->ende = seq->start + (VIERTELWERT << 2);
		seq->trans = 0;
		if (allocevents) seq->eventblock = IExec->AllocVecTags(sizeof(struct EVENTBLOCK), AVT_ClearWithValue,0,TAG_END);
		else seq->eventblock = NULL;
		seq->markiert = FALSE;
		seq->aliasorig = NULL;
		seq->aliasanz = 0;
		seq->mute = FALSE;
		seq->next = NULL;
	}
	return(seq);
}

BOOL AddEvbl(struct EVENTBLOCK *evbl) {
	evbl->next = IExec->AllocVecTags(sizeof(struct EVENTBLOCK), AVT_ClearWithValue,0,TAG_END);
	if (evbl->next) {
		evbl->next->prev = evbl;
		return(TRUE);
	} else {
		Meldung(CAT(MSG_0412, "Not enough memory for event block\n<Sequenzen.c>"));
		return(FALSE);
	}
}

void EvblsAbschneiden(struct EVENTBLOCK *evbl) {
	struct EVENTBLOCK *next;

	if (evbl->next) {
		evbl = evbl->next; evbl->prev->next = NULL;
		while (evbl) {
			next = evbl->next;
			IExec->FreeVec(evbl);
			evbl = next;
		}
	}
}

void SequenzenOrdnen(int16 s) {
	struct SEQUENZ *aktseq;
	struct SEQUENZ *altseq;
	struct SEQUENZ *startseq;

	if (spur[s].seq) {
		startseq = IExec->AllocVecTags(sizeof(struct SEQUENZ), AVT_ClearWithValue,0,TAG_END);
		if (startseq) {
			startseq->start = -1;
			startseq->next = spur[s].seq;

			altseq = startseq; aktseq = startseq->next;
			while (aktseq->next) {
				if (aktseq->start > aktseq->next->start) {
					altseq->next = aktseq->next;
					aktseq->next = aktseq->next->next;
					altseq->next->next = aktseq;

					altseq = startseq; aktseq = startseq->next;
				} else {
					altseq = aktseq; aktseq = aktseq->next;
				}
			}

			if ((aktseq->ende >> VIERTEL) > lied.taktanz) lied.taktanz = (aktseq->ende >> VIERTEL) + 30;

			spur[s].seq = startseq->next;
			IExec->FreeVec(startseq);
		} else Meldung(CAT(MSG_0413, "Not enough memory for sequence\n<Sequenzen.c>"));
	}
}

struct SEQUENZ *NeueSequenzEinordnen(int16 s) {
	struct SEQUENZ *seq;
	struct SEQUENZ *altseq;

	if (sp[s].neuseq) {
		sp[s].neuseq->spur = s;
		sp[s].anders = 2;
		if (spur[s].seq) {
			if (spur[s].seq->start >= sp[s].neuseq->start) {
				sp[s].neuseq->next = spur[s].seq;
				spur[s].seq = sp[s].neuseq;
			} else {

				seq = spur[s].seq->next; altseq = spur[s].seq;
				while (seq) {
					if (seq->start >= sp[s].neuseq->start) {
						sp[s].neuseq->next = seq;
						altseq->next = sp[s].neuseq;
						break;
					}
					altseq = seq; seq = seq->next;
				}
				if (!seq) {
					altseq->next = sp[s].neuseq;
					sp[s].neuseq->next = NULL;
				}

			}
		} else {
			spur[s].seq = sp[s].neuseq;
		}

		if ((sp[s].neuseq->ende >> VIERTEL) > lied.taktanz) lied.taktanz = (sp[s].neuseq->ende >> VIERTEL) + 30;

		seq = sp[s].neuseq; sp[s].neuseq = NULL;
		return(seq);
	}
	return(NULL);
}

void SequenzEntfernen(struct SEQUENZ *seq) {
	struct EVENTBLOCK *aktevbl;
	struct EVENTBLOCK *nextevbl;

	if (seq) {
		if (!seq->aliasorig) {
			aktevbl = seq->eventblock;
			while (aktevbl) {
				nextevbl = aktevbl->next;
				IExec->FreeVec(aktevbl);
				aktevbl = nextevbl;
			}
		} else {
			seq->aliasorig->aliasanz--;
		}
		IExec->FreeVec(seq);
	}
}

void SpurSequenzenEntfernen(int16 s) {
	struct SEQUENZ *aktseq;
	struct SEQUENZ *nextseq;
	struct EVENTBLOCK *aktevbl;
	struct EVENTBLOCK *nextevbl;

	aktseq = spur[s].seq;
	while (aktseq) {
		if (aktseq == edseq) EntferneEditorNotenFenster();
		if (!aktseq->aliasorig) {
			aktevbl = aktseq->eventblock;
			while (aktevbl) {
				nextevbl = aktevbl->next;
				IExec->FreeVec(aktevbl);
				aktevbl = nextevbl;
			}
		}
		nextseq = aktseq->next;
		IExec->FreeVec(aktseq);
		aktseq = nextseq;
	}
	spur[s].seq = NULL;
}

void SequenzAusSpurEntfernen(struct SEQUENZ *seq) {
	struct SEQUENZ *aktseq;
	struct SEQUENZ *altseq;
	int16 s;

	if (seq == edseq) EntferneEditorNotenFenster();
	s = seq->spur;
	if (seq == spur[s].seq) {
		spur[s].seq = seq->next;
	} else {
		aktseq = spur[s].seq->next; altseq = spur[s].seq;
		while (aktseq) {
			if (aktseq == seq) {
				altseq->next = aktseq->next;
				break;
			}
			altseq = aktseq; aktseq = aktseq->next;
		}
	}
	seq->next = NULL;
	seq->spur = -1;
}


struct SEQUENZ *TaktSequenz(int16 s, int32 t, int8 *p) {
	struct SEQUENZ *seq;
	struct SEQUENZ *wahl;

	seq = spur[s].seq; wahl = NULL;
	while (seq) {
		if ((t >= seq->start) && (t < seq->ende)) {
			wahl = seq; *p = 0;
			if (t < seq->start + VIERTELWERT) *p = 1;
			if (t >= seq->ende - VIERTELWERT) *p = 2;
		}
		seq = seq->next;
	}
	return(wahl);
}

BOOL HoleMarkSequenzenRahmen(int32 *start, int32 *ende) {
	struct SEQUENZ *seq;
	int16 s;
	
	*start = 0x7FFFFFFF;
	*ende = 0;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->start < *start) *start = seq->start;
				if (seq->ende > *ende) *ende = seq->ende;
			}
			seq = seq->next;
		}
	}
	
	return (*ende != 0);
}

void MarkSequenzenVerschiebenTest(int16 *sd, int32 *d) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->start + *d < 0) *d = -seq->start;
				if (s + *sd < 0) *sd = -s;
				if (s + *sd >= lied.spuranz) *sd = lied.spuranz - s - 1;
			}
			seq = seq->next;
		}
	}
}

void MarkSequenzenVerschieben(int16 s, int16 sd, int32 d) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	BOOL anders = FALSE;

	seq = spur[s].seq;
	while (seq) {
		nextseq = seq->next;

		if (seq->markiert) {
			if (seq->start + d < 0) d = -seq->start;
			if (s + sd < 0) sd = -s;
			if (s + sd >= lied.spuranz) sd = lied.spuranz - s - 1;

			if (d) {
				seq->start = seq->start + d;
				seq->ende = seq->ende + d;
			}
			if (sd) {
				SequenzAusSpurEntfernen(seq);
				sp[s + sd].neuseq = seq;
				NeueSequenzEinordnen(s + sd);
			}
			if (d || sd) anders = TRUE;
		}

		seq = nextseq;
	}
	if (anders) {
		SequenzenOrdnen(s);
		sp[s].anders = 2;
		sp[s + sd].anders = 2;
	}
}

void MarkSequenzenEntfernen(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	int16 s;
	BOOL hindernis = FALSE;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			nextseq = seq->next;
			if (seq->markiert) {
				if (!seq->aliasanz) {
					sp[s].anders = 2;
					SequenzAusSpurEntfernen(seq);
					SequenzEntfernen(seq);
				} else hindernis = TRUE;
			}
			seq = nextseq;
		}
	}
	if (hindernis) Meldung(CAT(MSG_0414, "Sequences with aliases (possibly in clipboard)\ncannot be deleted"));
}

struct SEQUENZ *NeuesAlias(struct SEQUENZ *original) {
	struct SEQUENZ *seq;

	seq = IExec->AllocVecTags(sizeof(struct SEQUENZ), TAG_END);
	if (seq) {
		seq->name[0] = 0;
		seq->start = original->start;
		seq->ende = original->ende;
		seq->trans = original->trans;
		seq->markiert = TRUE;
		if (original->aliasorig) {
			seq->aliasorig = original->aliasorig;
			seq->eventblock = original->aliasorig->eventblock;
		} else {
			seq->aliasorig = original;
			seq->eventblock = original->eventblock;
		}
		seq->aliasorig->aliasanz++;
		seq->aliasanz = 0;
		seq->spur = original->spur;
		seq->mute = original->mute;
		seq->next = NULL;
	} else Meldung(CAT(MSG_0415, "Not enough memory for alias\n<Sequenzen.c>"));
	return(seq);
}

struct SEQUENZ *MarkSequenzenAlias(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	struct SEQUENZ *erstesalias = NULL;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			nextseq = seq->next;
			if (seq->markiert) {
				seq->markiert = FALSE;
				seq = NeuesAlias(seq);
				sp[s].neuseq = seq;
				if (!erstesalias) erstesalias = seq;
				NeueSequenzEinordnen(s);
			}
			seq = nextseq;
		}
	}
	return(erstesalias);
}

struct SEQUENZ *NeueKopie(struct SEQUENZ *original) {
	struct SEQUENZ *seq;
	struct EVENTBLOCK *evbl;
	struct EVENTBLOCK *neuevbl;
	struct EVENTBLOCK *altevbl;

	seq = IExec->AllocVecTags(sizeof(struct SEQUENZ), TAG_END);
	if (seq) {
		if (original->aliasorig) {
			strncpy(seq->name, original->aliasorig->name, 128);
		} else {
			strncpy(seq->name, original->name, 128);
		}
		seq->start = original->start;
		seq->ende = original->ende;
		seq->trans = original->trans;
		seq->markiert = TRUE;
		seq->aliasorig = NULL;
		seq->aliasanz = 0;
		seq->spur = original->spur;
		seq->mute = original->mute;
		seq->next = NULL;

		evbl = original->eventblock; altevbl = NULL;
		while (evbl) {
			neuevbl = IExec->AllocVecTags(sizeof(struct EVENTBLOCK), TAG_END);
			if (neuevbl) {
				memcpy(neuevbl, evbl, sizeof(struct EVENTBLOCK));
				neuevbl->prev = altevbl;
			} else Meldung(CAT(MSG_0412, "Not enough memory for event block\n<Sequenzen.c>"));
			if (altevbl) altevbl->next = neuevbl; else seq->eventblock = neuevbl;
			if (!neuevbl) break;
			altevbl = neuevbl; evbl = evbl->next;
		}
	} else Meldung(CAT(MSG_0413, "Not enough memory for sequence\n<Sequenzen.c>"));
	return(seq);
}

struct SEQUENZ *MarkSequenzenKopieren(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	struct SEQUENZ *erstekopie = NULL;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			nextseq = seq->next;
			if (seq->markiert) {
				seq->markiert = FALSE;
				seq = NeueKopie(seq);
				sp[s].neuseq = seq;
				if (!erstekopie) erstekopie = seq;
				NeueSequenzEinordnen(s);
			}
			seq = nextseq;
		}
	}
	return(erstekopie);
}

void MarkSequenzenAliasZuReal(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			nextseq = seq->next;
			if (seq->markiert && seq->aliasorig) {
				sp[s].anders = 2;
				sp[s].neuseq = NeueKopie(seq);
				SequenzAusSpurEntfernen(seq);
				SequenzEntfernen(seq);
				NeueSequenzEinordnen(s);
			}
			seq = nextseq;
		}
	}
}

void SequenzenInSpurMarkieren(int16 s) {
	struct SEQUENZ *seq;

	seq = spur[s].seq;
	while (seq) {
		if (!seq->markiert) {
			sp[s].anders = 1; seq->markiert = TRUE;
		}
		seq = seq->next;
	}
}

void SequenzenAbXMarkieren(int32 t) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->ende > t) {
				seq->markiert = TRUE;
				sp[s].anders = 1;
			}
			seq = seq->next;
		}
	}
}

void NichtsMarkieren(void) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				sp[s].anders = 1; seq->markiert = FALSE;
			}
			seq = seq->next;
		}
	}
}

void BereichMarkieren(int16 vs, int16 bs, int32 vt, int32 bt) {
	struct SEQUENZ *seq;
	int16 s;
	int32 tausch;

	if (vs > bs) {tausch = vs; vs = bs; bs = tausch;}
	if (vt > bt) {tausch = vt; vt = bt; bt = tausch;}
	if (bs >= lied.spuranz) bs = lied.spuranz-1;

	for (s = vs; s <= bs; s++) {
		seq = spur[s].seq;
		while (seq) {
			if ((seq->start <= bt) && (seq->ende > vt)) {
				sp[s].anders = 1; seq->markiert = !seq->markiert;
			}
			seq = seq->next;
		}
	}
}

void SequenzenSpuren(int16 s) {
	struct SEQUENZ *seq;

	seq = spur[s].seq;
	while (seq) {
		seq->spur = s;
		seq = seq->next;
	}
}

BOOL SequenzZerschneiden(struct SEQUENZ *seq, int32 tp, int8 *trennart) {
	struct EVENTBLOCK *evbl;
	struct EVENTBLOCK *killevbl;
	int16 evnum;
	int16 killevnum;
	int32 ende;
	struct EVENTBLOCK *neuevbl;
	struct EVENTBLOCK *prevneuevbl;
	int16 neuevnum;
	int32 d;
	int16 s = 0; // TODO: check line 619

	if (!seq->aliasorig) {

		*trennart = ZerschneideSequenzNoten(seq, tp, *trennart);

		evbl = seq->eventblock; evnum = 0;
		ende = seq->start;
		while (evbl) {
			if (!evbl->event[evnum].status) break;

			if (seq->start + evbl->event[evnum].zeit >= tp) break;
			ende = seq->start + evbl->event[evnum].zeit;

			evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
		}

		if (evbl && evbl->event[evnum].status) {

			s = seq->spur;
			killevbl = evbl; killevnum = evnum;

			sp[s].neuseq = IExec->AllocVecTags(sizeof(struct SEQUENZ), TAG_END);
			if (sp[s].neuseq) {
				strncpy(sp[s].neuseq->name, seq->name, 128);
				sp[s].neuseq->start = (seq->start + evbl->event[evnum].zeit) & VIERTELMASKE;
				sp[s].neuseq->ende = seq->ende;
				sp[s].neuseq->trans = seq->trans;
				sp[s].neuseq->eventblock = IExec->AllocVecTags(sizeof(struct EVENTBLOCK), AVT_ClearWithValue,0,TAG_END);
				sp[s].neuseq->markiert = TRUE;
				sp[s].neuseq->aliasorig = NULL;
				sp[s].neuseq->aliasanz = 0;
				sp[s].neuseq->mute = seq->mute;
				sp[s].neuseq->next = NULL;
				seq->ende = (ende + VIERTELWERT - 1) & VIERTELMASKE;
				if (seq->ende == seq->start) seq->ende += VIERTELWERT;

				if (sp[s].neuseq->eventblock) {
					d = sp[s].neuseq->start - seq->start;
					neuevbl = sp[s].neuseq->eventblock; neuevnum = 0;

					do {
						neuevbl->event[neuevnum].zeit = evbl->event[evnum].zeit - d;
						neuevbl->event[neuevnum].status = evbl->event[evnum].status;
						neuevbl->event[neuevnum].data1 = evbl->event[evnum].data1;
						neuevbl->event[neuevnum].data2 = evbl->event[evnum].data2;

						evnum++;
						if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}

						neuevnum++;
						if (neuevnum == EVENTS) {
							prevneuevbl = neuevbl;
							neuevbl = IExec->AllocVecTags(sizeof(struct EVENTBLOCK), AVT_ClearWithValue, 0, TAG_END);
							if (neuevbl) {
								neuevbl->prev = prevneuevbl;
							} else Meldung(CAT(MSG_0412, "Not enough memory for event block\n<Sequenzen.c>"));
							prevneuevbl->next = neuevbl;
							neuevnum = 0;
						}
					} while (evbl && evbl->event[evnum].status && neuevbl);

					if (neuevbl) {
						killevbl->event[killevnum].status = 0;
						EvblsAbschneiden(killevbl);
					}
				} else Meldung(CAT(MSG_0412, "Not enough memory for event block\n<Sequenzen.c>"));

				NeueSequenzEinordnen(s);
				AliaseAnpassen(seq, 0);
				sp[s].anders = 2;
				return(TRUE);
			} else {
				Meldung(CAT(MSG_0413, "Not enough memory for sequence\n<Sequenzen.c>"));
			}
		} else {
			seq->ende = (ende + VIERTELWERT - 1) & VIERTELMASKE;
			AliaseAnpassen(seq, 0);
			sp[s].anders = 2;
			return(TRUE);
		}
	} else {
		Meldung(CAT(MSG_0421, "Aliases cannot be splitted"));
	}
	return(FALSE);
}

void MarkSequenzenZerschneiden(int32 tp) {
	struct SEQUENZ *seq;
	int16 s;
	int8 trennart = 0;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if ((seq->start < tp) && (seq->ende > tp)) {
					if (SequenzZerschneiden(seq, tp, &trennart)) break;
				}
			}
			seq = seq->next;
		}
	}
}

void SequenzUnterteilen(struct SEQUENZ *seq, int32 tp) {
	int32 d;
	int32 e;
	int8 trennart = 0;

	if (seq) {
		if ((seq->start < tp) && (seq->ende > tp)) {
			d = tp - seq->start;
			e = seq->ende;
			do {
				while (seq->start >= tp) tp = tp + d;
				SequenzZerschneiden(seq, tp, &trennart);
				tp = tp + d;
				seq = seq->next;
			} while (tp < e);
		}
	}
	
}

BOOL SequenzenVerbinden(struct SEQUENZ *seq1, struct SEQUENZ *seq2) {
	struct EVENTBLOCK *evbl1;
	int16 evnum1;
	struct EVENTBLOCK *evbl2;
	int16 evnum2;
	int32 d;

	if (!seq1->aliasorig && !seq2->aliasorig) {
		evbl1 = seq1->eventblock; evnum1 = 0;
		while (evbl1) {
			if (!evbl1->event[evnum1].status) break;
			evnum1++; if (evnum1 == EVENTS) {evnum1 = 0; evbl1 = evbl1->next;}
		}

		d = seq2->start - seq1->start;
		evbl2 = seq2->eventblock; evnum2 = 0;
		while (evbl2 && evbl1) {
			memcpy(&evbl1->event[evnum1], &evbl2->event[evnum2], sizeof(struct EVENT));
			evbl1->event[evnum1].zeit += d;

			evnum1++;
			if (evnum1 == EVENTS) {
				evnum1 = 0; AddEvbl(evbl1);
				evbl1 = evbl1->next;
			}

			if (!evbl2->event[evnum2].status) break;
			evnum2++; if (evnum2 == EVENTS) {evnum2 = 0; evbl2 = evbl2->next;}
		}

		OrdneEvents(seq1);
		if (evbl1) {
			seq1->ende = seq2->ende;
			SequenzAusSpurEntfernen(seq2);
			SequenzEntfernen(seq2);
		}
		AliaseAnpassen(seq1, 0);
		sp[seq1->spur].anders = 2;
		return(TRUE);
	} else {
		Meldung(CAT(MSG_0422, "Original sequences can be merged only"));
		return(FALSE);
	}
}

void MarkSequenzenVerbinden(void) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (!seq->next) break;
			if (seq->markiert && seq->next->markiert) {
				if (SequenzenVerbinden(seq, seq->next)) {
					seq = spur[s].seq;
				} else {
					seq = seq->next;
				}
			} else {
				seq = seq->next;
			}
		}
	}
}

void MarkSequenzenStartVerschieben(int16 d) {
	struct SEQUENZ *seq;
	struct SEQUENZ *orig;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->aliasorig) orig = seq->aliasorig; else orig = seq;
				if ((orig->start + d >= 0) && (orig->start + d < orig->ende)) {
					orig->start = orig->start + d;
					EventsVerschieben(orig, -d);
					sp[orig->spur].anders = 2;
					if (orig->aliasanz) AliaseAnpassen(orig, d);
				}
			}
			seq = seq->next;
		}
	}
	for (s = 0; s < lied.spuranz; s++) {
		if (sp[s].anders) SequenzenOrdnen(s);
	}
}

void MarkSequenzenEndeVerschieben(int16 d) {
	struct SEQUENZ *seq;
	struct SEQUENZ *orig;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->aliasorig) orig = seq->aliasorig; else orig = seq;
				if (orig->ende + d > orig->start) {
					orig->ende = orig->ende + d;
					sp[orig->spur].anders = 2;
					if (orig->aliasanz) AliaseAnpassen(orig, 0);
				}
			}
			seq = seq->next;
		}
	}
}

void AlleAliaseZuweisen(void) {
	int16 s, n;
	struct SEQUENZ *seq;
	struct SEQUENZ *seq2;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->aliasanz) {

				for (n = 0; n < lied.spuranz; n++) {
					seq2 = spur[n].seq;
					while (seq2) {
						if (seq2->aliasorig == seq->speicheradr) {
							seq2->aliasorig = seq;
							seq2->eventblock = seq->eventblock;
						}
						seq2 = seq2->next;
					}
				}

			}
			seq = seq->next;
		}
	}
}

void InitSequenzInfo(void) {
	seqinfo.benutzt = FALSE;
	seqinfo.name[0] = 0;
	seqinfo.namemulti = FALSE;
	seqinfo.trans = 0;
	seqinfo.transmulti = FALSE;
	seqinfo.mute = FALSE;
	seqinfo.mutemulti = FALSE;
	seqinfo.aliasanz = 0;
}

void MarkSequenzInfo(void) {
	int16 s;
	struct SEQUENZ *seq;
	STRPTR name;
	BOOL ns = FALSE, ts = FALSE, ms = FALSE;

	InitSequenzInfo();
	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->aliasorig) name = seq->aliasorig->name;
				else name = seq->name;
				if (!ns) { // Name
					strncpy(seqinfo.name, name, 128);
					ns = TRUE;
				} else if (strcmp(seqinfo.name, name) != 0) seqinfo.namemulti = TRUE;

				if (!ts) { // Trans
					seqinfo.trans = seq->trans;
					ts = TRUE;
				} else if (seqinfo.trans != seq->trans) seqinfo.transmulti = TRUE;

				if (!ms) { // Mute
					seqinfo.mute = seq->mute;
					ms = TRUE;
				} else if (seqinfo.mute != seq->mute) seqinfo.mutemulti = TRUE;

				seqinfo.aliasanz += seq->aliasanz; // Aliase summieren
			}
			seq = seq->next;
		}
	}

	if (ns || ts) seqinfo.benutzt = TRUE;
}

void MarkSequenzenSetzeName(STRPTR name) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->aliasorig) strncpy(seq->aliasorig->name, name, 128);
				else strncpy(seq->name, name, 128);
			}
			seq = seq->next;
		}
	}
}

void MarkSequenzenSetzeTrans(int8 trans) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				seq->trans = trans;
				sp[s].anders = 2;
			}
			seq = seq->next;
		}
	}
}

void MarkSequenzenSetzeMute(BOOL mute) {
	struct SEQUENZ *seq;
	int16 s;

	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				seq->mute = mute;
				sp[s].anders = 2;
			}
			seq = seq->next;
		}
	}
}
