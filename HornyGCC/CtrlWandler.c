#include <stdio.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <clib/alib_protos.h>

#include <proto/window.h>
#include <proto/layout.h>
#include <proto/button.h>
#include <proto/listbrowser.h>
#include <proto/string.h>

#include <intuition/classes.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/listbrowser.h>
#include <gadgets/string.h>

#include "locale.h"

#include "Strukturen.h"
#include "Gui.h"
#include "GuiFenster.h"
#include "Menu.h"
#include "Projekt.h"
#include "Requester.h"
#include "Instrumente.h"

#include "CtrlWandler.h"

#define GAD_CCLISTE 0
#define GAD_NAME 1
#define GAD_NEW 2
#define GAD_DEL 3
#define GAD_ORIG 4
#define GAD_DEST 5

#define NUM_OF_GADS 6

struct CTRLCHANGE *rootctrlchange = NULL;

extern struct Screen *hschirm;
extern struct Menu *minmenu;
extern struct FENSTERPOS fenp[];
extern struct SPUR spur[];
extern WORD snum;

struct Window *ccfenster = NULL;
Object *ccfensterobj = NULL;
struct Gadget *ccgad[NUM_OF_GADS];

struct ColumnInfo cccolinfo[] = {{60, NULL, 0}, {20, NULL, 0}, {20, NULL, 0}, {-1, NULL, 0}};
struct List cclist = {NULL};


struct CTRLCHANGE *AddChangeCtrl(STRPTR name) {
	struct CTRLCHANGE *neu;
	
	neu = (struct CTRLCHANGE *)AllocVec(sizeof(struct CTRLCHANGE), 0);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->original = 0;
		neu->ziel = 0;
		neu->aktiv = TRUE;
		neu->next = rootctrlchange;
		rootctrlchange = neu;
	}
	return(neu);
}

void EntferneChangeCtrl(struct CTRLCHANGE *cc) {
	struct CTRLCHANGE *akt;
	struct CTRLCHANGE *last;
	
	if (cc == rootctrlchange) {
		rootctrlchange = rootctrlchange->next;
		FreeVec(cc);
	} else {
		last = NULL;
		akt = rootctrlchange;
		while (akt) {
			if (akt == cc) {
				last->next = akt->next;
				FreeVec(cc);
				break;
			}
			last = akt;
			akt = akt->next;
		}
	} 
}

void EntferneAlleChangeCtrl(void) {
	struct CTRLCHANGE *akt;
	struct CTRLCHANGE *next;
	
	akt = rootctrlchange;
	while (akt) {
		next = akt->next;
		FreeVec(akt);
		akt = next;
	}
	rootctrlchange = NULL;
}

BYTE WandleController(BYTE data1) {
	struct CTRLCHANGE *akt;
	
	akt = rootctrlchange;
	while (akt) {
		if ((data1 == akt->original) && (akt->aktiv)) return(akt->ziel);
		akt = akt->next;
	}
	return(data1);
}




void AktualisiereCCListe(void) {
	struct Node *node;
	struct CTRLCHANGE *akt;
	char original[5];
	char ziel[5];
	
	if (cclist.lh_Head) {
		SetGadgetAttrs(ccgad[GAD_CCLISTE], ccfenster, NULL,
			LISTBROWSER_Labels, NULL,
			TAG_DONE);
		while (node = RemTail(&cclist)) FreeListBrowserNode(node);
	}

	akt = rootctrlchange;
	while (akt) {
		sprintf(original, "%d", akt->original);
		sprintf(ziel, "%d", akt->ziel);
		node = AllocListBrowserNode(3,
			LBNA_CheckBox, TRUE,
			LBNA_Checked, akt->aktiv,
			LBNA_Column, 0,
			LBNCA_Text, akt->name,
			LBNA_Column, 1,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, original,
			LBNCA_Justification, LCJ_RIGHT,
			LBNA_Column, 2,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, ziel,
			LBNCA_Justification, LCJ_RIGHT,
			TAG_DONE);
		if (node) AddTail(&cclist, node);
		akt = akt->next;
	}
	SetGadgetAttrs(ccgad[GAD_CCLISTE], ccfenster, NULL,
		LISTBROWSER_Labels, &cclist,
		LISTBROWSER_Selected, -1,
		TAG_DONE);
}

void ErstelleChangeCtrlFenster(void) {
	if (!ccfensterobj) {
	
		NewList(&cclist);
		cccolinfo[0].ci_Title = CAT(MSG_0005, "Name");
		cccolinfo[1].ci_Title = CAT(MSG_0005A, "Original");
		cccolinfo[2].ci_Title = CAT(MSG_0005B, "Destination");

		ccfensterobj = WindowObject,
			WA_PubScreen, hschirm,
			WA_Title, CAT(MSG_0005C, "Controller Transformer"),
			WA_Activate, TRUE,
			WA_CloseGadget, TRUE,
			WA_DepthGadget, TRUE,
			WA_DragBar, TRUE,
			WA_SizeGadget, TRUE,
			WA_SizeBBottom, TRUE,
			WA_IDCMP, IDCMP_MENUPICK,
			WINDOW_ParentGroup, VLayoutObject,
				LAYOUT_SpaceOuter, TRUE,
				
				LAYOUT_AddChild, ccgad[GAD_CCLISTE] = ListBrowserObject,
					GA_ID, 0, GA_RelVerify, TRUE,
					LISTBROWSER_Labels, NULL,
					LISTBROWSER_ShowSelected, TRUE,
					LISTBROWSER_ColumnInfo, cccolinfo,
					LISTBROWSER_ColumnTitles, TRUE,
				End,
					
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, ButtonObject, GA_ID, GAD_NEW, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0005D, "New"), End,
					LAYOUT_AddChild, ButtonObject, GA_ID, GAD_DEL, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0005E, "Delete"), End,
				End,
				CHILD_WeightedHeight, 0,

				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_Label, CAT(MSG_0005F, "Adjust Transformer"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

					LAYOUT_AddChild, ccgad[GAD_NAME] = StringObject,
						GA_ID, GAD_NAME, GA_RelVerify, TRUE,
						STRINGA_MaxChars, 127,
					End,
					
					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_ORIG, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0005G, "Set Original"), End,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_DEST, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0005H, "Set Destination"), End,
					End,
				End,
				CHILD_WeightedHeight, 0,
			End,
		End;
	}	
	
	if (ccfensterobj) {
		if (fenp[CC].b>0) {
			SetAttrs(ccfensterobj,
				WA_Left, fenp[CC].x, WA_Top, fenp[CC].y,
				WA_InnerWidth, fenp[CC].b, WA_InnerHeight, fenp[CC].h,
				TAG_DONE);
		}

		ccfenster=(struct Window *)RA_OpenWindow(ccfensterobj);
		SetMenuStrip(ccfenster, minmenu);
		AktualisiereCCListe();
	}
}

void EntferneChangeCtrlFenster(void) {
	struct Node *node;
	
	if (ccfensterobj) {
		if (ccfenster) {
			HoleFensterObjpos(ccfensterobj, CC);
			ClearMenuStrip(ccfenster);
		}
		DisposeObject(ccfensterobj);
		ccfensterobj = NULL;
		ccfenster = NULL;
		while (node = RemTail(&cclist)) FreeListBrowserNode(node);
	}
}

struct CTRLCHANGE *SucheCC(WORD n) {
	WORD i;
	struct CTRLCHANGE *cc;

	cc = rootctrlchange;
	if (cc) {
		for (i = 0; i < n; i++) {
			if (cc->next) cc = cc->next;
		}
	}
	return(cc);
}

void KontrolleChangeCtrlFenster(void) {
	BOOL schliessen = FALSE;
	ULONG result;
	UWORD code;
	struct Node *node;
	LONG var;
	BYTE wert;
	STRPTR name = NULL;
	struct CTRLCHANGE *cc;
	
	while ((result = RA_HandleInput(ccfensterobj, &code)) != WMHI_LASTMSG) {
		switch (result & WMHI_CLASSMASK) {
			case WMHI_CLOSEWINDOW:
			schliessen = TRUE;
			break;

			case WMHI_MENUPICK:
			MinMenuKontrolle(MinMenuPunkt(result & WMHI_MENUMASK));
			break;
			
			case WMHI_GADGETUP:
			switch (result & WMHI_GADGETMASK) {
				case GAD_CCLISTE:
				if ((WORD)code >= 0) {
					cc = SucheCC(code);
					SetGadgetAttrs(ccgad[GAD_NAME], ccfenster, NULL,
						STRINGA_TextVal, cc->name,
						TAG_DONE);
					GetAttr(LISTBROWSER_SelectedNode, ccgad[GAD_CCLISTE], (ULONG *)&node);
					GetListBrowserNodeAttrs(node, LBNA_Checked, &var, TAG_DONE);
					cc->aktiv = (BOOL)var;
				}
				break;
				
				case GAD_NAME:
				GetAttr(LISTBROWSER_Selected, ccgad[GAD_CCLISTE], (ULONG *)&var);
				if (var >= 0) {
					cc = SucheCC(var);
					GetAttr(STRINGA_TextVal, ccgad[GAD_NAME], (ULONG *)&name);
					strncpy(cc->name, name, 128);
					AktualisiereCCListe();
				}
				break;

				case GAD_NEW:
				cc = AddChangeCtrl(CAT(MSG_0005D, "New"));
				if (cc) {
					AktualisiereCCListe();
					SetGadgetAttrs(ccgad[GAD_NAME], ccfenster, NULL,
						STRINGA_TextVal, cc->name,
						TAG_DONE);
					SetGadgetAttrs(ccgad[GAD_CCLISTE], ccfenster, NULL,
						LISTBROWSER_Selected, 0,
						TAG_DONE);
				}
				break;
				
				case GAD_DEL:
				GetAttr(LISTBROWSER_Selected, ccgad[GAD_CCLISTE], (ULONG *)&var);
				if (var >= 0) {
					cc = SucheCC(var);
					EntferneChangeCtrl(cc);
					AktualisiereCCListe();
				}
				break;
				
				case GAD_ORIG:
				GetAttr(LISTBROWSER_Selected, ccgad[GAD_CCLISTE], (ULONG *)&var);
				if (var >= 0) {
					cc = SucheCC(var);
					wert = InstrControllerFenster(ccfenster, spur[snum].channel, spur[snum].port, cc->original);
					if (wert >= 0) {
						cc->original = wert;
						AktualisiereCCListe();
					}
				}
				break;

				case GAD_DEST:
				GetAttr(LISTBROWSER_Selected, ccgad[GAD_CCLISTE], (ULONG *)&var);
				if (var >= 0) {
					cc = SucheCC(var);
					wert = InstrControllerFenster(ccfenster, spur[snum].channel, spur[snum].port, cc->ziel);
					if (wert >= 0) {
						cc->ziel = wert;
						AktualisiereCCListe();
					}
				}
				break;
			}
			break;
		}
	}
	if (schliessen) {
		HoleFensterObjpos(ccfensterobj, CC);
		ClearMenuStrip(ccfenster);
		RA_CloseWindow(ccfensterobj);
		ccfenster = NULL;
	}
}
