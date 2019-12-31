#include "Strukturen.h"

struct SMPTE smpte = {
	0, // startticks
	FPS_25 // format
};


LONG Smpte2Ticks(BYTE hh, BYTE mm, BYTE ss, BYTE ff) {
	LONG ticks;
	
	ticks = (hh * 2160000L) + (mm * 36000L) + (ss * 600L);
	switch (smpte.format) {
		case FPS_24:   ticks += ff * 25L; break;
		case FPS_25:   ticks += ff * 24L; break;
		case FPS_30:   ticks += ff * 20L; break;
	}
	return(ticks);
}

BYTE Ticks2ff(LONG ticks) {
	switch (smpte.format) {
		case FPS_24:   return((ticks % 600) / 25);
		case FPS_25:   return((ticks % 600) / 24);
		case FPS_30:   return((ticks % 600) / 20);
	}
}
// Alle weiteren Umrechnungen als Makros in Smpte.h
