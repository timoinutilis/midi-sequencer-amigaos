#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <midi/mididefs.h>

#include <proto/exec.h>

#include "Strukturen.h"
#include "MidiEdit.h"
#include "Requester.h"

extern struct SPUR spur[128];
extern struct SEQUENZ *neuseq=NULL;
extern struct LIED lied;
extern struct SPURTEMP sp[128];


void SequenzenOrdnen(WORD s) {
	struct SEQUENZ *aktseq;
	struct SEQUENZ *altseq;
	struct SEQUENZ *startseq;

	if (spur[s].seq) {
		if (startseq=AllocVec(sizeof(struct SEQUENZ), MEMF_ANY | MEMF_CLEAR)) {
			startseq->start=-1;
			startseq->next=spur[s].seq;

			altseq=startseq; aktseq=startseq->next;
			while (aktseq->next) {
				if (aktseq->start > aktseq->next->start) {
					altseq->next=aktseq->next;
					aktseq->next=aktseq->next->next;
					altseq->next->next=aktseq;

					altseq=startseq; aktseq=startseq->next;
				} else {
					altseq=aktseq; aktseq=aktseq->next;
				};
			};

			if ((aktseq->ende>>10) > lied.taktanz) lied.taktanz=(aktseq->ende>>10)+30;

			spur[s].seq=startseq->next;
			FreeVec(startseq);
		};
	};
}

struct SEQUENZ *NeueSequenzEinordnen(WORD s) {
	struct SEQUENZ *seq;
	struct SEQUENZ *altseq;

	if (neuseq) {
		neuseq->spur=s;
		sp[s].anders=TRUE;
		if (spur[s].seq) {
			if (spur[s].seq->start >= neuseq->start) {
				neuseq->next=spur[s].seq;
				spur[s].seq=neuseq;
			} else {

				seq=spur[s].seq->next; altseq=spur[s].seq;
				while (seq) {
					if (seq->start >= neuseq->start) {
						neuseq->next=seq;
						altseq->next=neuseq;
						break;
					};
					altseq=seq; seq=seq->next;
				};
				if (!seq) {
					altseq->next=neuseq;
					neuseq->next=NULL;
				};

			};
		} else {
			spur[s].seq=neuseq;
		};

		if ((neuseq->ende>>10) > lied.taktanz) lied.taktanz=(neuseq->ende>>10)+30;

		seq=neuseq; neuseq=NULL;
		return(seq);
	};
	return(NULL);
}

void SequenzEntfernen(struct SEQUENZ *seq) {
	struct EVENTBLOCK *aktevbl;
	struct EVENTBLOCK *nextevbl;

	if (seq) {
		if (!seq->aliasorig) {
			aktevbl=seq->eventblock;
			while (aktevbl) {
				nextevbl=aktevbl->next;
				FreeVec(aktevbl);
				aktevbl=nextevbl;
			};
		} else {
			seq->aliasorig->aliasanz--;
		};
		FreeVec(seq);
	};
}

void SpurSequenzenEntfernen(WORD s) {
	struct SEQUENZ *aktseq;
	struct SEQUENZ *nextseq;
	struct EVENTBLOCK *aktevbl;
	struct EVENTBLOCK *nextevbl;

	aktseq=spur[s].seq;
	while (aktseq) {
		if (!aktseq->aliasorig) {
			aktevbl=aktseq->eventblock;
			while (aktevbl) {
				nextevbl=aktevbl->next;
				FreeVec(aktevbl);
				aktevbl=nextevbl;
			};
		};
		nextseq=aktseq->next;
		FreeVec(aktseq);
		aktseq=nextseq;
	};
	spur[s].seq=NULL;
}

void SequenzAusSpurEntfernen(struct SEQUENZ *seq) {
	struct SEQUENZ *aktseq;
	struct SEQUENZ *altseq;
	WORD s;

	s=seq->spur;
	if (seq==spur[s].seq) {
		spur[s].seq=seq->next;
	} else {
		aktseq=spur[s].seq->next; altseq=spur[s].seq;
		while (aktseq) {
			if (aktseq==seq) {
				altseq->next=aktseq->next;
				break;
			};
			altseq=aktseq; aktseq=aktseq->next;
		};
	};
	seq->next=NULL;
	seq->spur=-1;
}


struct SEQUENZ *TaktSequenz(WORD s, LONG t, BYTE *p) {
	struct SEQUENZ *seq;
	struct SEQUENZ *wahl;

	seq=spur[s].seq; wahl=NULL;
	while (seq) {
		if ((t>=seq->start) && (t<=seq->ende)) {
			wahl=seq; *p=0;
			if (t<seq->start+1024) *p=1;
			if (t>seq->ende-1024) *p=2;
		};
		seq=seq->next;
	};
	return(wahl);
}

void MarkSequenzenVerschieben(WORD s, WORD sd, LONG d) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	BOOL anders=FALSE;

	seq=spur[s].seq;
	while (seq) {
		nextseq=seq->next;

		if (seq->markiert) {
			if (seq->start+d<0) d=-seq->start;
			if (s+sd<0) sd=-s;
			if (s+sd>=lied.spuranz) sd=lied.spuranz-s-1;

			if (d) {
				seq->start=seq->start+d;
				seq->ende=seq->ende+d;
			};
			if (sd) {
				SequenzAusSpurEntfernen(seq);
				neuseq=seq;
				NeueSequenzEinordnen(s+sd);
			};
			if (d || sd) anders=TRUE;
		};

		seq=nextseq;
	};
	if (anders) {
		SequenzenOrdnen(s);
		sp[s].anders=TRUE;
		sp[s+sd].anders=TRUE;
	};
}

void MarkSequenzenEntfernen(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			nextseq=seq->next;
			if (seq->markiert) {
				if (!seq->aliasanz) {
					sp[s].anders=TRUE;
					SequenzAusSpurEntfernen(seq);
					SequenzEntfernen(seq);
				} else {
					Meldung("Sequenzen mit Aliasen können\nnicht gelöscht werden");
				};
			};
			seq=nextseq;
		};
	};
}

struct SEQUENZ *NeuesAlias(struct SEQUENZ *original) {
	if (neuseq=AllocVec(sizeof(struct SEQUENZ), 0)) {
		neuseq->name[0]=0;
		neuseq->start=original->start;
		neuseq->ende=original->ende;
		neuseq->trans=original->trans;
		neuseq->eventblock=original->eventblock;
		neuseq->markiert=TRUE;
		if (original->aliasorig) {
			neuseq->aliasorig=original->aliasorig;
		} else {
			neuseq->aliasorig=original;
		};
		neuseq->aliasorig->aliasanz++;
		neuseq->aliasanz=0;
		neuseq->next=NULL;
	};
	return(neuseq);
}

void MarkSequenzenAlias(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			nextseq=seq->next;
			if (seq->markiert) {
				sp[s].anders=TRUE;
				seq->markiert=FALSE;
				seq=NeuesAlias(seq);
				NeueSequenzEinordnen(s);
			};
			seq=nextseq;
		};
	};
}

struct SEQUENZ *NeueKopie(struct SEQUENZ *original) {
	struct EVENTBLOCK *evbl;
	struct EVENTBLOCK *neuevbl;
	struct EVENTBLOCK *altevbl;

	if (neuseq=AllocVec(sizeof(struct SEQUENZ), 0)) {
		if (original->aliasorig) {
			strcpy(neuseq->name, original->aliasorig->name);
		} else {
			strcpy(neuseq->name, original->name);
		};
		neuseq->start=original->start;
		neuseq->ende=original->ende;
		neuseq->trans=original->trans;
		neuseq->markiert=TRUE;
		neuseq->aliasorig=NULL;
		neuseq->aliasanz=0;
		neuseq->next=NULL;

		evbl=original->eventblock; altevbl=NULL;
		while (evbl) {
			neuevbl=AllocVec(sizeof(struct EVENTBLOCK), 0);
			memcpy(neuevbl, evbl, sizeof(struct EVENTBLOCK));
			if (altevbl) altevbl->next=neuevbl else neuseq->eventblock=neuevbl;
			neuevbl->prev=altevbl;
			altevbl=neuevbl; evbl=evbl->next;
		};
	};
	return(neuseq);
}

void MarkSequenzenKopieren(void) {
	struct SEQUENZ *seq;
	struct SEQUENZ *nextseq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			nextseq=seq->next;
			if (seq->markiert) {
				sp[s].anders=TRUE;
				seq->markiert=FALSE;
				seq=NeueKopie(seq);
				NeueSequenzEinordnen(s);
			};
			seq=nextseq;
		};
	};
}

void NichtsMarkieren(void) {
	struct SEQUENZ *seq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				sp[s].anders=TRUE; seq->markiert=FALSE;
			};
			seq=seq->next;
		};
	};
}

void SequenzenSpuren(WORD s) {
	struct SEQUENZ *seq;

	seq=spur[s].seq;
	while (seq) {
		seq->spur=s;
		seq=seq->next;
	};
}

void SpurenTauschen(WORD s1, WORD s2) {
	struct SPUR ksp;

	memcpy(&ksp, &spur[s1], sizeof(struct SPUR));
	memcpy(&spur[s1], &spur[s2], sizeof(struct SPUR));
	memcpy(&spur[s2], &ksp, sizeof(struct SPUR));
	SequenzenSpuren(s1);
	SequenzenSpuren(s2);
}

BOOL SpurLoeschen(WORD s) {
	struct SEQUENZ *seq;
	struct SEQUENZ *zseq;
	WORD n;

	seq=spur[s].seq;
	while (seq) {
		if (seq->aliasanz) {

			zseq=spur[s].seq; n=0;
			while (zseq) {
				if (zseq->aliasorig==seq) n++;
				zseq=zseq->next;
			};

			if (n==seq->aliasanz) seq=NULL;
			break;
		};
		seq=seq->next;
	};
	if (!seq) {
		seq=spur[s].seq;
		while (seq) {
			if (seq->aliasorig) seq->aliasorig->aliasanz--;
			seq=seq->next;
		};
		SpurSequenzenEntfernen(s);

		for (n=s; n<lied.spuranz-1; n++) {
			memcpy(&spur[n], &spur[n+1], sizeof(struct SPUR));
			SequenzenSpuren(n);
		};
		strcpy(spur[n].name, "Unbenannt");
		spur[n].port=0;
		spur[n].channel=0;
		spur[n].bank=-1;
		spur[n].prog=0;
		spur[n].shift=0;
		spur[n].mute=FALSE;
		spur[n].seq=NULL;
		if (lied.spuranz>1) lied.spuranz--;

		return(TRUE);
	} else {
		Meldung("Spur enthält Originalsequenz(en) von Aliasen");
		return(FALSE);
	};
}

BOOL SequenzZerschneiden(struct SEQUENZ *seq, LONG tp) {
	struct EVENTBLOCK *evbl;
	struct EVENTBLOCK *killevbl;
	WORD evnum;
	WORD killevnum;
	LONG ende;
	struct EVENTBLOCK *neuevbl;
	struct EVENTBLOCK *prevneuevbl;
	WORD neuevnum;
	LONG d;

	if (!seq->aliasorig) {
		evbl=seq->eventblock; evnum=0;
		ende=seq->start;
		do {
			if (seq->start + evbl->event[evnum].zeit >= tp) break;
			ende=seq->start + evbl->event[evnum].zeit;

			evnum++; if (evnum==EVENTS) {evbl=evbl->next; evnum=0};
		} while (evbl && evbl->event[evnum].status);

		if (evbl && evbl->event[evnum].status) {

			killevbl=evbl; killevnum=evnum;

			if (neuseq=AllocVec(sizeof(struct SEQUENZ), 0)) {
				strcpy(neuseq->name, seq->name);
				neuseq->start=(seq->start + evbl->event[evnum].zeit) & 0xFFFFFC00;
				neuseq->ende=seq->ende;
				neuseq->trans=seq->trans;
				neuseq->eventblock=AllocVec(sizeof(struct EVENTBLOCK), MEMF_CLEAR);
				neuseq->markiert=TRUE;
				neuseq->aliasorig=NULL;
				neuseq->aliasanz=0;
				neuseq->next=NULL;
				seq->ende=ende;

				d=neuseq->start - seq->start;
				neuevbl=neuseq->eventblock; neuevnum=0;

				do {
					neuevbl->event[neuevnum].zeit = evbl->event[evnum].zeit - d;
					neuevbl->event[neuevnum].status = evbl->event[evnum].status;
					neuevbl->event[neuevnum].data1 = evbl->event[evnum].data1;
					neuevbl->event[neuevnum].data2 = evbl->event[evnum].data2;

					evnum++;
					if (evnum==EVENTS) {evbl=evbl->next; evnum=0};

					neuevnum++;
					if (neuevnum==EVENTS) {
						prevneuevbl=neuevbl;
						neuevbl=AllocVec(sizeof(struct EVENTBLOCK), MEMF_CLEAR);
						prevneuevbl->next=neuevbl; neuevbl->prev=prevneuevbl;
						neuevnum=0
					};
				} while (evbl && evbl->event[evnum].status);

				killevbl->event[killevnum].status=0;
				killevbl=killevbl->next; killevbl->prev->next=NULL;
				while (killevbl) {
					evbl=killevbl->next;
					FreeVec(killevbl);
					killevbl=evbl;
				};

				NeueSequenzEinordnen(seq->spur);
				AliaseAnpassen(seq, 0);
				sp[seq->spur].anders=TRUE;
				return(TRUE);
			};
		};
	} else {
		Meldung("Aliase können nicht zerschnitten werden");
	};
	return(FALSE);
}

void MarkSequenzenZerschneiden(LONG tp) {
	struct SEQUENZ *seq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if ((seq->start < tp) && (seq->ende > tp)) {
					if (SequenzZerschneiden(seq, tp)) break;
				};
			};
			seq=seq->next;
		};
	};
}

BOOL SequenzenVerbinden(struct SEQUENZ *seq1, struct SEQUENZ *seq2) {
	struct EVENTBLOCK *evbl1;
	WORD evnum1;
	struct EVENTBLOCK *evbl2;
	WORD evnum2;
	LONG d;

	if (!seq1->aliasorig && !seq2->aliasorig) {
		evbl1=seq1->eventblock; evnum1=0;
		do {
			if (evnum1<EVENTS-1) {
				if (!evbl1->event[evnum1+1].status) break;
			} else {
				if (!evbl1->next) break;
			};
			evnum1++; if (evnum1==EVENTS) {evnum1=0; evbl1=evbl1->next};
		} while (TRUE);

		d=seq2->start - seq1->start;
		evbl2=seq2->eventblock; evnum2=0;
		do {

			evnum1++;
			if (evnum1==EVENTS) {
				evnum1=0;
				evbl1->next=AllocVec(sizeof(struct EVENTBLOCK), MEMF_CLEAR);
				evbl1->next->prev=evbl1;
				evbl1=evbl1->next;
			};

			evbl1->event[evnum1].zeit = evbl2->event[evnum2].zeit + d;
			evbl1->event[evnum1].status = evbl2->event[evnum2].status;
			evbl1->event[evnum1].data1 = evbl2->event[evnum2].data1;
			evbl1->event[evnum1].data2 = evbl2->event[evnum2].data2;

			evnum2++; if (evnum2==EVENTS) {evnum2=0; evbl2=evbl2->next};
		} while (evbl2 && evbl2->event[evnum2].status);

		OrdneEvents(seq1);
		SequenzAusSpurEntfernen(seq2);
		SequenzEntfernen(seq2);
		AliaseAnpassen(seq1, 0);
		sp[seq1->spur].anders=TRUE;
		return(TRUE);
	} else {
		Meldung("Die zu verbindenen Sequenzen müssen Originale sein");
		return(FALSE);
	};
}

void MarkSequenzenVerbinden(void) {
	struct SEQUENZ *seq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			if (!seq->next) break;
			if (seq->markiert && seq->next->markiert) {
				if (SequenzenVerbinden(seq, seq->next)) {
					seq=spur[s].seq;
				} else {
					seq=seq->next;
				};
			} else {
				seq=seq->next;
			};
		};
	};
}

void MarkSequenzenStartVerschieben(WORD d) {
	struct SEQUENZ *seq;
	struct SEQUENZ *orig;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->aliasorig) orig=seq->aliasorig else orig=seq;
				if ((orig->start+d>=0) && (orig->start+d<orig->ende)) {
					orig->start=orig->start+d;
					EventsVerschieben(orig, -d);
					sp[orig->spur].anders=TRUE;
					if (orig->aliasanz) AliaseAnpassen(orig, d);
				};
			};
			seq=seq->next;
		};
	};
	for (s=0; s<lied.spuranz; s++) {
		if (sp[s].anders) SequenzenOrdnen(s);
	};
}

void MarkSequenzenEndeVerschieben(WORD d) {
	struct SEQUENZ *seq;
	struct SEQUENZ *orig;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (seq->aliasorig) orig=seq->aliasorig else orig=seq;
				if (orig->ende+d>orig->start) {
					orig->ende=orig->ende+d;
					sp[orig->spur].anders=TRUE;
					if (orig->aliasanz) AliaseAnpassen(orig, 0);
				};
			};
			seq=seq->next;
		};
	};
}
