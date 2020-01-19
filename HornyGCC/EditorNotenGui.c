#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/button.h>
#include <proto/scroller.h>
#include <proto/slider.h>
#include <proto/chooser.h>

#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <gadgets/button.h>
#include <gadgets/scroller.h>
#include <gadgets/slider.h>
#include <gadgets/chooser.h>
#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "GuiFenster.h"
#include "Gui.h"
#include "Gui2.h"
#include "Marker.h"
#include "Menu.h"
#include "DTGrafik.h"
#include "Undo.h"

#include "oca.h"

#define EDGADANZ 18

extern struct TextFont *font;
extern struct Menu *edmenu;
extern struct Hook *backfill;

extern struct Screen *hschirm;
extern struct Window *hfenster;
extern struct Window *aktfenster;
extern struct FENSTERPOS fenp[];

struct Window *edfenster = NULL;
struct Gadget *edgad[EDGADANZ];

extern char fenstertitel[300];
char edtitel[300];

int16 edlr;
int16 edou;
int16 edguibox = 20;

extern struct GUI gui;
extern struct SPUR spur[];
extern struct LIED lied;
extern struct UMGEBUNG umgebung;

extern struct SEQUENZ *edseq;
extern struct EVENT *wahlnote;

extern int32 takt;
extern int32 edittakt;
extern int8 editnote;
int8 alteditnote = -1;

struct List edchlist;

struct EDGUI edgui = {
	EDMODUS_NOTEN, //modus
	14, //taste
	12, //tasth
	0, //tastsicht
	0, //contr
	60, //contrh
	0, //contrsicht
	1, //contranz
	0, //takt
	8, //taktb
	0, //takbsicht
	VIERTEL - 2, //raster
	2, //neulen
	FALSE //tripled
};

char oktnote[12][3] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "H "};
extern int8 ganz2halb[77];
extern int8 halb2ganz[132];

struct EVENT *startev[128];

struct TagItem mapscroller[] = {{SCROLLER_Top, ICSPECIAL_CODE}, {TAG_DONE}};
struct TagItem mapslider[] = {{SLIDER_Level, ICSPECIAL_CODE}, {TAG_DONE}};


void ErstelleEditorNotenFenster(void) {
	struct TextAttr textattr = {"helvetica.font", 11, FS_NORMAL, 0};
	int16 y;
	int8 n;
	STRPTR rasttxt[6] = {(STRPTR)"1/4", (STRPTR)"1/8", (STRPTR)"1/16", (STRPTR)"1/32", (STRPTR)"1/64", CAT(MSG_0505, "free")};
	STRPTR nlentxt[6] = {(STRPTR)"1/1", (STRPTR)"1/2", (STRPTR)"1/4", (STRPTR)"1/8", (STRPTR)"1/16", (STRPTR)"1/32"};

	if (fenp[ED].y == -1) {
		fenp[ED].x = hfenster->LeftEdge + 80;
		fenp[ED].y = hfenster->TopEdge + 40;
		fenp[ED].b = hfenster->Width - 160; if (fenp[ED].b < 400) fenp[ED].b = 400;
		fenp[ED].h = hfenster->Height - 80; if (fenp[ED].h < 300) fenp[ED].h = 300;
	}

	edfenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fenp[ED].x,
		WA_Top, fenp[ED].y,
		WA_Width, fenp[ED].b,
		WA_Height, fenp[ED].h,
		WA_MinWidth, 420,
		WA_MinHeight, 300,
		WA_MaxWidth, ~0,
		WA_MaxHeight, ~0,
		WA_IDCMP, IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_NEWSIZE |
					IDCMP_IDCMPUPDATE | IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN |
					IDCMP_MENUPICK | IDCMP_CLOSEWINDOW | IDCMP_EXTENDEDMOUSE,
		WA_SizeGadget, TRUE,
		WA_DragBar, TRUE,
		WA_DepthGadget, TRUE,
		WA_CloseGadget, TRUE,
		WA_NewLookMenus, TRUE,
		WA_ReportMouse, TRUE,
		WA_SmartRefresh, TRUE,
		WA_Activate, TRUE,
		WA_BackFill, backfill,
		TAG_DONE);

	if (edfenster) {
		IGraphics->SetDrMd(edfenster->RPort, JAM1);
		IGraphics->SetFont(edfenster->RPort, font);
		edlr = edfenster->BorderLeft + edfenster->BorderRight;
		edou = edfenster->BorderTop + edfenster->BorderBottom;

		//Gadgets
		edgad[0] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
			GA_Top, edfenster->BorderTop + 2, GA_Left, edfenster->BorderLeft + 2,
			GA_Height, 20, GA_Width, edfenster->Width - edlr - 4,
			GA_RelVerify, TRUE, GA_ID, 0,
			GA_TextAttr, &textattr,
			GA_Text, CAT(MSG_0512, "Piano Editor <-> Controller Editor"),
			TAG_DONE);

		edgad[1] = (struct Gadget *)IIntuition->NewObject(ScrollerClass, NULL,
			GA_Previous, edgad[0],
			GA_Top, edfenster->Height - edfenster->BorderBottom - edguibox - 55, GA_Left, edfenster->BorderLeft + 62,
			GA_Width, edfenster->Width - edlr - 64, GA_Height, 15,
			GA_RelVerify, TRUE, GA_ID, 1,
			SCROLLER_Top, 0,
			SCROLLER_Visible, 10,
			SCROLLER_Total, 10,
			SCROLLER_Orientation, SORIENT_HORIZ,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapscroller,
			TAG_DONE);

		edgad[2] = (struct Gadget *)IIntuition->NewObject(ScrollerClass, NULL,
			GA_Previous, edgad[1],
			GA_Top, edfenster->BorderTop + 84, GA_Left, edfenster->Width - edfenster->BorderRight - 17,
			GA_Width, 15, GA_Height, edfenster->Height - edou - edguibox - 141,
			GA_RelVerify, TRUE, GA_ID, 2,
			SCROLLER_Top, 0,
			SCROLLER_Visible, 75,
			SCROLLER_Total, 75,
			SCROLLER_Orientation, SORIENT_VERT,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapscroller,
			TAG_DONE);

		edgad[3] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
			GA_BackFill, backfill,
			GA_Previous, edgad[2],
			GA_Top, edfenster->Height - edfenster->BorderBottom - edguibox - 55, GA_Left, edfenster->BorderLeft + 2,
			GA_Width, 58, GA_Height, 15,
			GA_RelVerify, TRUE, GA_ID, 3,
			SLIDER_Min, 2, SLIDER_Max, 32,
			SLIDER_Level, 2,
			SLIDER_Orientation, SORIENT_HORIZ,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapslider,
			TAG_DONE);

		edgad[4] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
			GA_BackFill, backfill,
			GA_Previous, edgad[3],
			GA_Top, edfenster->BorderTop + 24, GA_Left, edfenster->Width - edfenster->BorderRight - 17,
			GA_Width, 15, GA_Height, 58,
			GA_RelVerify, TRUE, GA_ID, 4,
			SLIDER_Min, 6, SLIDER_Max, 30,
			SLIDER_Level, 12,
			SLIDER_Orientation, SORIENT_VERT,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapslider,
			TAG_DONE);

		y = edfenster->Height - edfenster->BorderBottom - 35;
		for (n = 0; n < 6; n++) {
			edgad[5 + n] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
				GA_Previous, edgad[4 + n],
				GA_Top, y, GA_Left, edfenster->BorderLeft + 2 + (n * 35),
				GA_Width, 35, GA_Height, 16, GA_RelVerify, TRUE, GA_TextAttr, &textattr,
				GA_ID, 5 + n, GA_Text, rasttxt[n],
				BUTTON_PushButton, TRUE,
				TAG_DONE);
		}

		edgad[17] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
			GA_Previous, edgad[10],
			GA_Top, y, GA_Left, edfenster->BorderLeft + 2 + (6 * 35) + 4,
			GA_Width, 60, GA_Height, 16, GA_RelVerify, TRUE, GA_TextAttr, &textattr,
			GA_ID, 17, GA_Text, CAT(MSG_0508, "Triplet"),
			BUTTON_PushButton, TRUE,
			TAG_DONE);


		y = edfenster->Height - edfenster->BorderBottom - 18;
		for (n = 0; n < 6; n++) {
			edgad[11 + n] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
				GA_Previous, edgad[10 + n],
				GA_Top, y, GA_Left, edfenster->BorderLeft + 2 + (n * 35),
				GA_Width, 35, GA_Height, 16, GA_RelVerify, TRUE, GA_TextAttr, &textattr,
				GA_ID, 11 + n, GA_Text, nlentxt[n],
				BUTTON_PushButton, TRUE,
				TAG_DONE);
		}

		IIntuition->AddGList(edfenster, edgad[0], 0, EDGADANZ, NULL);
		IIntuition->RefreshGList(edgad[0], edfenster, NULL, -1);
		IIntuition->SetMenuStrip(edfenster, edmenu);
		
		aktfenster = edfenster;
		Schreibe(1, 6 + 6 * 35 + 64, edfenster->Height - edou - 25, CAT(MSG_0506, "Grid"), 400);
		Schreibe(1, 6 + 6 * 35, edfenster->Height - edou - 8, CAT(MSG_0507, "Length"), 400);
	}
}

void EntferneEditorNotenFenster(void) {
	int8 n;

	if (edfenster) {
		EntferneAlleEdUndo();

		HoleFensterWinpos(edfenster, ED);

		IIntuition->RemoveGList(edfenster, edgad[0], EDGADANZ);
		for(n = 0; n < EDGADANZ; n++) {
			if (edgad[n]) {
				IIntuition->DisposeObject((Object *)edgad[n]);
				edgad[n] = NULL;
			}
		}
		IIntuition->ClearMenuStrip(edfenster); IIntuition->CloseWindow(edfenster); edfenster = NULL;
	}
}

void EdFensterTitel(void) {
	sprintf(edtitel, CAT(MSG_0514, "Editor [ %s: %s ]     UNDO: %s, REDO: %s"), spur[edseq->spur].name, edseq->name, EdUndoAktion(), EdRedoAktion());
	if (!(hschirm && umgebung.backdrop)) {
		IIntuition->SetWindowTitles(edfenster, edtitel, "Inutilis Horny Midi-Sequencer");
	} else {
		IIntuition->SetWindowTitles(edfenster, edtitel, fenstertitel);
	}
}


void PositioniereEdGadgets(void) {
	int8 n;
	int16 ur;

	IIntuition->RemoveGList(edfenster, edgad[0], EDGADANZ);
	IIntuition->RefreshWindowFrame(edfenster);

	ur = edfenster->Height - edfenster->BorderBottom;

	IIntuition->SetAttrs(edgad[0], GA_Width, edfenster->Width - edlr - 4, TAG_DONE);
	IIntuition->SetAttrs(edgad[1],
		GA_Top, ur - edguibox - 55,
		GA_Width, edfenster->Width - edlr - 64,
		TAG_DONE);
	IIntuition->SetAttrs(edgad[2],
		GA_Left, edfenster->Width - edfenster->BorderRight - 17,
		GA_Height, edfenster->Height - edou - edguibox - 141,
		TAG_DONE);
	IIntuition->SetAttrs(edgad[3], GA_Top, ur - edguibox - 55, TAG_DONE);
	IIntuition->SetAttrs(edgad[4],
		GA_Left, edfenster->Width - edfenster->BorderRight - 17,
		TAG_DONE);

	for(n = 0; n < 6; n++) IIntuition->SetAttrs(edgad[5 + n], GA_Top, ur - 35, TAG_DONE);
	IIntuition->SetAttrs(edgad[17], GA_Top, ur - 35, TAG_DONE);
	for(n = 0; n < 6; n++) IIntuition->SetAttrs(edgad[11 + n], GA_Top, ur - 18, TAG_DONE);

	IIntuition->AddGList(edfenster, edgad[0], 0, EDGADANZ, NULL);
	IIntuition->RefreshGList(edgad[0], edfenster, NULL, -1);

	aktfenster = edfenster;
	Schreibe(1, 6 + 6 * 35 + 64, edfenster->Height - edou - 25, CAT(MSG_0506, "Grid"), 400);
	Schreibe(1, 6 + 6 * 35, edfenster->Height - edou - 8, CAT(MSG_0507, "Length"), 400);
}

void AktualisiereEdGadgets(void) {
	IIntuition->SetGadgetAttrs(edgad[1], edfenster, NULL,
		SCROLLER_Top, edgui.takt >> (VIERTEL - 2),
		SCROLLER_Visible, edgui.taktsicht,
		SCROLLER_Total, lied.taktanz << 2,
		TAG_DONE);
	if (edgui.modus == EDMODUS_NOTEN) {
		IIntuition->SetGadgetAttrs(edgad[2], edfenster, NULL,
			SCROLLER_Top, 75 - edgui.taste - edgui.tastsicht,
			SCROLLER_Visible, edgui.tastsicht,
			SCROLLER_Total, 75,
			TAG_DONE);
		IIntuition->SetGadgetAttrs(edgad[4], edfenster, NULL,
			SLIDER_Min, 6, SLIDER_Max, 30,
			SLIDER_Level, edgui.tasth,
			TAG_DONE);
	} else {
		IIntuition->SetGadgetAttrs(edgad[2], edfenster, NULL,
			SCROLLER_Top, edgui.contr,
			SCROLLER_Visible, edgui.contrsicht,
			SCROLLER_Total, edgui.contranz + 1,
			TAG_DONE);
		IIntuition->SetGadgetAttrs(edgad[4], edfenster, NULL,
			SLIDER_Min, 20, SLIDER_Max, 160,
			SLIDER_Level, edgui.contrh,
			TAG_DONE);
	}
	IIntuition->SetGadgetAttrs(edgad[3], edfenster, NULL, SLIDER_Level, edgui.taktb, TAG_DONE);
}

void AktualisiereEdRasterGadgets(void) {
	int8 n;

	for(n = 0; n < 5; n++) {
		if (VIERTEL - edgui.raster == n) IIntuition->SetGadgetAttrs(edgad[5 + n], edfenster, NULL, GA_Selected, TRUE, TAG_DONE);
		else IIntuition->SetGadgetAttrs(edgad[5 + n], edfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	}
	if (edgui.raster == 0) IIntuition->SetGadgetAttrs(edgad[10], edfenster, NULL, GA_Selected, TRUE, TAG_DONE);
	else IIntuition->SetGadgetAttrs(edgad[10], edfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	IIntuition->SetGadgetAttrs(edgad[17], edfenster, NULL, GA_Selected, edgui.tripled, TAG_DONE);
}

void AktualisiereEdNeuLenGadgets(void) {
	int8 n;

	for(n = 0; n < 6; n++) {
		if (edgui.neulen == n) IIntuition->SetGadgetAttrs(edgad[11 + n], edfenster, NULL, GA_Selected, TRUE, TAG_DONE);
		else IIntuition->SetGadgetAttrs(edgad[11 + n], edfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	}
}

void AktualisiereEdModus(void) {
//	SetGadgetAttrs(edgad[0], edfenster, NULL, GA_Text, "Piano-Editor", TAG_DONE);
	if (edgui.modus == EDMODUS_NOTEN) {
		MenuDeaktivieren(1, 3, FALSE);
		MenuDeaktivieren(1, 4, TRUE);
	} else {
		MenuDeaktivieren(1, 3, TRUE);
		MenuDeaktivieren(1, 4, FALSE);
	}
}


//============

void ZeichneTastatur(void) {
	int8 oktave;
	int8 ton;
	int16 ntaste;
	int16 y, oy;
	int16 schwarz;
	int16 max;

	aktfenster = edfenster;
	oy = 43;
	y = edfenster->Height - edou - edguibox - 58;

	edgui.tastsicht = (y - oy - 1) / (edgui.tasth + 1);
	if (edgui.taste + edgui.tastsicht > 75) {
		if (edgui.tastsicht > 75) edgui.taste = 0; else edgui.taste = 75 - edgui.tastsicht;
	}

	ntaste = ganz2halb[edgui.taste];
	if (edgui.tastsicht > 75) max = 128; else max = ntaste + ganz2halb[edgui.tastsicht] - 1;
	schwarz = 0;

	RahmenEin(5, 0, 2, oy, 69, y);
	while (ntaste < max) {
		oktave = (ntaste / 12) - 2;
		ton = ntaste % 12;

		if (oktnote[ton][1] == ' ') {
			y = y - edgui.tasth - 1;
			BlitteBitMap(BMAP_KEY_WHITE, 0, 0, 3, y + 1, 48, edgui.tasth - 2);
			BlitteBitMap(BMAP_KEY_WHITE, 0, 28, 3, y + edgui.tasth - 1, 48, 2);
			Linie(0, 52, y, 68, y);
			if (ton == 0) SchreibeZahl(1, 56, y + 8, oktave);
			if (schwarz) {
				BlitteBitMap(BMAP_KEY_BLACK, 0, 0, 3, schwarz - (edgui.tasth / 3), 26, (edgui.tasth / 3) * 2 - 2);
				BlitteBitMap(BMAP_KEY_BLACK, 0, 19, 3, schwarz + (edgui.tasth / 3) - 2, 26, 2);
				schwarz = 0;
			}
		} else {
			schwarz = y;
		}
		ntaste++;
	}
}

void ZeichneEdZeitleiste(void) {
	int16 lr, rr;
	int16 x, xe;
	int32 t;
	int16 sechsz;
	struct MARKER *akt;

	aktfenster = edfenster;

	lr = 72;
	rr = edfenster->Width - edlr - 21;

	Balken(6, lr, 24, rr, 40);

	x = lr + ((((edseq->start - edgui.takt) >> (VIERTEL - 4)) * edgui.taktb) >> 2);
	xe = lr + ((((edseq->ende - edgui.takt) >> (VIERTEL - 4)) * edgui.taktb) >> 2);
	if (!((x < lr) && (xe < lr)) && !((x > rr) && (xe > rr))) {
		if (x < lr) x = lr;
		if (xe > rr) xe = rr;
		Balken(2, x, 24, xe, 40);
	}

	edgui.taktsicht = 0;
	x = lr; t = edgui.takt;
	akt = TaktMarker(NULL, M_TAKT, t);

	while (TRUE) {
		x = lr + ((((t - edgui.takt) >> (VIERTEL - 4)) * edgui.taktb) >> 2);
		if (x > rr) break;

		akt = TaktMarker(akt, M_TAKT, t);

		sechsz = (t - akt->takt) >> (VIERTEL - 2);

		if (sechsz % (akt->m_zaehler << 2) == 0) {
			if (x+20 < rr) SchreibeZahl(1, x + 2, 32, akt->m_taktnum + ((sechsz >> 2) / akt->m_zaehler));
			Linie(1, x, 26, x, 40);
		} else {
			if ((sechsz % 4) == 0) {
				Linie(1, x, 34, x, 40);
			} else {
				Linie(1, x, 38, x, 40);
			}
		}

		t = t + (VIERTELWERT >> 2); edgui.taktsicht++;
	}
}

void ZeichneNote(struct EVENT *sevent, struct EVENT *eevent, BOOL vorschau) {
	int16 xs, xe;
	int16 lr, rr;
	int16 y;
	int16 ta;
	int8 k;

	lr = 72; rr = edfenster->Width - edlr - 22;

	xs = lr + (((edseq->start + sevent->zeit - edgui.takt) * edgui.taktb) >> (VIERTEL - 2));
	if (eevent) {
		xe = lr + (((edseq->start + eevent->zeit - edgui.takt) * edgui.taktb) >> (VIERTEL - 2));
		k = 1;
	} else {
		xe = xs + 4; k = 0;
	}

	if (!((xs < lr) && (xe < lr)) && !((xs > rr) && (xe > rr))) {
		if (xs < lr) xs = lr;
		if (xe > rr) xe = rr;
		ta = halb2ganz[sevent->data1] - edgui.taste;
		if ((ta >= 0) && (ta < edgui.tastsicht)) {
			y = edfenster->Height - edou - edguibox - 58 - (ta * (edgui.tasth + 1));
			if (oktnote[sevent->data1 % 12][1] == '#') y = y - (edgui.tasth / 2);
			if (!vorschau) {
				if (sevent->markiert) {
					RahmenEin((10 * k) + 1, STIL_DH, xs, y - edgui.tasth, xe, y - 1);
				} else {
					RahmenAus((8 * k) + 1, STIL_HD, xs, y - edgui.tasth, xe, y - 1);
				}
			} else {
				IGraphics->SetDrMd(edfenster->RPort, COMPLEMENT);
				Balken(9, xs, y - edgui.tasth, xe, y - 1);
				IGraphics->SetDrMd(edfenster->RPort, JAM1);
			}
		}
	}

}

void ZeichneNotenfeld(BOOL hg, BOOL vorschau, BOOL anders) {
	int16 uy;
	int16 p;
	int16 n;
	struct EVENTBLOCK *evbl;
	int16 evnum;

	aktfenster = edfenster;

	if (!vorschau) KeinePosition();
	uy = edfenster->Height - edou - edguibox - 58;
	if (hg) {
		RahmenEin(29, 0, 71, 43, edfenster->Width - edlr - 20, uy);

		//Netz
		p = uy;
		for (n = 0; n < edgui.tastsicht; n++) {
			p = p - edgui.tasth - 1;
			Linie(7, 72, p, edfenster->Width - edlr - 22, p);
		}

		p = 72;
		for (n = 0; n < edgui.taktsicht; n++) {
			if (((edgui.takt >> (VIERTEL - 2)) + n) % 4 == 0) {
				edfenster->RPort->LinePtrn = 0xFFFF;
				Linie(7, p, 44, p, uy - 1);
			} else if (edgui.taktb > 12) {
				edfenster->RPort->LinePtrn = 0xAAAA;
				Linie(7, p, 44, p, uy - 1);
			}
			p = p + edgui.taktb;
		}
		edfenster->RPort->LinePtrn = 0xFFFF;
	}

	//Noten
	for (n = 0; n < 128; n++) startev[n] = NULL;

	evbl = edseq->eventblock; evnum = 0;
	while (evbl) {
		if (!evbl->event[evnum].status) break;

		p = evbl->event[evnum].data1;
		if (!vorschau || evbl->event[evnum].markiert) {
			if ((evbl->event[evnum].status & MS_StatBits) == MS_NoteOn) {
				if (startev[p]) {
					if (!vorschau) ZeichneNote(startev[p], NULL, FALSE);
				}
				startev[p] = &evbl->event[evnum];
			}
			if ((evbl->event[evnum].status & MS_StatBits) == MS_NoteOff) {
				if (startev[p]) {
					if (!vorschau || startev[p]->markiert) ZeichneNote(startev[p], &evbl->event[evnum], vorschau);
					startev[p] = NULL;
				} else {
					if (!vorschau) ZeichneNote(&evbl->event[evnum], NULL, FALSE);
				}
			}
		}

		evnum++; if (evnum == EVENTS) {evnum = 0; evbl = evbl->next;}
	}
	if (!vorschau) {
		if (anders) ZeichneSequenzen(edseq->spur, TRUE);
		ZeichnePosition(TRUE);
	}
}

void ZeichneNotenVelos(void) {
	struct EVENTBLOCK *evbl;
	int16 evnum;
	struct EVENT *ev;
	int16 lr, rr;
	int16 x, y, nx;
	int16 ta;
	int8 v;

	lr = 72; rr = edfenster->Width - edlr - 25;
	x = 69;

	evbl = edseq->eventblock; evnum = 0;
	while (evbl) {
		ev = &evbl->event[evnum];
		if (!ev->status) break;

		if ((ev->status & MS_StatBits) == MS_NoteOn) {
			nx = lr + (((edseq->start + ev->zeit - edgui.takt) * edgui.taktb) >> (VIERTEL - 2));
			if (nx > lr) {
				if (nx > x + 4) x = nx; else x = x + 4;
				if (x > rr) break;

				ta = halb2ganz[ev->data1] - edgui.taste;
				if ((ta >= 0) && (ta < edgui.tastsicht)) {
					y = edfenster->Height - edou - edguibox - 58 - (ta * (edgui.tasth + 1));
					if (oktnote[ev->data1 % 12][1] == '#') y = y - (edgui.tasth / 2);
					v = ev->data2 >> 2;
					if (v < 31) Balken(2, x, y - 31, x + 2, y - v - 1);
					if (v > 0) Balken(1, x, y - v, x + 2, y - 1);
				}
			}
		}

		evnum++; if (evnum == EVENTS) {evnum = 0; evbl = evbl->next;}
	}
}

void Tastendruck(int8 n, BOOL d) {
	int16 tg, y;

	tg = (halb2ganz[n] - edgui.taste);
	if ((tg >= 0) && (tg < edgui.tastsicht)) {
		y = edfenster->Height - edou - edguibox - 58 - ((tg + 1) * (edgui.tasth + 1));
		if (oktnote[n%12][1] == ' ') { //weiße Taste
			if (d) Balken(9, 30, y + 2, 48, y + edgui.tasth - 2);
			else BlitteBitMap(BMAP_KEY_WHITE, 27, 0, 30, y + 1, 21, edgui.tasth - 2);
		} else { //schwarze Taste
			if (d) Balken(11, 4, y - (edgui.tasth / 3) + 1, 27, y + (edgui.tasth / 3) - 3);
			else BlitteBitMap(BMAP_KEY_BLACK, 0, 0, 3, y - (edgui.tasth / 3), 26, (edgui.tasth / 3) * 2 - 2);
		}
	}
}

void ZeichneNotenanzeige(void) {
	char puf[8];
	int16 ton, okt;

	aktfenster = edfenster;
	ton = editnote % 12; okt = editnote / 12 - 2;
	sprintf(puf, "%s%d", oktnote[ton], okt);
	Balken(5, 2, 24, 68, 40);
	Schreibe(1, 8, 36, puf, 70);
	Tastendruck(alteditnote, FALSE);
	Tastendruck(editnote, TRUE); alteditnote = editnote;
}

void ZeichneEdInfobox(void) {
	int16 y;
	char puf[30];
	int32 z;

	aktfenster = edfenster;
	y = edfenster->Height - edou - 37 - edguibox;
	Gradient(4, STIL_DN, 0, y - 1, edfenster->Width - edlr - 1, y + edguibox - 1);
	Linie(1, 0, y, edfenster->Width - edlr - 1, y);
	Linie(2, 0, y + edguibox - 2, edfenster->Width - edlr - 1, y + edguibox - 2);
	Linie(8, 0, y + edguibox - 1, edfenster->Width - edlr - 1, y + edguibox - 1);

	if (wahlnote) {
		z = wahlnote->zeit;
		sprintf(puf, "%ld | %ld | %ld", z >> VIERTEL, (z & ~VIERTELMASKE) >> (VIERTEL - 2), (z & ~VIERTELMASKE) % (VIERTELWERT >> 2));
		Schreibe(2, 6, y + 12, CAT(MSG_0518, "Rel Pos:"), 48); Schreibe(1, 50, y + 12, puf, 144);
		sprintf(puf, "%s%d", oktnote[wahlnote->data1 % 12], (wahlnote->data1 / 12) - 2);
		Schreibe(2, 130, y + 12, CAT(MSG_0520, "Note:"), 178); Schreibe(1, 180, y + 12, puf, 244);
		Schreibe(2, 230, y + 12, CAT(MSG_0521, "Velocity:"), 278); SchreibeZahl(1, 280, y + 12, wahlnote->data2);
		Schreibe(2, 330, y + 12, CAT(MSG_0522, "Channel:"), 378); SchreibeZahl(1, 380, y + 12, (wahlnote->status & MS_ChanBits) + 1);
	}
}

int8 PunktTaste(int16 y) {
	int16 tg, th;
	int16 by;

	by = (edfenster->Height - edou - edguibox - 58) - (y - edfenster->BorderTop);
	tg = by / (edgui.tasth + 1);
	if (edgui.taste + tg < 0) tg = 0;
	if (edgui.taste + tg > 74) tg = 74 - edgui.taste;
	th = ganz2halb[edgui.taste + tg];
	if (th < 127) {
		if (halb2ganz[th] == halb2ganz[th + 1]) { //Halber Ton möglich?
			if (by > (tg * (edgui.tasth + 1)) + (edgui.tasth / 2)) th++;
		}
	}
	return((int8)th);
}

int32 EdPunktPosition(int16 x) {
	int32 p;

	x = x - edfenster->BorderLeft - 72;
	if (x >= 0) {
		p = (x << (VIERTEL - 2)) / edgui.taktb + edgui.takt;
	} else {
		p = edgui.takt;
	}
	return(p);
}

int8 TesteEdPunktBereich(int16 x, int16 y) {
	uint8 b = 0;
	int16 uy, rx;

	x = x-edfenster->BorderLeft;
	y = y-edfenster->BorderTop;
	uy = edfenster->Height - edou;
	rx = edfenster->Width - edlr;

	if ((x > 70) && (x < rx - 20)) {
		if ((y > 23) && (y < 41)) b = AREA_ZEIT;
		if ((y > 43) && (y < uy - 58 - edguibox)) b = AREA_SEQUENZEN;
	}
	if ((x < 70) && (y > 23) && (y < uy - 58 - edguibox)) b = AREA_SPUREN;
	if ((y > uy - 2 - edguibox) && (y < uy - 35)) b = AREA_INFOBOX;
	return(b);
}
