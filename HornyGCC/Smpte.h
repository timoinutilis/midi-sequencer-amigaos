#include <exec/types.h>

int32 Smpte2Ticks(int8 hh, int8 mm, int8 ss, int8 ff);

int8 Ticks2ff(int32 ticks);
#define Ticks2ss(ticks) ((ticks/600)%60)
#define Ticks2mm(ticks) ((ticks/36000)%60)
#define Ticks2hh(ticks) (ticks/2160000)
