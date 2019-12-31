#include <stdio.h>
#include <string.h>

#include <exec/exec.h>
#include <midi/mididefs.h>
#include <midi/camd.h>
#include <libraries/realtime.h>

#include <proto/exec.h>
#include <proto/camd.h>
#include <proto/realtime.h>

#include "Strukturen.h"
#include "Requester.h"

struct Library *CamdBase=NULL;
struct MidiNode *midi=NULL;
struct MidiLink *midiout[16];
struct MidiLink *midiin=NULL;
STRPTR midiname[16];

struct RealTimeBase *RealTimeBase=NULL;
struct Player *player=NULL;

struct SPUR spur[128];
struct SPURTEMP sp[128];
UBYTE notean[4][16][128];
LONG takt=0;
LONG zeit=0;
LONG altzeit=0;
struct SEQUENZ *neuseq=NULL;
WORD altmetro=0;
BYTE midisig=-1;
WORD bpm=120;
struct METRONOM metro={0, 9, 50, 127, 80, 10, TRUE, FALSE};

extern struct LIED lied;
extern struct LOOP loop;

void InitLied(void) {
	WORD s;
	UBYTE p;
	UBYTE n;

	for (s=0; s<128; s++) {
		strcpy(spur[s].name, "Unbenannt");
		spur[s].port=0;
		spur[s].channel=0;
		spur[s].bank=-1;
		spur[s].prog=0;
		spur[s].shift=0;
		spur[s].mute=FALSE;
		spur[s].seq=NULL;
		spur[s].aktseq=NULL;
		spur[s].aktevbl=NULL;
		spur[s].aktevnum=0;
	};

	for (p=0; p<4; p++) {
		for (s=0; s<16; s++) {
			for (n=0; n<128; n++) notean[p][s][n]=FALSE;
		};
	};

	strcpy(lied.name, "Unbenannt");
	lied.spuranz=1;
	lied.taktanz=100;
	bpm=120;
}

void ErstelleCamdRealTime(void) {
	WORD l;

	if (!(CamdBase=OpenLibrary("camd.library", 37))) Meldung("camd.library nicht geöffnet");
	if (!(RealTimeBase=(struct RealTimeBase *)OpenLibrary("realtime.library", 37))) Meldung("realtime.library nicht geöffnet");
	if (CamdBase && RealTimeBase) {
		midi=CreateMidi(MIDI_MsgQueue, 2048, MIDI_SysExSize, 0L, TAG_END);
		player=CreatePlayer(PLAYER_Name, "Sequencer", PLAYER_Conductor, ~0, TAG_DONE);
		SetConductorState(player, CONDSTATE_RUNNING, 0);
		midisig=AllocSignal(-1);
		SetMidiAttrs (midi,
			MIDI_TimeStamp, &takt,
			MIDI_RecvSignal, midisig,
			TAG_DONE);
	};
	for(l=0; l<16; l++) {
		midiout[l]=NULL;
		midiname[l]=NULL;
	};
}

void EntferneCamdRealTime(void) {
	if (RealTimeBase) DeletePlayer(player);
	if (CamdBase) DeleteMidi(midi);
	CloseLibrary((struct Library *)RealTimeBase);
	CloseLibrary(CamdBase);
	if (midisig!=-1) FreeSignal(midisig);
}

void EntferneLinks(void) {
	WORD l;

	RemoveMidiLink(midiin);
	for (l=0; l<16; l++) {
		if (!midiout[l]) break;
		RemoveMidiLink(midiout[l]);
		midiout[l]=NULL;
		midiname[l]=NULL;
	};
}

void ErstelleLinks(void) {
	struct MidiCluster *cluster=NULL;
	WORD l=0;
	STRPTR name;

	EntferneLinks();
	while (cluster=NextCluster(cluster)) {
		if (cluster->mcl_PublicParticipants) {
			name=(STRPTR)((ULONG)cluster+sizeof(struct MidiCluster));
			if (!midiin && (name[0]=='i')) {
				midiin=AddMidiLink(midi, MLTYPE_Receiver, MLINK_Name, "sequencer.in", MLINK_Location, "in.0", TAG_END);
				SetMidiLinkAttrs(midiin, MLINK_EventMask, CMF_Channel, TAG_DONE);
			};
			if ((l<16) && (name[0]=='o')) {
				midiout[l]=AddMidiLink(midi, MLTYPE_Sender, MLINK_Name, "sequencer.out", MLINK_Location, name, TAG_END);
				midiname[l]=name;
				l++;
			};
		};
	};
}

void ResetteZeit(void) {
	altzeit=player->pl_MetricTime;
}

void AktualisiereTakt(void) {
	LONG d;

	zeit=player->pl_MetricTime;
	d=zeit-altzeit; altzeit=zeit;
	takt=takt+((d<<10)*bpm/72000);
}

void TesteMidiThru(WORD s) {
	MidiMsg msg;

	while (GetMidi(midi, &msg)) {
		msg.mm_Status=(msg.mm_Status & MS_StatBits) | spur[s].channel;
		PutMidi(midiout[spur[s].port], msg.mm_Msg);
	};

}

void SendeInstrument(WORD s) {
	MidiMsg msg;
	UBYTE p=spur[s].port;
	UBYTE c=spur[s].channel;

	if (spur[s].bank>=0) {
		msg.mm_Status=MS_Ctrl | c;
		msg.mm_Data1=MC_Bank;
		msg.mm_Data2=spur[s].bank;
		PutMidi(midiout[p], msg.mm_Msg);

		msg.mm_Status=MS_Prog | c;
		msg.mm_Data1=spur[s].prog;
		msg.mm_Data2=0;
		PutMidi(midiout[p], msg.mm_Msg);
	};
}

void SpurAufnehmen(WORD s) {
	MidiMsg msg;
	struct SPUR *sp;

	if (GetMidi(midi, &msg)) {
		sp=&spur[s];

		msg.mm_Status=(msg.mm_Status & MS_StatBits) | sp->channel;
		PutMidi(midiout[sp->port], msg.mm_Msg);

		if (neuseq==NULL) {
			if (neuseq=AllocVec(sizeof(struct SEQUENZ), 0)) {
				strcpy(neuseq->name, sp->name);
				neuseq->start=msg.mm_Time & 0xFFFFFC00;
				neuseq->ende=msg.mm_Time;
				neuseq->trans=0;
				neuseq->eventblock=AllocVec(sizeof(struct EVENTBLOCK), MEMF_CLEAR);
				neuseq->markiert=FALSE;
				neuseq->aliasorig=NULL;
				neuseq->aliasanz=0;
				neuseq->next=NULL;
				sp->aktevbl=neuseq->eventblock;
				sp->aktevnum=0;
			};
		};

		if (neuseq && sp->aktevbl) {
			sp->aktevbl->event[sp->aktevnum].zeit=msg.mm_Time-(neuseq->start);
			sp->aktevbl->event[sp->aktevnum].status=msg.mm_Status;
			sp->aktevbl->event[sp->aktevnum].data1=msg.mm_Data1;
			sp->aktevbl->event[sp->aktevnum].data2=msg.mm_Data2;
			neuseq->ende=(msg.mm_Time+1023) & 0xFFFFFC00;

			sp->aktevnum++;
			if (sp->aktevnum==EVENTS) {
				sp->aktevbl->next=AllocVec(sizeof(struct EVENTBLOCK), MEMF_ANY | MEMF_CLEAR);
				sp->aktevbl->next->prev=sp->aktevbl;
				sp->aktevbl=sp->aktevbl->next;
				sp->aktevnum=0;
			};
		};
	};
}


void AbspielenVorbereiten(WORD s) {
	struct EVENT *event;
	MidiMsg msg;
	UBYTE n;
	UBYTE p=spur[s].port;
	UBYTE c=spur[s].channel;

	msg.mm_Status=MS_PitchBend | c;
	msg.mm_Data1=0;
	msg.mm_Data2=64;
	PutMidi(midiout[p], msg.mm_Msg);

	if (spur[s].seq) {
		spur[s].aktseq=spur[s].seq;
		spur[s].aktevbl=spur[s].aktseq->eventblock;
		spur[s].aktevnum=0;

		while ((spur[s].aktseq->ende + spur[s].shift < takt) && spur[s].aktseq) {
			spur[s].aktseq=spur[s].aktseq->next;
			spur[s].aktevbl=spur[s].aktseq->eventblock;
		};

		while (spur[s].aktseq && ((spur[s].aktseq->start + spur[s].aktevbl->event[spur[s].aktevnum].zeit + spur[s].shift) < takt)) {
			event=&spur[s].aktevbl->event[spur[s].aktevnum];

			if ((event->status & MS_StatBits)==MS_NoteOn) notean[p][c][event->data1 + spur[s].aktseq->trans]=event->data2;
			if ((event->status & MS_StatBits)==MS_NoteOff) notean[p][c][event->data1 + spur[s].aktseq->trans]=0;

			spur[s].aktevnum++;
			if (spur[s].aktevnum==EVENTS) {
				spur[s].aktevbl=spur[s].aktevbl->next;
				spur[s].aktevnum=0;
			};
			if (!spur[s].aktevbl || !spur[s].aktevbl->event[spur[s].aktevnum].status) {
				spur[s].aktseq=spur[s].aktseq->next;
				spur[s].aktevbl=spur[s].aktseq->eventblock;
				spur[s].aktevnum=0;
			};
		};


		for (n=0; n<128; n++) {
			if (notean[p][c][n]) {
				if (!spur[s].mute) {
					msg.mm_Status=MS_NoteOn | c;
					msg.mm_Data1=n;
					msg.mm_Data2=notean[p][c][n];
					PutMidi(midiout[p], msg.mm_Msg);
				};
			};
		};
	};
}

void LoopVorbereiten(WORD s) {
	if (spur[s].seq) {
		sp[s].loopseq=spur[s].seq;
		sp[s].loopevbl=spur[s].seq->eventblock;
		sp[s].loopevnum=0;

		while ((sp[s].loopseq->ende + spur[s].shift < loop.start) && sp[s].loopseq) {
			sp[s].loopseq=sp[s].loopseq->next;
			sp[s].loopevbl=sp[s].loopseq->eventblock;
		};

		while (sp[s].loopseq && ((sp[s].loopseq->start + sp[s].loopevbl->event[sp[s].loopevnum].zeit + spur[s].shift) < loop.start)) {
			sp[s].loopevnum++;
			if (sp[s].loopevnum==EVENTS) {
				sp[s].loopevbl=sp[s].loopevbl->next;
				sp[s].loopevnum=0;
			};
			if (!sp[s].loopevbl || !sp[s].loopevbl->event[sp[s].loopevnum].status) {
				sp[s].loopseq=sp[s].loopseq->next;
				sp[s].loopevbl=sp[s].loopseq->eventblock;
				sp[s].loopevnum=0;
			};
		};
	};
};

void SpurAbspielen(WORD s) {
	MidiMsg msg;
	UBYTE p=spur[s].port;
	UBYTE c=spur[s].channel;

	if (spur[s].aktseq) {
		while ((spur[s].aktseq->start + spur[s].aktevbl->event[spur[s].aktevnum].zeit + spur[s].shift) <= takt) {
			msg.mm_Status=spur[s].aktevbl->event[spur[s].aktevnum].status;
			msg.mm_Status=msg.mm_Status & MS_StatBits;
			msg.mm_Status=msg.mm_Status | c;

			if ((msg.mm_Status & MS_StatBits) <= MS_NoteOn) {
				msg.mm_Data1=spur[s].aktevbl->event[spur[s].aktevnum].data1 + spur[s].aktseq->trans;
			} else {
				msg.mm_Data1=spur[s].aktevbl->event[spur[s].aktevnum].data1;
			};
			msg.mm_Data2=spur[s].aktevbl->event[spur[s].aktevnum].data2;
			if (!spur[s].mute) PutMidi(midiout[p], msg.mm_Msg);

			if ((msg.mm_Status & MS_StatBits)==MS_NoteOn) notean[p][c][msg.mm_Data1]=1;
			if ((msg.mm_Status & MS_StatBits)==MS_NoteOff) notean[p][c][msg.mm_Data1]=0;

			spur[s].aktevnum++;
			if (spur[s].aktevnum==EVENTS) {
				spur[s].aktevbl=spur[s].aktevbl->next;
				spur[s].aktevnum=0;
			};
			if (!spur[s].aktevbl || !spur[s].aktevbl->event[spur[s].aktevnum].status) {
				if (spur[s].aktseq=spur[s].aktseq->next) {
					spur[s].aktevbl=spur[s].aktseq->eventblock;
				} else {
					spur[s].aktevbl=NULL;
					break;
				};
				spur[s].aktevnum=0;
			};
		};
	};
}

void SpurAbklingen(WORD s) {
	MidiMsg msg;
	UBYTE n;
	UBYTE p=spur[s].port;
	UBYTE c=spur[s].channel;

	msg.mm_Data2=0;
	for (n=0; n<128; n++) {
		if (notean[p][c][n]) {
			msg.mm_Status=MS_NoteOff | c;
			msg.mm_Data1=n;
			PutMidi(midiout[p], msg.mm_Msg);
			notean[p][c][n]=FALSE;
		};
	};
}

void SpieleMetronom(void) {
	MidiMsg msg;

	if (takt>>metro.raster!=altmetro) {
		msg.mm_Status=MS_NoteOn | metro.channel;
		msg.mm_Data1=metro.taste;
		msg.mm_Data2=metro.velo1;
		PutMidi(midiout[metro.port], msg.mm_Msg);

		msg.mm_Status=MS_NoteOff | metro.channel;
		PutMidi(midiout[metro.port], msg.mm_Msg);

		altmetro=takt>>metro.raster;
	};
}

