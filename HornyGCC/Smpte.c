#include "Strukturen.h"

struct SMPTE smpte = {
	0, // startticks
	FPS_25 // format
};


int32 Smpte2Ticks(int8 hh, int8 mm, int8 ss, int8 ff) {
	int32 ticks;
	
	ticks = (hh * 2160000L) + (mm * 36000L) + (ss * 600L);
	switch (smpte.format) {
		case FPS_24:   ticks += ff * 25L; break;
		case FPS_25:   ticks += ff * 24L; break;
		case FPS_30:   ticks += ff * 20L; break;
	}
	return(ticks);
}

int8 Ticks2ff(int32 ticks) {
	switch (smpte.format) {
		case FPS_24:   return((ticks % 600) / 25);
		case FPS_25:   return((ticks % 600) / 24);
		case FPS_30:   return((ticks % 600) / 20);
	}
	return(0);
}
// Alle weiteren Umrechnungen als Makros in Smpte.h
