#include <stdio.h>
#include <string.h>

#include <intuition/intuition.h>
#include <exec/exec.h>
#include <graphics/gfx.h>
#include <libraries/diskfont.h>
#include <intuition/gadgetclass.h>
#include <gadgets/button.h>
#include <gadgets/scroller.h>
#include <gadgets/checkbox.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/button.h>
#include <proto/scroller.h>
#include <proto/checkbox.h>

#include "Strukturen.h"
#include "Requester.h"
#include "Images.h"

struct Library *ButtonBase=NULL;
struct Library *ScrollerBase=NULL;
struct Library *ChooserBase=NULL;
struct Library *StringBase=NULL;
struct Library *IntegerBase=NULL;
struct Library *SliderBase=NULL;
struct Library *CheckBoxBase=NULL;
struct TextFont *font=NULL;

struct Window *hfenster=NULL;
struct Gadget *gadget[13];
struct Window *aktfenster=NULL;

extern WORD guibox=60;
WORD altposx=-1;
WORD randlr;
WORD randou;
LONG guifar[25];

extern struct GUI gui;
extern struct SPUR spur[12];
extern struct LIED lied;
extern struct LOOP loop;
extern struct METRONOM metro;

void ErstelleGuiClasses(void) {
	if (!(ButtonBase=OpenLibrary("button.gadget", 44))) Meldung("button.gadget nicht geöffnet");
	if (!(ScrollerBase=OpenLibrary("scroller.gadget", 44))) Meldung("scroller.gadget nicht geöffnet");
	if (!(ChooserBase=OpenLibrary("chooser.gadget", 44))) Meldung("chooser.gadget nicht geöffnet");
	if (!(StringBase=OpenLibrary("string.gadget", 44))) Meldung("string.gadget nicht geöffnet");
	if (!(IntegerBase=OpenLibrary("integer.gadget", 44))) Meldung("integer.gadget nicht geöffnet");
	if (!(SliderBase=OpenLibrary("slider.gadget", 44))) Meldung("slider.gadget nicht geöffnet");
	if (!(CheckBoxBase=OpenLibrary("checkbox.gadget", 44))) Meldung("checkbox.gadget nicht geöffnet");
}

void EntferneGuiClasses(void) {
	CloseLibrary(ButtonBase);
	CloseLibrary(ScrollerBase);
	CloseLibrary(ChooserBase);
	CloseLibrary(StringBase);
	CloseLibrary(IntegerBase);
	CloseLibrary(SliderBase);
	CloseLibrary(CheckBoxBase);
}

void ErstelleFarben(void) {
	BYTE f;

	guifar[0]=0; guifar[1]=1; guifar[2]=2; guifar[3]=3;
	for (f=0; f<21; f++) {
		guifar[f+4]=ObtainBestPen(hfenster->WScreen->ViewPort.ColorMap, farbe[f].r, farbe[f].g, farbe[f].b, OBP_Precision, PRECISION_GUI, TAG_DONE);
	};
}

void EntferneFarben(void) {
	BYTE f;

	for (f=4; f<25; f++) ReleasePen(hfenster->WScreen->ViewPort.ColorMap, guifar[f]);
}

void ErstelleHauptfenster(void) {
	struct TextAttr textattr={"helvetica.font", 11, FS_NORMAL, 0};

	ErstelleGuiClasses();
	font=OpenDiskFont(&textattr);
	hfenster=OpenWindowTags(NULL,
		WA_Width, 640,
		WA_Height, 480,
		WA_MinWidth, 450,
		WA_MinHeight, 240,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_Title, "Horny",
		WA_ScreenTitle, "Inutilis Horny Midi-Sequencer",
		WA_IDCMP, IDCMP_RAWKEY | IDCMP_SIZEVERIFY | IDCMP_NEWSIZE | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE |
					IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK | IDCMP_CLOSEWINDOW,
		WA_SizeGadget, TRUE,
		WA_DragBar, TRUE,
		WA_DepthGadget, TRUE,
		WA_CloseGadget, TRUE,
		WA_NewLookMenus, TRUE,
		WA_ReportMouse, TRUE,
		WA_Activate, TRUE,
		TAG_DONE);
	SetDrMd(hfenster->RPort, JAM1);
	SetFont(hfenster->RPort, font);
	randlr=hfenster->BorderLeft + hfenster->BorderRight;
	randou=hfenster->BorderTop + hfenster->BorderBottom;
	ErstelleFarben();
	aktfenster=hfenster;
}

void EntferneHauptfenster(void) {
	EntferneFarben();
	CloseWindow(hfenster);
	CloseFont(font);
	EntferneGuiClasses();
}

void SetzeFont(void) {
	SetFont(aktfenster->RPort, font);
}

void Fett(BOOL f) {
	if (f) {
		SetSoftStyle(aktfenster->RPort, FSF_BOLD, FSF_BOLD);
	} else {
		SetSoftStyle(aktfenster->RPort, FS_NORMAL, FSF_BOLD);
	};
}

void ErstelleGadgets(void) {
	WORD y;

	y=hfenster->Height - hfenster->BorderBottom - 34;

	gadget[0]=NewObject(BUTTON_GetClass(), NULL,
		GA_Top, y, GA_Left, hfenster->BorderLeft+6,
		GA_Width, 40, GA_Height, 30,
		GA_Immediate, TRUE, GA_RelVerify, TRUE,
		GA_ID, 0, GA_Image, &imgzero,
		TAG_DONE);

	gadget[1]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[0],
		GA_Top, y, GA_Left, hfenster->BorderLeft+50,
		GA_Width, 40, GA_Height, 30,
		GA_Immediate, TRUE, GA_RelVerify, TRUE,
		GA_ID, 1, GA_Image, &imgstop,
		TAG_DONE);

	gadget[2]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[1],
		GA_Top, y, GA_Left, hfenster->BorderLeft+94,
		GA_Width, 40, GA_Height, 30,
		GA_Immediate, TRUE, GA_RelVerify, TRUE,
		GA_ID, 2, GA_Image, &imgplay,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[3]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[2],
		GA_Top, y, GA_Left, hfenster->BorderLeft+138,
		GA_Width, 40, GA_Height, 30,
		GA_Immediate, TRUE, GA_RelVerify, TRUE,
		GA_ID, 3, GA_Image, &imgrec,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[4]=NewObject(SCROLLER_GetClass(), NULL,
		GA_Previous, gadget[3],
		GA_Top, y-guibox-20, GA_Left, hfenster->BorderLeft+2,
		GA_Width, hfenster->Width - randlr - 4, GA_Height, 15,
		GA_Immediate, TRUE, GA_RelVerify, TRUE, GA_FollowMouse, TRUE,
		GA_ID, 4,
		SCROLLER_Top, gui.takt>>10,
		SCROLLER_Visible, gui.tasicht,
		SCROLLER_Total, lied.taktanz,
		SCROLLER_Orientation, SORIENT_HORIZ,
		TAG_DONE);

	gadget[5]=NewObject(SCROLLER_GetClass(), NULL,
		GA_Previous, gadget[4],
		GA_Top, hfenster->BorderTop+20, GA_Left, hfenster->Width - randlr - 13,
		GA_Width, 15, GA_Height, hfenster->Height - randou - 74 - guibox,
		GA_Immediate, TRUE, GA_RelVerify, TRUE, GA_FollowMouse, TRUE,
		GA_ID, 5,
		SCROLLER_Top, gui.spur,
		SCROLLER_Visible, gui.spsicht,
		SCROLLER_Total, lied.spuranz+1,
		SCROLLER_Orientation, SORIENT_VERT,
		TAG_DONE);

	gadget[6]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[5],
		GA_Top, hfenster->BorderTop+2, GA_Left, hfenster->BorderLeft+2,
		GA_Width, 16, GA_Height, 16,
		GA_RelVerify, TRUE, GA_ID, 6,
		BUTTON_AutoButton, BAG_LFARROW,
		TAG_DONE);

	gadget[7]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[6],
		GA_Top, hfenster->BorderTop+2, GA_Left, hfenster->BorderLeft+20,
		GA_Width, 16, GA_Height, 16,
		GA_RelVerify, TRUE, GA_ID, 7,
		BUTTON_AutoButton, BAG_RTARROW,
		TAG_DONE);

	gadget[8]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[7],
		GA_Top, hfenster->BorderTop+2, GA_Left, hfenster->BorderLeft+38,
		GA_Width, 16, GA_Height, 16,
		GA_RelVerify, TRUE, GA_ID, 8,
		BUTTON_AutoButton, BAG_UPARROW,
		TAG_DONE);

	gadget[9]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[8],
		GA_Top, hfenster->BorderTop+2, GA_Left, hfenster->BorderLeft+56,
		GA_Width, 16, GA_Height, 16,
		GA_RelVerify, TRUE, GA_ID, 9,
		BUTTON_AutoButton, BAG_DNARROW,
		TAG_DONE);

	gadget[10]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[9],
		GA_Top, y, GA_Left, hfenster->BorderLeft+198,
		GA_Width, 40, GA_Height, 30,
		GA_Immediate, TRUE, GA_RelVerify, TRUE,
		GA_ID, 10, GA_Image, &imgmrec,
		GA_Selected, metro.rec,
		BUTTON_PushButton, TRUE,
		TAG_DONE);


	gadget[11]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[10],
		GA_Top, y, GA_Left, hfenster->BorderLeft+242,
		GA_Width, 40, GA_Height, 30,
		GA_RelVerify, TRUE,
		GA_ID, 11, GA_Image, &imgmplay,
		GA_Selected, metro.play,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[12]=NewObject(BUTTON_GetClass(), NULL,
		GA_Previous, gadget[11],
		GA_Top, y, GA_Left, hfenster->BorderLeft+286,
		GA_Width, 40, GA_Height, 30,
		GA_RelVerify, TRUE,
		GA_ID, 12, GA_Image, &imgloop,
		GA_Selected, loop.aktiv, GA_Disabled, TRUE,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	AddGList(hfenster, gadget[0], 0, 13, NULL);
	RefreshGList(gadget[0], hfenster, NULL, -1);
}

void EntferneGadgets(void) {
	BYTE n;

	for(n=0; n<13; n++) {
		if (gadget[n]) {
			RemoveGList(hfenster, gadget[n], 1);
			DisposeObject(gadget[n]); gadget[n]=NULL;
		};
	};
}

void AktualisiereGadgets(void) {
	SetGadgetAttrs(gadget[4], hfenster, NULL,
		SCROLLER_Top, gui.takt>>10,
		SCROLLER_Visible, gui.tasicht,
		SCROLLER_Total, lied.taktanz,
		TAG_DONE);
	SetGadgetAttrs(gadget[5], hfenster, NULL,
		SCROLLER_Top, gui.spur,
		SCROLLER_Visible, gui.spsicht,
		SCROLLER_Total, lied.spuranz+1,
		TAG_DONE);
	SetGadgetAttrs(gadget[10], hfenster, NULL, GA_Selected, metro.rec, TAG_DONE);
	SetGadgetAttrs(gadget[11], hfenster, NULL, GA_Selected, metro.play, TAG_DONE);
	SetGadgetAttrs(gadget[12], hfenster, NULL,
		GA_Selected, loop.aktiv,
		GA_Disabled, (loop.start==loop.ende),
		TAG_DONE);
}

void BildFrei(void) {
	SetAPen(aktfenster->RPort, 0);
	RectFill(aktfenster->RPort, aktfenster->BorderLeft, aktfenster->BorderTop, aktfenster->Width - aktfenster->BorderRight - 1, aktfenster->Height - aktfenster->BorderBottom - 1);
	altposx=-1;
}

void Schreibe(UBYTE f, WORD x, WORD y, STRPTR t, WORD xe) {
	LONG l;
	struct TextExtent te;

	Move(aktfenster->RPort, x+aktfenster->BorderLeft, y+aktfenster->BorderTop);
	SetAPen(aktfenster->RPort, guifar[f]);
	l=TextFit(aktfenster->RPort, t, strlen(t), &te, NULL, 1, xe-x, 100);
	Text(aktfenster->RPort, t, l);
}

void SchreibeZahl(UBYTE f, WORD x, WORD y, WORD z) {
	char t[10];
	sprintf(t, "%ld", z); Schreibe(f, x, y, t, x+100);
}

void RahmenEin(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2) {
	x1=x1+aktfenster->BorderLeft; x2=x2+aktfenster->BorderLeft;
	y1=y1+aktfenster->BorderTop; y2=y2+aktfenster->BorderTop;
	SetAPen(aktfenster->RPort, 2); Move(aktfenster->RPort, x1, y2);
	Draw(aktfenster->RPort, x2, y2); Draw(aktfenster->RPort, x2, y1);
	SetAPen(aktfenster->RPort, 1);
	Draw(aktfenster->RPort, x1, y1); Draw(aktfenster->RPort, x1, y2);
	SetAPen(aktfenster->RPort, guifar[f]);
	RectFill(aktfenster->RPort, x1+1, y1+1, x2-1, y2-1);
}

void RahmenAus(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2) {
	x1=x1+aktfenster->BorderLeft; x2=x2+aktfenster->BorderLeft;
	y1=y1+aktfenster->BorderTop; y2=y2+aktfenster->BorderTop;
	SetAPen(aktfenster->RPort, 1); Move(aktfenster->RPort, x1, y2);
	Draw(aktfenster->RPort, x2, y2); Draw(aktfenster->RPort, x2, y1);
	SetAPen(aktfenster->RPort, 2);
	Draw(aktfenster->RPort, x1, y1); Draw(aktfenster->RPort, x1, y2);
	SetAPen(aktfenster->RPort, guifar[f]);
	RectFill(aktfenster->RPort, x1+1, y1+1, x2-1, y2-1);
}

void Balken(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2) {
	SetAPen(aktfenster->RPort, guifar[f]);
	RectFill(aktfenster->RPort, x1+aktfenster->BorderLeft, y1+aktfenster->BorderTop, x2+aktfenster->BorderLeft, y2+aktfenster->BorderTop);
}

void Linie(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2) {
	SetAPen(aktfenster->RPort, guifar[f]);
	Move(aktfenster->RPort, x1+aktfenster->BorderLeft, y1+aktfenster->BorderTop); Draw(aktfenster->RPort, x2+aktfenster->BorderLeft, y2+aktfenster->BorderTop);
}

void Farbe(UBYTE f) {
	SetAPen(aktfenster->RPort, guifar[f]);
}

void Punkt(WORD x, WORD y) {
	WritePixel(aktfenster->RPort, x+aktfenster->BorderLeft, y+aktfenster->BorderTop);
}

LONG PunktPosition(WORD x) {
	LONG p;

	if (x>gui.spalte+1) {
		p=(((x-hfenster->BorderLeft-gui.spalte-2)*4/gui.tab)<<10)+gui.takt;
	} else {
		p=gui.takt;
	};
	return(p);
}

void ZeichnePosition(LONG takt) {
	WORD x;

	x=gui.spalte+2+((((takt-gui.takt)>>8)*gui.tab)>>4);
	if (x!=altposx) {
		SetDrMd(hfenster->RPort, COMPLEMENT);
		if (altposx>=0) Linie(1, altposx, 20, altposx, hfenster->Height - randou - 56 - guibox);
		if ((x>gui.spalte+1) && (x<hfenster->Width - randlr - 20)) {
			Linie(1, x, 20, x, hfenster->Height - randou - 56 - guibox);
			altposx=x;
		} else {
			altposx=-1;
		};
		SetDrMd(hfenster->RPort, JAM1);
	};
}

void KeinePosition(void) {
	SetDrMd(hfenster->RPort, COMPLEMENT);
	if (altposx>=0) Linie(1, altposx, 20, altposx, hfenster->Height - randou - 56 - guibox);
	altposx=-1;
	SetDrMd(hfenster->RPort, JAM1);
}
