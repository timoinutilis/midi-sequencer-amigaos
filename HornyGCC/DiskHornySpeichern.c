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
extern int16 snum;
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


uint16 *AddTagWord(uint16 *adr, uint16 tag, int16 wert) {
	*adr++ = tag;
	*adr++ = wert;
	return(adr);
}

uint16 *AddTagLong(uint16 *adr, uint16 tag, int32 wert) {
	int32 *l;
	
	*adr++ = tag;
	l = (int32 *)adr;
	*l++ = wert;
	return((uint16 *)l);
}

uint16 *AddTagString(uint16 *adr, uint16 tag, STRPTR str) {
	char *s;
	int16 len, n;
	
	len = strlen(str) + 1;
	*adr++ = tag;
	*adr++ = len;
	s = (char *)adr;
	for (n = 0; n < len; n++) {
		*s = str[n]; s++;
	}
	return((uint16 *)s);
}

uint16 *AddTagData(uint16 *adr, uint16 tag, uint32 len, int8 *data) {
	uint32 *l;
	int8 *b;
	uint32 n;
	
	*adr++ = tag;
	l = (uint32 *)adr;
	*l++ = len;
	b = (int8 *)l;
	for (n = 0; n < len; n++) {
		*b = data[n]; b++;
	}
	return((uint16 *)b);
}

void SchreibeArea(BPTR file, uint32 id, APTR area, uint16 *adr) {
	struct AREA head;
	
	head.id = id;
	head.len = (int32)adr - (int32)area;
	IDOS->Write(file, &head, sizeof(struct AREA));
	IDOS->Write(file, area, head.len);
}

uint16 *AddSequenzEventsTag(struct SEQUENZ *seq, uint16 *adr, APTR *area) {
	struct EVENTBLOCK *evbl;
	uint16 evnum;
	struct EVENT *ev;
	struct FEVENT *fevent;
	uint32 *l;
	
	*adr = TAG_MIDITRACK_SEQ_EVENTS; adr++;
	l = (uint32 *)adr;
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
		if ((uint32)fevent + sizeof(struct FEVENT) > (uint32)area + AREAPUFFER) {
			Meldung(CAT(MSG_0592, "Buffer too small"));
			break;
		}
		
		evnum++; if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
	}
	*l = (uint32)fevent - (uint32)l - 4; // Tag Länge
	return((uint16 *)fevent);
}


void SpeichernHorny(STRPTR datei) {
	BPTR file;
	struct HEAD head = {HORNYID, VERSION};
	APTR area;
	uint16 *adr;
	int16 n, nn;
	int8 p, c;
	char meldung[120];
	
	struct MARKER *marker;
	struct SYSEXUNIT *unit;
	struct SYSEXMSG *msg;
	struct CTRLCHANGE *cc;
	struct AUTOPUNKT *punkt;
	struct SEQUENZ *seq;

	area = IExec->AllocVecTags(AREAPUFFER, TAG_END);
	if (area) {
		file = IDOS->Open(datei, MODE_NEWFILE);
		if (file) {
			IDOS->Write(file, &head.hornyid, sizeof(head.hornyid));
			IDOS->Write(file, &head.version, sizeof(head.version));


			// FAREA_INFO
			adr = (uint16 *)area;
			adr = AddTagString(adr, TAG_INFO_NAME, lied.name);
			adr = AddTagWord(adr, TAG_INFO_SPURANZ, lied.spuranz);
			adr = AddTagWord(adr, TAG_INFO_TAKTANZ, lied.taktanz);
			adr = AddTagWord(adr, TAG_INFO_AKTSPUR, snum);
			SchreibeArea(file, FAREA_INFO, area, adr);
			
			// FAREA_MAINGUI
			adr = (uint16 *)area;
			adr = AddTagWord(adr, TAG_MAINGUI_SPUR, gui.spur);
			adr = AddTagWord(adr, TAG_MAINGUI_SPURH, gui.sph);
			adr = AddTagLong(adr, TAG_MAINGUI_TAKT, gui.takt);
			adr = AddTagWord(adr, TAG_MAINGUI_TAKTB, gui.tab);
			adr = AddTagWord(adr, TAG_MAINGUI_SPALTEX, gui.spalte);
			adr = AddTagWord(adr, TAG_MAINGUI_FOLGEN, gui.folgen);
			SchreibeArea(file, FAREA_MAINGUI, area, adr);
			
			// FAREA_EDGUI
			adr = (uint16 *)area;
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
			adr = (uint16 *)area;
			adr = AddTagLong(adr, TAG_LOOP_START, loop.start);
			adr = AddTagLong(adr, TAG_LOOP_ENDE, loop.ende);
			adr = AddTagWord(adr, TAG_LOOP_AKTIV, loop.aktiv);
			SchreibeArea(file, FAREA_LOOP, area, adr);
			
			// FAREA_METRONOM
			adr = (uint16 *)area;
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
			adr = (uint16 *)area;
			adr = AddTagLong(adr, TAG_SMPTE_STARTTICKS, smpte.startticks);
			adr = AddTagWord(adr, TAG_SMPTE_FORMAT, smpte.format);
			adr = AddTagWord(adr, TAG_SMPTE_SYNC, IstExtreamSyncAktiv());
			SchreibeArea(file, FAREA_SMPTE, area, adr);

			// FAREA_MARKER
			adr = (uint16 *)area;
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
			adr = (uint16 *)area;
			adr = AddTagString(adr, TAG_PHONOLITH_PROJEKT, lied.phonolithprojekt);
			SchreibeArea(file, FAREA_PHONOLITH, area, adr);

			// FAREA_OUTPORTS
			adr = (uint16 *)area;
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
			adr = (uint16 *)area;
			for (n = 0; n < verINPORTS; n++) {
				if (!inport[n].name[0]) break;
				adr = AddTagString(adr, TAG_INPORTS_NEUNAME, inport[n].name);
			}
			SchreibeArea(file, FAREA_INPORTS, area, adr);
			
			// FAREA_SYSEX
			adr = (uint16 *)area;
			unit = rootsexunit;
			while (unit) {
				adr = AddTagString(adr, TAG_SYSEX_NEUNAME, unit->name);
				adr = AddTagWord(adr, TAG_SYSEX_PORT, unit->port);
				adr = AddTagWord(adr, TAG_SYSEX_GESPERRT, unit->gesperrt);
				msg = unit->sysex;
				while (msg) {
					adr = AddTagString(adr, TAG_SYSEX_MSG_NEUNAME, msg->name);
					adr = AddTagData(adr, TAG_SYSEX_MSG_DATA, msg->len, (int8 *)msg->data);
					msg = msg->next;
				}
				unit = unit->next;
			}
			SchreibeArea(file, FAREA_SYSEX, area, adr);
			
			// FAREA_MIXER
			adr = (uint16 *)area;
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
			adr = (uint16 *)area;
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
			adr = (uint16 *)area;
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
				adr = (uint16 *)area;
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
						adr = AddTagLong(adr, TAG_MIDITRACK_SEQ_ALIASORIG, (uint32)seq->aliasorig);
					} else { // Original
						adr = AddTagString(adr, TAG_MIDITRACK_SEQ_NAME, seq->name);
						adr = AddTagLong(adr, TAG_MIDITRACK_SEQ_SPEICHERADR, (uint32)seq);
						adr = AddTagWord(adr, TAG_MIDITRACK_SEQ_ALIASANZ, seq->aliasanz);
						adr = AddSequenzEventsTag(seq, adr, area);
					}
					seq = seq->next;
				}
				SchreibeArea(file, FAREA_MIDITRACK, area, adr);
			}

			IDOS->Close(file);

		} else {
			IDOS->Fault(IDOS->IoErr(), CAT(MSG_0593, "Could not save project"), meldung, 120);
			Meldung(meldung);
		}

		IExec->FreeVec(area);
	} else Meldung(CAT(MSG_0594, "Not enough memory for area buffer\n<DiskHornySpeichern.c>"));
}
