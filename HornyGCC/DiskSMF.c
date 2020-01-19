#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Midi.h"
#include "Sequenzen.h"
#include "Requester.h"
#include "Marker.h"
#include "Projekt.h"
#include "SysEx.h"
#include "Smpte.h"

#define MTHD 0x4D546864
#define MTRK 0x4D54726B

extern struct LIED lied;
extern struct SPUR spur[];
extern struct MPKANAL mpkanal[OUTPORTS][16];
extern struct MARKER *rootmark;
extern struct SYSEXUNIT *rootsexunit;
extern struct SMPTE smpte;


uint8 *pb;
int32 chunkrest;
int8 lastStatus;

BOOL readerror = FALSE;

void SetzeLeser(APTR mem, int32 len) {
	pb = (uint8 *)mem;
	chunkrest = len;
}

uint8 LeseByte(void) {
	uint8 b = 0;
	if (chunkrest > 0) {
		b = *pb++;
		chunkrest--;
	} else readerror = TRUE;
	return(b);
}

int32 LeseVarLen(void) {
	int32 wert;
	int8 c;
	
	if ((wert = LeseByte()) & 0x80) {
		wert &= 0x7F;
		do {
			wert = (wert << 7) + ((c = LeseByte()) & 0x7F);
		} while (c & 0x80);
	}
	return(wert);
}

void SpringeBytes(int32 l) {
	if (chunkrest >= l) {
		pb = (uint8 *)((uint32)pb + l);
		chunkrest -= l;
	} else readerror = TRUE;
}


BOOL ImportSMF(STRPTR name) {
	APTR mem;
	BPTR file;
	struct CHUNK {
		uint32 type;
		uint32 len;
	} chunk;
	struct MTHEADER {
		int16 format;
		int16 tracks;
		int16 division;
	} mth;
	int16 n;
	int16 s;
	int32 l;
	uint8 event;
	uint8 meta;
	uint8 status;
	uint8 data1;
	uint8 data2;
	int32 zpos;
	int32 use;
	struct MARKER *mark;
	char *nullbyte;
	int8 spurkanal;
	uint8 vbyte[4];
	uint8 *data;
	BOOL SysExGelesen = FALSE;
	
	BOOL testmodus = FALSE;
	
	readerror = FALSE;

	file = IDOS->Open(name, MODE_OLDFILE);
	if (file) {

		IDOS->Read(file, &chunk, sizeof(struct CHUNK));
		if (chunk.type == MTHD) {

			EntferneLied();
			NeuesLied();

			// Header
			IDOS->Read(file, &mth, sizeof(struct MTHEADER));

			if (testmodus) {
				printf("Format %d\n", mth.format);
				printf("Division %d\n", mth.division);
			}
			if (mth.format == 1) {
				if (mth.tracks < verSPUREN) lied.spuranz = mth.tracks - 1;
				else lied.spuranz = verSPUREN - 1;
				
				for (s = 0; s < lied.spuranz; s++) spur[s].channel = 16; //erstmal Thru
			} else {
				if (verSPUREN >= 16) lied.spuranz = 16;
				else lied.spuranz = verSPUREN;

				for (s = 0; s < lied.spuranz; s++) spur[s].channel = s; //Channel 1-16
				mth.tracks = 1;
			}
			
			//Tracks
			n = 0;
			do {
			
				if (IDOS->Read(file, &chunk, sizeof(struct CHUNK)) == 0) break; // EOF?
				
				if (chunk.type == MTRK) {
					if (testmodus) printf("Midispur %d\n", n);

					mem = (APTR)IExec->AllocVecTags(chunk.len, TAG_END);
					if (mem) {
						IDOS->Read(file, mem, chunk.len);
						SetzeLeser(mem, chunk.len);

						if (mth.format == 1) s = n - 1;
						zpos = 0;
						status = 0;
						spurkanal = -1;
						
						do {
							zpos = zpos + LeseVarLen();
							event = LeseByte();
							
							if (event & 0x80) { // Neues Statusbyte oder Running Status?
								if (event == 0xFF) { // Meta
									status = 0;
									meta = LeseByte();
									l = LeseVarLen();
									if (testmodus) printf("Meta %X (L%ld): ", meta, l);
									
									switch (meta) {
										case 0x01: // Text Event
										case 0x03: //Spurname
										if ((meta == 0x01) && zpos) break; // Kein Spurname
										if (testmodus) printf("Spurname");
										if (mth.format == 1) {
											use = l;
											if (s >= 0) {
												if (use > 127) use = 127;
												memcpy(spur[s].name, pb, use); spur[s].name[use] = 0;
											} else {
												if (use > 127) use = 127;
												memcpy(lied.name, pb, use); lied.name[use] = 0;
											}
										}
										break;
										
										case 0x06: //Textmarker
										case 0x07: // Cue Point
										if (testmodus) printf("Textmarker");
										mark = NeuerMarker(M_TEXT, ((zpos << VIERTEL) / mth.division) & VIERTELMASKE, 0, 0);
										if (mark) {
											use = l; if (use > 127) use = 127;
											memcpy(&mark->text, pb, use);
											nullbyte = &mark->text; nullbyte = (char *)((uint32)nullbyte + use);
											*nullbyte = 0;
										}
										break;
										
										case 0x20: //Channel
										if (testmodus) printf("Channel");
										spur[s].channel = *pb;
										break;
										
										case 0x51: //Tempo
										if (testmodus) printf("Tempo");
										memcpy(&use, pb, 3); use = use >> 8; use = 60000000 / use;
										NeuerMarker(M_TEMPO, ((zpos << VIERTEL) / mth.division) & VIERTELMASKE, (int16)use, 0);
										break;
										
										case 0x54: // SMPTE Offset
										memcpy(vbyte, pb, 4);
										smpte.startticks = Smpte2Ticks(vbyte[0], vbyte[1], vbyte[2], vbyte[3]);
										break;
										
										case 0x58: //Taktart
										if (testmodus) printf("Taktart");
										NeuerMarker(M_TAKT, ((zpos << VIERTEL) / mth.division) & VIERTELMASKE, 0, *pb);
										break;
										
									}
									
									SpringeBytes(l);
									if (testmodus) printf("\n");
									
								} else if (event == 0xF0) { // SysEx
									status = 0;
									l = LeseVarLen();
									if (testmodus) printf("SysEx F0\n");
									
									data = (uint8 *)IExec->AllocVecTags(l + 1, TAG_END);
									*data = 0xF0;
									memcpy((void*)((uint32)data + 1), (void*)pb, l);
									NeuesSysEx(rootsexunit, (STRPTR)"SysEx", l + 1, data);
									
									SpringeBytes(l);
									
									SysExGelesen = TRUE;
									
								} else if (event == 0xF7) { // SysEx
									status = 0;
									l = LeseVarLen();
									if (testmodus) printf("SysEx F7\n");
									SpringeBytes(l);
									
								} else {
									status = event;
									data1 = LeseByte();
								}
							} else {
								data1 = event;
							}
							if (status) {
								if (spurkanal == -1)
									spurkanal = status & MS_ChanBits;
								else
									if (spurkanal != (status & MS_ChanBits))
										spurkanal = -128;
									
								switch (status & MS_StatBits) { // Zweites Datenbyte?
									case MS_NoteOff:
									case MS_NoteOn:
									case MS_PolyPress:
									case MS_Ctrl:
									case MS_PitchBend:
									data2 = LeseByte();
									break;
									default:
									data2 = 0;
									break;
								}

								if (mth.format == 0) s = status & MS_ChanBits;
								if (s < lied.spuranz) {
									if (!AddEvent(s, (zpos << VIERTEL) / mth.division, status, data1, data2)) break;
								}
							}
							
							if (readerror || (uint32)pb > (uint32)mem + chunk.len) {
								Meldung(CAT(MSG_0437, "Error while importing"));
								break;
							}
							
						} while (meta != 0x2F);
						
						if ((spurkanal >= 0) && (spur[s].channel == 16)) spur[s].channel = spurkanal;
						
						IExec->FreeVec(mem);
					} else {
						Meldung(CAT(MSG_0438, "Not enough memory for SMF chunk\n<DiskSMF.c>"));
						break;
					}
					n++;

				} else {
					printf("Unbekannter Chunk %lX\n", chunk.type);
//					Seek(file, chunk.len, OFFSET_CURRENT);
					IDOS->ChangeFilePosition(file, chunk.len, OFFSET_CURRENT);
				}
			} while (n < mth.tracks);
			
			TakteAktualisieren();
			SmpteTicksAktualisieren();
			for (s = 0; s < lied.spuranz; s++) NeueSequenzEinordnen(s);
			
			if (SysExGelesen) Meldung(CAT(MSG_0438A, "SysEx data was found and copied to SysEx Manager"));
			
		} else {
			Meldung(CAT(MSG_0439, "Not a SMF file"));
		}
		IDOS->Close(file);
	}
	return(TRUE);
}

void SchreibeVarLen(int32 wert) {
	int32 b;
	
	b = wert & 0x7F;
	while ((wert >>= 7)>0) {
		b <<= 8;
		b |= 0x80;
		b += (wert & 0x7F);
	}
	
	while (TRUE) {
		*pb = (int8)(b & 0x000000FF); pb++;
		if (b & 0x80) b >>= 8; else break;
	}
}

void SchreibeMidiEvent(int32 delta, int16 s, int8 status, int8 data1, int8 data2) {
	SchreibeVarLen(delta);
	
	if (spur[s].channel < 16) {
		status &= MS_StatBits;
		status |= spur[s].channel;
	}

	if ((status & MS_StatBits) == MS_NoteOff) {
		status = ((status & MS_ChanBits) | MS_NoteOn);
		data2 = 0;
	}
		
	if (status != lastStatus) { //Running Status
		*pb++ = status;
		lastStatus = status;
	}
	*pb++ = data1;

	switch (status & MS_StatBits) { // Zweites Datenbyte?
		case MS_NoteOff:
		case MS_NoteOn:
		case MS_PolyPress:
		case MS_Ctrl:
		case MS_PitchBend:
		*pb++ = data2;
		break;
	}
}

void SchreibeSysEx(uint8 *data, int32 len) {
	SchreibeVarLen(0);
	
	*pb++ = 0xF0;
	SchreibeVarLen(len - 1);
	memcpy(pb, (void *)((uint32)data + 1), len - 1);
	pb = (uint8*)((int32)pb + len - 1);
}

void SchreibeMetaEvent(int32 delta, int8 type, int32 len, int8 *bytes) {
	int16 n;
	
	SchreibeVarLen(delta);
	*pb++ = 0xFF;
	*pb++ = type;
	SchreibeVarLen(len);
	for (n = 0; n < len; n++) *pb++ = bytes[n];
}

void ExportSMF(STRPTR name) {
	APTR mem;
	BPTR file;
	struct CHUNK {
		uint32 type;
		uint32 len;
	} chunk;
	struct MTHEADER {
		int16 format;
		int16 tracks;
		int16 division;
	} mth;
	int32 use;
	int8 useb[4];
	int16 n;
	struct MARKER *mark;
	struct SEQUENZ *seq;
	struct EVENTBLOCK *evbl;
	int16 evnum;
	struct EVENT *event;
	int32 zpos;
	int32 altzpos;
	int8 p, c;
	struct SYSEXUNIT *unit;
	struct SYSEXMSG *sysex;

	
	mem = IExec->AllocVecTags(300000, AVT_ClearWithValue,0,TAG_END);
	if (mem) {

		file = IDOS->Open(name, MODE_NEWFILE);
		if (file) {
			chunk.type = MTHD;
			chunk.len = 6;
			IDOS->Write(file, &chunk, sizeof(struct CHUNK));
			mth.format = 1;
			mth.tracks = lied.spuranz + 1;
			mth.division = 256;
			IDOS->Write(file, &mth, sizeof(struct MTHEADER));
		
			pb = (uint8 *)mem;

			//Liedname
			SchreibeMetaEvent(0, 0x03, strlen(lied.name), (int8 *)lied.name);
			
			//Marker
			altzpos = 0;
			mark = rootmark;
			do {
				switch (mark->typ) {
					case M_TAKT:
					useb[0] = mark->m_zaehler;
					useb[1] = 2;
					useb[2] = 32;
					useb[3] = 8;
					SchreibeMetaEvent(mark->takt - altzpos, 0x58, 4, useb);
					break;
					
					case M_TEMPO:
					use = 60000000 / (int32)mark->m_bpm;
					use = use << 8;
					SchreibeMetaEvent(mark->takt - altzpos, 0x51, 3, (int8 *)&use);
					break;
					
					case M_TEXT:
					SchreibeMetaEvent(mark->takt - altzpos, 0x06, strlen(&mark->text), (int8*)&mark->text);
					break;
				}
				altzpos = mark->takt;
				mark = mark->next;
			} while (mark);
						
			//EOT
			SchreibeMetaEvent(0, 0x2F, 0, NULL);

			chunk.type = MTRK;
			chunk.len = (uint32)pb - (uint32)mem;
			IDOS->Write(file, &chunk, sizeof(struct CHUNK));
			IDOS->Write(file, mem, chunk.len);


			for(n = 0; n < lied.spuranz; n++) {
				pb = (uint8 *)mem;
				lastStatus = -1;
				
				//Spurname
				SchreibeMetaEvent(0, 0x03, strlen(spur[n].name), (int8 *)spur[n].name);
				
				//Midi Channel
				if (spur[n].channel < 16) SchreibeMetaEvent(0, 0x20, 1, (int8 *)&spur[n].channel);
				
				//Programm
				if (spur[n].prog >= 0) {
					if (spur[n].bank0 >= 0) SchreibeMidiEvent(0, n, MS_Ctrl, MC_Bank, spur[n].bank0);
					if (spur[n].bank32 >= 0) SchreibeMidiEvent(0, n, MS_Ctrl, MC_Bank + 0x20, spur[n].bank32);
					SchreibeMidiEvent(0, n, MS_Prog, spur[n].prog, 0);
				}
				
				//Mischpult
				p = spur[n].port;
				c = spur[n].channel;
				SchreibeMidiEvent(0, n, MS_Ctrl, MC_Volume, mpkanal[p][c].fader);
				if (mpkanal[p][c].pan != MCCenter) SchreibeMidiEvent(0, n, MS_Ctrl, MC_Pan, mpkanal[p][c].pan);
				
				//SysEx
				if (n == 0) {
					unit = rootsexunit;
					while (unit) {
						if (!unit->gesperrt) {
							sysex = unit->sysex;
							while (sysex) {
								SchreibeSysEx(sysex->data, sysex->len);
								sysex = sysex->next;
							}
						}
						unit = unit->next;
					}
				}
				
				//Midi Events schreiben...
				altzpos = 0;
				seq = spur[n].seq;
				while (seq) {
					evbl = seq->eventblock; evnum = 0;
					do {
						event = &evbl->event[evnum];
					
						zpos = event->zeit + seq->start + spur[n].shift;
						if (zpos < 0) zpos = 0;
						if (zpos < altzpos) zpos = altzpos;
						
						if ((event->status & MS_StatBits) <= MS_NoteOn) {
							SchreibeMidiEvent(zpos - altzpos, n, event->status, event->data1 + seq->trans, event->data2);
						} else {
							SchreibeMidiEvent(zpos - altzpos, n, event->status, event->data1, event->data2);
						}
						altzpos = zpos;
						
						evnum++;
						if (evnum == EVENTS) {evbl = evbl->next; evnum = 0;}
					} while (evbl && evbl->event[evnum].status);
					
					seq = seq->next;
				}
				
				
				//EOT
				SchreibeMetaEvent(0, 0x2F, 0, NULL);

				chunk.type = MTRK;
				chunk.len = (uint32)pb - (uint32)mem;
				IDOS->Write(file, &chunk, sizeof(struct CHUNK));
				IDOS->Write(file, mem, chunk.len);
			}
			
			IDOS->Close(file);
		
		} else {
			Meldung(CAT(MSG_0440, "Could not save file"));
		}
		
		IExec->FreeVec(mem);

	} else {
		Meldung(CAT(MSG_0441, "Not enough memory for write buffer\n<DiskSMF.c>"));
	}
}
