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
	LONG zeit;
	UBYTE status;
	UBYTE data1;
	UBYTE data2;
	BOOL markiert;
};

struct EVENTBLOCK {
	struct EVENT event[EVENTS];
	struct EVENTBLOCK *prev;
	struct EVENTBLOCK *next;
};

struct SEQUENZ {
	char name[128];
	LONG start;
	LONG ende;
	BYTE trans;
	struct EVENTBLOCK *eventblock;
	BOOL markiert;
	struct SEQUENZ *aliasorig;
	WORD aliasanz;
	WORD spur;
	BOOL mute;
	struct SEQUENZ *next;
	struct SEQUENZ *speicheradr;
};

struct SPUR {
	char name[128];
	UBYTE port;
	UBYTE channel;
	BYTE bank0;
	BYTE bank32;
	BYTE prog;
	WORD shift;
	BOOL mute;
	struct SEQUENZ *seq;
	struct SEQUENZ *aktseq;
	struct EVENTBLOCK *aktevbl;
	WORD aktevnum;
	UBYTE autostatus; // 0=Nichts 1=Vol 2=Pan 3=Contr0...
};

struct SPURTEMP {
	BYTE anders;
	struct SEQUENZ *loopseq;
	struct EVENTBLOCK *loopevbl;
	WORD loopevnum;
	struct SEQUENZ *neuseq;
};

struct KANALTEMP {
	UBYTE note[128];
	BYTE aktbank0;
	BYTE aktbank32;
	BYTE aktprog;
};

struct LIED {
	char name[128];
	UBYTE spuranz;
	WORD taktanz;
	char phonolithprojekt[1024];
};

struct AUTOPUNKT {
	struct AUTOPUNKT *prev;
	struct AUTOPUNKT *next;
	LONG takt;
	BYTE wert;
};

struct AUTOKANAL {
	struct AUTOPUNKT *liste[8]; // 0=Vol, 1=Pan, 2=Contr0...
	struct AUTOPUNKT *aktpunkt[8];
	struct AUTOPUNKT *looppunkt[8];
};

struct GUI {
	WORD spur;
	WORD spsicht;
	WORD sph;
	LONG takt;
	WORD tasicht;
	WORD tab;
	WORD spalte;
	BOOL folgen;
};

struct LOOP {
	LONG start;
	LONG ende;
	BOOL aktiv;
};

struct METRONOM {
	UBYTE port;
	UBYTE channel;
	BYTE taste1;
	BYTE taste2;
	BYTE velo1;
	BYTE velo2;
	WORD raster;
	BOOL rec;
	BOOL play;
};

#define FPS_24   0
#define FPS_25   1
#define FPS_30   2

struct SMPTE {
	LONG startticks;
	BYTE format;
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
	ULONG takt;
	BYTE typ;
	UWORD d1;
	UWORD d2;
	ULONG ticks;
	char text;
};

struct LOOPZEIT {
	struct MARKER *ltmark;
	struct MARKER *lkmark;
	struct MARKER *lxmark;
	LONG starttakt;
	LONG delay;
};

#define TASTATUR_AMIGA 0
#define TASTATUR_PC 1

struct UMGEBUNG {
	BOOL wbscreen;
	ULONG screenmode;
	WORD scrbreite;
	WORD scrhoehe;
	UBYTE scrtiefe;
	BOOL backdrop;
	STRPTR pfadproj;
	STRPTR pfadsmf;
	STRPTR pfadsysex;
	STRPTR pfadphonolith;
	STRPTR startproj;
	BOOL startaktiv;
	UBYTE tastatur;
	BYTE playerPri;
	BYTE thruPri;
	WORD sysexpuffer;
	BOOL mausradtauschen;
};

#define EDMODUS_NOTEN 0
#define EDMODUS_CONTR 1

struct EDGUI {
	BYTE modus;
	WORD taste;
	WORD tasth;
	WORD tastsicht;
	WORD contr;
	WORD contrh;
	WORD contrsicht;
	WORD contranz;
	LONG takt;
	WORD taktb;
	WORD taktsicht;
	UBYTE raster;
	BYTE neulen;
	BOOL tripled;
};

struct OUTINSTR {
	char name[128];
	BYTE unten;
};

struct OUTPORT {
	char name[128];
	struct OUTINSTR outinstr[4];
	BOOL thru;
	WORD latenz;
};

struct INPORT {
	char name[128];
};

struct SYSEXMSG {
	char name[128];
	ULONG len;
	UBYTE *data;
	struct SYSEXMSG *prev;
	struct SYSEXMSG *next;
};

struct SYSEXUNIT {
	char name[128];
	UBYTE port;
	BOOL gesperrt;
	struct SYSEXMSG *sysex;
	struct SYSEXUNIT *prev;
	struct SYSEXUNIT *next;
};

struct PROGRAMM {
	char name[128];
	BYTE bank0;
	BYTE bank32;
	BYTE prog;
	struct PROGRAMM *next;
};

struct KATEGORIE {
	char name[128];
	struct PROGRAMM *programm;
	BYTE chanvon;
	BYTE chanbis;
	struct KATEGORIE *next;
};

#define CONTR_MITTE 0x01
#define CONTR_ONOFF 0x02
#define CONTR_SET   0x04
struct INSTRCONTR {
	char name[128][128];
	UBYTE flags[128];
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
	BYTE trans;
	BOOL transmulti;
	BOOL mute;
	BOOL mutemulti;
	WORD aliasanz;
};

struct MPDATA {
	WORD kanalanz;
	WORD kanalerst;
	WORD kanalsicht;
};

struct MPKANALNUM {
	BYTE port;
	BYTE channel;
};

struct MPKANAL {
	BYTE contr[6];
	BYTE contrwert[6];
	BYTE pan;
	BYTE fader;
	BOOL mute;
	BYTE meter;
	WORD bezspur[3];
	BOOL autoupdate; // Automation GUI Update
	UBYTE updateflags;
};
