#include <exec/types.h>

#define AREA_MARKER 1
#define AREA_ZEIT 2
#define AREA_GUISPALTE 3
#define AREA_SPUREN 4
#define AREA_SEQUENZEN 5
#define AREA_INFOBOX 6
#define AREA_UEBERSICHT 7

#define VIERTEL 8
#define VIERTELMASKE 0xFFFFFF00L
#define VIERTELWERT 256

#define EVENTS 64

#define STATUS_UNINIT -128
#define STATUS_STOP 0
#define STATUS_PLAY 1
#define STATUS_REC 2
#define STATUS_ENDE -1

struct EVENT {
	int32 zeit;
	uint8 status;
	uint8 data1;
	uint8 data2;
	BOOL markiert;
};

struct EVENTBLOCK {
	struct EVENT event[EVENTS];
	struct EVENTBLOCK *prev;
	struct EVENTBLOCK *next;
};

struct SEQUENZ {
	char name[128];
	int32 start;
	int32 ende;
	int8 trans;
	struct EVENTBLOCK *eventblock;
	BOOL markiert;
	struct SEQUENZ *aliasorig;
	int16 aliasanz;
	int16 spur;
	BOOL mute;
	struct SEQUENZ *next;
	struct SEQUENZ *speicheradr;
};

struct SPUR {
	char name[128];
	uint8 port;
	uint8 channel;
	int8 bank0;
	int8 bank32;
	int8 prog;
	int16 shift;
	BOOL mute;
	struct SEQUENZ *seq;
	struct SEQUENZ *aktseq;
	struct EVENTBLOCK *aktevbl;
	int16 aktevnum;
	uint8 autostatus; // 0=Nichts 1=Vol 2=Pan 3=Contr0...
};

struct SPURTEMP {
	int8 anders;
	struct SEQUENZ *loopseq;
	struct EVENTBLOCK *loopevbl;
	int16 loopevnum;
	struct SEQUENZ *neuseq;
};

struct KANALTEMP {
	uint8 note[128];
	int8 aktbank0;
	int8 aktbank32;
	int8 aktprog;
};

struct LIED {
	char name[128];
	uint8 spuranz;
	int16 taktanz;
	char phonolithprojekt[1024];
};

struct AUTOPUNKT {
	struct AUTOPUNKT *prev;
	struct AUTOPUNKT *next;
	int32 takt;
	int8 wert;
};

struct AUTOKANAL {
	struct AUTOPUNKT *liste[8]; // 0=Vol, 1=Pan, 2=Contr0...
	struct AUTOPUNKT *aktpunkt[8];
	struct AUTOPUNKT *looppunkt[8];
};

struct GUI {
	int16 spur;
	int16 spsicht;
	int16 sph;
	int32 takt;
	int16 tasicht;
	int16 tab;
	int16 spalte;
	BOOL folgen;
};

struct LOOP {
	int32 start;
	int32 ende;
	BOOL aktiv;
};

struct METRONOM {
	uint8 port;
	uint8 channel;
	int8 taste1;
	int8 taste2;
	int8 velo1;
	int8 velo2;
	int16 raster;
	BOOL rec;
	BOOL play;
};

#define FPS_24   0
#define FPS_25   1
#define FPS_30   2

struct SMPTE {
	int32 startticks;
	int8 format;
};

#define M_TEMPO	0
#define M_TAKT		1
#define M_TEXT		2
#define m_bpm		d1
#define m_taktnum	d1
#define m_zaehler	d2

struct MARKER {
	struct MARKER *prev;
	struct MARKER *next;
	uint32 takt;
	int8 typ;
	uint16 d1;
	uint16 d2;
	uint32 ticks;
	char text;
};

struct LOOPZEIT {
	struct MARKER *ltmark;
	struct MARKER *lkmark;
	struct MARKER *lxmark;
	int32 starttakt;
	int32 delay;
};

#define TASTATUR_AMIGA 0
#define TASTATUR_PC 1

struct UMGEBUNG {
	BOOL wbscreen;
	uint32 screenmode;
	int16 scrbreite;
	int16 scrhoehe;
	uint8 scrtiefe;
	BOOL backdrop;
	STRPTR pfadproj;
	STRPTR pfadsmf;
	STRPTR pfadsysex;
	STRPTR pfadphonolith;
	STRPTR startproj;
	BOOL startaktiv;
	uint8 tastatur;
	int8 playerPri;
	int8 thruPri;
	int16 sysexpuffer;
	BOOL mausradtauschen;
};

#define EDMODUS_NOTEN 0
#define EDMODUS_CONTR 1

struct EDGUI {
	int8 modus;
	int16 taste;
	int16 tasth;
	int16 tastsicht;
	int16 contr;
	int16 contrh;
	int16 contrsicht;
	int16 contranz;
	int32 takt;
	int16 taktb;
	int16 taktsicht;
	uint8 raster;
	int8 neulen;
	BOOL tripled;
};

struct OUTINSTR {
	char name[128];
	int8 unten;
};

struct OUTPORT {
	char name[128];
	struct OUTINSTR outinstr[4];
	BOOL thru;
	int16 latenz;
};

struct INPORT {
	char name[128];
};

struct SYSEXMSG {
	char name[128];
	uint32 len;
	uint8 *data;
	struct SYSEXMSG *prev;
	struct SYSEXMSG *next;
};

struct SYSEXUNIT {
	char name[128];
	uint8 port;
	BOOL gesperrt;
	struct SYSEXMSG *sysex;
	struct SYSEXUNIT *prev;
	struct SYSEXUNIT *next;
};

struct PROGRAMM {
	char name[128];
	int8 bank0;
	int8 bank32;
	int8 prog;
	struct PROGRAMM *next;
};

struct KATEGORIE {
	char name[128];
	struct PROGRAMM *programm;
	int8 chanvon;
	int8 chanbis;
	struct KATEGORIE *next;
};

#define CONTR_MITTE 0x01
#define CONTR_ONOFF 0x02
#define CONTR_SET   0x04
struct INSTRCONTR {
	char name[128][128];
	uint8 flags[128];
};

struct INSTRUMENT {
	char name[128];
	struct KATEGORIE *kategorie;
	struct INSTRCONTR *contr;
	struct INSTRUMENT *next;
};

struct SEQUENZINFO {
	BOOL benutzt;
	char name[128];
	BOOL namemulti;
	int8 trans;
	BOOL transmulti;
	BOOL mute;
	BOOL mutemulti;
	int16 aliasanz;
};

struct MPDATA {
	int16 kanalanz;
	int16 kanalerst;
	int16 kanalsicht;
};

struct MPKANALNUM {
	int8 port;
	int8 channel;
};

struct MPKANAL {
	int8 contr[6];
	int8 contrwert[6];
	int8 pan;
	int8 fader;
	BOOL mute;
	int8 meter;
	int16 bezspur[3];
	BOOL autoupdate; // Automation GUI Update
	uint8 updateflags;
};
