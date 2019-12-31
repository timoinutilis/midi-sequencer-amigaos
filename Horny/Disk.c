#include <stdio.h>

#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "Strukturen.h"
#include "Midi.h"
#include "Sequenzen.h"
#include "Requester.h"

#define HORNYID 0x48524E59
#define VERSION 0x0000

struct HEAD {
	ULONG hornyid;
	UWORD version;
} head;

extern struct GUI gui;
extern struct LIED lied;
extern struct SPUR spur[128];

void Speichern(STRPTR name) {
	BPTR file;
	WORD s;
	struct SEQUENZ *seq;
	struct EVENTBLOCK *evbl;
	char meldung[120];
	
	if (file=Open(name, MODE_NEWFILE)) {
		head.hornyid=HORNYID;
		head.version=VERSION;
		Write(file, &head, sizeof(struct HEAD));
		
		Write(file, &lied, sizeof(struct LIED));
		
		Write(file, &gui, sizeof(struct GUI));
		
		for (s=0; s<lied.spuranz; s++) {
			spur[s].aktseq=NULL;
			spur[s].aktevbl=NULL;
			spur[s].aktevnum=0;
			Write(file, &spur[s], sizeof(struct SPUR));
			
			seq=spur[s].seq;
			while (seq) {
				seq->speicheradr=seq;
				Write(file, seq, sizeof(struct SEQUENZ));
				
				evbl=seq->eventblock;
				while (evbl) {
					Write(file, evbl, sizeof(struct EVENTBLOCK));
					evbl=evbl->next;
				};
				seq=seq->next;
			};
		};
		
		Close(file);
	} else {
		Fault(IoErr(), "Konnte Lied nicht speichern", meldung, 120);
		Meldung(meldung);
	};
}

void Laden(STRPTR name) {
	BPTR file;
	WORD s;
	WORD n;
	struct SEQUENZ *seq;
	struct SEQUENZ *altseq;
	struct EVENTBLOCK *evbl;
	struct EVENTBLOCK *altevbl;
	char meldung[120];
	
	if (file=Open(name, MODE_OLDFILE)) {
		Read(file, &head, sizeof(struct HEAD));
		if (head.hornyid==HORNYID) {
			if (head.version==VERSION) {
				for(s=0; s<128; s++) SpurSequenzenEntfernen(s);
				InitLied();
				
				Read(file, &lied, sizeof(struct LIED));
				
				Read(file, &gui, sizeof(struct GUI));
				
				for (s=0; s<lied.spuranz; s++) {
					Read(file, &spur[s], sizeof(struct SPUR));
					
					seq=spur[s].seq; altseq=NULL;
					while (seq) {
						seq=AllocVec(sizeof(struct SEQUENZ), MEMF_ANY);
						Read(file, seq, sizeof(struct SEQUENZ));
						
						if (altseq) altseq->next=seq else spur[s].seq=seq;
						
						evbl=seq->eventblock; altevbl=NULL;
						while (evbl) {
							evbl=AllocVec(sizeof(struct EVENTBLOCK), MEMF_ANY);
							Read(file, evbl, sizeof(struct EVENTBLOCK));
							
							if (altevbl) {
								altevbl->next=evbl;
								evbl->prev=altevbl;
							} else {
								seq->eventblock=evbl;
							};
							
							altevbl=evbl;
							evbl=evbl->next;
						};
						
						altseq=seq;
						seq=seq->next;
					};
				};
				
				
				for (s=0; s<lied.spuranz; s++) {
					seq=spur[s].seq;
					while (seq) {
						if (seq->aliasanz) {
						
							for (n=0; n<lied.spuranz; n++) {
								altseq=spur[n].seq;
								while (altseq) {
									if (altseq->aliasorig==seq->speicheradr) {
										altseq->aliasorig=seq;
										altseq->eventblock=seq->eventblock;
									};
									altseq=altseq->next;
								};
							};						
						
						};
						seq=seq->next;
					};
				};
				
			} else {
				Meldung("Datei ist von inkompatibler Horny-Version");
			};
		} else {
			Meldung("Keine Horny-Datei");
		};
		
		Close(file);
	} else {
		Fault(IoErr(), "Konnte Lied nicht laden", meldung, 120);
		Meldung(meldung);
	};
}
