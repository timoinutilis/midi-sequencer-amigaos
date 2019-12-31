#include <exec/types.h>

#define EVENTS 256

struct EVENT {
	LONG zeit;
	UBYTE status;
	UBYTE data1;
	UBYTE data2;
};

struct EVENTBLOCK {
	struct EVENT event[EVENTS];
	struct EVENTBLOCK *prev;
	struct EVENTBLOCK *next;
};

struct SEQUENZ {
	char name[31];
	LONG start;
	LONG ende;
	BYTE trans;
	struct EVENTBLOCK *eventblock;
	BOOL markiert;
	struct SEQUENZ *aliasorig;
	WORD aliasanz;
	WORD spur;
	struct SEQUENZ *next;
	struct SEQUENZ *speicheradr;
};

struct SPUR {
	char name[31];
	UBYTE port;
	UBYTE channel;
	BYTE bank;
	BYTE prog;
	WORD shift;
	BOOL mute;
	struct SEQUENZ *seq;
	struct SEQUENZ *aktseq;
	struct EVENTBLOCK *aktevbl;
	WORD aktevnum;
};

struct SPURTEMP {
	BOOL anders;
	struct SEQUENZ *loopseq;
	struct EVENTBLOCK *loopevbl;
	WORD loopevnum;
};

struct LIED {
	char name[61];
	UBYTE spuranz;
	WORD taktanz;
	WORD bpm;
};

struct GUI {
	WORD spur;
	WORD spsicht;
	WORD sph;
	LONG takt;
	WORD tasicht;
	WORD tab;
	WORD spalte;
};

struct LOOP {
	LONG start;
	LONG ende;
	BOOL aktiv;
};

struct METRONOM {
	UBYTE port;
	UBYTE channel;
	BYTE taste;
	BYTE velo1;
	BYTE velo2;
	WORD raster;
	BOOL rec;
	BOOL play;
};
