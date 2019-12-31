#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <dos/dos.h>
#include <midi/mididefs.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Midi.h"
#include "Sequenzen.h"
#include "Requester.h"
#include "Marker.h"
#include "Projekt.h"
#include "SysEx.h"

#define MTHD 0x4D546864
#define MTRK 0x4D54726B

extern struct GUI gui;
extern struct LIED lied;
extern struct SPUR spur[];
extern struct LOOP loop;
extern struct METRONOM metro;
extern struct MARKER *rootmark;
extern struct SYSEXUNIT *wahlsexunit;

UWORD headformat;
UWORD headtracknumber;
UWORD solution;

ULONG chunklen;
ULONG deltatime;
ULONG ourdeltatime;

UBYTE runningstatus;
UBYTE trackend;


UBYTE *bp = NULL;

UBYTE getbyte() {
	UBYTE b = 0;
	if (chunklen) {
		b = *bp++;
		chunklen--;
	}
	return(b);
}

UWORD getword() {
	UWORD w = 0;
	if (chunklen >= 2) {
		w = (UWORD)(*bp++) << 8;
		w |= (*bp++);
		chunklen -= 2;
	}
	return(w);
}

ULONG getuword() {
	ULONG w = 0;
	if (chunklen >= 4) {
		w = (ULONG)(*bp++) << 24;
		w |= (ULONG)(*bp++) << 16;
		w |= (ULONG)(*bp++) << 8;
		w |= (ULONG)(*bp++);
		chunklen -= 4;
	}
	return (w);
}

ULONG getchunkuword() {
	ULONG w = 0;
	w = (ULONG)(*bp++) << 24;
	w |= (ULONG)(*bp++) << 16;
	w |= (ULONG)(*bp++) << 8;
	w |= (ULONG)(*bp++);
	return (w);
}

void ReadDeltaTime() {
	ULONG delta;
	UBYTE rbyte;
	
	delta = rbyte = getbyte();
	
	if (rbyte & 0x80) { // Bit 7 gesetzt ?
		delta &= 0x7f;
		
		do {
			rbyte = getbyte();
			delta <<= 7;
			delta += (rbyte & 0x7f);
		}
		while (rbyte & 0x80);  // Solange in Delta einlesen bis Bit 7 nicht mehr gesetzt
	}
	
	deltatime += delta;
	ourdeltatime = deltatime;
	
	// PPQ Factor
	ourdeltatime = (ULONG)((DOUBLE)ourdeltatime * (VIERTELWERT / (DOUBLE)solution));
}


ULONG GetDeltaLen() {
	ULONG delta;
	UBYTE rbyte;
	
	delta = getbyte();
	
	if (delta & 0x80) {     // Bit 7 gesetzt ?
		// Bit 7 löschen 
		delta &= 0x7f;
		
		do {
			rbyte = getbyte();
			
			delta <<= 7;
			delta += (rbyte & 0x7f);
		}
		while (rbyte & 0x80);  // Solange in Delta einlesen bis Bit 7 nicht mehr gesetzt
	}
	//	printf("MIDI Delta-Len %d\n",deltatime);
	return (delta);
}


void ReadEvent (WORD track) {
	UBYTE status, userunning, checkbyte, noteoff = 0;
	ULONG eventlength = 0;
	UBYTE byte1, byte2;

	ULONG syselen;
	UBYTE sbyte;
	
	ULONG metalen;

	UBYTE chl;

	ULONG a, i, v;
	UBYTE *name;

	UBYTE nn, dd, cc, bb;

	DOUBLE h = 60000000;
	DOUBLE h2;
	ULONG tempo;
	
	ULONG slen;

	checkbyte = getbyte();
	
	if (checkbyte < 128) {
		status = runningstatus; // No new Status Byte use runningstatus
		userunning = 1;
	} else {
		runningstatus = checkbyte;
		status = checkbyte;
		userunning = 0; // New Statusbyte found
	}
	
	// printf("Read Event CheckByte %d Status %d running %d\n",checkbyte,status,userunning);
	
	switch (status & MS_StatBits) {
		case MS_NoteOff:
		case MS_NoteOn:
		case MS_PolyPress:
		case MS_Ctrl:
		case MS_PitchBend:
			eventlength = 2;
			break;
		
		case MS_Prog:
		case MS_ChanPress:
			eventlength = 1;
			break;
		
		case MS_SysEx:
			syselen = GetDeltaLen();
			while (syselen--) {
				sbyte = getbyte ();
			}
			break;
	}
	
	if (status == 0xFF) { // Meta-Event
			printf(" #### Meta Event ####\n");
			checkbyte = getbyte();
			metalen = GetDeltaLen();
			switch (checkbyte) { // Typ
				case 0: // Sequence Number
					getbyte();
					getbyte();
					break;
	
				case 0x20: // Track->Channel
					chl = getbyte();      // Channel 1 = Byte 1 !!!
					break;
				
				case 0x21: // ?
					chl = getbyte();
					break;
				
				// Text Event/Copyright
				case 1: // Trackname
				case 2:
				case 3:
				case 4:
				case 5: // Lyric
				case 6: // Blockname
				case 7:
					a = 0; // Len
					name = 0;
					
					i = metalen;
					while (i--) { // Text einlesen
						v = getbyte();
	/*						switch (nextbyte) { // Was für ein Text
							case 3:
							case 1:       // Track Name
								if (a < 24) {
									if (name) {
										*name++ = v;
										*name = 0;
									}
								}
								break;
						
							case 6:       // Block Name
								if (name) {
									if (a < 24) {
										if (name) {
											*name = v;
											name++;
											*name = 0;
										}
									}
								}
								break;
						}
						*/
						a++;
					}
					break;
	
			case 0x2F: // Ende des Tracks
				getbyte();
				trackend = 1;
				break;
			
			case 0x58: // Time Signature
				nn = getbyte (); // nn numerator
				dd = getbyte (); // dd denominator
				// Anzahl der MIDI-Clocks in einem Metronom-Click 
				cc = getbyte ();
				bb = getbyte (); // Anzahl der ppq in 1 denominator 
				break;
	
			case 0x51:	// Tempo
				h = 60000000;
				tempo = getbyte();
				tempo <<= 8;
				tempo |= getbyte();
				tempo <<= 8;
				tempo |= getbyte();
				
				// TempoWert = microsekunden pro MIDI-Quarter
				h2 = tempo;
				h /= h2;
				
				if (h < 5) h = 5;
				else if (h > 480) h = 480;
				
				printf(" >>>>>>>>>>>>>>>>> Tempo %f \n",h);
				break;
			
			case 0x54: // SMPTE 
				getbyte();
				getbyte();
				getbyte();
				getbyte();
				getbyte();
				break;
			
			case 0x7F: // SysEx Daten 
				slen = GetDeltaLen ();
				while (slen--) {
					getbyte();
				}
				break;
	
			case 0x59: // Key Signature
				getbyte(); // Sf
				getbyte(); // mi 
				break;
		}
	}
	
	if (eventlength) { // Found Event ?
		
		if (userunning) byte1 = checkbyte;
		else byte1 = getbyte();
		
		if (eventlength == 2) byte2 = getbyte();
		
		AddEvent(track, ourdeltatime, runningstatus, byte1, byte2);
	}
}


void ReadMidiFile(STRPTR filename) {
	BPTR file;
	char head[4];
	APTR mem;
	LONG len;
	WORD track;

	chunklen = 0;
	runningstatus = 0;

	if (file = Open(filename, MODE_OLDFILE)) {
		Seek(file, 0, OFFSET_END);
		len = Seek(file, 0, OFFSET_BEGINNING);
		if (mem = AllocVec(len, 0)) {
			Read(file, mem, len);
			
			bp = (UBYTE *)mem; // Lesezeiger auf Anfang der Datei
		
			head[0] = *bp++; head[1] = *bp++; head[2] = *bp++; head[3] = *bp++;
			
			if (head[0] == 'M' && head[1] == 'T' && head[2] == 'h' && head[3] == 'd') {
				
				EntferneLied();
				NeuesLied();
				
				// Read Header
				chunklen = getchunkuword();
				
				headformat = getword();
				headtracknumber = getword();
				solution = getword();
				
				if (headtracknumber <= SPUREN) lied.spuranz = headtracknumber;
				else lied.spuranz = SPUREN;
				
				track = 0;
				
				printf("HFormat %d Number of Tracks %d Solution %d PPQ\n", headformat, headtracknumber, solution);
				
				// Read Tracks
				while(headtracknumber) {
					// Track
					head[0] = *bp++; head[1] = *bp++; head[2] = *bp++; head[3] = *bp++;
					
					if (head[0] == 'M' && head[1] == 'T' && head[2] == 'r' && head[3] == 'k') {
						
						deltatime = 0;
						ourdeltatime = 0;

						if (track < lied.spuranz) {
							chunklen = getchunkuword();
							trackend = 0;
							
							printf("Track Chunklen %d\n", chunklen);
							
							while(chunklen && !trackend) {
								ReadDeltaTime(); // Set Deltatime
								ReadEvent(track);
							}
						}
					}
					
					headtracknumber--;
					track++;
				}
				TakteAktualisieren();
				SmpteTicksAktualisieren();
				for (track = 0; track < lied.spuranz; track++) NeueSequenzEinordnen(track);

			}
			FreeVec(mem);
		} else Meldung(CAT(MSG_0438, "Not enough memory for Midi file\n<DiskSMF.c>"));
		Close(file);
	}
}
