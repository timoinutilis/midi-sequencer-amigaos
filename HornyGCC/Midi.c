#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/camd.h>
#ifdef __amigaos4__
#include <proto/eXtreamSync.h>
#endif
#define __NOLIBBASE__
#include <proto/timer.h>
#undef __NOLIBBASE__
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <midi/mididefs.h>
#include <midi/camd.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Requester.h"
#include "Marker.h"
#include "Spuren.h"
#include "Sequenzen.h"
#include "SysEx.h"
#include "Mischpult.h"
#include "Automation.h"
#include "CtrlWandler.h"
#include "Projekt.h"
#include "Midi.h"

#include "Gui.h"

struct Library *CamdBase = NULL;

#ifdef __amigaos4__
struct MsgPort *TimerMP = NULL;
struct timerequest *TimerIO = NULL;
struct Library *TimerBase = NULL;
struct TimerIFace *ITimer = NULL;
struct CamdIFace *ICamd = NULL;
struct Library *eXtreamSyncBase = NULL;
struct eXtreamSyncIFace *IeXtreamSync = NULL;
#endif

extern struct Window *edfenster;
extern struct Process *thruproc;
struct MidiNode *midi = NULL;
struct MidiLink *midiout[OUTPORTS];
struct MidiLink *midiin[INPORTS];
struct OUTPORT outport[OUTPORTS];
struct INPORT inport[INPORTS];

struct EClockVal eclock;
ULONG efreq = 0;
struct timeval warteende;

struct Task *hornytask = NULL;

struct SPUR spur[SPUREN];
struct SPURTEMP sp[SPUREN];
struct KANALTEMP kt[16][16];
extern struct MPKANAL mpkanal[OUTPORTS][16];
extern struct UMGEBUNG umgebung;

LONG takt = 0;
LONG tick = 0;

LONG starttakt = 0;
ULONG zeit = 0;
ULONG startzeit = 0;
ULONG starthiclock = 0;

WORD altmetro = 0;
BYTE midisig = -1;
BYTE playsig = -1;
BYTE playerprocsig = -1;
BYTE thruprocsig = -1;
LONG delay = 0;
struct METRONOM metro = {
	0, 9, // port, channel
	75, 76, // taste1, taste2
	127, 127, // velo1, velo2
	VIERTEL, //raster
	TRUE, FALSE // rec, play
};
BYTE hornystatus = STATUS_UNINIT;
BOOL tmarkwechsel = FALSE;
struct MARKER *ltmark = NULL;
struct MARKER *lkmark = NULL;
struct MARKER *lxmark = NULL;
struct LOOPZEIT lzeit;
UBYTE playsigaktion = 0;

extern struct MARKER *rootmark;
extern struct LIED lied;
extern struct LOOP loop;
extern WORD snum;
extern BOOL sysexrec;

struct MsgPort *syncport = NULL;
BOOL syncaktiv = FALSE;
//--------------------------------------------------------------------------------------------------

void InitOutPortInstr(BYTE n) {
	BYTE i;
	for (i = 0; i < 4; i++) strcpy(outport[n].outinstr[i].name, "???");
	outport[n].outinstr[0].unten = 0;
	for (i = 1; i < 4; i++) {
		outport[n].outinstr[i].unten = 16;
	}
}

void ErstelleCamd(void) {
	BYTE n;

	CamdBase = OpenLibrary("camd.library", 37);
	if (CamdBase) {
#ifdef __amigaos4__
		ICamd = (struct CamdIFace *)GetInterface(CamdBase, "main", 1, NULL);
		if (!ICamd) Meldung("Could not obtain camd interface");
#endif
	} else Meldung("Could not open camd.library");

#ifdef __amigaos4__
	TimerMP = AllocSysObject(ASOT_PORT, NULL);
	if (TimerMP) {
		TimerIO = AllocSysObjectTags(ASOT_IOREQUEST,
			ASOIOR_Size, sizeof(struct timerequest),
			ASOIOR_ReplyPort, TimerMP,
			TAG_DONE);

		if (TimerIO) {
			if (OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIO, 0L) == 0) {
				TimerBase = (struct Library *)TimerIO->tr_node.io_Device;
				ITimer = (struct TimerIFace *)GetInterface(TimerBase, "main", 1, NULL);
				if (!ITimer) Meldung("Could not obtain timer interface");
			} else Meldung("Could not open timer device");
		} else Meldung("Could not allocate TimerIO");
	} else Meldung("Could not allocate TimerMP");
#endif

	if (CamdBase) {
#ifdef __amigaos4__
		if (ICamd) {
#endif

			while (midisig == -1) {
				Delay(10);
			}
			
			midi = CreateMidi(
				MIDI_MsgQueue, 2048,
				MIDI_SysExSize, umgebung.sysexpuffer * 1024,
				MIDI_TimeStamp, &takt,
				MIDI_SignalTask, &thruproc->pr_Task,
				MIDI_RecvSignal, midisig,
				MIDI_ClientType, CCType_Sequencer,
				TAG_DONE);
			playsig = AllocSignal(-1);
			hornytask = FindTask(NULL);
#ifdef __amigaos4__
		}
#endif
	}

	for(n = 0; n < verOUTPORTS; n++) {
		midiout[n] = NULL;
		outport[n].name[0] = 0;
		outport[n].thru = TRUE;
		outport[n].latenz = 0;
		InitOutPortInstr(n);
	}
	for(n = 0; n < verINPORTS; n++) {
		midiin[n] = NULL;
		inport[n].name[0] = 0;
	}
	
#ifdef __amigaos4__
	eXtreamSyncBase = OpenLibrary("eXtreamSync.library", 0);
	if (eXtreamSyncBase) {
		IeXtreamSync = (struct eXtreamSyncIFace *)GetInterface(eXtreamSyncBase, "main", 1, NULL);
		if (IeXtreamSync) {
			syncport = CreateMsgPort();
		} else Meldung("Could not obtain eXtreamSync interface");
	};
#endif

}

void EntferneCamd(void) {
#ifdef __amigaos4__
	DeaktiviereExtreamSync();
	DeleteMsgPort(syncport);
	DropInterface((struct Interface *)IeXtreamSync);
	CloseLibrary(eXtreamSyncBase);
#endif

	if (CamdBase) DeleteMidi(midi);
#ifdef __amigaos4__
	DropInterface((struct Interface *)ICamd);
#endif
	CloseLibrary(CamdBase);

#ifdef __amigaos4__
	if (!CheckIO((struct IORequest *)TimerIO)) {
		AbortIO((struct IORequest *)TimerIO);
		WaitIO((struct IORequest *)TimerIO);
	}
	DropInterface((struct Interface *)ITimer);
	CloseDevice((struct IORequest *)TimerIO);
	FreeSysObject(ASOT_IOREQUEST, TimerIO);
	FreeSysObject(ASOT_PORT, TimerMP);
#endif
}

void WarteStart(WORD s) {
	struct timeval warte = {s, 0};
	
	GetSysTime(&warteende);
	AddTime(&warteende, &warte);
}

void WarteEnde(void) {
	struct timeval aktzeit;
	
	do {
		GetSysTime(&aktzeit);
		if (CmpTime(&aktzeit, &warteende) == -1) break;
		Delay(10);
	} while (TRUE);
}

void EntferneLinks(void) {
    BYTE l;

	for (l = 0; l < verINPORTS; l++) {
		if (midiin[l]) {
			RemoveMidiLink(midiin[l]);
			midiin[l] = NULL;
		}
		inport[l].name[0] = 0;
	}
	for (l = 0; l < verOUTPORTS; l++) {
		if (midiout[l]) {
			RemoveMidiLink(midiout[l]);
			midiout[l] = NULL;
		}
		outport[l].name[0] = 0;
	}
}

void ErneuereLinks(void) {
    BYTE l;
	char linkname[32];

	for (l = 0; l < verINPORTS; l++) {
		if (midiin[l]) {
			RemoveMidiLink(midiin[l]);
			midiin[l] = NULL;
		}
	}
	for (l = 0; l < verOUTPORTS; l++) {
		if (midiout[l]) {
			RemoveMidiLink(midiout[l]);
			midiout[l] = NULL;
		}
	}

	for (l = 0; l < verINPORTS; l++) {
		if (inport[l].name[0]) {
			sprintf(linkname, "horny%d.in", l);
			midiin[l] = AddMidiLink(midi, MLTYPE_Receiver,
				MLINK_Name, linkname,
				MLINK_Location, inport[l].name,
				TAG_END);
		}
	}
	for (l = 0; l < verOUTPORTS; l++) {
		if (outport[l].name[0]) {
			sprintf(linkname, "horny%d.out", l);
			midiout[l] = AddMidiLink(midi, MLTYPE_Sender,
				MLINK_Name, linkname,
				MLINK_Location, outport[l].name,
				TAG_END);
		}
	}
}

void ErstelleLinks(void) {
	struct MidiCluster *cluster = NULL;
	BYTE lin = 0;
	BYTE lout = 0;
	STRPTR name;
	APTR camdlock;

	camdlock = LockCAMD(CD_Linkages);
	while (cluster = NextCluster(cluster)) {
		name = cluster->mcl_Node.ln_Name;

		if ((lin < verINPORTS) && (strstr(name, "in."))) {
			strncpy(inport[lin].name, name, 128);
			lin++;
		}
		if ((lout < verOUTPORTS) && (strstr(name, "out.") || strstr(name, "thru"))) {
			strncpy(outport[lout].name, name, 128);
			lout++;
		}
	}
	UnlockCAMD(camdlock);
	ErneuereLinks();
}

BOOL LinkVorhanden(STRPTR name) {
	struct MidiCluster *cluster = NULL;
	APTR camdlock;
	BOOL vorhanden = FALSE;

	if (strcmp(name, "phonolith.thru") == 0) {
		return TRUE;
	}

	camdlock = LockCAMD(CD_Linkages);
	while (cluster = NextCluster(cluster)) {
		if (strcmp(name, cluster->mcl_Node.ln_Name) == 0) {
			vorhanden = TRUE;
			break;
		}
	}
	UnlockCAMD(camdlock);
	return(vorhanden);
}

void InitOutportLatenzen()
{
	WORD p;
	for (p = 0; p < verOUTPORTS; p++) {
		outport[p].latenz = 0;
	}
}

//--------------------------------------------------------------------------------

void AktiviereExtreamSync(void) {
#ifdef __amigaos4__
	if (!syncaktiv) {
		if (syncport && Connect(syncport) == 0) {
			syncaktiv = TRUE;
		} else Meldung("Could not connect to eXtreamSync");
	}
#endif
}

void DeaktiviereExtreamSync(void) {
#ifdef __amigaos4__
	if (syncaktiv) {
		Disconnect(syncport);
		syncaktiv = FALSE;
	}
#endif
}

BOOL IstExtreamSyncAktiv(void) {
	return syncaktiv;
}

void KontrolleExtreamSync(void) {
#ifdef __amigaos4__
	struct eXtreamSyncStandardMessage *mes;
	DOUBLE seconds;
	
	while (mes = (struct eXtreamSyncStandardMessage*)GetMsg(syncport)) {

		switch (mes->type) {
			case SYNC_Start:
			WiedergabeStarten(TRUE);
			break;
			
			case SYNC_Stop:
			Stoppen();
			break;

			case SYNC_Hold:
			Stoppen();
			break;

			case SYNC_Continue:
			WiedergabeStarten(TRUE);
			break;

			case SYNC_Locate:
			seconds = ((struct eXtreamSyncLocateMessage*)mes)->timecode;
			takt = SmpteTicksTakt((LONG)(seconds * 600));
			tick = TaktSmpteTicks(takt);
			ResetteLMarker();
			ZeichneAnzeigen(TRUE);
			ZeichnePosition(FALSE);
			break;

			case SYNC_PreLoad:
			PreloadReady(syncport, mes->SystemMsg.mn_ReplyPort);
			break;
		}
	}
#endif
}

void StartExtreamSync(void) {
#ifdef __amigaos4__
	struct eXtreamSyncStandardMessage *mes;
	BOOL weiter = FALSE;
	
	if (syncaktiv) {
		LocateExtreamSync();
		
		Start(syncport);
		
		do {
			WaitPort(syncport);
			while (mes = (struct eXtreamSyncStandardMessage*)GetMsg(syncport)) {
				if (mes->type == SYNC_PreLoadDone) {
					weiter = TRUE;
					break;
				} else {
					printf("got unexpected eXtreamSync message type: %d\n", mes->type);
					weiter = TRUE;
					break;
				}
			}
		} while (!weiter);
	}
#endif
}

void StopExtreamSync(void) {
#ifdef __amigaos4__
	if (syncaktiv) {
		Stop(syncport);
	}
#endif
}

void LocateExtreamSync(void) {
#ifdef __amigaos4__
	DOUBLE timecode;
	LONG timeticks;
	
	if (syncaktiv) {
		timeticks = TaktSmpteTicks(takt);
		timecode = (DOUBLE)timeticks / 600;

		Locate(syncport, timecode);
	}
#endif
}

//--------------------------------------------------------------------------------

void ResetteLMarker(void) {
	ltmark = TaktMarker(NULL, M_TEMPO, takt);
	lkmark = TaktMarker(NULL, M_TAKT, takt);
	lxmark = TaktMarker(NULL, M_TEXT, takt);
}

void ResetteZeit(void) {
	efreq = ReadEClock(&eclock) / 1200;
	startzeit = eclock.ev_lo / efreq;
	starthiclock = eclock.ev_hi;

	starttakt = takt - ltmark->takt;

	ResetteLMarker();

	zeit = startzeit;
	delay = 60000000L / ltmark->m_bpm / VIERTELWERT;
}

void AktualisiereTakt(void) {
	ULONG z;
	struct MARKER *next;

	ReadEClock(&eclock);
	zeit = eclock.ev_lo / efreq;
	zeit += (eclock.ev_hi - starthiclock) * ((ULONG)0xFFFFFFFF / efreq);
	if (zeit < startzeit) Meldung("Internal Time Error");

	z = zeit - startzeit;
	//takt = starttakt + ltakt + (z << VIERTEL) * bpm / 72000; (eigentliche Rechnung der nächsten Zeile)
	takt = starttakt + ltmark->takt + (((z * ltmark->m_bpm) << (VIERTEL - 6)) / 1125);
	tick =  ltmark->ticks + (((takt - ltmark->takt) * 60 / ltmark->m_bpm * 600) >> VIERTEL);

	next = NextMarker(ltmark);
	if (next) {
		if (takt >= next->takt) {
			ltmark = next;
			startzeit = zeit;
			starttakt = takt - ltmark->takt;
			z = zeit - startzeit;
			takt = starttakt + ltmark->takt + (((z * ltmark->m_bpm) << (VIERTEL - 6)) / 1125);
			delay = 60000000L / ltmark->m_bpm / VIERTELWERT;
			tmarkwechsel = TRUE;
		}
	}
	next = NextMarker(lkmark);
	if (next) {
		if (takt >= next->takt) {
			lkmark = next;
			tmarkwechsel = TRUE;
		}
	}
	next = NextMarker(lxmark);
	if (next) {
		if (takt >= next->takt) {
			lxmark = next;
			tmarkwechsel = TRUE;
		}
	}
}

void SendeEvent(WORD s, UBYTE status, UBYTE data1, UBYTE data2) {
	MidiMsg msg;

	if (midiout[spur[s].port]) {
		msg.mm_Status = status;
		if (spur[s].channel < 16) msg.mm_Status = (msg.mm_Status & MS_StatBits) | spur[s].channel;
		msg.mm_Data1 = data1;
		msg.mm_Data2 = data2;
		PutMidi(midiout[spur[s].port], msg.mm_Msg);
	}
}

void SendeKanalEvent(BYTE port, BYTE channel, UBYTE status, UBYTE data1, UBYTE data2) {
	MidiMsg msg;

	if (midiout[port]) {
		msg.mm_Status = status;
		msg.mm_Status = (msg.mm_Status & MS_StatBits) | channel;
		msg.mm_Data1 = data1;
		msg.mm_Data2 = data2;
		PutMidi(midiout[port], msg.mm_Msg);
	}
}

void SetzeAktInstrument(BYTE p, BYTE c, BYTE bank0, BYTE bank32, BYTE prog) {
	kt[p][c].aktbank0 = bank0;
	kt[p][c].aktbank32 = bank32;
	kt[p][c].aktprog = prog;
}

void SendeKanalInstrument(BYTE p, BYTE c, BYTE bank0, BYTE bank32, BYTE prog) {
	if (bank0 >= 0) SendeKanalEvent(p, c, MS_Ctrl, MC_Bank, bank0);
	if (bank32 >= 0) SendeKanalEvent(p, c, MS_Ctrl, MC_Bank + 0x20, bank32);
	SendeKanalEvent(p, c, MS_Prog, prog, 0);
}

void SendeInstrument(WORD s) {
	UBYTE p = spur[s].port;
	UBYTE c = spur[s].channel;

	if (spur[s].prog >= 0) {
		if (spur[s].bank0 >= 0) SendeEvent(s, MS_Ctrl, MC_Bank, spur[s].bank0);
		if (spur[s].bank32 >= 0) SendeEvent(s, MS_Ctrl, MC_Bank + 0x20, spur[s].bank32);
		SendeEvent(s, MS_Prog, spur[s].prog, 0);

		kt[p][c].aktbank0 = spur[s].bank0;
		kt[p][c].aktbank32 = spur[s].bank32;
		kt[p][c].aktprog = spur[s].prog;
	}
}

void Panik(void) {
	BYTE p, c;

	for (p = 0; p < verOUTPORTS; p++) {
		if (midiout[p]) {
			for (c = 0; c < 16; c++) SendeKanalEvent(p, c, MS_Ctrl, MM_AllOff, 0);
		}
	}
}

BOOL AddEvent(WORD s, LONG t, UBYTE status, UBYTE data1, UBYTE data2) {
	struct SPUR *spr;
	struct EVENT *ev;
	BOOL ok = TRUE;

	if (((status & MS_StatBits) == MS_NoteOn) && (data2 == 0)) status = (status & MS_ChanBits) | MS_NoteOff;

	spr = &spur[s];
	if (!sp[s].neuseq) {
		sp[s].neuseq = ErstelleSequenz(s, t, TRUE);
		if (sp[s].neuseq) {
			spr->aktevbl = sp[s].neuseq->eventblock;
			spr->aktevnum = 0;
		}
	}

	if (sp[s].neuseq && spr->aktevbl) {
		ev = &spr->aktevbl->event[spr->aktevnum];
		ev->zeit = t - (sp[s].neuseq->start);
		ev->status = status;
		ev->data1 = data1;
		ev->data2 = data2;
		if (t > sp[s].neuseq->ende) {
			sp[s].neuseq->ende = (t + VIERTELWERT - 1) & VIERTELMASKE;
		}

		spr->aktevnum++;
		if (spr->aktevnum == EVENTS) {
			if (AddEvbl(spr->aktevbl)) spr->aktevbl = spr->aktevbl->next; else ok = FALSE;
			spr->aktevnum = 0;
		}
	} else {
		Meldung(CAT(MSG_0158, "Not enough memory for sequence/event block\n<Midi.c>"));
		ok = FALSE;
	}

	return(ok);
}

LONG verschobenerTakt(WORD s, LONG zeit)
{
	return verschobenerPortTakt(spur[s].port, zeit) + spur[s].shift;
}

LONG verschobenerPortTakt(WORD p, LONG zeit)
{
	LONG shift = (outport[p].latenz * (ltmark->m_bpm << VIERTEL) / 60000L);
	return zeit - shift;
}

LONG vorgeschobenerPortTakt(WORD p, LONG zeit)
{
	LONG shift = (outport[p].latenz * (ltmark->m_bpm << VIERTEL) / 60000L);
	return zeit + shift;
}

LONG aufnahmeLatenzAusgleich(WORD s, LONG zeit)
{
	LONG shift = (outport[spur[s].port].latenz * (ltmark->m_bpm << VIERTEL) / 60000L);
	return zeit + shift;
}


BOOL SpurAufnehmen(WORD s) {
	MidiMsg msg;
	UBYTE p = spur[s].port;

	if (GetMidi(midi, &msg)) {
		if (MidiMsgType(&msg) < CMB_RealTime) {
			// Controller Wandler...
			if ((msg.mm_Status & MS_StatBits) == MS_Ctrl) msg.mm_Data1 = WandleController(msg.mm_Data1);

			AddEvent(s, aufnahmeLatenzAusgleich(s, msg.mm_Time), msg.mm_Status, msg.mm_Data1, msg.mm_Data2);
			if (midiout[p]) {
				if (spur[s].channel < 16) msg.mm_Status = (msg.mm_Status & MS_StatBits) | spur[s].channel;
				PutMidi(midiout[p], msg.mm_Msg);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

void AbspielenVorbereiten(WORD s) {
	struct EVENT *event;
	MidiMsg msg;
	UBYTE n;
	UBYTE p = spur[s].port;
	UBYTE c = spur[s].channel;

	if (midiout[p]) {
		msg.mm_Status = MS_PitchBend | c;
		msg.mm_Data1 = 0;
		msg.mm_Data2 = 64;
		PutMidi(midiout[p], msg.mm_Msg);
	}

	spur[s].aktseq = spur[s].seq;
	spur[s].aktevnum = 0;

	if (spur[s].seq) {

		while (spur[s].aktseq->ende + spur[s].shift < takt) {
			spur[s].aktseq = spur[s].aktseq->next;
			if (!spur[s].aktseq) break;
		}
		
		if (!spur[s].aktseq) {

			spur[s].aktevbl = NULL;

		} else {
			
			spur[s].aktevbl = spur[s].aktseq->eventblock;

			while (spur[s].aktseq && (verschobenerTakt(s, spur[s].aktseq->start + spur[s].aktevbl->event[spur[s].aktevnum].zeit) < takt)) {
				event = &spur[s].aktevbl->event[spur[s].aktevnum];

				if ((event->status & MS_StatBits) == MS_NoteOn) kt[p][c].note[event->data1 + spur[s].aktseq->trans] = event->data2;
				if ((event->status & MS_StatBits) == MS_NoteOff) kt[p][c].note[event->data1 + spur[s].aktseq->trans] = 0;

				spur[s].aktevnum++;
				if (spur[s].aktevnum == EVENTS) {
					spur[s].aktevbl = spur[s].aktevbl->next;
					spur[s].aktevnum = 0;
				}
				if (!spur[s].aktevbl || !spur[s].aktevbl->event[spur[s].aktevnum].status) {
					spur[s].aktseq = spur[s].aktseq->next;
					if (spur[s].aktseq) {
						spur[s].aktevbl = spur[s].aktseq->eventblock;
					} else {
						spur[s].aktevbl = NULL;
						break;
					}
					spur[s].aktevnum = 0;
				}
			}

		
			if (midiout[p]) {
				msg.mm_Status = MS_NoteOn | c;
				for (n = 0; n < 128; n++) {
					if (kt[p][c].note[n]) {
						if ((!mpkanal[p][c].mute) && (!spur[s].mute)) { // Mutes testen
							msg.mm_Data1 = n;
							msg.mm_Data2 = kt[p][c].note[n];
							PutMidi(midiout[p], msg.mm_Msg);
						}
					}
				}
			}
			
		}
	}
}

void LoopVorbereiten(WORD s) {
	sp[s].loopseq = spur[s].seq;
	sp[s].loopevnum = 0;

	if (sp[s].loopseq) {
		sp[s].loopevbl = sp[s].loopseq->eventblock;

		while (sp[s].loopseq && (verschobenerTakt(s, sp[s].loopseq->ende) < loop.start)) {
			sp[s].loopseq = sp[s].loopseq->next;
		}
		
		if (!sp[s].loopseq) {
		
			sp[s].loopevbl = NULL;
		
		} else {

			sp[s].loopevbl = sp[s].loopseq->eventblock;

			while (sp[s].loopseq && (verschobenerTakt(s, sp[s].loopseq->start + sp[s].loopevbl->event[sp[s].loopevnum].zeit) < loop.start)) {
				sp[s].loopevnum++;
				if (sp[s].loopevnum == EVENTS) {
					sp[s].loopevbl = sp[s].loopevbl->next;
					sp[s].loopevnum = 0;
				}
				if (!sp[s].loopevbl || !sp[s].loopevbl->event[sp[s].loopevnum].status) {
					sp[s].loopseq = sp[s].loopseq->next;
					if (sp[s].loopseq) {
						sp[s].loopevbl = sp[s].loopseq->eventblock;
					} else {
						sp[s].loopevbl = NULL;
						break;
					}
					sp[s].loopevnum = 0;
				}
			}
		}
	}
}

void ResetteLoopZeit(void) {
	if (loop.start < 0) { // Absicherung gegen alte Projekte
		loop.start = 0;
		loop.ende = 4 << VIERTEL;
	}
	lzeit.ltmark = TaktMarker(NULL, M_TEMPO, loop.start);
	lzeit.lkmark = TaktMarker(NULL, M_TAKT, loop.start);
	lzeit.lxmark = TaktMarker(NULL, M_TEXT, loop.start);

	lzeit.starttakt = loop.start - lzeit.ltmark->takt;
	lzeit.delay = 60000000L / lzeit.ltmark->m_bpm / VIERTELWERT;
}

void SpurAbspielen(WORD s, LONG *nexteventtakt) {
	MidiMsg msg;
	UBYTE p = spur[s].port;
	UBYTE c = spur[s].channel;
	LONG eventtakt;

	if (spur[s].aktseq && midiout[p]) {
		if (takt >= spur[s].aktseq->start) {
			if ((spur[s].bank0 != kt[p][c].aktbank0)
			   || (spur[s].bank32 != kt[p][c].aktbank32)
			   || (spur[s].prog != kt[p][c].aktprog))
			   SendeInstrument(s);
		}

		do {
			eventtakt = verschobenerTakt(s, spur[s].aktseq->start + spur[s].aktevbl->event[spur[s].aktevnum].zeit);
			if ((eventtakt < *nexteventtakt) && (eventtakt > takt)) *nexteventtakt = eventtakt;
			if (eventtakt > takt) break;

			msg.mm_Status = spur[s].aktevbl->event[spur[s].aktevnum].status;
			if (c < 16) {
				msg.mm_Status = msg.mm_Status & MS_StatBits;
				msg.mm_Status = msg.mm_Status | c;
			} else {
				c = msg.mm_Status & MS_ChanBits;
			}

			if (!mpkanal[p][c].mute && !spur[s].mute && !spur[s].aktseq->mute) { // Mutes testen
				if ((msg.mm_Status & MS_StatBits) <= MS_NoteOn) {
					msg.mm_Data1 = spur[s].aktevbl->event[spur[s].aktevnum].data1 + spur[s].aktseq->trans;
				} else {
					msg.mm_Data1 = spur[s].aktevbl->event[spur[s].aktevnum].data1;
				}
				msg.mm_Data2 = spur[s].aktevbl->event[spur[s].aktevnum].data2;
				PutMidi(midiout[p], msg.mm_Msg);

				if ((msg.mm_Status & MS_StatBits) == MS_NoteOn) {
					kt[p][c].note[msg.mm_Data1] = 1;
					SetzeMeter(p, c, msg.mm_Data2);
				}
				if ((msg.mm_Status & MS_StatBits) == MS_NoteOff) kt[p][c].note[msg.mm_Data1] = 0;
				if ((msg.mm_Status & MS_StatBits) == MS_Ctrl) ControllerAnpassen(p, c, msg.mm_Data1, msg.mm_Data2);
			}

			spur[s].aktevnum++;
			if (spur[s].aktevnum == EVENTS) {
				spur[s].aktevbl = spur[s].aktevbl->next;
				spur[s].aktevnum = 0;
			}
			if (!spur[s].aktevbl || !spur[s].aktevbl->event[spur[s].aktevnum].status) {
				spur[s].aktseq = spur[s].aktseq->next;
				if (spur[s].aktseq) {
					spur[s].aktevbl = spur[s].aktseq->eventblock;
				} else {
					spur[s].aktevbl = NULL;
					break;
				}
				spur[s].aktevnum = 0;
			}
		} while(TRUE);
	}
}

void KanalAbklingen(BYTE port, BYTE channel) {
	UBYTE n;

	if (midiout[port]) {
		for (n = 0; n < 128; n++) {
			if (kt[port][channel].note[n]) {
				kt[port][channel].note[n] = FALSE;
				SendeKanalEvent(port, channel, MS_NoteOff, n, 0);
			}
		}
	}
}

void SpurAbklingen(WORD s) {
	KanalAbklingen(spur[s].port, spur[s].channel);
}

void SpieleMetronom(void) {
	MidiMsg msg;

	LONG t = vorgeschobenerPortTakt(metro.port, takt);

	if (midiout[metro.port]) {
		if (t >> metro.raster != altmetro) {
			if (((t - lkmark->takt) >> VIERTEL) % lkmark->m_zaehler == 0) {
				msg.mm_Data1 = metro.taste1;
				msg.mm_Data2 = metro.velo1;
			} else {
				msg.mm_Data1 = metro.taste2;
				msg.mm_Data2 = metro.velo2;
			}

			msg.mm_Status = MS_NoteOn;
			if (metro.channel < 16) msg.mm_Status |= metro.channel;
			PutMidi(midiout[metro.port], msg.mm_Msg);

			msg.mm_Status = MS_NoteOff;
			if (metro.channel < 16) msg.mm_Status |= metro.channel;
			PutMidi(midiout[metro.port], msg.mm_Msg);

			altmetro = t >> metro.raster;
		}
	}
}

void TesteMetronom(BYTE taste) {
	MidiMsg msg;

	if (midiout[metro.port]) {
		msg.mm_Status = MS_NoteOn | metro.channel;
		if (taste == 1) {
			msg.mm_Data1 = metro.taste1;
			msg.mm_Data2 = metro.velo1;
		} else {
			msg.mm_Data1 = metro.taste2;
			msg.mm_Data2 = metro.velo2;
		}
		PutMidi(midiout[metro.port], msg.mm_Msg);

		msg.mm_Status = MS_NoteOff | metro.channel;
		PutMidi(midiout[metro.port], msg.mm_Msg);
	}
}

// =========== Prozesse ==============

void TesteMidiThru(void) {
	MidiMsg msg;
	UBYTE p = spur[snum].port;

	while (GetMidi(midi, &msg)) {
		//printf("Bekam: %X %d %d\n", msg.mm_Status, msg.mm_Data1, msg.mm_Data2);
		if (msg.mm_Status >= MS_RealTime) { //Real Time Messages

			//noch nix

		} else if (msg.mm_Status >= MS_System) { //System Messages
			if (msg.mm_Status == MS_SysEx) {
				if (sysexrec) {
					SysExAufnehmen();
					playsigaktion = 1;
					Signal(hornytask, 1L << playsig);
				} else {
					SkipSysEx(midi);
				}
			}
		} else { //Channel Voice Messages
			if (midiout[p] && outport[p].thru) {
				msg.mm_Status = (msg.mm_Status & MS_StatBits) | spur[snum].channel;
				// Controller Wandler...
				if ((msg.mm_Status & MS_StatBits) == MS_Ctrl) msg.mm_Data1 = WandleController(msg.mm_Data1);

				PutMidi(midiout[p], msg.mm_Msg);
				//printf("Taste: %d\n", msg.mm_Data1);
			}
		}
	}
}

void ThruProcess(void) {
	int n = 0;
	thruprocsig = AllocSignal(-1);
	midisig = AllocSignal(-1);
	do {
		Wait((1L << midisig) | (1L << thruprocsig));
		if (hornystatus == STATUS_STOP || hornystatus == STATUS_PLAY) TesteMidiThru();
	} while (hornystatus != STATUS_ENDE);
	if (thruprocsig != -1) FreeSignal(thruprocsig);
	if (midisig != -1) FreeSignal(midisig);
}

void ShortDelay(ULONG val) {
	struct MsgPort *ReplyMP2 = CreatePort( (STRPTR)NULL, 0 );
	struct timerequest *TimerIO2 = NULL;

	if (ReplyMP2) {
		TimerIO2 = (struct timerequest *)CreateIORequest( ReplyMP2, sizeof( struct timerequest) );
		if (TimerIO2) {
			if (OpenDevice( "timer.device", UNIT_MICROHZ, (struct IORequest *) TimerIO2, 0) == 0) {
				TimerIO2->tr_node.io_Command = TR_ADDREQUEST; /* Add a request.   */
				TimerIO2->tr_time.tv_secs = 0;                /* 0 seconds.      */
				TimerIO2->tr_time.tv_micro = val;             /* 'val' micro seconds. */
				DoIO( (struct IORequest *) TimerIO2 );
				CloseDevice( (struct IORequest *) TimerIO2 );
			}
			DeleteIORequest( (struct IORequest *) TimerIO2);
		}
		DeletePort( ReplyMP2);
	}
}

void PlayerProcess(void) {
	WORD s;
	LONG alttakt = 0;
	BOOL midiaufgenommen;
	LONG nexteventtakt;
	LONG warte;

	playerprocsig = AllocSignal(-1);
	do {
		while (hornystatus == STATUS_STOP || hornystatus == STATUS_UNINIT) Wait(1L << playerprocsig);
		
		if (hornystatus == STATUS_PLAY) {
			for (s = 0; s < lied.spuranz; s++) {
				AbspielenVorbereiten(s);
				LoopVorbereiten(s);
				AutomationVorbereiten(spur[s].port, spur[s].channel, takt);
				LoopAutomationVorbereiten(spur[s].port, spur[s].channel, takt);
			}
			ResetteLoopZeit();
			ResetteZeit();
			tmarkwechsel = TRUE;			
			do {
				AktualisiereTakt();
				if (loop.aktiv && (takt >= loop.ende)) {
					ltmark = lzeit.ltmark;
					lkmark = lzeit.lkmark;
					lxmark = lzeit.lxmark;
					startzeit = zeit;
					starttakt = lzeit.starttakt;
					delay = lzeit.delay;
					AktualisiereTakt();
					for (s = 0; s < lied.spuranz; s++) {
						SpurAbklingen(s);
						spur[s].aktseq = sp[s].loopseq;
						spur[s].aktevbl = sp[s].loopevbl;
						spur[s].aktevnum = sp[s].loopevnum;
						LoopAutomationResetten(spur[s].port, spur[s].channel);
					}
					tmarkwechsel = TRUE;
				}
				if (metro.play) SpieleMetronom();

				nexteventtakt = 0x7FFFFFFF;
				for(s = 0; s < lied.spuranz; s++) SpieleAutomation(spur[s].port, spur[s].channel, takt);
				for(s = 0; s < lied.spuranz; s++) SpurAbspielen(s, &nexteventtakt);
				warte = nexteventtakt - takt;

				if ((takt >> (VIERTEL - 3)) != alttakt) {
					alttakt = takt >> (VIERTEL - 3);
					Signal(hornytask, 1L << playsig);
				}

				if (warte > (VIERTELWERT >> 3)) warte = (VIERTELWERT >> 3) * delay;
				else warte = warte * delay;

				ShortDelay(warte/2);

			} while (hornystatus > STATUS_STOP);
			for (s = 0; s < lied.spuranz; s++) SpurAbklingen(s);
		} else if (hornystatus == STATUS_REC) {
			midiaufgenommen = FALSE;
			for(s = 0; s < lied.spuranz; s++) {
				if (s != snum) {
					AbspielenVorbereiten(s);
					LoopVorbereiten(s);
					AutomationVorbereiten(spur[s].port, spur[s].channel, takt);
					LoopAutomationVorbereiten(spur[s].port, spur[s].channel, takt);
				}
			}
			ResetteLoopZeit();
			ResetteZeit();
			tmarkwechsel = TRUE;
			do {
				AktualisiereTakt();
				if (loop.aktiv && (takt >= loop.ende) && !midiaufgenommen) {
					ltmark = lzeit.ltmark;
					lkmark = lzeit.lkmark;
					lxmark = lzeit.lxmark;
					startzeit = zeit;
					starttakt = lzeit.starttakt;
					delay = lzeit.delay;
					AktualisiereTakt();
					for (s = 0; s < lied.spuranz; s++) {
						if (s != snum) {
							SpurAbklingen(s);
							spur[s].aktseq = sp[s].loopseq;
							spur[s].aktevbl = sp[s].loopevbl;
							spur[s].aktevnum = sp[s].loopevnum;
						}
						LoopAutomationResetten(spur[s].port, spur[s].channel);
					}
					tmarkwechsel = TRUE;
				}
				if (metro.rec) SpieleMetronom();

				nexteventtakt = 0x7FFFFFFF;
				for(s = 0; s < lied.spuranz; s++) SpieleAutomation(spur[s].port, spur[s].channel, takt);
				for(s = 0; s < lied.spuranz; s++) {
					if (s != snum) SpurAbspielen(s, &nexteventtakt);
				}
				if (SpurAufnehmen(snum)) midiaufgenommen = TRUE;

				if ((takt >> (VIERTEL - 3)) != alttakt) {
					alttakt = takt >> (VIERTEL - 3);
					Signal(hornytask, 1L << playsig);
				}
				
				ShortDelay(delay);
				
			} while (hornystatus > STATUS_STOP);
			for (s = 0; s < lied.spuranz; s++) SpurAbklingen(s);
		}
	} while (hornystatus != STATUS_ENDE && hornystatus != STATUS_UNINIT);

	if (playerprocsig != -1) FreeSignal(playerprocsig);
}
