#include <stdio.h>

#include <midi/mididefs.h>
#include "Strukturen.h"

extern struct SPUR spur[128];
extern struct SPURTEMP sp[128];
extern struct SEQUENZ *neuseq;
extern struct LIED lied;

void AliaseAnpassen(struct SEQUENZ *original, WORD d) {
	struct SEQUENZ *seq;
	WORD s;

	if (original->aliasanz) {
		for (s=0; s<lied.spuranz; s++) {
			seq=spur[s].seq;
			while (seq) {
				if (seq->aliasorig==original) {
					seq->start=seq->start + d;
					seq->ende=seq->start + (original->ende - original->start);
					seq->eventblock=original->eventblock;
					sp[s].anders=TRUE;
				};
				seq=seq->next;
			};
		};
	};
}

void EventsVerschieben(struct SEQUENZ *seq, WORD d) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	
	if (d) {
		evbl=seq->eventblock; evnum=0;
		do {
			evbl->event[evnum].zeit += d;
			
			evnum++;
			if (evnum==EVENTS) {evbl=evbl->next; evnum=0};
		} while (evbl && evbl->event[evnum].status);
	};
}

void OrdneEvents(struct SEQUENZ *seq) {
	struct EVENT *ev1;
	struct EVENT *ev2;
	struct EVENT evk;
	struct EVENTBLOCK *evbl;
	WORD evnum;
	
	evbl=seq->eventblock; evnum=0;
	do {
		
		ev1=&evbl->event[evnum];
		evnum++; if (evnum==EVENTS) {evbl=evbl->next; evnum=0};
		if (!evbl) break;
		ev2=&evbl->event[evnum];
		if (!ev2->status) break;
		
		if (ev1->zeit > ev2->zeit) {
			evk.zeit=ev1->zeit;
			evk.status=ev1->status;
			evk.data1=ev1->data1;
			evk.data2=ev1->data2;
			
			ev1->zeit=ev2->zeit; ev2->zeit=evk.zeit;
			ev1->status=ev2->status; ev2->status=evk.status;
			ev1->data1=ev2->data1; ev2->data1=evk.data1;
			ev1->data2=ev2->data2; ev2->data2=evk.data2;

			evnum=evnum-2;
			if (evnum<0) {evbl=evbl->prev; evnum=evnum+EVENTS};
			if (!evbl) {evbl=seq->eventblock; evnum=0};
		};

	} while (TRUE);
	seq->ende=seq->start + ev1->zeit;
	
}

void QuantisiereSequenz(struct SEQUENZ *seq, WORD quant) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	LONG z;
	LONG oz;
	LONG nstart;
	LONG d;
	
	evbl=seq->eventblock; evnum=0;
	do {
		z=evbl->event[evnum].zeit; oz=z;
		
		if ((evbl->event[evnum].status & MS_StatBits)==MS_NoteOn) {
			z=z>>(quant-1);
			if ((z & 0x00000001)==1) z++;
			z=z>>1;
			z=z<<quant;
			evbl->event[evnum].zeit=z;
		};
			
		evnum++;
		if (evnum==EVENTS) {
			evbl=evbl->next;
			evnum=0;
		};
	} while (seq->start+oz < seq->ende);

	nstart=(seq->start + seq->eventblock->event[0].zeit) & 0xFFFFFC00;
	d=seq->start-nstart;
	EventsVerschieben(seq, d);
	seq->start=nstart;
	
	OrdneEvents(seq);
	AliaseAnpassen(seq, -d);
}

void MarkSequenzenQuantisieren(WORD quant) {
	struct SEQUENZ *seq;
	WORD s;

	for (s=0; s<lied.spuranz; s++) {
		seq=spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (!seq->aliasorig) {
					QuantisiereSequenz(seq, quant);
				} else {
					QuantisiereSequenz(seq->aliasorig, quant);
				};
				sp[s].anders=TRUE;
			};
			seq=seq->next;
		};
	};
}

void PrintEvents(struct SEQUENZ *seq) {
	struct EVENTBLOCK *evbl;
	WORD evnum;
	
	if (seq) {
		evbl=seq->eventblock;
		evnum=0;
		
		do {
			printf("%lX| ", evbl->event[evnum].zeit);
			printf("%X: ", evbl->event[evnum].status);
			printf("%d, ", evbl->event[evnum].data1);
			printf("%d\n", evbl->event[evnum].data2);
			
			evnum++;
			if (evnum==EVENTS) {
				evbl=evbl->next;
				evnum=0;
			};
		} while (evbl && evbl->event[evnum].status);
		printf("\n\n\n");
	};
}
