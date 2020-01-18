#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/button.h>
#include <proto/chooser.h>
#include <proto/string.h>
#include <proto/integer.h>
#include <proto/slider.h>

#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <gadgets/button.h>
#include <gadgets/chooser.h>
#include <gadgets/string.h>
#include <gadgets/integer.h>
#include <gadgets/slider.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Gui.h"

extern struct Hook *backfill;
extern struct Screen *hschirm;
extern struct Window *aktfenster;
extern struct OUTPORT outport[];
struct TextAttr fentextattr = {"helvetica.font", 11, FS_NORMAL, 0};

void ChannelPortFenster(struct Window *fen, uint8 *ch, uint8 *po) {
	struct Window *fenster;
	struct Gadget *gadget[18];
	struct IntuiMessage *mes;
	struct Gadget *g;
	int8 xg, yg;
	int8 n;
	char zahl[16][3] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
	BOOL stopp = FALSE;
	uint32 portn;
	struct List portlist;
	struct Node *node;
	
	IExec->NewList(&portlist);
	for (n = 0; n < verOUTPORTS; n++) {
		if (outport[n].name[0]) {
			node = IChooser->AllocChooserNode(CNA_Text, outport[n].name, TAG_DONE);
			IExec->AddTail(&portlist, node);
		} else break;
	}
	
	fenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 127,
		WA_Height, 174,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE);
	if (fenster) {

		aktfenster = fenster;

		gadget[0] = (struct Gadget *)IIntuition->NewObject(ChooserClass, NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 123, GA_Height, 22, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_ID, 0,
			CHOOSER_PopUp, TRUE,
			CHOOSER_Labels, &portlist,
			CHOOSER_Selected, *po,
			CHOOSER_MaxLabels, verOUTPORTS,
			TAG_DONE);
	
		n = 1;
		for (yg = 0; yg < 4; yg++) {
			for (xg = 0; xg < 4; xg++) {
				gadget[n] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
					GA_Previous, gadget[n - 1],
					GA_Top, 28 + (yg * 31), GA_Left, 2 + (xg * 31),
					GA_Width, 30, GA_Height, 30, GA_TextAttr, &fentextattr,
					GA_RelVerify, TRUE, GA_Immediate, TRUE,
					GA_ID, n, GA_Text, zahl[n - 1],
					BUTTON_PushButton, TRUE,
					TAG_DONE);
				n++;
			}
		}
		gadget[17] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
			GA_Previous, gadget[16],
			GA_Top, 152, GA_Left, 33,
			GA_Width, 61, GA_Height, 20, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 17, GA_Text, "(Thru)",
			BUTTON_PushButton, TRUE,
			TAG_DONE);
		IIntuition->SetGadgetAttrs(gadget[*ch + 1], NULL, NULL, GA_Selected, TRUE, TAG_DONE);
		
		RahmenAus(0, 0, 0, 0, 126, 173);
		RahmenRundungAus(1, 1, 125, 172);
		
		IIntuition->AddGList(fenster, gadget[0], 0, 18, NULL);
		IIntuition->RefreshGList(gadget[0], fenster, NULL, -1);


		do {
			IExec->WaitPort(fenster->UserPort);
			while ((mes = (struct IntuiMessage *)IExec->GetMsg(fenster->UserPort))) {
				switch (mes->Class) {
					case IDCMP_GADGETUP:
					g = (struct Gadget *)mes->IAddress;
					if (g->GadgetID > 0) {
						*ch = g->GadgetID - 1;
						IIntuition->GetAttr(CHOOSER_Selected, (Object *)gadget[0], &portn);
						*po = (uint8)portn;
						stopp = TRUE;
					}
					break;
					
					case IDCMP_INACTIVEWINDOW:
					IIntuition->GetAttr(CHOOSER_Selected, (Object *)gadget[0], &portn);
					*po = (uint8)portn;
					stopp = TRUE;
					break;
				}
				IExec->ReplyMsg((struct Message *)mes);
			}
		} while (!stopp);

		for(n = 0; n < 18; n++) {
			if (gadget[n]) {IIntuition->RemoveGList(fenster, gadget[n], 1); IIntuition->DisposeObject((Object *)gadget[n]);}
		}

		IIntuition->CloseWindow(fenster);
	
	}
	
	while ((node = IExec->RemTail(&portlist)))
	{
		IChooser->FreeChooserNode(node);
	}
}

void StringFenster(struct Window *fen, STRPTR t, int16 l) {
	struct Window *fenster;
	struct Gadget *gadget;
	struct IntuiMessage *mes;
	BOOL stopp = FALSE;
	STRPTR gt;
	
	fenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 200,
		WA_Height, 25,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE);
	if (fenster) {

		aktfenster = fenster;

		gadget = (struct Gadget *)IIntuition->NewObject(StringClass, NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 196, GA_Height, 21, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_ID, 0,
			STRINGA_MaxChars, l,
			STRINGA_TextVal, t,
			TAG_DONE);
	
		RahmenAus(0, 0, 0, 0, 199, 24);
		RahmenRundungAus(1, 1, 198, 23);
		
		IIntuition->AddGadget(fenster, gadget, 0);
		IIntuition->RefreshGList(gadget, fenster, NULL, -1);
		
		IIntuition->ActivateGadget(gadget, fenster, NULL);
		
		do {
			IExec->WaitPort(fenster->UserPort);
			while ((mes = (struct IntuiMessage *)IExec->GetMsg(fenster->UserPort))) {
				if ((mes->Class == IDCMP_GADGETUP) || (mes->Class == IDCMP_INACTIVEWINDOW)) {
					IIntuition->GetAttr(STRINGA_TextVal, (Object *)gadget, (uint32 *)&gt);
					strncpy(t, gt, l);
					stopp = TRUE;
				}
				IExec->ReplyMsg((struct Message *)mes);
			}
		} while (!stopp);

		if (gadget) {
			IIntuition->RemoveGadget(fenster, gadget); IIntuition->DisposeObject((Object *)gadget);
		}

		IIntuition->CloseWindow(fenster);
	
	}
	
}

int16 QuantisierungsFenster(struct Window *fen, BOOL fein) {
	struct Window *fenster;
	struct Gadget *gadget[6];
	struct IntuiMessage *mes;
	struct Gadget *g;
	int8 n;
	char tx[5][5] = {"1/4", "1/8", "1/16", "1/32", "1/64"};
	char txf[5][6] = {"1/16", "1/32", "1/64", "1/128", "1/256"};
	BOOL stopp = FALSE;
	int16 q = 0;
	
	fenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 74,
		WA_Height, 132,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE);
	if (fenster) {

		aktfenster = fenster;

		gadget[0] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 70, GA_Height, 22, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 0, GA_Text, CAT(MSG_0194, "Cancel"),
			TAG_DONE);

		for(n = 1; n < 6; n++) {
			gadget[n] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
				GA_Previous, gadget[n - 1],
				GA_Top, 5+(n * 21), GA_Left, 2,
				GA_Width, 70, GA_Height, 20, GA_TextAttr, &fentextattr,
				GA_RelVerify, TRUE, GA_Immediate, TRUE,
				GA_ID, n, GA_Text, !fein ? tx[n - 1] : txf[n - 1],
				TAG_DONE);
		}
		
		RahmenAus(0, 0, 0, 0, 73, 131);
		RahmenRundungAus(1, 1, 72, 130);
		
		IIntuition->AddGList(fenster, gadget[0], 0, 6, NULL);
		IIntuition->RefreshGList(gadget[0], fenster, NULL, -1);


		do {
			IExec->WaitPort(fenster->UserPort);
			while ((mes = (struct IntuiMessage *)IExec->GetMsg(fenster->UserPort))) {
				switch (mes->Class) {
					case IDCMP_GADGETUP:
					g = (struct Gadget *)mes->IAddress;
					q = g->GadgetID;
					if (q > 0) {
						if (!fein) q = VIERTEL - q + 1;
						else q = VIERTEL - q - 1;
					}
					stopp = TRUE;
					break;
					
					case IDCMP_INACTIVEWINDOW:
					q = 0;
					stopp = TRUE;
					break;
				}
				IExec->ReplyMsg((struct Message *)mes);
			}
		} while (!stopp);

		for(n = 0; n < 6; n++) {
			if (gadget[n]) {IIntuition->RemoveGList(fenster, gadget[n], 1); IIntuition->DisposeObject((Object *)gadget[n]);}
		}

		IIntuition->CloseWindow(fenster);
	}
	return(q);
}

int16 IntegerFenster(struct Window *fen, int16 w, int16 min, int16 max) {
	struct Window *fenster;
	struct Gadget *gadget[2];
	struct IntuiMessage *mes;
	BOOL stopp = FALSE;
	int32 gw = w;
	struct TagItem mapizs[] = {{INTEGER_Number, SLIDER_Level}, {TAG_DONE}};
	struct TagItem mapszi[] = {{SLIDER_Level, INTEGER_Number}, {TAG_DONE}};
	
	fenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 240,
		WA_Height, 25,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE);
	if (fenster) {

		aktfenster = fenster;

		gadget[0] = (struct Gadget *)IIntuition->NewObject(IntegerClass, NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 48, GA_Height, 21, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_ID, 10,
			INTEGER_Number, w,
			INTEGER_Minimum, min,
			INTEGER_Maximum, max,
			INTEGER_Arrows, FALSE,
			ICA_MAP, mapizs,
			TAG_DONE);

		gadget[1] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
			GA_Previous, gadget[0],
			GA_BackFill, backfill,
			GA_Top, 2, GA_Left, 52,
			GA_Width, 186, GA_Height, 21,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 2,
			SLIDER_Min, min, SLIDER_Max, max,
			SLIDER_Level, w,
			SLIDER_Orientation, SORIENT_HORIZ,
			ICA_MAP, mapszi,
			TAG_DONE);


		RahmenAus(0, 0, 0, 0, 239, 24);
		RahmenRundungAus(1, 1, 238, 23);
		
		IIntuition->AddGList(fenster, gadget[0], 0, 2, NULL);
		IIntuition->RefreshGList(gadget[0], fenster, NULL, -1);

		IIntuition->SetAttrs(gadget[0], ICA_TARGET, gadget[1], TAG_END);
		IIntuition->SetAttrs(gadget[1], ICA_TARGET, gadget[0], TAG_END);
		
		IIntuition->ActivateGadget(gadget[0], fenster, NULL);
		
		do {
			IExec->WaitPort(fenster->UserPort);
			while ((mes = (struct IntuiMessage *)IExec->GetMsg(fenster->UserPort))) {
				if ((mes->Class == IDCMP_INACTIVEWINDOW) || (mes->Class == IDCMP_GADGETUP)) {
					IIntuition->GetAttr(INTEGER_Number, (Object *)gadget[0], (uint32 *)&gw);
					stopp = TRUE;
				}
				IExec->ReplyMsg((struct Message *)mes);
			}
		} while (!stopp);

		if (gadget[0]) {IIntuition->RemoveGList(fenster, gadget[0], 1); IIntuition->DisposeObject((Object *)gadget[0]);}
		if (gadget[1]) {IIntuition->RemoveGList(fenster, gadget[1], 1); IIntuition->DisposeObject((Object *)gadget[1]);}

		IIntuition->CloseWindow(fenster);
	
	}
	return(gw);	
}

void ZeichneDynKurve(int8 thresh, int8 ratio, int8 gain) {
	int16 w, x;
	
	RahmenEin(1, 0, 3, 3, 68, 68);
	thresh = thresh / 2; gain = gain/2;
	for (x = 0; x < 64; x++) {
		w = x;
		if ((ratio > 0) && (x > thresh)) w = ((x - thresh) / (1 + ratio)) + thresh;
		if ((ratio < 0) && (x < thresh)) w = ((x - thresh) * (1 - ratio)) + thresh;
		w += gain;
		if (w < 0) w = 0;
		if (w > 63) w = 63;
		Gradient(15, STIL_HD, 4 + x, 67 - w, 4 + x, 67);
	}
	Linie(21, 4, 67, 67, 4);
}

BOOL DynamikFenster(struct Window *fen, int8 *thresh, int8 *ratio, int8 *gain) {
	struct Window *fenster;
	struct Gadget *gadget[5];
	struct IntuiMessage *mes;
	struct Gadget *g;
	int16 n;
	uint32 var;
	int8 stopp = 0;
	struct TagItem mapslider[] = {{SLIDER_Level, ICSPECIAL_CODE}, {TAG_DONE, TAG_DONE}};
	
	fenster = IIntuition->OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 210,
		WA_Height, 115,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW | IDCMP_IDCMPUPDATE,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE);
	if (fenster) {

		aktfenster = fenster;
		SetzeFont();

		gadget[0] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
			GA_BackFill, backfill,
			GA_Top, 20, GA_Left, 72,
			GA_Width, 128, GA_Height, 15,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 1,
			SLIDER_Min, 0, SLIDER_Max, 127,
			SLIDER_Level, *thresh,
			SLIDER_Orientation, SORIENT_HORIZ,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapslider,
			TAG_DONE);

		gadget[1] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
			GA_Previous, gadget[0],
			GA_BackFill, backfill,
			GA_Top, 50, GA_Left, 72,
			GA_Width, 128, GA_Height, 15,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 2,
			SLIDER_Min, -10, SLIDER_Max, 10,
			SLIDER_Level, *ratio,
			SLIDER_Orientation, SORIENT_HORIZ,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapslider,
			TAG_DONE);

		gadget[2] = (struct Gadget *)IIntuition->NewObject(SliderClass, NULL,
			GA_Previous, gadget[1],
			GA_BackFill, backfill,
			GA_Top, 80, GA_Left, 72,
			GA_Width, 128, GA_Height, 15,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 3,
			SLIDER_Min, -127, SLIDER_Max, 127,
			SLIDER_Level, *gain,
			SLIDER_Orientation, SORIENT_HORIZ,
			ICA_TARGET, ICTARGET_IDCMP,
			ICA_MAP, mapslider,
			TAG_DONE);

		gadget[3] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
			GA_Previous, gadget[2],
			GA_Top, 72, GA_Left, 3,
			GA_Width, 66, GA_Height, 20, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 4, GA_Text, CAT(MSG_0195, "Okay"),
			TAG_DONE);

		gadget[4] = (struct Gadget *)IIntuition->NewObject(ButtonClass, NULL,
			GA_Previous, gadget[3],
			GA_Top, 93, GA_Left, 3,
			GA_Width, 66, GA_Height, 20, GA_TextAttr, &fentextattr,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 5, GA_Text, CAT(MSG_0194, "Cancel"),
			TAG_DONE);

		SetzeFont();
		RahmenAus(0, 0, 0, 0, 209, 114);
		RahmenRundungAus(1, 1, 208, 113);
		ZeichneDynKurve(*thresh, *ratio, *gain);
		Schreibe(1, 72, 16, CAT(MSG_0197, "Threshold"), 200);
		Schreibe(1, 72, 46, CAT(MSG_0198, "Compression"), 200);
		Schreibe(1, 72, 76, CAT(MSG_0199, "Gain"), 200);
		
		IIntuition->AddGList(fenster, gadget[0], 0, 5, NULL);
		IIntuition->RefreshGList(gadget[0], fenster, NULL, -1);

		do {
			IExec->WaitPort(fenster->UserPort);
			while ((mes = (struct IntuiMessage *)IExec->GetMsg(fenster->UserPort))) {
				switch (mes->Class) {
					case IDCMP_GADGETUP:
					g = (struct Gadget *)mes->IAddress;
					switch (g->GadgetID) {
						case 1: *thresh = (int8)mes->Code; ZeichneDynKurve(*thresh, *ratio, *gain); break;
						case 2: *ratio = (int8)mes->Code; ZeichneDynKurve(*thresh, *ratio, *gain); break;
						case 3: *gain = (int8)mes->Code; ZeichneDynKurve(*thresh, *ratio, *gain); break;
						case 4: stopp = 2; break;
						case 5: stopp = 1; break;
					}
					break;

					case IDCMP_IDCMPUPDATE:
					IIntuition->GetAttr(SLIDER_Level, (APTR)gadget[0], &var); *thresh = (int8)var;
					IIntuition->GetAttr(SLIDER_Level, (APTR)gadget[1], &var); *ratio = (int8)var;
					IIntuition->GetAttr(SLIDER_Level, (APTR)gadget[2], &var); *gain = (int8)var;
					ZeichneDynKurve(*thresh, *ratio, *gain);
					break;
					
					case IDCMP_INACTIVEWINDOW:
					stopp = 1;
					break;
				}
				IExec->ReplyMsg((struct Message *)mes);
			}
		} while (!stopp);

		for (n = 0; n < 5; n++) {
			if (gadget[n]) {IIntuition->RemoveGList(fenster, gadget[n], 1); IIntuition->DisposeObject((Object *)gadget[n]);}
		}

		IIntuition->CloseWindow(fenster);
	
	}
	return((BOOL)stopp - 1);	
}
