#include <exec/types.h>

#define IMG_PREV 0
#define IMG_NEXT 1
#define IMG_STOP 2
#define IMG_PLAY 3
#define IMG_PLAY_A 4
#define IMG_REC 5
#define IMG_REC_A 6
#define IMG_MREC 7
#define IMG_MPLAY 8
#define IMG_LOOP 9
#define IMG_FOLLOW 10
#define IMG_THRU 11
#define IMG_SYNC 12
#define IMG_ANZ 13

#define BMAP_METER_OFF 0
#define BMAP_METER_ON 1
#define BMAP_METER_INACTIVE 2
#define BMAP_POTI 3
#define BMAP_PAN_BG 4
#define BMAP_PAN_BG_ACTIVE 5
#define BMAP_PAN_POINTER 6
#define BMAP_AUTO 7
#define BMAP_KEY_WHITE 8
#define BMAP_KEY_BLACK 9
#define BMAP_MUTE_OFF 10
#define BMAP_MUTE_ON 11
#define BMAP_ANZ 12


void OeffneTitel(void);
void SchliesseTitel();
void AboutTitel();
void BlitteBitMap(uint16 id, int16 qx, int16 qy, int16 zx, int16 zy, int16 b, int16 h);
void OeffneAlleGfx(void);
void SchliesseAlleGfx(void);
