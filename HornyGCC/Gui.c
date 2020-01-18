#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <proto/locale.h>
#include <proto/button.h>
#include <proto/scroller.h>
#include <proto/checkbox.h>
#include <proto/slider.h>
#include <proto/chooser.h>
#include <proto/layers.h>

#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <gadgets/button.h>
#include <gadgets/scroller.h>
#include <gadgets/checkbox.h>
#include <gadgets/slider.h>
#include <gadgets/chooser.h>
#include <gadgets/page.h>

#include "catalogids.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "GuiFenster.h"
#include "Requester.h"
#include "Images.h"
#include "Gui.h"
#include "Marker.h"
#include "DTGrafik.h"
#include "Midi.h"

#define CAT(X,Y) ILocale->GetCatalogStr(catalog,X,Y)

struct TextFont *font = NULL;
struct Catalog *catalog = NULL;

struct Screen *hschirm = NULL;
struct Window *hfenster = NULL;
struct Gadget *gadget[GADANZ];
struct Window *aktfenster = NULL;
struct FENSTERPOS fenp[] = {
	{0, -1, 640, 480}, // HAUPT
	{20, 30, 0, 0}, // SET
	{20, 30, 0, 0}, //ENV
	{0, -1, 0, 0}, //ED
	{20, 30, 0, 0}, //SEX
	{0, -1, 0, 0}, //MISCHER
	{20, 30, 0, 0} //CC
};

extern int16 guibox;
extern int16 guileiste;
int16 altposx = -1;
int16 alteditx = -1;
int16 altprjstart = -1;
int16 altprjende = -1;
int16 randlr;
int16 randou;
int32 guifar[FARBEN_ANZ];
char fenstertitel[55];

extern Object *setfensterobj;
extern Object *envfensterobj;
extern Object *sexfensterobj;
extern struct Window *edfenster;
extern struct Window *mpfenster;
extern Object *ccfensterobj;
int16 edaltposx = -1;
int16 edalteditx = -1;
extern int16 edlr;
extern int16 edou;
extern int16 edguibox;
extern struct EDGUI edgui;

extern struct GUI gui;
extern struct OUTPORT outport[];
extern struct SPUR spur[];
extern struct LIED lied;
extern struct LOOP loop;
extern struct METRONOM metro;
extern struct UMGEBUNG umgebung;

extern int32 takt;
extern int32 edittakt;
extern int16 snum;

extern struct MARKER *rootmark;
struct List markerlist = {NULL};
extern struct MARKER *sprung[20];

struct TagItem guimapscroller[] = {{SCROLLER_Top, ICSPECIAL_CODE}, {TAG_DONE, TAG_DONE}};
struct TagItem guimapslider[] = {{SLIDER_Level, ICSPECIAL_CODE}, {TAG_DONE, TAG_DONE}};

extern Object *bmo[];

struct Hook *backfill = NULL;
struct DrawInfo *drinfo = NULL;
struct GradientSpec gradspec;
struct Library *LayersBase = NULL;
struct LayersIFace *ILayers = NULL;

void OeffneFont(void) {
	struct TextAttr textattr = {"helvetica.font", 11, FS_NORMAL, 0};

	font = IDiskfont->OpenDiskFont(&textattr);
}

void EntferneFont(void) {
	IGraphics->CloseFont(font);
}

void ErstelleGuiClasses(void) {

	LayersBase = IExec->OpenLibrary("layers.library", 45);
	if (LayersBase) {
		ILayers = (struct LayersIFace *)IExec->GetInterface(LayersBase, "main", 1, NULL);
		if (!ILayers) Meldung((STRPTR)"Could not obtain layers interface");
	} else Meldung((STRPTR)"Could not open layers.library");

	catalog = ILocale->OpenCatalogA(NULL, "Horny.catalog", NULL);
}

void EntferneGuiClasses(void) {

	ILocale->CloseCatalog(catalog);
	IExec->DropInterface((struct Interface *)ILayers);
	IExec->CloseLibrary(LayersBase);
}

void ErstelleFarben(void) {
	int16 f;

	guifar[0] = IGraphics->ObtainBestPen(hfenster->WScreen->ViewPort.ColorMap, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, OBP_Precision, PRECISION_EXACT, TAG_DONE);
	for (f = 1; f < 4; f++) guifar[f] = f;
	for (f = 0; f < FARBEN_ANZ - 4; f++) {
		guifar[f + 4] = IGraphics->ObtainBestPen(hfenster->WScreen->ViewPort.ColorMap, farbe[f].r, farbe[f].g, farbe[f].b, OBP_Precision, PRECISION_EXACT, TAG_DONE);
	}

	drinfo = IIntuition->GetScreenDrawInfo(hfenster->WScreen);
	memset(&gradspec, 0, sizeof(gradspec));
	gradspec.Mode = GRADMODE_SIMPLE | GRADMODE_PALETTE; //COLOR;
	
	backfill = ILayers->CreateBackFillHook(BFHA_APen, guifar[0], TAG_DONE);
}

void EntferneFarben(void) {
	int16 f;

	IGraphics->ReleasePen(hfenster->WScreen->ViewPort.ColorMap, guifar[0]);
	for (f = 4; f < FARBEN_ANZ; f++) IGraphics->ReleasePen(hfenster->WScreen->ViewPort.ColorMap, guifar[f]);

	IIntuition->FreeScreenDrawInfo(hfenster->WScreen, drinfo);
	drinfo = NULL;
	
	ILayers->DeleteBackFillHook(backfill);
	backfill = NULL;
}

void ErstelleBildschirm(void) {
	uint16 pens = ~0;

	if (!umgebung.wbscreen) {
		hschirm = IIntuition->OpenScreenTags(NULL,
			SA_Width, umgebung.scrbreite,
			SA_Height, umgebung.scrhoehe,
			SA_Depth, umgebung.scrtiefe,
			SA_Title, CAT(MSG_0138, "Horny's Screen"),
			SA_SysFont, 1,
			SA_DisplayID, umgebung.screenmode,
			SA_AutoScroll, TRUE,
			SA_PubName, "Horny",
			SA_Pens, &pens,
			SA_FullPalette, TRUE,
			SA_SharePens, TRUE,
			SA_Behind, TRUE,
			TAG_DONE);
		if (!hschirm) Meldung((STRPTR)CAT(MSG_0140, "Could not open screen"));
	}
}

void ZeigeBildschirm(void) {
	if (hschirm) IIntuition->ScreenToFront(hschirm);
}

void ErstelleHauptfenster(void) {
	struct Screen *scr;
	BOOL fensterform;
	int16 winx, winy, winbr, winho;

	if (hschirm) {
		IIntuition->PubScreenStatus(hschirm, 0);
		scr = hschirm;
	} else {
		scr = IIntuition->LockPubScreen(NULL);
	}
	if (hschirm && umgebung.backdrop) {
		fensterform = FALSE;
		winx = 0; winy = hschirm->BarHeight + 1;
		winbr = umgebung.scrbreite; winho = umgebung.scrhoehe - scr->BarHeight - 1;
	} else {
		fensterform = TRUE;
		if (fenp[HAUPT].y == -1) fenp[HAUPT].y = scr->BarHeight + 1;
		winx = fenp[HAUPT].x; winy = fenp[HAUPT].y;
		winbr = fenp[HAUPT].b; winho = fenp[HAUPT].h;
	}

	hfenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, winx,
		WA_Top, winy,
		WA_Width, winbr,
		WA_Height, winho,
		WA_MinWidth, 615 + scr->WBorLeft + scr->WBorRight,
		WA_MinHeight, 240,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_IDCMP, IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_NEWSIZE |
					IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK |
					IDCMP_IDCMPUPDATE | IDCMP_CLOSEWINDOW
					| IDCMP_EXTENDEDMOUSE
					,
		WA_SizeGadget, fensterform,
		WA_DragBar, fensterform,
		WA_DepthGadget, fensterform,
		WA_CloseGadget, fensterform,
		WA_Borderless, !fensterform,
		WA_Backdrop, !fensterform,
		WA_NewLookMenus, TRUE,
		WA_ReportMouse, TRUE,
		WA_SmartRefresh, TRUE,
		WA_Activate, TRUE,
		TAG_DONE);
	if (!hschirm) IIntuition->UnlockPubScreen(NULL, scr);
	IGraphics->SetDrMd(hfenster->RPort, JAM1);
	IGraphics->SetFont(hfenster->RPort, font);
	randlr = hfenster->BorderLeft + hfenster->BorderRight;
	randou = hfenster->BorderTop + hfenster->BorderBottom;
	ErstelleFarben();
	ILayers->InstallLayerHook(hfenster->RPort->Layer, backfill);
	aktfenster = hfenster;
}

void EntferneHauptfenster(void) {
	HoleFensterWinpos(hfenster, HAUPT);
	EntferneFarben();
	IIntuition->CloseWindow(hfenster); hfenster = NULL; aktfenster = NULL;
}

void EntferneBildschirm(void) {
	if (hschirm) {
		while (!IIntuition->CloseScreen(hschirm)) {
			aktfenster = IIntuition->OpenWindowTags(NULL, WA_PubScreen, hschirm, WA_Width, 16, WA_Height, 16, WA_Borderless, TRUE, WA_Backdrop, TRUE, TAG_DONE);
			Meldung((STRPTR)CAT(MSG_0141, "There are open windows.\nCannot close screen."));
			if (aktfenster) {
				IIntuition->CloseWindow(aktfenster); aktfenster = NULL;
			}
		}
		hschirm = NULL;
	}
}

void FensterTitel(STRPTR projekt) {
	int16 l;

	l = strlen(projekt);
	if (l > 0) {
		if (l <= 30) {
			if (verLITE) sprintf(fenstertitel, "Horny Lite [ %s ]", projekt);
			else sprintf(fenstertitel, "Horny [ %s ]", projekt);
		} else {
			projekt = (STRPTR)((uint32)projekt + l - 30);
			if (verLITE) sprintf(fenstertitel, "Horny Lite [ ...%s ]", projekt);
			else sprintf(fenstertitel, "Horny [ ...%s ]", projekt);
		}
	} else {
		if (verLITE) strcpy(fenstertitel, "Horny Lite");
		else strcpy(fenstertitel, "Horny");
	}
	if (!(hschirm && umgebung.backdrop)) {
		if (verLITE) IIntuition->SetWindowTitles(hfenster, fenstertitel, "Inutilis Horny Lite Midi-Sequencer");
		else IIntuition->SetWindowTitles(hfenster, fenstertitel, "Inutilis Horny Midi-Sequencer");
	} else {
		IIntuition->SetWindowTitles(hfenster, (STRPTR)-1, fenstertitel);
	}
}

void SetzeFont(void) {
	IGraphics->SetDrMd(aktfenster->RPort, JAM1);
	IGraphics->SetFont(aktfenster->RPort, font);
}

void Fett(BOOL f) {
	if (f) {
		IGraphics->SetSoftStyle(aktfenster->RPort, FSF_BOLD, FSF_BOLD);
	} else {
		IGraphics->SetSoftStyle(aktfenster->RPort, FS_NORMAL, FSF_BOLD);
	}
}

void EntferneSprungliste(void) {
	struct Node *node;

	while ((node = IExec->RemTail(&markerlist))) {
		IChooser->FreeChooserNode(node);
	}
}

void AktualisiereSprungliste(void) {
	struct Node *node;
	struct MARKER *mark;
	int8 n;

	if (!markerlist.lh_Head) IExec->NewList(&markerlist);
	if (hfenster) IIntuition->SetAttrs(gadget[GAD_T_MARKER], CHOOSER_Labels, NULL, TAG_DONE);
	EntferneSprungliste();

	for (n = 0; n < 20; n++) sprung[n] = NULL;
	IExec->NewList(&markerlist);
	mark = TaktMarker(rootmark, M_TEXT, 0);
	n = 0;
	while (mark) {
		node = IChooser->AllocChooserNode(CNA_Text, &mark->text, TAG_DONE);
		if (node) IExec->AddTail(&markerlist, node);
		sprung[n] = mark;
		mark = NextMarker(mark);
		n++; if (n == 20) break;
	}
	if (hfenster) IIntuition->SetAttrs(gadget[GAD_T_MARKER], CHOOSER_Labels, &markerlist, TAG_DONE);
}

void ErstelleGadgets(void) {
	int16 y;

	y = hfenster->Height - hfenster->BorderBottom - 34;

	gadget[GAD_T_PREV] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Top, y, GA_Left, hfenster->BorderLeft + 6,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_T_PREV, GA_Image, bmo[IMG_PREV], BUTTON_BevelStyle, BVS_NONE,
		TAG_DONE);

	gadget[GAD_T_NEXT] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_T_PREV],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 36,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_T_NEXT, GA_Image, bmo[IMG_NEXT], BUTTON_BevelStyle, BVS_NONE,
		TAG_DONE);

	gadget[GAD_T_STOP] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_T_NEXT],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 76,
		GA_Width, 40, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_T_STOP, GA_Image, bmo[IMG_STOP], BUTTON_BevelStyle, BVS_NONE,
		TAG_DONE);

	gadget[GAD_T_PLAY] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_T_STOP],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 120,
		GA_Width, 40, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_T_PLAY, GA_Image, bmo[IMG_PLAY], BUTTON_BevelStyle, BVS_NONE,
		TAG_DONE);

	gadget[GAD_T_REC] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_T_PLAY],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 164,
		GA_Width, 40, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_T_REC, GA_Image, bmo[IMG_REC], BUTTON_BevelStyle, BVS_NONE,
		TAG_DONE);

	gadget[GAD_T_MARKER] = (struct Gadget *)IIntuition->NewObject(ChooserClass, NULL,
		GA_Previous, gadget[GAD_T_REC],
		GA_Top, hfenster->BorderTop + 1, GA_Left, hfenster->BorderLeft + 1,
		GA_Width, 20, GA_Height, 14, GA_RelVerify, TRUE,
		GA_ID, GAD_T_MARKER,
		CHOOSER_DropDown, TRUE,
		CHOOSER_Labels, NULL,
		CHOOSER_AutoFit, TRUE,
		CHOOSER_MaxLabels, 20,
		TAG_DONE);

	gadget[GAD_S_H] = (struct Gadget *)IIntuition->NewObject(ScrollerClass, NULL,
		GA_BackFill, backfill,
		GA_Previous, gadget[GAD_T_MARKER],
		GA_Top, y - guibox - 20, GA_Left, hfenster->BorderLeft + 62,
		GA_Width, hfenster->Width - randlr - 64, GA_Height, 15,
		GA_RelVerify, TRUE,
		GA_ID, GAD_S_H,
		SCROLLER_Top, gui.takt >> VIERTEL,
		SCROLLER_Visible, gui.tasicht,
		SCROLLER_Total, lied.taktanz,
		SCROLLER_Orientation, SORIENT_HORIZ,
		ICA_TARGET, ICTARGET_IDCMP,
		ICA_MAP, guimapscroller,
		TAG_DONE);

	gadget[GAD_S_V] = (struct Gadget *)IIntuition->NewObject(ScrollerClass, NULL,
		GA_BackFill, backfill,
		GA_Previous, gadget[GAD_S_H],
		GA_Top, hfenster->BorderTop + 62, GA_Left, hfenster->Width - hfenster->BorderRight - 17,
		GA_Width, 15, GA_Height, hfenster->Height - randou - 118 - guibox,
		GA_RelVerify, TRUE,
		GA_ID, GAD_S_V,
		SCROLLER_Top, gui.spur,
		SCROLLER_Visible, gui.spsicht,
		SCROLLER_Total, lied.spuranz + 1,
		SCROLLER_Orientation, SORIENT_VERT,
		ICA_TARGET, ICTARGET_IDCMP,
		ICA_MAP, guimapscroller,
		TAG_DONE);

	gadget[GAD_Z_H] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
		GA_BackFill, backfill,
		GA_Previous, gadget[GAD_S_V],
		GA_Top, y - guibox - 20, GA_Left, hfenster->BorderLeft + 2,
		GA_Width, 58, GA_Height, 15,
		GA_RelVerify, TRUE,
		GA_ID, GAD_Z_H,
		SLIDER_Min, 2, SLIDER_Max, 64,
		SLIDER_Level, gui.tab,
		SLIDER_Orientation, SORIENT_HORIZ,
		ICA_TARGET, ICTARGET_IDCMP,
		ICA_MAP, guimapslider,
		TAG_DONE);

	gadget[GAD_Z_V] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
		GA_BackFill, backfill,
		GA_Previous, gadget[GAD_Z_H],
		GA_Top, hfenster->BorderTop + 2, GA_Left, hfenster->Width - hfenster->BorderRight - 17,
		GA_Width, 15, GA_Height, 58,
		GA_RelVerify, TRUE,
		GA_ID, GAD_Z_V,
		SLIDER_Min, 8, SLIDER_Max, 100,
		SLIDER_Level, gui.sph,
		SLIDER_Orientation, SORIENT_VERT,
		ICA_TARGET, ICTARGET_IDCMP,
		ICA_MAP, guimapslider,
		TAG_DONE);

	gadget[GAD_F_MREC] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_Z_V],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 222,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_F_MREC, GA_Image, bmo[IMG_MREC], BUTTON_BevelStyle, BVS_NONE,
		GA_Selected, metro.rec,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[GAD_F_MPLAY] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_F_MREC],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 252,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_F_MPLAY, GA_Image, bmo[IMG_MPLAY], BUTTON_BevelStyle, BVS_NONE,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[GAD_F_LOOP] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_F_MPLAY],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 282,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_F_LOOP, GA_Image, bmo[IMG_LOOP], BUTTON_BevelStyle, BVS_NONE,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[GAD_F_FOLLOW] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_F_LOOP],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 312,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_F_FOLLOW, GA_Image, bmo[IMG_FOLLOW], BUTTON_BevelStyle, BVS_NONE,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[GAD_F_THRU] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_F_FOLLOW],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 342,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_F_THRU, GA_Image, bmo[IMG_THRU], BUTTON_BevelStyle, BVS_NONE,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	gadget[GAD_F_SYNC] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
		GA_Previous, gadget[GAD_F_THRU],
		GA_Top, y, GA_Left, hfenster->BorderLeft + 372,
		GA_Width, 27, GA_Height, 30, GA_RelVerify, TRUE,
		GA_ID, GAD_F_SYNC, GA_Image, bmo[IMG_SYNC], BUTTON_BevelStyle, BVS_NONE,
		BUTTON_PushButton, TRUE,
		TAG_DONE);

	IIntuition->AddGList(hfenster, gadget[0], 0, GADANZ, NULL);
	IIntuition->RefreshGList(gadget[0], hfenster, NULL, -1);
}

void EntferneGadgets(void) {
	int8 n;

	IIntuition->RemoveGList(hfenster, gadget[0], GADANZ);
	for(n = 0; n < GADANZ; n++) {
		if (gadget[n]) {
			IIntuition->DisposeObject((Object *)gadget[n]);
			gadget[n] = NULL;
		}
	}
}

void PositioniereGadgets(void) {
	int16 y;

	y = hfenster->Height - hfenster->BorderBottom - 34;
	IIntuition->RemoveGList(hfenster, gadget[0], GADANZ);
	IIntuition->RefreshWindowFrame(hfenster);

	IIntuition->SetAttrs(gadget[GAD_T_PREV], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_T_NEXT], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_T_STOP], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_T_PLAY], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_T_REC], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_S_H],
		GA_Top, y - guibox - 20,
		GA_Width, hfenster->Width - randlr - 64,
		TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_S_V],
		GA_Left, hfenster->Width - hfenster->BorderRight - 17,
		GA_Height, hfenster->Height - randou - 118 - guibox,
		TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_Z_H], GA_Top, y - guibox - 20, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_Z_V], GA_Left, hfenster->Width - hfenster->BorderRight - 17, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_F_MREC], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_F_MPLAY], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_F_LOOP], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_F_FOLLOW], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_F_THRU], GA_Top, y, TAG_DONE);
	IIntuition->SetAttrs(gadget[GAD_F_SYNC], GA_Top, y, TAG_DONE);


	IIntuition->AddGList(hfenster, gadget[0], 0, GADANZ, NULL);
	IIntuition->RefreshGList(gadget[0], hfenster, NULL, -1);
}

void AktualisiereGadgets(void) {
	int16 totalhoriz;
	int16 totalvert;

	totalhoriz = (gui.takt >> VIERTEL) + gui.tasicht;
	if (totalhoriz < lied.taktanz) totalhoriz = lied.taktanz;

	totalvert = lied.spuranz + 1;
	if (lied.spuranz - gui.spur < gui.spsicht) totalvert = gui.spur + gui.spsicht;

	IIntuition->SetGadgetAttrs(gadget[GAD_S_H], hfenster, NULL,
		SCROLLER_Top, gui.takt >> VIERTEL,
		SCROLLER_Visible, gui.tasicht,
		SCROLLER_Total, totalhoriz,
		TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_S_V], hfenster, NULL,
		SCROLLER_Top, gui.spur,
		SCROLLER_Visible, gui.spsicht,
		SCROLLER_Total, totalvert,
		TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_Z_H], hfenster, NULL, SLIDER_Level, gui.tab, TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_Z_V], hfenster, NULL, SLIDER_Level, gui.sph, TAG_DONE);
}

void AktualisiereFunctGadgets(void) {
	IIntuition->SetGadgetAttrs(gadget[GAD_F_MREC], hfenster, NULL, GA_Selected, metro.rec, TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_F_MPLAY], hfenster, NULL, GA_Selected, metro.play, TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_F_LOOP], hfenster, NULL, GA_Selected, loop.aktiv, TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_F_FOLLOW], hfenster, NULL, GA_Selected, gui.folgen, TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_F_THRU], hfenster, NULL, GA_Selected, outport[spur[snum].port].thru, TAG_DONE);
	IIntuition->SetGadgetAttrs(gadget[GAD_F_SYNC], hfenster, NULL, GA_Selected, IstExtreamSyncAktiv(), TAG_DONE);
}

void HoleFensterWinpos(struct Window *fenster, uint8 fenid) {
	if (fenster) {
		if ((fenid != HAUPT) || fenster->Flags & WFLG_DRAGBAR) {
			fenp[fenid].x = fenster->LeftEdge;
			fenp[fenid].y = fenster->TopEdge;
			fenp[fenid].b = fenster->Width;
			fenp[fenid].h = fenster->Height;
		}
	}
}

void HoleFensterObjpos(Object *fensterobj, uint8 fenid) {
	uint32 x, y, w, h;
	if (fensterobj) {
		#if __amigaos4__
		IIntuition->GetAttrs(fensterobj,
			WA_Left, &x,
			WA_Top, &y,
			WA_InnerWidth, &w,
			WA_InnerHeight, &h,
			TAG_DONE);
		#else
		IIntuition->GetAttr(WA_Left, fensterobj, &x);
		IIntuition->GetAttr(WA_Top, fensterobj, &y);
		IIntuition->GetAttr(WA_InnerWidth, fensterobj, &w);
		IIntuition->GetAttr(WA_InnerHeight, fensterobj, &h);
		#endif

		fenp[fenid].x = x;
		fenp[fenid].y = y;
		fenp[fenid].b = w;
		fenp[fenid].h = h;
	}
}

void HoleAlleFensterpos(void) {
	HoleFensterWinpos(hfenster, HAUPT);
	HoleFensterObjpos(setfensterobj, SET);
	HoleFensterObjpos(envfensterobj, ENV);
	HoleFensterWinpos(edfenster, ED);
	HoleFensterObjpos(sexfensterobj, SEX);
	HoleFensterWinpos(mpfenster, MISCHER);
	HoleFensterObjpos(ccfensterobj, CC);
}

void BildFrei(void) {
	IGraphics->SetAPen(aktfenster->RPort, guifar[0]);
	IGraphics->RectFill(aktfenster->RPort, aktfenster->BorderLeft, aktfenster->BorderTop, aktfenster->Width - aktfenster->BorderRight - 1, aktfenster->Height - aktfenster->BorderBottom - 1);
	if (aktfenster == hfenster) altposx = -1;
}

void Schreibe(uint8 f, int16 x, int16 y, STRPTR t, int16 xe) {
	int32 l;
	struct TextExtent te;

	IGraphics->Move(aktfenster->RPort, x + aktfenster->BorderLeft, y + aktfenster->BorderTop);
	IGraphics->SetAPen(aktfenster->RPort, guifar[f]);
	if (xe < x) xe = x;
	l = IGraphics->TextFit(aktfenster->RPort, t, strlen(t), &te, NULL, 1, xe - x, 100);
	IGraphics->Text(aktfenster->RPort, t, l);
}

void SchreibeSys(uint8 f, int16 x, int16 y, STRPTR t) {
	IGraphics->Move(aktfenster->RPort, x, y);
	IGraphics->SetAPen(aktfenster->RPort, f);
	IGraphics->Text(aktfenster->RPort, t, strlen(t));
}

void SchreibeZahl(uint8 f, int16 x, int16 y, int16 z) {
	char t[10];
	sprintf(t, "%ld", (int32)z); 
	Schreibe(f, x, y, t, x + 100);
}

void Gradient(uint8 f, uint8 stil, int16 x1, int16 y1, int16 x2, int16 y2) {
	if ((x2 >= x1) && (y2 >= y1)) {
		gradspec.Specs.Rel.BasePen = guifar[f];
		switch (stil) {
		case STIL_DH:
			gradspec.Direction = IIntuition->DirectionVector(0);
			gradspec.Specs.Rel.Contrast[0] = 20;
			gradspec.Specs.Rel.Contrast[1] = 20;
			break;
		case STIL_HD:
			gradspec.Direction = IIntuition->DirectionVector(180);
			gradspec.Specs.Rel.Contrast[0] = 20;
			gradspec.Specs.Rel.Contrast[1] = 20;
			break;
		case STIL_DN:
			gradspec.Direction = IIntuition->DirectionVector(0);
			gradspec.Specs.Rel.Contrast[0] = 40;
			gradspec.Specs.Rel.Contrast[1] = 0;
			break;
		case STIL_ND:
			gradspec.Direction = IIntuition->DirectionVector(180);
			gradspec.Specs.Rel.Contrast[0] = 40;
			gradspec.Specs.Rel.Contrast[1] = 0;
			break;
		}

		IIntuition->DrawGradient(aktfenster->RPort, x1 + aktfenster->BorderLeft, y1 + aktfenster->BorderTop, x2 - x1 + 1, y2 - y1 + 1, NULL, 0L, &gradspec, drinfo);
	}
}

void Balken(uint8 f, int16 x1, int16 y1, int16 x2, int16 y2) {
	IGraphics->SetAPen(aktfenster->RPort, guifar[f]);
	IGraphics->RectFill(aktfenster->RPort, x1 + aktfenster->BorderLeft, y1 + aktfenster->BorderTop, x2 + aktfenster->BorderLeft, y2 + aktfenster->BorderTop);
}

void Rahmen(uint8 f1, uint8 f2, int16 x1, int16 y1, int16 x2, int16 y2) {
	x1 = x1 + aktfenster->BorderLeft; x2 = x2 + aktfenster->BorderLeft;
	y1 = y1 + aktfenster->BorderTop; y2 = y2 + aktfenster->BorderTop;
	IGraphics->SetAPen(aktfenster->RPort, guifar[f2]); IGraphics->Move(aktfenster->RPort, x1, y2);
	IGraphics->Draw(aktfenster->RPort, x2, y2); IGraphics->Draw(aktfenster->RPort, x2, y1);
	IGraphics->SetAPen(aktfenster->RPort, guifar[f1]);
	IGraphics->Draw(aktfenster->RPort, x1, y1); IGraphics->Draw(aktfenster->RPort, x1, y2);
}

void RahmenEin(int16 f, uint8 stil, int16 x1, int16 y1, int16 x2, int16 y2) {
	Rahmen(1, 2, x1, y1, x2, y2);
	if (f >= 0) {
		if (stil)
			Gradient(f, stil, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
		else
		Balken(f, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	}
}

void RahmenAus(int16 f, uint8 stil, int16 x1, int16 y1, int16 x2, int16 y2) {
	Rahmen(2, 1, x1, y1, x2, y2);
	if (f >= 0) {
		if (stil)
			Gradient(f, stil, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
		else
		Balken(f, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	}
}

void RahmenRundungEin(int16 x1, int16 y1, int16 x2, int16 y2) {
	Rahmen(4, 8, x1, y1, x2, y2);
}

void RahmenRundungAus(int16 x1, int16 y1, int16 x2, int16 y2) {
	Rahmen(8, 4, x1, y1, x2, y2);
}

void Linie(uint8 f, int16 x1, int16 y1, int16 x2, int16 y2) {
	IGraphics->SetAPen(aktfenster->RPort, guifar[f]);
	IGraphics->Move(aktfenster->RPort, x1 + aktfenster->BorderLeft, y1 + aktfenster->BorderTop); IGraphics->Draw(aktfenster->RPort, x2 + aktfenster->BorderLeft, y2 + aktfenster->BorderTop);
}

void PunktLinie(uint8 f, int16 x1, int16 y1, int16 x2, int16 y2) {
	hfenster->RPort->LinePtrn = 0x5555;
	IGraphics->SetAPen(aktfenster->RPort, guifar[f]);
	IGraphics->Move(aktfenster->RPort, x1 + aktfenster->BorderLeft, y1 + aktfenster->BorderTop); IGraphics->Draw(aktfenster->RPort, x2 + aktfenster->BorderLeft, y2 + aktfenster->BorderTop);
	hfenster->RPort->LinePtrn = 0xFFFF;
}

void Farbe(uint8 f) {
	IGraphics->SetAPen(aktfenster->RPort, guifar[f]);
}

void Punkt(int16 x, int16 y) {
	IGraphics->WritePixel(aktfenster->RPort, x + aktfenster->BorderLeft, y + aktfenster->BorderTop);
}

void ZeichnePosition(BOOL edit) {
	int16 x;
	int16 uy, rr, lr;

	//Hauptfenster
	aktfenster = hfenster;
	IGraphics->SetDrMd(hfenster->RPort, COMPLEMENT);
	uy = hfenster->Height - randou - 56 - guibox;
	lr = gui.spalte + 2; rr = hfenster->Width - randlr - 21;
	x = lr + ((((takt - gui.takt) >> (VIERTEL - 2)) * gui.tab) >> 4);
	if (x != altposx) {
		if (altposx >= 0) Balken(1, altposx, guileiste + 19, altposx + 1, uy);
		if ((x >= lr) && (x < rr)) {
			Balken(1, x, guileiste + 19, x + 1, uy);
			altposx = x;
		} else altposx = -1;
	}
	if (edit) {
		x = lr + ((((edittakt - gui.takt) >> (VIERTEL - 2)) * gui.tab) >> 4);
		if (x != alteditx) {
			hfenster->RPort->LinePtrn = 0xF0F0;
			if (alteditx >= 0) Linie(1, alteditx, guileiste + 19, alteditx, uy);
			if ((x >= lr) && (x < rr)) {
				Linie(1, x, guileiste + 19, x, uy);
				alteditx = x;
			} else alteditx = -1;
			hfenster->RPort->LinePtrn = 0xFFFF;
		}
	}
	IGraphics->SetDrMd(hfenster->RPort, JAM1);

	//Editfenster
	if (edfenster) {
		aktfenster = edfenster;
		IGraphics->SetDrMd(edfenster->RPort, COMPLEMENT);
		uy = edfenster->Height - edou - edguibox - 59;
		lr = 72; rr = edfenster->Width - edlr - 21;
		x = lr + (((takt - edgui.takt) * edgui.taktb) >> (VIERTEL - 2));

		if (x != edaltposx) {
			if (edaltposx >= 0) Balken(1, edaltposx, 44, edaltposx + 1, uy);
			if ((x >= lr) && (x < rr)) {
				Balken(1, x, 44, x + 1, uy);
				edaltposx = x;
			} else edaltposx = -1;
		}
		if (edit) {
			x = lr + (((edittakt - edgui.takt) * edgui.taktb) >> (VIERTEL - 2));
			if (x != edalteditx) {
				edfenster->RPort->LinePtrn = 0xF0F0;
				if (edalteditx >= 0) Linie(1, edalteditx, 44, edalteditx, uy);
				if ((x >= lr) && (x < rr)) {
					Linie(1, x, 44, x, uy);
					edalteditx = x;
				} else edalteditx = -1;
				edfenster->RPort->LinePtrn = 0xFFFF;
			}
		}
		IGraphics->SetDrMd(edfenster->RPort, JAM1);
	}
}

void KeinePosition(void) {
	int16 uy;

	//Hauptfenster
	aktfenster = hfenster;
	IGraphics->SetDrMd(hfenster->RPort, COMPLEMENT);
	uy = hfenster->Height - randou - 56 - guibox;
	if (altposx >= 0) {
		Balken(1, altposx, guileiste + 19, altposx + 1, uy);
		altposx = -1;
	}
	if (alteditx >= 0) {
		hfenster->RPort->LinePtrn = 0xF0F0;
		Linie(1, alteditx, guileiste + 19, alteditx, uy);
		alteditx = -1;
		hfenster->RPort->LinePtrn = 0xFFFF;
	}
	IGraphics->SetDrMd(hfenster->RPort, JAM1);

	//Editfenster
	if (edfenster) {
		aktfenster = edfenster;
		IGraphics->SetDrMd(edfenster->RPort, COMPLEMENT);
		uy = edfenster->Height - edou - edguibox - 59;
		if (edaltposx >= 0) {
			Balken(1, edaltposx, 44, edaltposx + 1, uy);
			edaltposx = -1;
		}
		if (edalteditx >= 0) {
			edfenster->RPort->LinePtrn = 0xF0F0;
			Linie(1, edalteditx, 44, edalteditx, uy);
			edalteditx = -1;
			edfenster->RPort->LinePtrn = 0xFFFF;
		}
		IGraphics->SetDrMd(edfenster->RPort, JAM1);
	}
}

void ZeichneProjektion(int32 start, int32 ende) {
	int16 lr, rr;
	int16 xs, xe;

	IGraphics->SetDrMd(hfenster->RPort, COMPLEMENT);
	aktfenster = hfenster;
	lr = gui.spalte + 2;
	rr = hfenster->Width - randlr - 20;

	if (altprjstart != -1) Balken(6, altprjstart, guileiste + 10, altprjende, guileiste + 14);
	xs = gui.spalte + 2 + ((((start - gui.takt) >> VIERTEL) * gui.tab) >> 2);
	xe = gui.spalte + 1 + ((((ende - gui.takt) >> VIERTEL) * gui.tab) >> 2);
	if (!((xs < lr) && (xe < lr)) && !((xs > rr) && (xe > rr))) {
		if (xs < lr) xs = lr;
		if (xe > rr) xe = rr;
		Balken(6, xs, guileiste + 10, xe, guileiste + 14);
		altprjstart = xs;
		altprjende = xe;
	}
	IGraphics->SetDrMd(hfenster->RPort, JAM1);
}

void KeineProjektion(void) {
	IGraphics->SetDrMd(hfenster->RPort, COMPLEMENT);
	aktfenster = hfenster;
	if (altprjstart != -1) Balken(6, altprjstart, guileiste + 10, altprjende, guileiste + 14);
	altprjstart = -1;
	IGraphics->SetDrMd(hfenster->RPort, JAM1);
}

void Lasso(int16 x1, int16 y1, int16 x2, int16 y2) {
	IGraphics->SetDrMd(aktfenster->RPort, COMPLEMENT);

	IGraphics->Move(aktfenster->RPort, x1, y1);
	IGraphics->Draw(aktfenster->RPort, x2, y1);
	IGraphics->Draw(aktfenster->RPort, x2, y2);
	IGraphics->Draw(aktfenster->RPort, x1, y2);
	IGraphics->Draw(aktfenster->RPort, x1, y1 + 1);

	x1++; x2--; y1++; y2--;
	IGraphics->Move(aktfenster->RPort, x1, y1);
	IGraphics->Draw(aktfenster->RPort, x2, y1);
	IGraphics->Draw(aktfenster->RPort, x2, y2);
	IGraphics->Draw(aktfenster->RPort, x1, y2);
	IGraphics->Draw(aktfenster->RPort, x1, y1 + 1);

	IGraphics->SetDrMd(aktfenster->RPort, JAM1);
}

BOOL LassoWahl(int16 mx1, int16 my1, int16 *mx2, int16 *my2) {
	BOOL verlassen = FALSE;
	struct IntuiMessage *mes;

	*mx2 = -1;
	do {
		IExec->WaitPort(aktfenster->UserPort);
		while ((mes = (struct IntuiMessage *)IExec->GetMsg(aktfenster->UserPort))) {
			switch (mes->Class) {
				case IDCMP_MOUSEMOVE:
				if (*mx2 >= 0) Lasso(mx1, my1, *mx2, *my2);
				*mx2 = mes->MouseX; *my2 = mes->MouseY;
				if (*mx2 < 0) *mx2 = 0;
				Lasso(mx1, my1, *mx2, *my2);
				break;

				case IDCMP_MOUSEBUTTONS: verlassen = TRUE; break;
			}
			IExec->ReplyMsg((struct Message *)mes);
		}
	} while (!verlassen);
	if (*mx2 >= 0) {
		Lasso(mx1, my1, *mx2, *my2);
		return(TRUE);
	}
	return(FALSE);
}

void SchleifeUpdate(struct TagItem *tags, int16 *id, int16 *code) {
	while (tags->ti_Tag != TAG_DONE) {
		if (tags->ti_Tag == GA_ID) *id = tags->ti_Data;
		if (tags->ti_Tag == ICSPECIAL_CODE) *code = tags->ti_Data;
		tags++;
	}
}
