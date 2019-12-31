#include <stdio.h>

#include "Strukturen.h"
#include "Versionen.h"
#include "Sequenzen.h"

struct CLIPBOARD {
	LONG ersttakt;
	struct SEQUENZ *seq;
};

struct CLIPBOARD clip = {0, NULL};

extern struct SPURTEMP sp[];
extern struct LIED lied;
extern struct SPUR spur[];


void ClipboardLoeschen(void) {
	struct SEQUENZ *akt;
	struct SEQUENZ *next;
	
	akt = clip.seq;
	while (akt) {
		next = akt->next;
		SequenzEntfernen(akt);
		akt = next;
	}
	
	clip.seq = NULL;
}

void ClipboardAliaseRechnen(BYTE add) {
	struct SEQUENZ *akt;
	
	akt = clip.seq;
	while (akt) {
		if (akt->aliasorig) akt->aliasorig->aliasanz += add;
		akt = akt->next;
	}
}
		

void ClipboardAliasZuReal(void) {
	struct SEQUENZ *akt;
	struct SEQUENZ *alt;
	struct SEQUENZ *neu;
	struct SEQUENZ *next;
	
	akt = clip.seq; alt = NULL;
	while (akt) {
		if (akt->aliasorig) {
			next = akt->next;
			neu = NeueKopie(akt);
			if (alt) alt->next = neu;
			else clip.seq = neu;
			neu->next = next;
			SequenzEntfernen(akt);
			akt = neu;
		}
		alt = akt;
		akt = akt->next;
	}
}

void ClipboardKopieren(void) {
	WORD s;
	struct SEQUENZ *seq;
	struct SEQUENZ *seqkopie;
	
	ClipboardLoeschen();
	clip.ersttakt = 0x7FFFFFFF;
	
	for (s = 0; s < lied.spuranz; s++) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {
				if (clip.ersttakt > seq->start) clip.ersttakt = seq->start;
				
				seqkopie  = NeuesAlias(seq);
				if (seqkopie) { // Sequenz ans Clipboard hängen...
					seqkopie->next = clip.seq;
					clip.seq = seqkopie;
				}
				seq->markiert = FALSE;
				sp[seq->spur].anders = 1;
			}
			seq = seq->next;
		}
	}
}

LONG ClipboardEinfuegen(LONG t) {
	struct SEQUENZ *seq;
	struct SEQUENZ *seqkopie;
	LONG ende;
	WORD s;
	
	ende = t;
	seq = clip.seq;
	while (seq) {
		if (seq->aliasorig) seqkopie = NeuesAlias(seq);
		else seqkopie = NeueKopie(seq);
		
		if (seqkopie) {
			seqkopie->start = seqkopie->start - clip.ersttakt + t;
			seqkopie->ende = seqkopie->ende - clip.ersttakt + t;
			if (seqkopie->ende > ende) ende = seqkopie->ende;
			
			if (seqkopie->spur >= lied.spuranz) s = lied.spuranz - 1;
			else s = seqkopie->spur;
			
			sp[s].neuseq = seqkopie;
			NeueSequenzEinordnen(s);
		}
		seq = seq->next;
	}
	return(ende);
}
