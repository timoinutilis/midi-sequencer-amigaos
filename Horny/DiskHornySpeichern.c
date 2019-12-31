#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Requester.h"
#include "CtrlWandler.h"
#include "Midi.h"

#include "dateiformat.h"

extern struct LIED lied;
extern WORD snum;
extern struct GUI gui;
extern struct EDGUI edgui;
extern struct LOOP loop;
extern struct METRONOM metro;
extern struct SMPTE smpte;
extern struct MARKER *rootmark;
extern struct OUTPORT outport[];
extern struct INPORT inport[];
extern struct SYSEXUNIT *rootsexunit;
extern struct SPUR spur[];
extern struct MPDATA mpdata;
extern struct MPKANAL mpkanal[OUTPORTS][16];
extern struct MPKANALNUM mpkanalnum[];
extern struct AUTOKANAL autokanal[OUTPORTS][16];
extern struct CTRLCHANGE *rootctrlchange;


#define AREAPUFFER 300000


UWORD *AddTagWord(UWORD *adr, UWORD tag, WORD wert) {
	*adr++ = tag;
	*adr++ = wert;
	return(adr);
}

UWORD *AddTagLong(UWORD *adr, UWORD tag, LONG wert) {
	LONG *l;
	
	*adr++ = tag;
	l = (LONG *)adr;
	*l++ = wert;
	return((UWORD *)l);
}

UWORD *AddTagString(UWORD *adr, UWORD tag, STRPTR str) {
	char *s;
	WORD len, n;
	
	len = strlen(str) + 1;
	*adr++ = tag;
	*adr++ = len;
	s = (char *)adr;
	for (n = 0; n < len; n++) {
		*s = str[n]; s++;
	}
	return((UWORD *)s);
}

UWORD *AddTagData(UWORD *adr, UWORD tag, ULONG len, BYTE *data) {
	ULONG *l;
	BYTE *b;
	ULONG n;
	
	*adr++ = tag;
	l = (ULONG *)adr;
	*l++ = len;
	b = (BYTE *)l;
	for (n = 0; n < len; n++) {
		*b = data[n]; b++;
	}
	return((UWORD *)b);
}

void SchreibeArea(BPTR file, ULONG id, APTR area, UWORD *adr) {
	struct AREA head;
	
	head.id = id;
	head.len = (LONG)adr - (LONG)area;
	Write(file, &head, sizeof(struct AREA));
	Write(file, area, head.len);
}

UWORD *AddSequenzEventsTag(struct SEQUENZ *seq, UWORD *adr, APTR *area) {
	struct EVENTBLOCK *evbl;
	UWORD evnum;
	struct EVENT *ev;
	struct FEVENT *fevent;
	ULONG *l;
	
	*adr = TAG_MIDITRACK_SEQ_EVENTS; adr++;
	l = (ULONG *)adr;
	adr++; adr++; // Platz halten für Längenangabe
	
	fevent = (struct FEVENT *)adr;
	evbl = seq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;
		
		fevent->zeit = ev->zeit;
		fevent->status = ev->status;
		fevent->data1 = ev->data1;
		fevent->data2 = ev->data2;
		
		fevent++;
		if ((ULONG)fevent + sizeof(struct FEVENT) > (ULONG)area + AREAPUFFER) {
			Meldung(CAT(MSG_0592, "Buffer too small"));
			break;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	*l = (ULONG)fevent - (ULONG)l - 4; // Tag Länge
	return((UWORD *)fevent);
}


void SpeichernHorny(STRPTR datei) {
	BPTR file;
	struct HEAD head = {HORNYID, VERSION};
	APTR area;
	UWORD *adr;
	WORD n, nn;
	BYTE p, c;
	char meldung[120];
	
	struct MARKER *marker;
	struct SYSEXUNIT *unit;
	struct SYSEXMSG *msg;
	struct CTRLCHANGE *cc;
	struct AUTOPUNKT *punkt;
	struct SEQUENZ *seq;

	area = AllocVec(AREAPUFFER, 0);
	if (area) {
		file = Open(datei, MODE_NEWFILE);
		if (file) {
			Write(file, &head.hornyid, sizeof(head.hornyid));
			Write(file, &head.version, sizeof(head.version));


			// FAREA_INFO
			adr = (UWORD *)area;
			adr = AddTagString(adr, TAG_INFO_NAME, lied.name);
			adr = AddTagWord(adr, TAG_INFO_SPURANZ, lied.spuranz);
			adr = AddTagWord(adr, TAG_INFO_TAKTANZ, lied.taktanz);
			adr = AddTagWord(adr, TAG_INFO_AKTSPUR, snum);
			SchreibeArea(file, FAREA_INFO, area, adr);
			
			// FAREA_MAINGUI
			adr = (UWORD *)area;
			adr = AddTagWord(adr, TAG_MAINGUI_SPUR, gui.spur);
			adr = AddTagWord(adr, TAG_MAINGUI_SPURH, gui.sph);
			adr = AddTagLong(adr, TAG_MAINGUI_TAKT, gui.takt);
			adr = AddTagWord(adr, TAG_MAINGUI_TAKTB, gui.tab);
			adr = AddTagWord(adr, TAG_MAINGUI_SPALTEX, gui.spalte);
			adr = AddTagWord(adr, TAG_MAINGUI_FOLGEN, gui.folgen);
			SchreibeArea(file, FAREA_MAINGUI, area, adr);
			
			// FAREA_EDGUI
			adr = (UWORD *)area;
			adr = AddTagWord(adr, TAG_EDGUI_MODUS, edgui.modus);
			adr = AddTagWord(adr, TAG_EDGUI_TASTE, edgui.taste);
			adr = AddTagWord(adr, TAG_EDGUI_TASTEH, edgui.tasth);
			adr = AddTagWord(adr, TAG_EDGUI_CONTR, edgui.contr);
			adr = AddTagWord(adr, TAG_EDGUI_CONTRH, edgui.contrh);
			adr = AddTagLong(adr, TAG_EDGUI_TAKT, edgui.takt);
			adr = AddTagWord(adr, TAG_EDGUI_TAKTB, edgui.taktb);
			adr = AddTagWord(adr, TAG_EDGUI_RASTER, edgui.raster);
			adr = AddTagWord(adr, TAG_EDGUI_NEULEN, edgui.neulen);
			SchreibeArea(file, FAREA_EDGUI, area, adr);
			
			// FAREA_LOOP
			adr = (UWORD *)area;
			adr = AddTagLong(adr, TAG_LOOP_START, loop.start);
			adr = AddTagLong(adr, TAG_LOOP_ENDE, loop.ende);
			adr = AddTagWord(adr, TAG_LOOP_AKTIV, loop.aktiv);
			SchreibeArea(file, FAREA_LOOP, area, adr);
			
			// FAREA_METRONOM
			adr = (UWORD *)area;
			adr = AddTagWord(adr, TAG_METRONOM_PORT, metro.port);
			adr = AddTagWord(adr, TAG_METRONOM_CHANNEL, metro.channel);
			adr = AddTagWord(adr, TAG_METRONOM_TASTE1, metro.taste1);
			adr = AddTagWord(adr, TAG_METRONOM_TASTE2, metro.taste2);
			adr = AddTagWord(adr, TAG_METRONOM_VELO1, metro.velo1);
			adr = AddTagWord(adr, TAG_METRONOM_VELO2, metro.velo2);
			adr = AddTagWord(adr, TAG_METRONOM_RASTER, metro.raster);
			adr = AddTagWord(adr, TAG_METRONOM_REC, metro.rec);
			adr = AddTagWord(adr, TAG_METRONOM_PLAY, metro.play);
			SchreibeArea(file, FAREA_METRONOM, area, adr);
			
			// FAREA_SMPTE
			adr = (UWORD *)area;
			adr = AddTagLong(adr, TAG_SMPTE_STARTTICKS, smpte.startticks);
			adr = AddTagWord(adr, TAG_SMPTE_FORMAT, smpte.format);
			adr = AddTagWord(adr, TAG_SMPTE_SYNC, IstExtreamSyncAktiv());
			SchreibeArea(file, FAREA_SMPTE, area, adr);

			// FAREA_MARKER
			adr = (UWORD *)area;
			marker = rootmark;
			while (marker) {
				adr = AddTagWord(adr, TAG_MARKER_NEUTYP, marker->typ);
				adr = AddTagLong(adr, TAG_MARKER_TAKT, marker->takt);
				adr = AddTagWord(adr, TAG_MARKER_DATA1, marker->d1);
				adr = AddTagWord(adr, TAG_MARKER_DATA2, marker->d2);
				if (marker->typ == M_TEXT) adr = AddTagString(adr, TAG_MARKER_TEXT, &marker->text);
				marker = marker->next;
			}
			SchreibeArea(file, FAREA_MARKER, area, adr);
			
			// FAREA_PHONOLITH
			#ifdef __amigaos4__
			adr = (UWORD *)area;
			adr = AddTagString(adr, TAG_PHONOLITH_PROJEKT, lied.phonolithprojekt);
			SchreibeArea(file, FAREA_PHONOLITH, area, adr);
			#endif

			// FAREA_OUTPORTS
			adr = (UWORD *)area;
			for (n = 0; n < verOUTPORTS; n++) {
				if (!outport[n].name[0]) break;
				adr = AddTagString(adr, TAG_OUTPORTS_NEUNAME, outport[n].name);
				adr = AddTagWord(adr, TAG_OUTPORTS_THRU, outport[n].thru);
				adr = AddTagWord(adr, TAG_OUTPORTS_LATENZ, outport[n].latenz);
				adr = AddTagString(adr, TAG_OUTPORTS_INSTR1_NAME, outport[n].outinstr[0].name);
				adr = AddTagWord(adr, TAG_OUTPORTS_INSTR1_UNTEN, outport[n].outinstr[0].unten);
				adr = AddTagString(adr, TAG_OUTPORTS_INSTR2_NAME, outport[n].outinstr[1].name);
				adr = AddTagWord(adr, TAG_OUTPORTS_INSTR2_UNTEN, outport[n].outinstr[1].unten);
				adr = AddTagString(adr, TAG_OUTPORTS_INSTR3_NAME, outport[n].outinstr[2].name);
				adr = AddTagWord(adr, TAG_OUTPORTS_INSTR3_UNTEN, outport[n].outinstr[2].unten);
				adr = AddTagString(adr, TAG_OUTPORTS_INSTR4_NAME, outport[n].outinstr[3].name);
				adr = AddTagWord(adr, TAG_OUTPORTS_INSTR4_UNTEN, outport[n].outinstr[3].unten);
			}
			SchreibeArea(file, FAREA_OUTPORTS, area, adr);
			
			// FAREA_INPORTS
			adr = (UWORD *)area;
			for (n = 0; n < verINPORTS; n++) {
				if (!inport[n].name[0]) break;
				adr = AddTagString(adr, TAG_INPORTS_NEUNAME, inport[n].name);
			}
			SchreibeArea(file, FAREA_INPORTS, area, adr);
			
			// FAREA_SYSEX
			adr = (UWORD *)area;
			unit = rootsexunit;
			while (unit) {
				adr = AddTagString(adr, TAG_SYSEX_NEUNAME, unit->name);
				adr = AddTagWord(adr, TAG_SYSEX_PORT, unit->port);
				adr = AddTagWord(adr, TAG_SYSEX_GESPERRT, unit->gesperrt);
				msg = unit->sysex;
				while (msg) {
					adr = AddTagString(adr, TAG_SYSEX_MSG_NEUNAME, msg->name);
					adr = AddTagData(adr, TAG_SYSEX_MSG_DATA, msg->len, msg->data);
					msg = msg->next;
				}
				unit = unit->next;
			}
			SchreibeArea(file, FAREA_SYSEX, area, adr);
			
			// FAREA_MIXER
			adr = (UWORD *)area;
			for (n = 0; n < mpdata.kanalanz; n++) {
				p = mpkanalnum[n].port;
				c = mpkanalnum[n].channel;
				adr = AddTagWord(adr, TAG_MIXER_NEUPORT, p);
				adr = AddTagWord(adr, TAG_MIXER_CHANNEL, c);
				adr = AddTagWord(adr, TAG_MIXER_PAN, mpkanal[p][c].pan);
				adr = AddTagWord(adr, TAG_MIXER_FADER, mpkanal[p][c].fader);
				adr = AddTagWord(adr, TAG_MIXER_MUTE, mpkanal[p][c].mute);
				for (nn = 0; nn < 6; nn++) {
					if (mpkanal[p][c].contr[nn] != -1) {
						adr = AddTagWord(adr, TAG_MIXER_NEUPOTI, nn);
						adr = AddTagWord(adr, TAG_MIXER_CONTR, mpkanal[p][c].contr[nn]);
						adr = AddTagWord(adr, TAG_MIXER_CONTRWERT, mpkanal[p][c].contrwert[nn]);
					}
				}
			}
			SchreibeArea(file, FAREA_MIXER, area, adr);

			// FAREA_CTRLCHANGE
			adr = (UWORD *)area;
			cc = rootctrlchange;
			while (cc) {
				adr = AddTagString(adr, TAG_CTRLCHANGE_NEUNAME, cc->name);
				adr = AddTagWord(adr, TAG_CTRLCHANGE_ORIGINAL, cc->original);
				adr = AddTagWord(adr, TAG_CTRLCHANGE_ZIEL, cc->ziel);
				adr = AddTagWord(adr, TAG_CTRLCHANGE_AKTIV, cc->aktiv);
				cc = cc->next;
			}
			SchreibeArea(file, FAREA_CTRLCHANGE, area, adr);
						
			// FAREA_AUTOMATION
			adr = (UWORD *)area;
			for (p = 0; p < verOUTPORTS; p++) {
				for (c = 0; c < 16; c++) {
					for (n = 0; n < 8; n++) {
						if (autokanal[p][c].liste[n]) {
							adr = AddTagWord(adr, TAG_AUTOMATION_NEUNUM, n);
							adr = AddTagWord(adr, TAG_AUTOMATION_PORT, p);
							adr = AddTagWord(adr, TAG_AUTOMATION_CHANNEL, c);
							punkt = autokanal[p][c].liste[n];
							while (punkt) {
								adr = AddTagLong(adr, TAG_AUTOMATION_NEUPUNKT_TAKT, punkt->takt);
								adr = AddTagWord(adr, TAG_AUTOMATION_PUNKT_WERT, punkt->wert);
								punkt = punkt->next;
							}
						}
					}
				}
			}
			SchreibeArea(file, FAREA_AUTOMATION, area, adr);
			
			for (n = 0; n < lied.spuranz; n++) {
				// FAREA_MIDITRACK
				adr = (UWORD *)area;
				adr = AddTagWord(adr, TAG_MIDITRACK_SPUR, n);
				adr = AddTagString(adr, TAG_MIDITRACK_NAME, spur[n].name);
				adr = AddTagWord(adr, TAG_MIDITRACK_PORT, spur[n].port);
				adr = AddTagWord(adr, TAG_MIDITRACK_CHANNEL, spur[n].channel);
				adr = AddTagWord(adr, TAG_MIDITRACK_BANK0, spur[n].bank0);
				adr = AddTagWord(adr, TAG_MIDITRACK_BANK32, spur[n].bank32);
				adr = AddTagWord(adr, TAG_MIDITRACK_PROG, spur[n].prog);
				adr = AddTagWord(adr, TAG_MIDITRACK_SHIFT, spur[n].shift);
				adr = AddTagWord(adr, TAG_MIDITRACK_MUTE, spur[n].mute);
				adr = AddTagWord(adr, TAG_MIDITRACK_AUTOSTATUS, spur[n].autostatus);
				seq = spur[n].seq;
				while (seq) {
					adr = AddTagLong(adr, TAG_MIDITRACK_SEQ_NEUSTART, seq->start);
					adr = AddTagLong(adr, TAG_MIDITRACK_SEQ_ENDE, seq->ende);
					adr = AddTagWord(adr, TAG_MIDITRACK_SEQ_TRANS, seq->trans);
					adr = AddTagWord(adr, TAG_MIDITRACK_SEQ_MARKIERT, seq->markiert);
					if (seq->aliasorig) { // Alias
						adr = AddTagLong(adr, TAG_MIDITRACK_SEQ_ALIASORIG, (ULONG)seq->aliasorig);
					} else { // Original
						adr = AddTagString(adr, TAG_MIDITRACK_SEQ_NAME, seq->name);
						adr = AddTagLong(adr, TAG_MIDITRACK_SEQ_SPEICHERADR, (ULONG)seq);
						adr = AddTagWord(adr, TAG_MIDITRACK_SEQ_ALIASANZ, seq->aliasanz);
						adr = AddSequenzEventsTag(seq, adr, area);
					}
					seq = seq->next;
				}
				SchreibeArea(file, FAREA_MIDITRACK, area, adr);
			}

			Close(file);

		} else {
			Fault(IoErr(), CAT(MSG_0593, "Could not save project"), meldung, 120);
			Meldung(meldung);
		}

		FreeVec(area);
	} else Meldung(CAT(MSG_0594, "Not enough memory for area buffer\n<DiskHornySpeichern.c>"));
}
