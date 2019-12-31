#include <stdio.h>
#include <string.h>

#include <intuition/intuition.h>
#include <exec/exec.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <gadgets/button.h>
#include <gadgets/chooser.h>
#include <gadgets/string.h>
#include <gadgets/integer.h>
#include <gadgets/slider.h>

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/button.h>
#include <proto/chooser.h>
#include <proto/string.h>
#include <proto/integer.h>
#include <proto/slider.h>

#include "Strukturen.h"
#include "Gui.h"

extern struct Window *aktfenster;
extern STRPTR midiname[16];

void ChannelPortFenster(struct Window *fen, UBYTE *ch, UBYTE *po) {
	struct Window *cpfenster;
	struct Gadget *gadget[17];
	struct List list;
	struct Node *portnode[16];
	struct IntuiMessage *mes;
	struct Gadget *g;
	BYTE xg, yg;
	BYTE n;
	char zahl[16][3]={"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
	BOOL stopp=FALSE;
	ULONG portn;
	
	if (cpfenster=OpenWindowTags(NULL,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 127,
		WA_Height, 153,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE)) {

		aktfenster=cpfenster;

		NewList(&list);
		for (n=0; n<16; n++) {
			if (midiname[n]) {
				portnode[n]=AllocChooserNode(CNA_Text, midiname[n], TAG_DONE);
				AddTail(&list, portnode[n]);
			} else {
				portnode[n]=NULL;
			};
		};
		
		gadget[0]=NewObject(CHOOSER_GetClass(), NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 123, GA_Height, 22,
			GA_RelVerify, TRUE, GA_ID, 0,
			CHOOSER_PopUp, TRUE,
			CHOOSER_Labels, &list,
			CHOOSER_Selected, *po,
			TAG_DONE);
	
		n=1;
		for (yg=0; yg<4; yg++) {
			for (xg=0; xg<4; xg++) {
				gadget[n]=NewObject(BUTTON_GetClass(), NULL,
					GA_Previous, gadget[n-1],
					GA_Top, 28+(yg*31), GA_Left, 2+(xg*31),
					GA_Width, 30, GA_Height, 30,
					GA_RelVerify, TRUE, GA_Immediate, TRUE,
					GA_ID, n, GA_Text, zahl[n-1],
					BUTTON_PushButton, TRUE,
					TAG_DONE);
				n++;
			};
		};
		SetGadgetAttrs(gadget[*ch+1], NULL, NULL, GA_Selected, TRUE, TAG_DONE);
		
		RahmenAus(0, 0, 0, 126, 152);
		
		AddGList(cpfenster, gadget[0], 0, 17, NULL);
		RefreshGList(gadget[0], cpfenster, NULL, -1);


		do {
			WaitPort(cpfenster->UserPort);
			while (mes=(struct IntuiMessage *)GetMsg(cpfenster->UserPort)) {
				switch (mes->Class) {
					case IDCMP_GADGETUP:
					g=(struct Gadget *)mes->IAddress;
					if (g->GadgetID>0) {
						*ch=g->GadgetID-1;
						GetAttr(CHOOSER_Selected, gadget[0], &portn);
						*po=(UBYTE)portn;
						stopp=TRUE;
					};
					break;
					
					case IDCMP_INACTIVEWINDOW:
					GetAttr(CHOOSER_Selected, gadget[0], &portn);
					*po=(UBYTE)portn;
					stopp=TRUE;
					break;
				};
				ReplyMsg((struct Message *)mes);
			};
		} while (!stopp);

		for(n=0; n<17; n++) {
			if (gadget[n]) {RemoveGList(cpfenster, gadget[n], 1); DisposeObject(gadget[n])};
		};

		CloseWindow(cpfenster);
	
		while (RemHead(&list));
		for (n=0; n<16; n++) {
			if (!portnode[n]) break;
			FreeChooserNode(portnode[n]);
		};
		
		aktfenster=fen;
	};
}

void StringFenster(struct Window *fen, STRPTR t, WORD l) {
	struct Window *sfenster;
	struct Gadget *gadget;
	struct IntuiMessage *mes;
	BOOL stopp=FALSE;
	STRPTR gt;
	
	if (sfenster=OpenWindowTags(NULL,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 200,
		WA_Height, 25,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE)) {

		aktfenster=sfenster;

		gadget=NewObject(STRING_GetClass(), NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 196, GA_Height, 21,
			GA_RelVerify, TRUE, GA_ID, 0,
			STRINGA_MaxChars, l,
			STRINGA_TextVal, t,
			TAG_DONE);
	
		RahmenAus(0, 0, 0, 199, 24);
		
		AddGadget(sfenster, gadget, 0);
		RefreshGList(gadget, sfenster, NULL, -1);
		
		do {
			WaitPort(sfenster->UserPort);
			while (mes=(struct IntuiMessage *)GetMsg(sfenster->UserPort)) {
				if ((mes->Class==IDCMP_GADGETUP) || (mes->Class==IDCMP_INACTIVEWINDOW)) {
					GetAttr(STRINGA_TextVal, gadget, (ULONG *)&gt);
					strcpy(t, gt);
					stopp=TRUE;
				};
				ReplyMsg((struct Message *)mes);
			};
		} while (!stopp);

		if (gadget) {
			RemoveGadget(sfenster, gadget); DisposeObject(gadget);
		};

		CloseWindow(sfenster);
	
		aktfenster=fen;
	};
	
}

WORD QuantisierungsFenster(struct Window *fen) {
	struct Window *qfenster;
	struct Gadget *gadget[6];
	struct IntuiMessage *mes;
	struct Gadget *g;
	BYTE n;
	char tx[5][5]={"1/4", "1/8", "1/16", "1/32", "1/64"};
	BOOL stopp=FALSE;
	WORD q=0;
	
	if (qfenster=OpenWindowTags(NULL,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 74,
		WA_Height, 132,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE)) {

		aktfenster=qfenster;

		gadget[0]=NewObject(BUTTON_GetClass(), NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 70, GA_Height, 22,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 0, GA_Text, "Abbruch",
			TAG_DONE);

		for(n=1; n<6; n++) {
			gadget[n]=NewObject(BUTTON_GetClass(), NULL,
				GA_Previous, gadget[n-1],
				GA_Top, 5+(n*21), GA_Left, 2,
				GA_Width, 70, GA_Height, 20,
				GA_RelVerify, TRUE, GA_Immediate, TRUE,
				GA_ID, n, GA_Text, tx[n-1],
				TAG_DONE);
		};
		
		RahmenAus(0, 0, 0, 73, 131);
		
		AddGList(qfenster, gadget[0], 0, 6, NULL);
		RefreshGList(gadget[0], qfenster, NULL, -1);


		do {
			WaitPort(qfenster->UserPort);
			while (mes=(struct IntuiMessage *)GetMsg(qfenster->UserPort)) {
				switch (mes->Class) {
					case IDCMP_GADGETUP:
					g=(struct Gadget *)mes->IAddress;
					q=g->GadgetID;
					if (q>0) q=11-q;
					stopp=TRUE;
					break;
					
					case IDCMP_INACTIVEWINDOW:
					q=0;
					stopp=TRUE;
					break;
				};
				ReplyMsg((struct Message *)mes);
			};
		} while (!stopp);

		for(n=0; n<6; n++) {
			if (gadget[n]) {RemoveGList(qfenster, gadget[n], 1); DisposeObject(gadget[n])};
		};

		CloseWindow(qfenster);
		aktfenster=fen;
	};
	return(q);
}

WORD IntegerFenster(struct Window *fen, WORD w, WORD min, WORD max) {
	struct Window *ifenster;
	struct Gadget *gadget;
	struct IntuiMessage *mes;
	BOOL stopp=FALSE;
	LONG gw=w;
	
	if (ifenster=OpenWindowTags(NULL,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 100,
		WA_Height, 25,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE)) {

		aktfenster=ifenster;

		gadget=NewObject(INTEGER_GetClass(), NULL,
			GA_Top, 2, GA_Left, 2,
			GA_Width, 96, GA_Height, 21,
			GA_RelVerify, TRUE, GA_ID, 10,
			INTEGER_Number, w,
			INTEGER_Minimum, min,
			INTEGER_Maximum, max,
			INTEGER_Arrows, TRUE,
			TAG_DONE);
	
		RahmenAus(0, 0, 0, 99, 24);
		
		AddGadget(ifenster, gadget, 0);
		RefreshGList(gadget, ifenster, NULL, -1);
		
		do {
			WaitPort(ifenster->UserPort);
			while (mes=(struct IntuiMessage *)GetMsg(ifenster->UserPort)) {
				if (mes->Class==IDCMP_INACTIVEWINDOW) {
					GetAttr(INTEGER_Number, gadget, (ULONG *)&gw);
					stopp=TRUE;
				};
				ReplyMsg((struct Message *)mes);
			};
		} while (!stopp);

		if (gadget) {
			RemoveGadget(ifenster, gadget); DisposeObject(gadget);
		};

		CloseWindow(ifenster);
	
		aktfenster=fen;
	};
	return(gw);	
}

void InstrumentenFenster(struct Window *fen, BYTE *bank, BYTE *prog) {
	struct Window *ifenster;
	struct Gadget *gadget[5];
	struct IntuiMessage *mes;
	struct Gadget *g;
	BOOL stopp=FALSE;
	ULONG gw;
	BYTE n;
	struct TagItem mapizs[]={INTEGER_Number, SLIDER_Level, TAG_DONE};
	struct TagItem mapszi[]={SLIDER_Level, INTEGER_Number, TAG_DONE};

	if (ifenster=OpenWindowTags(NULL,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 200,
		WA_Height, 72,
		WA_IDCMP, IDCMP_GADGETUP | IDCMP_INACTIVEWINDOW,
		WA_Activate, TRUE,
		WA_Borderless, TRUE,
		TAG_DONE)) {

		aktfenster=ifenster;

		gadget[0]=NewObject(INTEGER_GetClass(), NULL,
			GA_Top, 2, GA_Left, 30,
			GA_Width, 60, GA_Height, 22,
			GA_RelVerify, TRUE, GA_ID, 0,
			INTEGER_Number, *prog+1,
			INTEGER_Minimum, 1,
			INTEGER_Maximum, 128,
			INTEGER_Arrows, TRUE,
			ICA_MAP, mapizs,
			TAG_DONE);

		gadget[1]=NewObject(INTEGER_GetClass(), NULL,
			GA_Previous, gadget[0],
			GA_Top, 2, GA_Left, 136,
			GA_Width, 60, GA_Height, 22,
			GA_RelVerify, TRUE, GA_ID, 1,
			INTEGER_Number, *bank+1,
			INTEGER_Minimum, 1,
			INTEGER_Maximum, 128,
			INTEGER_Arrows, TRUE,
			TAG_DONE);

		gadget[2]=NewObject(SLIDER_GetClass(), NULL,
			GA_Previous, gadget[1],
			GA_Top, 26, GA_Left, 2,
			GA_Width, 196, GA_Height, 22,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 2,
			SLIDER_Min, 1, SLIDER_Max, 128,
			SLIDER_Level, *prog+1,
			SLIDER_Orientation, SORIENT_HORIZ,
			ICA_MAP, mapszi,
			TAG_DONE);

		gadget[3]=NewObject(BUTTON_GetClass(), NULL,
			GA_Previous, gadget[2],
			GA_Top, 50, GA_Left, 2,
			GA_Width, 140, GA_Height, 20,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 3, GA_Text, "nichts wählen",
			TAG_DONE);

		gadget[4]=NewObject(BUTTON_GetClass(), NULL,
			GA_Previous, gadget[3],
			GA_Top, 50, GA_Left, 144,
			GA_Width, 54, GA_Height, 20,
			GA_RelVerify, TRUE, GA_Immediate, TRUE,
			GA_ID, 4, GA_Text, "Okay",
			TAG_DONE);

		SetzeFont();
		RahmenAus(0, 0, 0, 199, 71);
		Schreibe(1, 4, 16, "Prog.", 200);
		Schreibe(1, 112, 16, "Bank", 200);
		
		AddGList(ifenster, gadget[0], 0, 6, NULL);
		RefreshGList(gadget[0], ifenster, NULL, -1);

		SetAttrs(gadget[0], ICA_TARGET, gadget[2], TAG_END);
		SetAttrs(gadget[2], ICA_TARGET, gadget[0], TAG_END);

		do {
			WaitPort(ifenster->UserPort);
			while (mes=(struct IntuiMessage *)GetMsg(ifenster->UserPort)) {
				switch (mes->Class) {
					case IDCMP_GADGETUP:
					g=(struct Gadget *)mes->IAddress;
					switch (g->GadgetID) {
						case 3:
						*bank=-1; *prog=0;
						stopp=TRUE;
						break;
						
						case 4:
						GetAttr(INTEGER_Number, gadget[0], &gw); *prog=(BYTE)gw-1;
						GetAttr(INTEGER_Number, gadget[1], &gw); *bank=(BYTE)gw-1;
						stopp=TRUE;
						break;
					};
					break;
					
					case IDCMP_INACTIVEWINDOW:
					GetAttr(INTEGER_Number, gadget[0], &gw); *prog=(BYTE)gw-1;
					GetAttr(INTEGER_Number, gadget[1], &gw); *bank=(BYTE)gw-1;
					stopp=TRUE;
					break;
				};
				ReplyMsg((struct Message *)mes);
			};
		} while (!stopp);

		for(n=0; n<5; n++) {
			if (gadget[n]) {RemoveGList(ifenster, gadget[n], 1); DisposeObject(gadget[n])};
		};

		CloseWindow(ifenster);
		aktfenster=fen;
	};
}
