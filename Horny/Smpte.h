#include <exec/types.h>

LONG Smpte2Ticks(BYTE hh, BYTE mm, BYTE ss, BYTE ff);

BYTE Ticks2ff(LONG ticks);
#define Ticks2ss(ticks) ((ticks/600)%60)
#define Ticks2mm(ticks) ((ticks/36000)%60)
#define Ticks2hh(ticks) (ticks/2160000)
