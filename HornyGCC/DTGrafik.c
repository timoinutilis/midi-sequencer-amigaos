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
#include "oca.h"

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
	int32 breite, hoehe;
	struct BitMap *bm = NULL;

	if (hschirm) scr = hschirm;
	else scr = IIntuition->LockPubScreen(NULL);
	
	titeldto = IDataTypes->NewDTObject("PROGDIR:System/Graphics/Titel",
		PDTA_Screen, scr,
		PDTA_DestMode, PMODE_V43,
		TAG_DONE);
	if (titeldto) {

		IDataTypes->DoDTMethod(titeldto, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE); 

		IDataTypes->GetDTAttrs(titeldto,
			DTA_NominalHoriz, &breite,
			DTA_NominalVert, &hoehe,
			PDTA_DestBitMap, &bm,
			TAG_DONE);
			
		IDataTypes->SetDTAttrs(titeldto, NULL, NULL,
			GA_Left, 0,
			GA_Top, 0,
			GA_Width, breite,
			GA_Height, hoehe,
			TAG_DONE);
	
		titelfenster = IIntuition->OpenWindowTags(NULL,
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
			
			IGraphics->BltBitMapRastPort (bm, 0, 0, titelfenster->RPort, 0, 0, breite, hoehe, 0xC0);

			aktfenster = titelfenster;
			SetzeFont();
			if (verLITE) {
				SchreibeSys(0, 6, hoehe - 8, (STRPTR)"LITE version");
			} else {
				SchreibeSys(0, 6, hoehe - 20, (STRPTR)"registered to:");
				SchreibeSys(0, 6, hoehe - 8, RegisterName());
			}
		}
		IDataTypes->DisposeDTObject(titeldto);
		titeldto = NULL;
	}
	if (!hschirm) IIntuition->UnlockPubScreen(NULL, scr);
}

void SchliesseTitel() {
	if (titelfenster) {
		IIntuition->CloseWindow(titelfenster);
		titelfenster = NULL;
	}
}

void AboutTitel(void) {
	struct Message *msg;
	
	if (titelfenster) {
		aktfenster = titelfenster;
		Fett(TRUE);
		if (verLITE)
			Schreibe(1, 13, 20, (STRPTR)"Horny Lite", 300);
		else
			Schreibe(1, 13, 20, (STRPTR)"Horny", 300);
		Fett(FALSE);
		Schreibe(1, 13, 32, (STRPTR)"Midi Sequencer", 300);
		Schreibe(1, 13, 44, (STRPTR)"Version 1.4 (for OS4 PPC)", 300);
		Schreibe(1, 13, 68, (STRPTR)"© 2003-07 Inutilis Software / TK", 300);
		Schreibe(1, 13, 80, (STRPTR)"Developed by Timo Kloss", 300);
		Schreibe(1, 13, 104, (STRPTR)"http://www.inutilis.de/horny/", 300);
		Schreibe(1, 13, 116, (STRPTR)"eMail: Timo@inutilis.de", 300);
		
		IExec->WaitPort(titelfenster->UserPort);
		while ((msg = IExec->GetMsg(titelfenster->UserPort))) 
		{
			IExec->ReplyMsg(msg);
		}
	}
}

void OeffneBitMap(uint16 id, STRPTR datei) {
	struct Screen *scr;

	if (hschirm) scr = hschirm;
	else scr = IIntuition->LockPubScreen(NULL);
	
	dto[id] = IDataTypes->NewDTObject(datei,
		PDTA_Screen, scr,
		PDTA_DestMode, PMODE_V43,
		TAG_DONE);
	if (dto[id]) {

		IDataTypes->DoDTMethod(dto[id], NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE); 

		IDataTypes->GetDTAttrs(dto[id],
			PDTA_DestBitMap, &bitmap[id],
			TAG_DONE);
	} else bitmap[id] = NULL;
	
	if (!hschirm) IIntuition->UnlockPubScreen(NULL, scr);
}

void SchliesseBitMap(uint16 id) {
	if (dto[id]) {
		IDataTypes->DisposeDTObject(dto[id]);
		dto[id] = NULL;
		bitmap[id] = NULL;
	}
}

void BlitteBitMap(uint16 id, int16 qx, int16 qy, int16 zx, int16 zy, int16 b, int16 h) {
	if (bitmap[id]) {
		IGraphics->BltBitMapRastPort(bitmap[id], qx, qy,
			aktfenster->RPort, zx + aktfenster->BorderLeft, zy + aktfenster->BorderTop,
			b, h, 0xC0);
	}
}

void OeffneImg(uint16 id, STRPTR datei) {
	struct Screen *scr;

	if (hschirm) scr = hschirm;
	else scr = IIntuition->LockPubScreen(NULL);
	
	bmo[id] = IIntuition->NewObject(BitmapClass, NULL,
		BITMAP_SourceFile, datei,
		BITMAP_Screen, scr,
		TAG_DONE);
	
	if (!hschirm) IIntuition->UnlockPubScreen(NULL, scr);
}

void OeffneImg2(uint16 id, STRPTR datei) {
	struct Screen *scr;
	char seldatei[1024];

	if (hschirm) scr = hschirm;
	else scr = IIntuition->LockPubScreen(NULL);
	
	snprintf(seldatei, sizeof(seldatei), "%s_S", datei);
	
	bmo[id] = IIntuition->NewObject(BitmapClass, NULL,
		BITMAP_SourceFile, datei,
		BITMAP_SelectSourceFile, seldatei,
		BITMAP_Screen, scr,
		TAG_DONE);
	
	if (!hschirm) IIntuition->UnlockPubScreen(NULL, scr);
}

void SchliesseImg(uint16 id) {
	if (bmo[id]) {
		IIntuition->DisposeObject(bmo[id]);
		bmo[id] = NULL;
	}
}

void OeffneAlleGfx(void) {
	int16 n;
	
	for (n = 0; n < IMG_ANZ; n++) bmo[n] = NULL;
	for (n = 0; n < BMAP_ANZ; n++) {
		dto[n] = NULL;
		bitmap[n] = NULL;
	}
	OeffneImg2(IMG_PREV, (STRPTR)"PROGDIR:System/Graphics/Button_Prev");
	OeffneImg2(IMG_NEXT, (STRPTR)"PROGDIR:System/Graphics/Button_Next");
	OeffneImg2(IMG_STOP, (STRPTR)"PROGDIR:System/Graphics/Button_Stop");
	OeffneImg2(IMG_PLAY, (STRPTR)"PROGDIR:System/Graphics/Button_Play");
	OeffneImg(IMG_PLAY_A, (STRPTR)"PROGDIR:System/Graphics/Button_Play_A");
	OeffneImg2(IMG_REC, (STRPTR)"PROGDIR:System/Graphics/Button_Rec");
	OeffneImg(IMG_REC_A, (STRPTR)"PROGDIR:System/Graphics/Button_Rec_A");
	OeffneImg2(IMG_MREC, (STRPTR)"PROGDIR:System/Graphics/Button_MRec");
	OeffneImg2(IMG_MPLAY, (STRPTR)"PROGDIR:System/Graphics/Button_MPlay");
	OeffneImg2(IMG_LOOP, (STRPTR)"PROGDIR:System/Graphics/Button_Loop");
	OeffneImg2(IMG_FOLLOW, (STRPTR)"PROGDIR:System/Graphics/Button_Follow");
	OeffneImg2(IMG_THRU, (STRPTR)"PROGDIR:System/Graphics/Button_Thru");
	OeffneImg2(IMG_SYNC, (STRPTR)"PROGDIR:System/Graphics/Button_Sync");
	
	OeffneBitMap(BMAP_METER_OFF, (STRPTR)"PROGDIR:System/Graphics/Gui_Meter_off");
	OeffneBitMap(BMAP_METER_ON, (STRPTR)"PROGDIR:System/Graphics/Gui_Meter_on");
	OeffneBitMap(BMAP_METER_INACTIVE, (STRPTR)"PROGDIR:System/Graphics/Gui_Meter_inactive");
	OeffneBitMap(BMAP_POTI, (STRPTR)"PROGDIR:System/Graphics/Gui_Poti");
	OeffneBitMap(BMAP_PAN_BG, (STRPTR)"PROGDIR:System/Graphics/Gui_Pan_bg");
	OeffneBitMap(BMAP_PAN_BG_ACTIVE, (STRPTR)"PROGDIR:System/Graphics/Gui_Pan_bg_active");
	OeffneBitMap(BMAP_PAN_POINTER, (STRPTR)"PROGDIR:System/Graphics/Gui_Pan_pointer");
	OeffneBitMap(BMAP_AUTO, (STRPTR)"PROGDIR:System/Graphics/Gui_Automation");
	OeffneBitMap(BMAP_KEY_WHITE, (STRPTR)"PROGDIR:System/Graphics/Gui_Key_white");
	OeffneBitMap(BMAP_KEY_BLACK, (STRPTR)"PROGDIR:System/Graphics/Gui_Key_black");
	OeffneBitMap(BMAP_MUTE_OFF, (STRPTR)"PROGDIR:System/Graphics/Gui_Mute_off");
	OeffneBitMap(BMAP_MUTE_ON, (STRPTR)"PROGDIR:System/Graphics/Gui_Mute_on");
}

void SchliesseAlleGfx(void) {
	int16 n;
	
	for (n = 0; n < IMG_ANZ; n++) SchliesseImg(n);
	for (n = 0; n < BMAP_ANZ; n++) SchliesseBitMap(n);
}
