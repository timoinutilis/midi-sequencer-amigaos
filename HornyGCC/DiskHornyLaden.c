#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Projekt.h"
#include "Requester.h"
#include "Marker.h"
#include "Sequenzen.h"
#include "Midi.h"
#include "SysEx.h"
#include "Automation.h"
#include "CtrlWandler.h"

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
extern struct SPURTEMP sp[];
extern struct MPKANAL mpkanal[OUTPORTS][16];
extern struct SYSEXUNIT *wahlsexunit;

void newPhonolithProject(STRPTR datei); //Start.c


uint16 *LeseTag(uint16 *adr, uint16 *tag, int32 *ergebnis, int32 *len) {
	uint8 typ;
	int32 *l;
	
	*tag = *adr++;
	typ = (uint8)(*tag & 0x000F);
	if (typ == 0) { // Word
		*ergebnis = (int32)*adr++;
		return(adr);
	}
	if (typ == 1) { // Long
		l = (int32 *)adr;
		*ergebnis = *l++;
		return((uint16 *)l);
	}
	if (typ == 2) { // String
		*len = *adr++;
		*ergebnis = (int32)adr; adr = (uint16 *)((uint32)adr + *len);
		return(adr);
	}
	if (typ == 3) { // Data
		l = (int32 *)adr;
		*len = *l++;
		*ergebnis = (int32)l; l = (int32 *)((uint32)l + *len);
		return((uint16 *)l);
	}
	return(NULL);
}

void LeseAreaINFO(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_INFO_NAME: strncpy(lied.name, (STRPTR)ergebnis, 128); break;
			case TAG_INFO_SPURANZ:
			lied.spuranz = (uint8)ergebnis;
			if (lied.spuranz > verSPUREN) lied.spuranz = verSPUREN;
			break;
			case TAG_INFO_TAKTANZ: lied.taktanz = (int16)ergebnis; break;
			case TAG_INFO_AKTSPUR: snum = (int16)ergebnis; break;
		}
	}
}

void LeseAreaMAINGUI(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_MAINGUI_SPUR: gui.spur = (int16)ergebnis; break;
			case TAG_MAINGUI_SPURH: gui.sph = (int16)ergebnis; break;
			case TAG_MAINGUI_TAKT: gui.takt = (int32)ergebnis; break;
			case TAG_MAINGUI_TAKTB: gui.tab = (int16)ergebnis; break;
			case TAG_MAINGUI_SPALTEX: gui.spalte = (int16)ergebnis; break;
			case TAG_MAINGUI_FOLGEN: gui.folgen = (BOOL)ergebnis; break;
		}
	}
}

void LeseAreaEDGUI(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_EDGUI_MODUS: edgui.modus = (int8)ergebnis; break;
			case TAG_EDGUI_TASTE: edgui.taste = (int16)ergebnis; break;
			case TAG_EDGUI_TASTEH: edgui.tasth = (int16)ergebnis; break;
			case TAG_EDGUI_CONTR: edgui.contr = (int16)ergebnis; break;
			case TAG_EDGUI_CONTRH: edgui.contrh = (int16)ergebnis; break;
			case TAG_EDGUI_TAKT: edgui.takt = (int32)ergebnis; break;
			case TAG_EDGUI_TAKTB: edgui.taktb = (int16)ergebnis; break;
			case TAG_EDGUI_RASTER: edgui.raster = (uint8)ergebnis; break;
			case TAG_EDGUI_NEULEN: edgui.neulen = (int8)ergebnis; break;
		}
	}
}

void LeseAreaLOOP(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_LOOP_START: loop.start = (int32)ergebnis; break;
			case TAG_LOOP_ENDE: loop.ende = (int32)ergebnis; break;
			case TAG_LOOP_AKTIV: loop.aktiv = (BOOL)ergebnis; break;
		}
	}
}

void LeseAreaMETRONOM(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_METRONOM_PORT: metro.port = (uint8)ergebnis; break;
			case TAG_METRONOM_CHANNEL: metro.channel = (uint8)ergebnis; break;
			case TAG_METRONOM_TASTE1: metro.taste1 = (int8)ergebnis; break;
			case TAG_METRONOM_TASTE2: metro.taste2 = (int8)ergebnis; break;
			case TAG_METRONOM_VELO1: metro.velo1 = (int8)ergebnis; break;
			case TAG_METRONOM_VELO2: metro.velo2 = (int8)ergebnis; break;
			case TAG_METRONOM_RASTER: metro.raster = (int16)ergebnis; break;
			case TAG_METRONOM_REC: metro.rec = (BOOL)ergebnis; break;
			case TAG_METRONOM_PLAY: metro.play = (BOOL)ergebnis; break;
		}
	}
}

void LeseAreaSMPTE(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_SMPTE_STARTTICKS: smpte.startticks = (int32)ergebnis; break;
			case TAG_SMPTE_FORMAT: smpte.format = (int8)ergebnis; break;
			case TAG_SMPTE_SYNC:
			if ((BOOL)ergebnis) {
				AktiviereExtreamSync();
			}
			break;
		}
	}
}

void LeseAreaMARKER(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	struct MARKER marker = {
		NULL, NULL, // prev, next
		0, // takt
		-1, // typ
		0, 0, // d1, d2
		0 // text
	};
	char text[128];
	struct MARKER *neu;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		
		switch (tag) {
			case TAG_MARKER_NEUTYP:
			if ((marker.typ >= 0) && (marker.takt >= 0)) {
				neu = NeuerMarker(marker.typ, marker.takt, marker.d1, marker.d2);
				if (neu) {
					if (marker.typ == M_TEXT) strncpy(&neu->text, text, 128);
				}
			}
			marker.typ = (int8)ergebnis;
			break;
			case TAG_MARKER_TAKT: marker.takt = (int32)ergebnis; break;
			case TAG_MARKER_DATA1: marker.d1 = (int16)ergebnis; break;
			case TAG_MARKER_DATA2: marker.d2 = (int16)ergebnis; break;
			case TAG_MARKER_TEXT: strncpy(text, (STRPTR)ergebnis, 128); break;
		}
	}
	if (marker.typ >= 0) {
		neu = NeuerMarker(marker.typ, marker.takt, marker.d1, marker.d2);
		if (neu) {
			if (marker.typ == M_TEXT) strncpy(&neu->text, text, 128);
		}
	}
}

void LeseAreaOUTPORTS(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	int8 n = -1;
	BOOL linktest = FALSE;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		if (tag == TAG_OUTPORTS_NEUNAME) {
			n++;
			if (n == verOUTPORTS) break;
			if (!linktest) {
				if (!LinkVorhanden((STRPTR)ergebnis)) {
					Meldung(CAT(MSG_0549, "This project was created in another midi configuration.\nPlease adjust port settings."));
					linktest = TRUE;
				}
			}
			strncpy(outport[n].name, (STRPTR)ergebnis, 128);
		}
		if (n >= 0) {
			switch (tag) {
				case TAG_OUTPORTS_THRU: outport[n].thru = (BOOL)ergebnis; break;
				case TAG_OUTPORTS_LATENZ: outport[n].latenz = (int16)ergebnis; break;
				case TAG_OUTPORTS_INSTR1_NAME: strncpy(outport[n].outinstr[0].name, (STRPTR)ergebnis, 128); break;
				case TAG_OUTPORTS_INSTR1_UNTEN: outport[n].outinstr[0].unten = (int8)ergebnis; break;
				case TAG_OUTPORTS_INSTR2_NAME: strncpy(outport[n].outinstr[1].name, (STRPTR)ergebnis, 128); break;
				case TAG_OUTPORTS_INSTR2_UNTEN: outport[n].outinstr[1].unten = (int8)ergebnis; break;
				case TAG_OUTPORTS_INSTR3_NAME: strncpy(outport[n].outinstr[2].name, (STRPTR)ergebnis, 128); break;
				case TAG_OUTPORTS_INSTR3_UNTEN: outport[n].outinstr[2].unten = (int8)ergebnis; break;
				case TAG_OUTPORTS_INSTR4_NAME: strncpy(outport[n].outinstr[3].name, (STRPTR)ergebnis, 128); break;
				case TAG_OUTPORTS_INSTR4_UNTEN: outport[n].outinstr[3].unten = (int8)ergebnis; break;
			}
		}
	}
}

void LeseAreaINPORTS(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	int8 n = -1;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		if (tag == TAG_INPORTS_NEUNAME) {
			n++;
			if (n == verINPORTS) break;
			strncpy(inport[n].name, (STRPTR)ergebnis, 128);
		}
	}
}

void LeseAreaPHONOLITH(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	STRPTR str = NULL;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_PHONOLITH_PROJEKT: str = (STRPTR)ergebnis; break;
		}
	}
	if (str) strncpy(lied.phonolithprojekt, str, 1024);

	if (lied.phonolithprojekt[0] != 0) {
		newPhonolithProject(lied.phonolithprojekt);
	}
}

void LeseAreaSYSEX(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	struct SYSEXUNIT *unit = NULL;
	struct SYSEXMSG *msg = NULL;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		
		if (tag == TAG_SYSEX_NEUNAME) wahlsexunit = unit = NeueSysExUnit((STRPTR)ergebnis);
		if (unit) {
			if (tag == TAG_SYSEX_PORT) unit->port = (uint8)ergebnis;
			if (tag == TAG_SYSEX_GESPERRT) unit->gesperrt = (BOOL)ergebnis;
			if (tag == TAG_SYSEX_MSG_NEUNAME) msg = NeuesSysEx(unit, (STRPTR)ergebnis, 0, NULL);
			if (msg) {
				if (tag == TAG_SYSEX_MSG_DATA) {
					if (!msg->data) {
						msg->data = (uint8 *)IExec->AllocVecTags(taglen, TAG_END);
						if (msg->data) memcpy(msg->data, (APTR)ergebnis, taglen);
						else Meldung(CAT(MSG_0550, "Not enough memory for SysEx data\n<DiskHornyLaden.c>"));
						msg->len = taglen;
					}
				}
			}
		}
	}
}

void LeseAreaMIXER(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	int8 p = 0, c = 0;
	int8 poti = 0;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_MIXER_NEUPORT: p = (int8)ergebnis; break;
			case TAG_MIXER_CHANNEL: c = (int8)ergebnis; break;
		}
		if (p < verOUTPORTS) {
			switch (tag) {
				case TAG_MIXER_PAN: mpkanal[p][c].pan = (int8)ergebnis; break;
				case TAG_MIXER_FADER: mpkanal[p][c].fader = (int8)ergebnis; break;
				case TAG_MIXER_MUTE: mpkanal[p][c].mute = (BOOL)ergebnis; break;
				case TAG_MIXER_NEUPOTI: poti = (int8)ergebnis; break;
				case TAG_MIXER_CONTR: mpkanal[p][c].contr[poti] = (int8)ergebnis; break;
				case TAG_MIXER_CONTRWERT: mpkanal[p][c].contrwert[poti] = (int8)ergebnis; break;
			}		
		}
	}
}

void LeseAreaCTRLCHANGE(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	struct CTRLCHANGE *cc = NULL;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		
		if (tag == TAG_CTRLCHANGE_NEUNAME)
			cc = AddChangeCtrl((STRPTR)ergebnis);
			
		if (cc) {
			switch (tag) {
				case TAG_CTRLCHANGE_ORIGINAL: cc->original = (int8)ergebnis; break;
				case TAG_CTRLCHANGE_ZIEL: cc->ziel = (int8)ergebnis; break;
				case TAG_CTRLCHANGE_AKTIV: cc->aktiv = (BOOL)ergebnis; break;
			}
		}
	}
}

void LeseAreaAUTOMATION(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	int8 p = 0, c = 0;
	int8 num = -1;
	struct AUTOPUNKT *punkt = NULL;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		switch (tag) {
			case TAG_AUTOMATION_NEUNUM: num = (int8)ergebnis; break;
			case TAG_AUTOMATION_PORT: p = (int8)ergebnis; break;
			case TAG_AUTOMATION_CHANNEL: c = (int8)ergebnis; break;
			case TAG_AUTOMATION_NEUPUNKT_TAKT:
			punkt = NeuerAutoPunkt(p, c, num, (int32)ergebnis, 0);
			break;
		}
		if (punkt) {
			switch (tag) {
				case TAG_AUTOMATION_PUNKT_WERT: 
				punkt->wert = (int8)ergebnis;
				break;
			}
		}
	}
}

void HoleSequenzEvents(APTR *adr, int32 len, struct SEQUENZ *seq) {
	struct FEVENT *fevent;
	struct EVENTBLOCK *evbl;
	uint16 evnum;
	struct EVENT *ev;
	
	seq->eventblock = (struct EVENTBLOCK *)IExec->AllocVecTags(sizeof(struct EVENTBLOCK), AVT_ClearWithValue,0,TAG_END);
	if (seq->eventblock) {
	
		fevent = (struct FEVENT *)adr;
		evbl = seq->eventblock;
		evnum = 0;
		while ((uint32)fevent < (uint32)adr + len) {
			ev = &evbl->event[evnum];
			
			ev->zeit = fevent->zeit;
			ev->status = fevent->status;
			ev->data1 = fevent->data1;
			ev->data2 = fevent->data2;
			
			evnum++;
			if (evnum == EVENTS) {
				if (AddEvbl(evbl)) evbl = evbl->next;
				else break;
				evnum = 0;
			}
			fevent++;
		}
		
	} else Meldung(CAT(MSG_0551, "Not enough memory for event block\n<DiskHornyLaden.c>"));
}

void LeseAreaMIDITRACK(APTR *area, int32 len) {
	uint16 *adr;
	uint16 tag;
	int32 ergebnis, taglen;
	int16 s = 0;
	struct SEQUENZ *seq = NULL;

	adr = (uint16 *)area;
	while ((uint32)adr - (uint32)area < len) {
		adr = LeseTag(adr, &tag, &ergebnis, &taglen);
		if (tag == TAG_MIDITRACK_SPUR) {
			s = (int16)ergebnis;
			if (s >= verSPUREN) break;
		}
		switch (tag) {
			case TAG_MIDITRACK_NAME: strncpy(spur[s].name, (STRPTR)ergebnis, 128); break;
			case TAG_MIDITRACK_PORT: spur[s].port = (uint8)ergebnis; break;
			case TAG_MIDITRACK_CHANNEL: spur[s].channel = (uint8)ergebnis; break;
			case TAG_MIDITRACK_BANK0: spur[s].bank0 = (int8)ergebnis; break;
			case TAG_MIDITRACK_BANK32: spur[s].bank32 = (int8)ergebnis; break;
			case TAG_MIDITRACK_PROG: spur[s].prog = (int8)ergebnis; break;
			case TAG_MIDITRACK_SHIFT: spur[s].shift = (int16)ergebnis; break;
			case TAG_MIDITRACK_MUTE: spur[s].mute = (BOOL)ergebnis; break;
			case TAG_MIDITRACK_AUTOSTATUS: spur[s].autostatus = (BOOL)ergebnis; break;
		}
		if (tag == TAG_MIDITRACK_SEQ_NEUSTART) {
			seq = ErstelleSequenz(s, (int32)ergebnis, FALSE);
			sp[s].neuseq = seq;
			NeueSequenzEinordnen(s);
		}
		if (seq) {
			switch (tag) {
				case TAG_MIDITRACK_SEQ_ENDE: seq->ende = (int32)ergebnis; break;
				case TAG_MIDITRACK_SEQ_NAME: strncpy(seq->name, (STRPTR)ergebnis, 128); break;
				case TAG_MIDITRACK_SEQ_TRANS: seq->trans = (int8)ergebnis; break;
				case TAG_MIDITRACK_SEQ_MARKIERT: seq->markiert = (BOOL)ergebnis; break;
				case TAG_MIDITRACK_SEQ_ALIASANZ: seq->aliasanz = (int16)ergebnis; break;
				case TAG_MIDITRACK_SEQ_ALIASORIG: seq->aliasorig = (struct SEQUENZ *)ergebnis; break;
				case TAG_MIDITRACK_SEQ_SPEICHERADR: seq->speicheradr = (struct SEQUENZ *)ergebnis; break;
				case TAG_MIDITRACK_SEQ_EVENTS: HoleSequenzEvents((APTR)ergebnis, taglen, seq); break;
			}
		}
	}
}

BOOL LadenHorny(STRPTR datei) {
	BPTR file;
	char meldung[120];
	struct HEAD head;
	struct AREA areahead;
	APTR area;
	int32 test;
	//char *t;
	
	//printf("'%s'\n", datei);
	file = IDOS->Open(datei, MODE_OLDFILE);
	if (file) {
		IDOS->Read(file, &head.hornyid, sizeof(head.hornyid));
		IDOS->Read(file, &head.version, sizeof(head.version));
		if (head.hornyid == HORNYID) {
			if (head.version >= TAGFORMAT) {

				EntferneLied();
				InitLied();
				EntferneLinks();
				InitOutportLatenzen();

			    // FAREAS laden...
				while (IDOS->Read(file, &areahead, sizeof(struct AREA))) {
					//t = (char *)&areahead.id;
					//printf("area: %c%c%c%c\n", t[0], t[1], t[2], t[3]);
					if (areahead.len > 0) {
						area = (APTR)IExec->AllocVecTags(areahead.len, TAG_END);
						if (area) {
							test = IDOS->Read(file, area, areahead.len);
							if (test == areahead.len) {
							
								switch (areahead.id) {
									case FAREA_INFO: LeseAreaINFO(area, areahead.len); break;
									case FAREA_MAINGUI: LeseAreaMAINGUI(area, areahead.len); break;
									case FAREA_EDGUI: LeseAreaEDGUI(area, areahead.len); break;
									case FAREA_LOOP: LeseAreaLOOP(area, areahead.len); break;
									case FAREA_METRONOM: LeseAreaMETRONOM(area, areahead.len); break;
									case FAREA_SMPTE: LeseAreaSMPTE(area, areahead.len); break;
									case FAREA_MARKER: LeseAreaMARKER(area, areahead.len); break;
									case FAREA_PHONOLITH: LeseAreaPHONOLITH(area, areahead.len); break;
									case FAREA_OUTPORTS: LeseAreaOUTPORTS(area, areahead.len); break;
									case FAREA_INPORTS: LeseAreaINPORTS(area, areahead.len); break;
									case FAREA_SYSEX: LeseAreaSYSEX(area, areahead.len); break;
									case FAREA_MIXER: LeseAreaMIXER(area, areahead.len); break;
									case FAREA_CTRLCHANGE: LeseAreaCTRLCHANGE(area, areahead.len); break;
									case FAREA_AUTOMATION: LeseAreaAUTOMATION(area, areahead.len); break;
									case FAREA_MIDITRACK: LeseAreaMIDITRACK(area, areahead.len); break;
								}
							
							} else Meldung(CAT(MSG_0552, "Incorrect file"));
							
							IExec->FreeVec(area);
						} else {
//							Seek(file, areahead.len, OFFSET_CURRENT);
							IDOS->ChangeFilePosition(file, areahead.len, OFFSET_CURRENT);
							Meldung(CAT(MSG_0553, "Not enough memory for area\n<DiskHornyLaden.c>"));
						}
					}
				}

				AlleAliaseZuweisen();
				SmpteTicksAktualisieren();
				ErneuereLinks();

			} else Meldung(CAT(MSG_0554, "Obsolete Horny file format\nis not supported anymore"));
		} else Meldung(CAT(MSG_0555, "Not a Horny file"));
	
		IDOS->Close(file);
	} else {
		IDOS->Fault(IDOS->IoErr(), CAT(MSG_0556, "Could not load project"), meldung, 120);
		Meldung(meldung);
	}
	return(TRUE);
}
