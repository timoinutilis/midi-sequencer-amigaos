#define __USE_OLD_TIMEVAL__ 1

#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/camd.h>
#include <proto/eXtreamSync.h>
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
#include "Gui2.h"

struct Library *CamdBase = NULL;

struct MsgPort *TimerMP = NULL;
struct timerequest *TimerIO = NULL;
struct Library *TimerBase = NULL;
struct TimerIFace *ITimer = NULL;
struct CamdIFace *ICamd = NULL;
struct Library *eXtreamSyncBase = NULL;
struct eXtreamSyncIFace *IeXtreamSync = NULL;

extern struct Window *edfenster;
extern struct Process *thruproc;
struct MidiNode *midi = NULL;
struct MidiLink *midiout[OUTPORTS];
struct MidiLink *midiin[INPORTS];
struct OUTPORT outport[OUTPORTS];
struct INPORT inport[INPORTS];

struct EClockVal eclock;
uint32 efreq = 0;
struct TimeVal warteende;

struct Task *hornytask = NULL;

struct SPUR spur[SPUREN];
struct SPURTEMP sp[SPUREN];
struct KANALTEMP kt[16][16];
extern struct MPKANAL mpkanal[OUTPORTS][16];
extern struct UMGEBUNG umgebung;

int32 takt = 0;
int32 tick = 0;

int32 starttakt = 0;
uint32 zeit = 0;
uint32 startzeit = 0;
uint32 starthiclock = 0;

int16 altmetro = 0;
int8 midisig = -1;
int8 playsig = -1;
int8 playerprocsig = -1;
int8 thruprocsig = -1;
int32 delay = 0;
struct METRONOM metro = {
	0, 9, // port, channel
	75, 76, // taste1, taste2
	127, 127, // velo1, velo2
	VIERTEL, //raster
	TRUE, FALSE // rec, play
};
int8 hornystatus = STATUS_UNINIT;
BOOL tmarkwechsel = FALSE;
struct MARKER *ltmark = NULL;
struct MARKER *lkmark = NULL;
struct MARKER *lxmark = NULL;
struct LOOPZEIT lzeit;
uint8 playsigaktion = 0;

extern struct MARKER *rootmark;
extern struct LIED lied;
extern struct LOOP loop;
extern int16 snum;
extern BOOL sysexrec;

struct MsgPort *syncport = NULL;
BOOL syncaktiv = FALSE;
//--------------------------------------------------------------------------------------------------

void InitOutPortInstr(int8 n) {
	int8 i;
	for (i = 0; i < 4; i++) strcpy(outport[n].outinstr[i].name, "???");
	outport[n].outinstr[0].unten = 0;
	for (i = 1; i < 4; i++) {
		outport[n].outinstr[i].unten = 16;
	}
}

void ErstelleCamd(void) {
	int8 n;

	CamdBase = IExec->OpenLibrary("camd.library", 37);
	if (CamdBase) {
		ICamd = (struct CamdIFace *)IExec->GetInterface(CamdBase, "main", 1, NULL);
		if (!ICamd) Meldung((STRPTR)"Could not obtain camd interface");
	} else Meldung((STRPTR)"Could not open camd.library");

	TimerMP = IExec->AllocSysObject(ASOT_PORT, NULL);
	if (TimerMP) {
		TimerIO = IExec->AllocSysObjectTags(ASOT_IOREQUEST,
			ASOIOR_Size, sizeof(struct timerequest),
			ASOIOR_ReplyPort, TimerMP,
			TAG_DONE);

		if (TimerIO) {
			if (IExec->OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIO, 0L) == 0) {
				TimerBase = (struct Library *)TimerIO->tr_node.io_Device;
				ITimer = (struct TimerIFace *)IExec->GetInterface(TimerBase, "main", 1, NULL);
				if (!ITimer) Meldung((STRPTR)"Could not obtain timer interface");
			} else Meldung((STRPTR)"Could not open timer device");
		} else Meldung((STRPTR)"Could not allocate TimerIO");
	} else Meldung((STRPTR)"Could not allocate TimerMP");

	if (CamdBase) {
		if (ICamd) {

			while (midisig == -1) {
				IDOS->Delay(10);
			}
			
			midi = ICamd->CreateMidi(
				MIDI_MsgQueue, 2048,
				MIDI_SysExSize, umgebung.sysexpuffer * 1024,
				MIDI_TimeStamp, &takt,
				MIDI_SignalTask, &thruproc->pr_Task,
				MIDI_RecvSignal, midisig,
				MIDI_ClientType, CCType_Sequencer,
				TAG_DONE);
			playsig = IExec->AllocSignal(-1);
			hornytask = IExec->FindTask(NULL);
		}
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
	
	eXtreamSyncBase = IExec->OpenLibrary("eXtreamSync.library", 0);
	if (eXtreamSyncBase) {
		IeXtreamSync = (struct eXtreamSyncIFace *)IExec->GetInterface(eXtreamSyncBase, "main", 1, NULL);
		if (IeXtreamSync) {
			syncport = IExec->AllocSysObjectTags(ASOT_PORT, TAG_END);
		} else Meldung((STRPTR)"Could not obtain eXtreamSync interface");
	};

}

void EntferneCamd(void) {
	DeaktiviereExtreamSync();
	IExec->FreeSysObject(ASOT_PORT, syncport);
	IExec->DropInterface((struct Interface *)IeXtreamSync);
	IExec->CloseLibrary(eXtreamSyncBase);

	if (CamdBase) ICamd->DeleteMidi(midi);
	IExec->DropInterface((struct Interface *)ICamd);
	IExec->CloseLibrary(CamdBase);

	if (!IExec->CheckIO((struct IORequest *)TimerIO)) {
		IExec->AbortIO((struct IORequest *)TimerIO);
		IExec->WaitIO((struct IORequest *)TimerIO);
	}
	IExec->DropInterface((struct Interface *)ITimer);
	IExec->CloseDevice((struct IORequest *)TimerIO);
	IExec->FreeSysObject(ASOT_IOREQUEST, TimerIO);
	IExec->FreeSysObject(ASOT_PORT, TimerMP);

	if(-1 != playsig)
	{
		IExec->FreeSignal(playsig);
		playsig = -1;
	}
}

void WarteStart(int16 s) {
	struct TimeVal warte = {s, 0};
	
	ITimer->GetSysTime(&warteende);
	ITimer->AddTime(&warteende, &warte);
}

void WarteEnde(void) {
	struct TimeVal aktzeit;
	
	do {
		ITimer->GetSysTime(&aktzeit);
		if (ITimer->CmpTime(&aktzeit, &warteende) == -1) break;
		IDOS->Delay(10);
	} while (TRUE);
}

void EntferneLinks(void) {
    int8 l;

	for (l = 0; l < verINPORTS; l++) {
		if (midiin[l]) {
			ICamd->RemoveMidiLink(midiin[l]);
			midiin[l] = NULL;
		}
		inport[l].name[0] = 0;
	}
	for (l = 0; l < verOUTPORTS; l++) {
		if (midiout[l]) {
			ICamd->RemoveMidiLink(midiout[l]);
			midiout[l] = NULL;
		}
		outport[l].name[0] = 0;
	}
}

void ErneuereLinks(void) {
    int8 l;
	char linkname[32];

	for (l = 0; l < verINPORTS; l++) {
		if (midiin[l]) {
			ICamd->RemoveMidiLink(midiin[l]);
			midiin[l] = NULL;
		}
	}
	for (l = 0; l < verOUTPORTS; l++) {
		if (midiout[l]) {
			ICamd->RemoveMidiLink(midiout[l]);
			midiout[l] = NULL;
		}
	}

	for (l = 0; l < verINPORTS; l++) {
		if (inport[l].name[0]) {
			sprintf(linkname, "horny%d.in", l);
			midiin[l] = ICamd->AddMidiLink(midi, MLTYPE_Receiver,
				MLINK_Name, linkname,
				MLINK_Location, inport[l].name,
				TAG_END);
		}
	}
	for (l = 0; l < verOUTPORTS; l++) {
		if (outport[l].name[0]) {
			sprintf(linkname, "horny%d.out", l);
			midiout[l] = ICamd->AddMidiLink(midi, MLTYPE_Sender,
				MLINK_Name, linkname,
				MLINK_Location, outport[l].name,
				TAG_END);
		}
	}
}

void ErstelleLinks(void) {
	struct MidiCluster *cluster = NULL;
	int8 lin = 0;
	int8 lout = 0;
	STRPTR name;
	APTR camdlock;

	camdlock = ICamd->LockCAMD(CD_Linkages);
	while ((cluster = ICamd->NextCluster(cluster))) {
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
	ICamd->UnlockCAMD(camdlock);
	ErneuereLinks();
}

BOOL LinkVorhanden(STRPTR name) {
	struct MidiCluster *cluster = NULL;
	APTR camdlock;
	BOOL vorhanden = FALSE;

	if (strcmp(name, "phonolith.thru") == 0) {
		return TRUE;
	}

	camdlock = ICamd->LockCAMD(CD_Linkages);
	while ((cluster = ICamd->NextCluster(cluster))) {
		if (strcmp(name, cluster->mcl_Node.ln_Name) == 0) {
			vorhanden = TRUE;
			break;
		}
	}
	ICamd->UnlockCAMD(camdlock);
	return(vorhanden);
}

void InitOutportLatenzen()
{
	int16 p;
	for (p = 0; p < verOUTPORTS; p++) {
		outport[p].latenz = 0;
	}
}

//--------------------------------------------------------------------------------

void AktiviereExtreamSync(void) {
	if (!syncaktiv) {
		if (syncport && IeXtreamSync->Connect(syncport) == 0) {
			syncaktiv = TRUE;
		} else Meldung((STRPTR)"Could not connect to eXtreamSync");
	}
}

void DeaktiviereExtreamSync(void) {
	if (syncaktiv) {
		IeXtreamSync->Disconnect(syncport);
		syncaktiv = FALSE;
	}
}

BOOL IstExtreamSyncAktiv(void) {
	return syncaktiv;
}

void KontrolleExtreamSync(void) {
	struct eXtreamSyncStandardMessage *mes;
	DOUBLE seconds;
	
	while ((mes = (struct eXtreamSyncStandardMessage*)IExec->GetMsg(syncport))) {

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
			takt = SmpteTicksTakt((int32)(seconds * 600));
			tick = TaktSmpteTicks(takt);
			ResetteLMarker();
			ZeichneAnzeigen(TRUE);
			ZeichnePosition(FALSE);
			break;

			case SYNC_PreLoad:
			IeXtreamSync->PreloadReady(syncport, mes->SystemMsg.mn_ReplyPort);
			break;

			default:
			break;
		}
	}
}

void StartExtreamSync(void) {
	struct eXtreamSyncStandardMessage *mes;
	BOOL weiter = FALSE;
	
	if (syncaktiv) {
		LocateExtreamSync();
		
		IeXtreamSync->Start(syncport);
		
		do {
			IExec->WaitPort(syncport);
			while ((mes = (struct eXtreamSyncStandardMessage*)IExec->GetMsg(syncport))) {
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
}

void StopExtreamSync(void) {
	if (syncaktiv) {
		IeXtreamSync->Stop(syncport);
	}
}

void LocateExtreamSync(void) {
	DOUBLE timecode;
	int32 timeticks;
	
	if (syncaktiv) {
		timeticks = TaktSmpteTicks(takt);
		timecode = (DOUBLE)timeticks / 600;
		IeXtreamSync->Locate(syncport, timecode);
	}
}

//--------------------------------------------------------------------------------

void ResetteLMarker(void) {
	ltmark = TaktMarker(NULL, M_TEMPO, takt);
	lkmark = TaktMarker(NULL, M_TAKT, takt);
	lxmark = TaktMarker(NULL, M_TEXT, takt);
}

void ResetteZeit(void) {
	efreq = ITimer->ReadEClock(&eclock) / 1200;
	startzeit = eclock.ev_lo / efreq;
	starthiclock = eclock.ev_hi;

	starttakt = takt - ltmark->takt;

	ResetteLMarker();

	zeit = startzeit;
	delay = 60000000L / ltmark->m_bpm / VIERTELWERT;
}

void AktualisiereTakt(void) {
	uint32 z;
	struct MARKER *next;

	ITimer->ReadEClock(&eclock);
	zeit = eclock.ev_lo / efreq;
	zeit += (eclock.ev_hi - starthiclock) * ((uint32)0xFFFFFFFF / efreq);
	if (zeit < startzeit) Meldung((STRPTR)"Internal Time Error");

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

void SendeEvent(int16 s, uint8 status, uint8 data1, uint8 data2) {
	MidiMsg msg;

	if (midiout[spur[s].port]) {
		msg.mm_Status = status;
		if (spur[s].channel < 16) msg.mm_Status = (msg.mm_Status & MS_StatBits) | spur[s].channel;
		msg.mm_Data1 = data1;
		msg.mm_Data2 = data2;
		ICamd->PutMidi(midiout[spur[s].port], msg.mm_Msg);
	}
}

void SendeKanalEvent(int8 port, int8 channel, uint8 status, uint8 data1, uint8 data2) {
	MidiMsg msg;

	if (midiout[port]) {
		msg.mm_Status = status;
		msg.mm_Status = (msg.mm_Status & MS_StatBits) | channel;
		msg.mm_Data1 = data1;
		msg.mm_Data2 = data2;
		ICamd->PutMidi(midiout[port], msg.mm_Msg);
	}
}

void SetzeAktInstrument(int8 p, int8 c, int8 bank0, int8 bank32, int8 prog) {
	kt[p][c].aktbank0 = bank0;
	kt[p][c].aktbank32 = bank32;
	kt[p][c].aktprog = prog;
}

void SendeKanalInstrument(int8 p, int8 c, int8 bank0, int8 bank32, int8 prog) {
	if (bank0 >= 0) SendeKanalEvent(p, c, MS_Ctrl, MC_Bank, bank0);
	if (bank32 >= 0) SendeKanalEvent(p, c, MS_Ctrl, MC_Bank + 0x20, bank32);
	SendeKanalEvent(p, c, MS_Prog, prog, 0);
}

void SendeInstrument(int16 s) {
	uint8 p = spur[s].port;
	uint8 c = spur[s].channel;

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
	int8 p, c;

	for (p = 0; p < verOUTPORTS; p++) {
		if (midiout[p]) {
			for (c = 0; c < 16; c++) SendeKanalEvent(p, c, MS_Ctrl, MM_AllOff, 0);
		}
	}
}

BOOL AddEvent(int16 s, int32 t, uint8 status, uint8 data1, uint8 data2) {
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

int32 verschobenerTakt(int16 s, int32 zeit)
{
	return verschobenerPortTakt(spur[s].port, zeit) + spur[s].shift;
}

int32 verschobenerPortTakt(int16 p, int32 zeit)
{
	int32 shift = (outport[p].latenz * (ltmark->m_bpm << VIERTEL) / 60000L);
	return zeit - shift;
}

int32 vorgeschobenerPortTakt(int16 p, int32 zeit)
{
	int32 shift = (outport[p].latenz * (ltmark->m_bpm << VIERTEL) / 60000L);
	return zeit + shift;
}

int32 aufnahmeLatenzAusgleich(int16 s, int32 zeit)
{
	int32 shift = (outport[spur[s].port].latenz * (ltmark->m_bpm << VIERTEL) / 60000L);
	return zeit + shift;
}


BOOL SpurAufnehmen(int16 s) {
	MidiMsg msg;
	uint8 p = spur[s].port;

	if (ICamd->GetMidi(midi, &msg)) {
		if (ICamd->MidiMsgType(&msg) < CMB_RealTime) {
			// Controller Wandler...
			if ((msg.mm_Status & MS_StatBits) == MS_Ctrl) msg.mm_Data1 = WandleController(msg.mm_Data1);

			AddEvent(s, aufnahmeLatenzAusgleich(s, msg.mm_Time), msg.mm_Status, msg.mm_Data1, msg.mm_Data2);
			if (midiout[p]) {
				if (spur[s].channel < 16) msg.mm_Status = (msg.mm_Status & MS_StatBits) | spur[s].channel;
				ICamd->PutMidi(midiout[p], msg.mm_Msg);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

void AbspielenVorbereiten(int16 s) {
	struct EVENT *event;
	MidiMsg msg;
	uint8 n;
	uint8 p = spur[s].port;
	uint8 c = spur[s].channel;

	if (midiout[p]) {
		msg.mm_Status = MS_PitchBend | c;
		msg.mm_Data1 = 0;
		msg.mm_Data2 = 64;
		ICamd->PutMidi(midiout[p], msg.mm_Msg);
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
							ICamd->PutMidi(midiout[p], msg.mm_Msg);
						}
					}
				}
			}
			
		}
	}
}

void LoopVorbereiten(int16 s) {
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

void SpurAbspielen(int16 s, int32 *nexteventtakt) {
	MidiMsg msg;
	uint8 p = spur[s].port;
	uint8 c = spur[s].channel;
	int32 eventtakt;

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
				ICamd->PutMidi(midiout[p], msg.mm_Msg);

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

void KanalAbklingen(int8 port, int8 channel) {
	uint8 n;

	if (midiout[port]) {
		for (n = 0; n < 128; n++) {
			if (kt[port][channel].note[n]) {
				kt[port][channel].note[n] = FALSE;
				SendeKanalEvent(port, channel, MS_NoteOff, n, 0);
			}
		}
	}
}

void SpurAbklingen(int16 s) {
	KanalAbklingen(spur[s].port, spur[s].channel);
}

void SpieleMetronom(void) {
	MidiMsg msg;

	int32 t = vorgeschobenerPortTakt(metro.port, takt);

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
			ICamd->PutMidi(midiout[metro.port], msg.mm_Msg);

			msg.mm_Status = MS_NoteOff;
			if (metro.channel < 16) msg.mm_Status |= metro.channel;
			ICamd->PutMidi(midiout[metro.port], msg.mm_Msg);

			altmetro = t >> metro.raster;
		}
	}
}

void TesteMetronom(int8 taste) {
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
		ICamd->PutMidi(midiout[metro.port], msg.mm_Msg);

		msg.mm_Status = MS_NoteOff | metro.channel;
		ICamd->PutMidi(midiout[metro.port], msg.mm_Msg);
	}
}

// =========== Prozesse ==============

void TesteMidiThru(void) {
	MidiMsg msg;
	uint8 p = spur[snum].port;

	while (ICamd->GetMidi(midi, &msg)) {
		//printf("Bekam: %X %d %d\n", msg.mm_Status, msg.mm_Data1, msg.mm_Data2);
		if (msg.mm_Status >= MS_RealTime) { //Real Time Messages

			//noch nix

		} else if (msg.mm_Status >= MS_System) { //System Messages
			if (msg.mm_Status == MS_SysEx) {
				if (sysexrec) {
					SysExAufnehmen();
					playsigaktion = 1;
					IExec->Signal(hornytask, 1L << playsig);
				} else {
					ICamd->SkipSysEx(midi);
				}
			}
		} else { //Channel Voice Messages
			if (midiout[p] && outport[p].thru) {
				msg.mm_Status = (msg.mm_Status & MS_StatBits) | spur[snum].channel;
				// Controller Wandler...
				if ((msg.mm_Status & MS_StatBits) == MS_Ctrl) msg.mm_Data1 = WandleController(msg.mm_Data1);

				ICamd->PutMidi(midiout[p], msg.mm_Msg);
				//printf("Taste: %d\n", msg.mm_Data1);
			}
		}
	}
}

void ThruProcess(void) {
	thruprocsig = IExec->AllocSignal(-1);
	midisig = IExec->AllocSignal(-1);
	do {
		IExec->Wait((1L << midisig) | (1L << thruprocsig));
		if (hornystatus == STATUS_STOP || hornystatus == STATUS_PLAY) TesteMidiThru();
	} while (hornystatus != STATUS_ENDE);
	if (thruprocsig != -1) IExec->FreeSignal(thruprocsig);
	if (midisig != -1) IExec->FreeSignal(midisig);
}

void ShortDelay(uint32 val) {
//	struct MsgPort *ReplyMP2 = CreatePort( (STRPTR)NULL, 0 );
	struct MsgPort *ReplyMP2 = IExec->AllocSysObjectTags( ASOT_PORT, TAG_END );
	struct TimeRequest *TimerIO2 = NULL;

	if (ReplyMP2) {
//		TimerIO2 = (struct timerequest *)CreateIORequest( ReplyMP2, sizeof( struct timerequest) );
		TimerIO2 = (struct TimeRequest *)IExec->AllocSysObjectTags( ASOT_IOREQUEST,ASOIOR_ReplyPort, ReplyMP2, ASOIOR_Size, sizeof( struct TimeRequest) ,TAG_END);
		if (TimerIO2) {
			if (IExec->OpenDevice( "timer.device", UNIT_MICROHZ, (struct IORequest *) TimerIO2, 0) == 0) {
				TimerIO2->Request.io_Command = TR_ADDREQUEST; /* Add a request.   */
				TimerIO2->Time.Seconds = 0;                /* 0 seconds.      */
				TimerIO2->Time.Microseconds = val;             /* 'val' micro seconds. */
				IExec->DoIO( (struct IORequest *) TimerIO2 );
				IExec->CloseDevice( (struct IORequest *) TimerIO2 );
			}
			IExec->FreeSysObject(ASOT_IOREQUEST, TimerIO2);
		}
		IExec->FreeSysObject(ASOT_PORT, ReplyMP2);
	}
}

void PlayerProcess(void) {
	int16 s;
	int32 alttakt = 0;
	BOOL midiaufgenommen;
	int32 nexteventtakt;
	int32 warte;

	playerprocsig = IExec->AllocSignal(-1);
	do {
		while (hornystatus == STATUS_STOP || hornystatus == STATUS_UNINIT) IExec->Wait(1L << playerprocsig);
		
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
					IExec->Signal(hornytask, 1L << playsig);
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
					IExec->Signal(hornytask, 1L << playsig);
				}
				
				ShortDelay(delay);
				
			} while (hornystatus > STATUS_STOP);
			for (s = 0; s < lied.spuranz; s++) SpurAbklingen(s);
		}
	} while (hornystatus != STATUS_ENDE && hornystatus != STATUS_UNINIT);

	if (playerprocsig != -1) IExec->FreeSignal(playerprocsig);
}
