#include <stdio.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/datatypes.h>
#include <proto/bitmap.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <clib/alib_protos.h>

#include <intuition/gadgetclass.h>
#include <datatypes/pictureclass.h>
#include <images/bitmap.h>

#include "Versionen.h"
#include "DTGrafik.h"
#include "Gui.h"

STRPTR RegisterName(void); // Versionen.c


extern struct Screen *hschirm;
extern struct Window *hfenster;
extern struct Window *aktfenster;

struct Window *titelfenster = NULL;
Object *titeldto = NULL;

Object *dto[BMAP_ANZ];
struct BitMap *bitmap[BMAP_ANZ];

Object *bmo[IMG_ANZ];

void OeffneTitel(void) {
	struct Screen *scr;
	LONG breite, hoehe;
	struct BitMap *bm = NULL;

	if (hschirm) scr = hschirm;
	else scr = LockPubScreen(NULL);
	
	titeldto = NewDTObject("PROGDIR:System/Graphics/Titel",
		PDTA_Screen, scr,
		PDTA_DestMode, PMODE_V43,
		TAG_DONE);
	if (titeldto) {

		DoDTMethod(titeldto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE); 

		GetDTAttrs(titeldto,
			DTA_NominalHoriz, &breite,
			DTA_NominalVert, &hoehe,
			PDTA_DestBitMap, &bm,
			TAG_DONE);
			
		SetDTAttrs(titeldto, NULL, NULL,
			GA_Left, 0,
			GA_Top, 0,
			GA_Width, breite,
			GA_Height, hoehe,
			TAG_DONE);
	
		titelfenster = OpenWindowTags(NULL,
			WA_PubScreen, scr,
			WA_Left, (scr->Width - breite) / 2,
			WA_Top, (scr->Height - hoehe) / 2,
			WA_Width, breite,
			WA_Height, hoehe,
			WA_Borderless, TRUE,
			WA_ScreenTitle, "Inutilis Horny",
			WA_SmartRefresh, TRUE,
			WA_Activate, TRUE,
			WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_INACTIVEWINDOW,
			TAG_DONE);
		if (titelfenster) {
			
			BltBitMapRastPort (bm, 0, 0, titelfenster->RPort, 0, 0, breite, hoehe, 0xC0);

			aktfenster = titelfenster;
			SetzeFont();
			if (verLITE) {
				SchreibeSys(0, 6, hoehe - 8, "LITE version");
			} else {
				SchreibeSys(0, 6, hoehe - 20, "registered to:");
				SchreibeSys(0, 6, hoehe - 8, RegisterName());
			}
		}
		DisposeDTObject(titeldto);
		titeldto = NULL;
	}
	if (!hschirm) UnlockPubScreen(NULL, scr);
}

void SchliesseTitel() {
	if (titelfenster) {
		CloseWindow(titelfenster);
		titelfenster = NULL;
	}
}

void AboutTitel(void) {
	struct Message *msg;
	
	if (titelfenster) {
		aktfenster = titelfenster;
		Fett(TRUE);
		if (verLITE)
			Schreibe(1, 13, 20, "Horny Lite", 300);
		else
			Schreibe(1, 13, 20, "Horny", 300);
		Fett(FALSE);
		Schreibe(1, 13, 32, "Midi Sequencer", 300);
#ifdef __amigaos4__
		Schreibe(1, 13, 44, "Version 1.3 (for OS4 PPC)", 300);
#else
		Schreibe(1, 13, 44, "Version 1.3 (for OS3 68k)", 300);
#endif
		Schreibe(1, 13, 68, "© 2003-07 Inutilis Software / TK", 300);
		Schreibe(1, 13, 80, "Developed by Timo Kloss", 300);
		Schreibe(1, 13, 104, "http://www.inutilis.de/horny/", 300);
		Schreibe(1, 13, 116, "eMail: Timo@inutilis.de", 300);
		
		WaitPort(titelfenster->UserPort);
		while (msg = GetMsg(titelfenster->UserPort)) ReplyMsg(msg);
	}
}

void OeffneBitMap(UWORD id, STRPTR datei) {
	struct Screen *scr;

	if (hschirm) scr = hschirm;
	else scr = LockPubScreen(NULL);
	
	dto[id] = NewDTObject(datei,
		PDTA_Screen, scr,
		PDTA_DestMode, PMODE_V43,
		TAG_DONE);
	if (dto[id]) {

		DoDTMethod(dto[id], NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE); 

		GetDTAttrs(dto[id],
			PDTA_DestBitMap, &bitmap[id],
			TAG_DONE);
	} else bitmap[id] = NULL;
	
	if (!hschirm) UnlockPubScreen(NULL, scr);
}

void SchliesseBitMap(UWORD id) {
	if (dto[id]) {
		DisposeDTObject(dto[id]);
		dto[id] = NULL;
		bitmap[id] = NULL;
	}
}

void BlitteBitMap(UWORD id, WORD qx, WORD qy, WORD zx, WORD zy, WORD b, WORD h) {
	if (bitmap[id]) {
		BltBitMapRastPort(bitmap[id], qx, qy,
			aktfenster->RPort, zx + aktfenster->BorderLeft, zy + aktfenster->BorderTop,
			b, h, 0xC0);
	}
}

void OeffneImg(UWORD id, STRPTR datei) {
	struct Screen *scr;

	if (hschirm) scr = hschirm;
	else scr = LockPubScreen(NULL);
	
	bmo[id] = NewObject(BITMAP_GetClass(), NULL,
		BITMAP_SourceFile, datei,
		BITMAP_Screen, scr,
		TAG_DONE);
	
	if (!hschirm) UnlockPubScreen(NULL, scr);
}

void OeffneImg2(UWORD id, STRPTR datei) {
	struct Screen *scr;
	char seldatei[1024];

	if (hschirm) scr = hschirm;
	else scr = LockPubScreen(NULL);
	
	strncpy(seldatei, datei, 1024);
	strncat(seldatei, "_S", 1024);
	bmo[id] = NewObject(BITMAP_GetClass(), NULL,
		BITMAP_SourceFile, datei,
		BITMAP_SelectSourceFile, seldatei,
		BITMAP_Screen, scr,
		TAG_DONE);
	
	if (!hschirm) UnlockPubScreen(NULL, scr);
}

void SchliesseImg(UWORD id) {
	if (bmo[id]) {
		DisposeObject(bmo[id]);
		bmo[id] = NULL;
	}
}

void OeffneAlleGfx(void) {
	WORD n;
	
	for (n = 0; n < IMG_ANZ; n++) bmo[n] = NULL;
	for (n = 0; n < BMAP_ANZ; n++) {
		dto[n] = NULL;
		bitmap[n] = NULL;
	}
	OeffneImg2(IMG_PREV, "PROGDIR:System/Graphics/Button_Prev");
	OeffneImg2(IMG_NEXT, "PROGDIR:System/Graphics/Button_Next");
	OeffneImg2(IMG_STOP, "PROGDIR:System/Graphics/Button_Stop");
	OeffneImg2(IMG_PLAY, "PROGDIR:System/Graphics/Button_Play");
	OeffneImg(IMG_PLAY_A, "PROGDIR:System/Graphics/Button_Play_A");
	OeffneImg2(IMG_REC, "PROGDIR:System/Graphics/Button_Rec");
	OeffneImg(IMG_REC_A, "PROGDIR:System/Graphics/Button_Rec_A");
	OeffneImg2(IMG_MREC, "PROGDIR:System/Graphics/Button_MRec");
	OeffneImg2(IMG_MPLAY, "PROGDIR:System/Graphics/Button_MPlay");
	OeffneImg2(IMG_LOOP, "PROGDIR:System/Graphics/Button_Loop");
	OeffneImg2(IMG_FOLLOW, "PROGDIR:System/Graphics/Button_Follow");
	OeffneImg2(IMG_THRU, "PROGDIR:System/Graphics/Button_Thru");
	OeffneImg2(IMG_SYNC, "PROGDIR:System/Graphics/Button_Sync");
	
	OeffneBitMap(BMAP_METER_OFF, "PROGDIR:System/Graphics/Gui_Meter_off");
	OeffneBitMap(BMAP_METER_ON, "PROGDIR:System/Graphics/Gui_Meter_on");
	OeffneBitMap(BMAP_METER_INACTIVE, "PROGDIR:System/Graphics/Gui_Meter_inactive");
	OeffneBitMap(BMAP_POTI, "PROGDIR:System/Graphics/Gui_Poti");
	OeffneBitMap(BMAP_PAN_BG, "PROGDIR:System/Graphics/Gui_Pan_bg");
	OeffneBitMap(BMAP_PAN_BG_ACTIVE, "PROGDIR:System/Graphics/Gui_Pan_bg_active");
	OeffneBitMap(BMAP_PAN_POINTER, "PROGDIR:System/Graphics/Gui_Pan_pointer");
	OeffneBitMap(BMAP_AUTO, "PROGDIR:System/Graphics/Gui_Automation");
	OeffneBitMap(BMAP_KEY_WHITE, "PROGDIR:System/Graphics/Gui_Key_white");
	OeffneBitMap(BMAP_KEY_BLACK, "PROGDIR:System/Graphics/Gui_Key_black");
	OeffneBitMap(BMAP_MUTE_OFF, "PROGDIR:System/Graphics/Gui_Mute_off");
	OeffneBitMap(BMAP_MUTE_ON, "PROGDIR:System/Graphics/Gui_Mute_on");
}

void SchliesseAlleGfx(void) {
	WORD n;
	
	for (n = 0; n < IMG_ANZ; n++) SchliesseImg(n);
	for (n = 0; n < BMAP_ANZ; n++) SchliesseBitMap(n);
}
