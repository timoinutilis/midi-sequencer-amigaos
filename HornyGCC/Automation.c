#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>

#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Midi.h"
#include "Sequenzen.h"
#include "Requester.h"

extern struct LIED lied;
extern struct SPUR spur[];
extern struct SPURTEMP sp[];

struct AUTOKANAL autokanal[OUTPORTS][16];
extern struct MPKANAL mpkanal[OUTPORTS][16];
struct AUTOPUNKT *autocopy = NULL;

void InitAutokanaele(void) {
	int8 p, c;
	int8 n;
	
	for (p = 0; p < verOUTPORTS; p++) {
		for (c = 0; c < 16; c++) {
			for (n = 0; n < 8; n++) {
				autokanal[p][c].liste[n] = NULL;
				autokanal[p][c].aktpunkt[n] = NULL;
				autokanal[p][c].looppunkt[n] = NULL;
			}
		}
	}
}

void KanalSpurenBearbeitet(int8 p, int8 c) {
	int16 s;
	
	for (s = 0; s < lied.spuranz; s++) {
		if ((spur[s].port == p) && (spur[s].channel == c)) {
			if (spur[s].autostatus) sp[s].anders = 2; // Ganze Spur
			else sp[s].anders = 3; // Nur Spalte
		}
	}
}

void EntferneAutoPunkte(struct AUTOPUNKT *akt) {
	struct AUTOPUNKT *next;
	
	while (akt) {
		next = akt->next;
		IExec->FreeVec(akt);
		akt = next;
	}
}

struct AUTOPUNKT *AutomationDuplizieren(struct AUTOPUNKT *quelle) {
	struct AUTOPUNKT *anf = NULL;
	struct AUTOPUNKT *neu;
	struct AUTOPUNKT *last = NULL;	
	
	while (quelle) {
		neu = (struct AUTOPUNKT *)IExec->AllocVecTags(sizeof(struct AUTOPUNKT), TAG_END);
		if (neu) {
			neu->takt = quelle->takt;
			neu->wert = quelle->wert;
			neu->prev = last;
			neu->next = NULL;
			if (last) last->next = neu;
			else anf = neu;
		}
		last = neu;
		quelle = quelle->next;
	}
	return(anf);
}

void AutomationKopieren(int8 p, int8 c, int8 num) {
	if (autocopy) EntferneAutoPunkte(autocopy);
	autocopy = AutomationDuplizieren(autokanal[p][c].liste[num]);
}

void AutomationEinfuegen(int8 p, int8 c, int8 num) {
	if (autocopy) {
		if (autokanal[p][c].liste[num]) EntferneAutoPunkte(autokanal[p][c].liste[num]);
		autokanal[p][c].liste[num] = AutomationDuplizieren(autocopy);
	}
}

void EntferneAutomationsKopie(void) {
	if (autocopy) {
		EntferneAutoPunkte(autocopy);
		autocopy = NULL;
	}
}

struct AUTOPUNKT *NeuerAutoPunkt(int8 p, int8 c, int8 num, int32 t, int8 wert) {
	struct AUTOPUNKT *neu;
	struct AUTOPUNKT *akt;

	neu = (struct AUTOPUNKT *)IExec->AllocVecTags(sizeof(struct AUTOPUNKT), TAG_END);
	if (neu) {
		neu->takt = t;
		neu->wert = wert;
		
		if (autokanal[p][c].liste[num]) {
			akt = autokanal[p][c].liste[num];
			if (akt->takt > t) {
				neu->next = akt;
				neu->prev = NULL;
				akt->prev = neu;
				autokanal[p][c].liste[num] = neu;
			} else {
				while (akt->next) {
					if (akt->next->takt > t) break;
					akt = akt->next;
				}
				neu->prev = akt;
				neu->next = akt->next;
				akt->next = neu;
				if (neu->next) neu->next->prev = neu;
			}
		} else {
			autokanal[p][c].liste[num] = neu;
			neu->prev = NULL;
			neu->next = NULL;
		}
	}
	
	return(neu);
}

void EntferneAutoPunkt(int8 p, int8 c, int8 num, struct AUTOPUNKT *punkt) {
	if (punkt != autokanal[p][c].liste[num]) {
		punkt->prev->next = punkt->next;
		if (punkt->next) punkt->next->prev = punkt->prev;
	} else {
		if (punkt->next) punkt->next->prev = NULL;
		autokanal[p][c].liste[num] = punkt->next;
	}
	IExec->FreeVec(punkt);
}

void EntferneAlleAutoPunkte(int8 p, int8 c, int8 num) {
	EntferneAutoPunkte(autokanal[p][c].liste[num]);
	autokanal[p][c].liste[num] = NULL;
}

struct AUTOPUNKT *TaktAutoPunkt(int8 p, int8 c, int8 num, int32 t) {
	struct AUTOPUNKT *akt;
	
	akt = autokanal[p][c].liste[num];
	while (akt) {
		if ((akt->takt & VIERTELMASKE) == t) return(akt);
		akt = akt->next;
	}
	return(NULL);
}

void AutoAnpassen(int8 p, int8 c, int8 num, int8 wert) {
	if (num == 0) {
		if (wert != mpkanal[p][c].fader) {
			SendeKanalEvent(p, c, MS_Ctrl, MC_Volume, wert);
			mpkanal[p][c].fader = wert;
			mpkanal[p][c].autoupdate = TRUE;
			mpkanal[p][c].updateflags |= 0x01;
		}
	}
	if (num == 1) {
		if (wert != mpkanal[p][c].pan) {
			SendeKanalEvent(p, c, MS_Ctrl, MC_Pan, wert);
			mpkanal[p][c].pan = wert;
			mpkanal[p][c].autoupdate = TRUE;
			mpkanal[p][c].updateflags |= 0x02;
		}
	}
	if (num >= 2) {
		if (wert != mpkanal[p][c].contrwert[num - 2]) {
			SendeKanalEvent(p, c, MS_Ctrl, mpkanal[p][c].contr[num - 2], wert);
			mpkanal[p][c].contrwert[num - 2] = wert;
			mpkanal[p][c].autoupdate = TRUE;
			mpkanal[p][c].updateflags |= (0x04 << (num - 2));
		}
	}
}

void AutomationVorbereiten(int8 p, int8 c, int32 t) {
	int8 num;
	struct AUTOPUNKT *akt;

	t = vorgeschobenerPortTakt(p, t);
	
	for (num = 0; num < 8; num++) {
		autokanal[p][c].aktpunkt[num] = NULL;
		akt = autokanal[p][c].liste[num];
		while (akt) {
			if (akt->takt <= t) autokanal[p][c].aktpunkt[num] = akt;
			else break;
			akt = akt->next;
		}
	}
}

void LoopAutomationVorbereiten(int8 p, int8 c, int32 t) {
	int8 num;
	struct AUTOPUNKT *akt;
	
	t = vorgeschobenerPortTakt(p, t);

	for (num = 0; num < 8; num++) {
		autokanal[p][c].looppunkt[num] = NULL;
		akt = autokanal[p][c].liste[num];
		while (akt) {
			if (akt->takt <= t) autokanal[p][c].looppunkt[num] = akt;
			else break;
			akt = akt->next;
		}
	}
}

void LoopAutomationResetten(int8 p, int8 c) {
	int8 num;
	
	for (num = 0; num < 8; num++) {
		autokanal[p][c].aktpunkt[num] = autokanal[p][c].looppunkt[num];
	}
}

void SpieleAutomation(int8 p, int8 c, int32 t) {
	int8 num;
	struct AUTOPUNKT *akt;
	int32 wert;
	
	t = vorgeschobenerPortTakt(p, t);

	for (num = 0; num < 8; num++) {
		if (autokanal[p][c].liste[num]) { // Automation vorhanden?
			akt = autokanal[p][c].aktpunkt[num];
			if (!akt) { // Vor dem ersten
				if (autokanal[p][c].liste[num]->takt <= t) akt = autokanal[p][c].liste[num];
			}
			if (akt) { // Auto zu senden
				while (akt->next) {
					if (akt->next->takt <= t) akt = akt->next;
					else break;
				}

				if (akt->next) {
					if (akt->wert != akt->next->wert) {
						wert = ((int32)akt->next->wert * (t - akt->takt)) + ((int32)akt->wert * (akt->next->takt - t));
						wert = wert / (akt->next->takt - akt->takt);
					} else wert = akt->wert;
					AutoAnpassen(p, c, num, (int8)wert);

				} else {
					AutoAnpassen(p, c, num, akt->wert);
				}
			}
			autokanal[p][c].aktpunkt[num] = akt;
		}
	}
}

void LoescheAutoBereich(int8 p, int8 c, int8 num, int32 von, int32 bis) {
	struct AUTOPUNKT *punkt;
	struct AUTOPUNKT *next;
	
	punkt = autokanal[p][c].liste[num];
	while (punkt) {
		if (punkt->takt > bis) break;
		
		next = punkt->next;
		
		if ((punkt->takt >= von) && (punkt->takt <= bis)) {
			if (punkt != autokanal[p][c].liste[num]) {
				punkt->prev->next = punkt->next;
				if (punkt->next) punkt->next->prev = punkt->prev;
			} else {
				if (punkt->next) punkt->next->prev = NULL;
				autokanal[p][c].liste[num] = punkt->next;
			}
			IExec->FreeVec(punkt);
		}
		
		punkt = next;
	}
}

void KonvertiereContrZuAuto(int16 s) {
	int8 p, c;
	int8 num;
	int8 data1;
	struct SEQUENZ *seq;
	struct EVENTBLOCK *evbl;
	int16 evnum;
	struct EVENT *ev;
	int8 data2 = 0;
	int8 altdata2 = -1;
	int32 zeit;
	int32 altzeit = -1;
	struct AUTOPUNKT *lastpunkt;
	int16 delta1, delta2;
	int32 deltat1, deltat2;
	BOOL neuerpunkt;

	if (spur[s].autostatus) {
		p = spur[s].port;
		c = spur[s].channel;
		num = spur[s].autostatus - 1;
		lastpunkt = NULL;
		
		// Controller suchen...
		if (num == 0) data1 = MC_Volume;
		else if (num == 1) data1 = MC_Pan;
		else data1 = mpkanal[p][c].contr[num - 2];
		
		// Markierte Sequenzen abarbeiten...
		seq = spur[s].seq;
		while (seq) {
			if (seq->markiert) {

				LoescheAutoBereich(p, c, num, seq->start, seq->ende);
				
				evbl = seq->eventblock; evnum = 0;
				while (evbl) {
					ev = &evbl->event[evnum];
					if (!ev->status) break;
					
					if (((ev->status & MS_StatBits) == MS_Ctrl) && (ev->data1 == data1)) data2 = ev->data2;
					
					zeit = (seq->start + ev->zeit) & VIERTELMASKE;
					if (zeit  != altzeit) {
						if (data2 != altdata2) {
							neuerpunkt = TRUE;
							if (lastpunkt) {
								if (lastpunkt->prev) {
									deltat1 = (lastpunkt->takt - lastpunkt->prev->takt) >> VIERTEL;
									deltat2 = (zeit - lastpunkt->takt) >> VIERTEL;
									if ((deltat1 != 0) && (deltat2 != 0)) {
										delta1 = (lastpunkt->wert - lastpunkt->prev->wert) / deltat1;
										delta2 = (data2 - lastpunkt->wert) / deltat2;
									
										if (abs(delta1 - delta2) <= 1) {
											lastpunkt->wert = data2;
											lastpunkt->takt = zeit;
											neuerpunkt = FALSE;
										}
									}
								}
							}
								
							if (neuerpunkt) {
								if (altzeit > -1) {
									deltat1 = (zeit - lastpunkt->takt) >> VIERTEL;
									if (deltat1 > 2) NeuerAutoPunkt(p, c, num, zeit, altdata2);
								}
								lastpunkt = NeuerAutoPunkt(p, c, num, zeit, data2);
							}
							altdata2 = data2;
						}
						altzeit = zeit;
					}
					
					evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
				}

			}
			
			seq = seq->next;
		}
		if (altdata2 == -1) Meldung(CAT(MSG_0000, "There were no fitting controller events in marked sequences"));
	} else Meldung(CAT(MSG_0001, "The automation you want to convert to must be selected"));
}

void KonvertiereAutoZuContr(int16 s) {
	int8 p, c;
	int8 num;
	int8 data1;
	struct AUTOPUNKT *akt;
	int32 wert;
	int32 altwert = -1;
	int32 zeit;
	
	if (spur[s].autostatus) {
		p = spur[s].port;
		c = spur[s].channel;
		num = spur[s].autostatus - 1;

		// Controller suchen...
		if (num == 0) data1 = MC_Volume;
		else if (num == 1) data1 = MC_Pan;
		else data1 = mpkanal[p][c].contr[num - 2];
		
		akt = autokanal[p][c].liste[num];
		if (akt) { // Automation vorhanden?
			zeit = akt->takt & VIERTELMASKE;

			do {
				while (akt->next) {
					if (akt->next->takt <= zeit) akt = akt->next;
					else break;
				}
	
				if (akt->next) {
					if (akt->wert != akt->next->wert) {
						wert = ((int32)akt->next->wert * (zeit - akt->takt)) + ((int32)akt->wert * (akt->next->takt - zeit));
						wert = wert / (akt->next->takt - akt->takt);
					} else wert = akt->wert;
					
					if (wert != altwert) {
						AddEvent(s, zeit, MS_Ctrl, data1, (int8)wert);
						altwert = wert;
					}
				} else break;
				zeit += VIERTELWERT >> 3;
			} while (1);
			
			NeueSequenzEinordnen(s);
		}
	}
}
