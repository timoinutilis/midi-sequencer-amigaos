#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <clib/alib_protos.h>

#include <proto/window.h>
#include <proto/layout.h>
#include <proto/clicktab.h>
#include <proto/label.h>
#include <proto/button.h>
#include <proto/checkbox.h>
#include <proto/getscreenmode.h>
#include <proto/getfile.h>
#include <proto/radiobutton.h>
#include <proto/integer.h>

#include <intuition/classes.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/clicktab.h>
#include <images/label.h>
#include <gadgets/button.h>
#include <gadgets/checkbox.h>
#include <gadgets/getscreenmode.h>
#include <gadgets/getfile.h>
#include <gadgets/radiobutton.h>
#include <gadgets/integer.h>

#include "locale.h"

#include "Strukturen.h"
#include "GuiFenster.h"
#include "Gui.h"
#include "Menu.h"
#include "Projekt.h"
#include "Requester.h"
#include "Dynamic_Strings.h"


#define GAD_CLICKTAB 0
#define GAD_PAGE 1

#define GAD_WBSCREEN 2
#define GAD_SCREENMODE 3
#define GAD_BACKDROP 4

#define GAD_PFADPROJ 5
#define GAD_PFADSMF 6
#define GAD_PFADSYSEX 7
#define GAD_PFADPHONOLITH 8
#define GAD_SPBENUTZEN 9
#define GAD_STARTPROJ 10

#define GAD_TASTATUR 11
#define GAD_MAUSRADTAUSCHEN 12

#define GAD_PLAYERPRI 13
#define GAD_THRUPRI 14
#define GAD_SYSEXPUFFER 15

#define GAD_SPEICHERN 16
#define GAD_BENUTZEN 17
#define GAD_ABBRECHEN 18

#define NUM_OF_GADS 19

extern struct Screen *hschirm;
extern struct Menu *minmenu;
extern struct FENSTERPOS fenp[];

struct Window *envfenster = NULL;
Object *envfensterobj = NULL;
struct Gadget *envgad[NUM_OF_GADS];
struct List envctlist;
struct List envkeyblist;
Object *seitescreenmode = NULL;
Object *seitepfade = NULL;
Object *seitesteuerung = NULL;
Object *seitesystem = NULL;

struct UMGEBUNG umgebung = {
	TRUE, // wbscreen
	0, // screenmode
	640, // scrbreite
	480, // scrhoehe
	8, // scrtiefe
	TRUE, // backdrop
	NULL, // pfadproj
	NULL, // pfadsmf
	NULL, // pfadsysex
	NULL, // pfadphonolith
	NULL, // startproj
	FALSE, // startaktiv
	TASTATUR_PC,
	40, //playerPri
	30, //thruPri
	64, //sysexpuffer
	FALSE //mausradtauschen
};
BOOL scranders = FALSE;


void SpeichereUmgebung(void);

void InitPfadStrings(void) {
	umgebung.pfadproj = String_Copy(NULL, (STRPTR)"Projects/");
	umgebung.pfadsmf = String_Copy(NULL, (STRPTR)"Midi/");
	umgebung.pfadsysex = String_Copy(NULL, (STRPTR)"SysEx/");
	umgebung.pfadphonolith = String_Copy(NULL, (STRPTR)"Phonolith");
	umgebung.startproj = String_Copy(NULL, (STRPTR)"PROGDIR:System/Autoload.horny");
}

void EntfernePfadStrings(void) {
	String_Free(umgebung.pfadproj);
	String_Free(umgebung.pfadsmf);
	String_Free(umgebung.pfadsysex);
	String_Free(umgebung.pfadphonolith);
	String_Free(umgebung.startproj);
}

void ErstelleEnvSeiten(void) {

	// Bildschirm-Einstellungen

	seitescreenmode = LayoutObject,
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_SpaceOuter, TRUE,
			
			LAYOUT_AddChild, envgad[GAD_WBSCREEN] = (struct Gadget *)CheckBoxObject,
				GA_ID, GAD_WBSCREEN, GA_Text, CAT(MSG_0053, "Use Workbench"),
				GA_RelVerify, TRUE,
			End,
			
			LAYOUT_AddChild, envgad[GAD_SCREENMODE] = (struct Gadget *)GetScreenModeObject,
				GA_ID, GAD_SCREENMODE, GA_RelVerify, TRUE,
				GETSCREENMODE_DoWidth, TRUE,
				GETSCREENMODE_DoHeight, TRUE,
				GETSCREENMODE_DoDepth, TRUE,
				GETSCREENMODE_MinWidth, 640,
				GETSCREENMODE_MinHeight, 480,
				GETSCREENMODE_DisplayID, umgebung.screenmode,
				GETSCREENMODE_DisplayWidth, umgebung.scrbreite,
				GETSCREENMODE_DisplayHeight, umgebung.scrhoehe,
				GETSCREENMODE_DisplayDepth, umgebung.scrtiefe,
			End,
			Label(CAT(MSG_0054, "Mode")),

			LAYOUT_AddChild, envgad[GAD_BACKDROP] = (struct Gadget *)CheckBoxObject,
				GA_ID, GAD_BACKDROP, GA_Text, CAT(MSG_0055, "Main Window as Background"),
				GA_RelVerify, TRUE,
			End,
			Label(""),
		End,
		CHILD_WeightedHeight, 0,
	End;
	
	// Pfade-Einstellungen
	
	seitepfade = LayoutObject,
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_SpaceOuter, TRUE,
			
			LAYOUT_AddChild, envgad[GAD_PFADPROJ] = (struct Gadget *)GetFileObject,
				GA_ID, GAD_PFADPROJ, GA_RelVerify, TRUE,
				GETFILE_DrawersOnly, TRUE,
				GETFILE_FullFileExpand, FALSE,
			End,
			Label(CAT(MSG_0057, "Horny Projects")),

			LAYOUT_AddChild, envgad[GAD_PFADSMF] = (struct Gadget *)GetFileObject,
				GA_ID, GAD_PFADSMF, GA_RelVerify, TRUE,
				GETFILE_DrawersOnly, TRUE,
				GETFILE_FullFileExpand, FALSE,
			End,
			Label(CAT(MSG_0058, "Midi Files (SMF)")),

			LAYOUT_AddChild, envgad[GAD_PFADSYSEX] = (struct Gadget *)GetFileObject,
				GA_ID, GAD_PFADSYSEX, GA_RelVerify, TRUE,
				GETFILE_DrawersOnly, TRUE,
				GETFILE_FullFileExpand, FALSE,
			End,
			Label(CAT(MSG_0058A, "SysEx Files")),

			LAYOUT_AddChild, envgad[GAD_PFADPHONOLITH] = (struct Gadget *)GetFileObject,
				GA_ID, GAD_PFADPHONOLITH, GA_RelVerify, TRUE,
			End,
			Label("Phonolith"),

			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0059, "Start Project"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

				LAYOUT_AddChild, envgad[GAD_SPBENUTZEN] = (struct Gadget *)CheckBoxObject,
					GA_ID, GAD_SPBENUTZEN, GA_Text, CAT(MSG_0060, "Use for New Projects"),
					GA_RelVerify, TRUE,
				End,

				LAYOUT_AddChild, envgad[GAD_STARTPROJ] = (struct Gadget *)GetFileObject,
					GA_ID, GAD_STARTPROJ, GA_RelVerify, TRUE,
					GA_Disabled, TRUE,
					GETFILE_FullFileExpand, FALSE,
					GETFILE_Pattern, "#?.horny",
					GETFILE_DoPatterns, TRUE,
				End,
				Label(CAT(MSG_0062, "File")),
			End,
		End,
		CHILD_WeightedHeight, 0,
	End;

	// Steuerungs-Einstellungen
	IExec->NewList(&envkeyblist);
	IExec->AddTail(&envkeyblist, IRadioButton->AllocRadioButtonNode(1, RBNA_Label, CAT(MSG_0062B, "Amiga Keyboard (Rec: <*> Fast: <[> <]> Marker: <+> <->)"), TAG_DONE));
	IExec->AddTail(&envkeyblist, IRadioButton->AllocRadioButtonNode(1, RBNA_Label, CAT(MSG_0062C, "PC Keyboard (Rec: <-> Fast: </> <*> Marker: <PgUp> <PgDn>)"), TAG_DONE));
	
	seitesteuerung = LayoutObject,
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_SpaceOuter, TRUE,

			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0062A, "Transport Functions (Numpad)"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

				LAYOUT_AddChild, envgad[GAD_TASTATUR] = (struct Gadget *)RadioButtonObject,
					GA_ID, GAD_TASTATUR,
					RADIOBUTTON_Labels, &envkeyblist,
					GA_RelVerify, TRUE,
				End,
			End,

			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0062I, "Mouse Wheels"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

				LAYOUT_AddChild, envgad[GAD_MAUSRADTAUSCHEN] = (struct Gadget *)CheckBoxObject,
					GA_ID, GAD_MAUSRADTAUSCHEN, GA_Text, CAT(MSG_0062J, "Swap Horizontal/Vertical"),
					GA_RelVerify, TRUE,
				End,
			End,

		End,
		CHILD_WeightedHeight, 0,
	End;

	// System-Einstellungen
	seitesystem = LayoutObject,
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_SpaceOuter, TRUE,

			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_AddChild, HLayoutObject,
				End,
				Label(CAT(MSG_0062D, "System settings take affect after application restart")),
			End,

			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0062E, "Multitasking Priorities"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

				LAYOUT_AddChild, envgad[GAD_PLAYERPRI] = (struct Gadget *)IntegerObject,
					GA_ID, GAD_PLAYERPRI, GA_RelVerify, TRUE,
					INTEGER_Minimum, -128,
					INTEGER_Maximum, 127,
					INTEGER_MinVisible, 8,
				End,
				Label(CAT(MSG_0062F, "Player Process")),
				CHILD_WeightedWidth, 0,

				LAYOUT_AddChild, envgad[GAD_THRUPRI] = (struct Gadget *)IntegerObject,
					GA_ID, GAD_THRUPRI, GA_RelVerify, TRUE,
					INTEGER_Minimum, -128,
					INTEGER_Maximum, 127,
					INTEGER_MinVisible, 8,
				End,
				Label(CAT(MSG_0062G, "Thru Process")),
				CHILD_WeightedWidth, 0,
			End,

			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, "CAMD", LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

				LAYOUT_AddChild, envgad[GAD_SYSEXPUFFER] = (struct Gadget *)IntegerObject,
					GA_ID, GAD_PLAYERPRI, GA_RelVerify, TRUE,
					INTEGER_Minimum, 1,
					INTEGER_MinVisible, 8,
				End,
				Label(CAT(MSG_0062H, "SysEx Receive Buffer Size (kB)")),
				CHILD_WeightedWidth, 0,
			End,
		End,
		CHILD_WeightedHeight, 0,
	End;

}

void ErstelleUmgebungsfenster(void) {
	struct Screen *scr;

	if (!envfensterobj) {
		if (!umgebung.screenmode) {
			scr = IIntuition->LockPubScreen(NULL);
			if (scr) {
				umgebung.screenmode = IGraphics->GetVPModeID(&scr->ViewPort);
				umgebung.scrbreite = scr->Width;
				umgebung.scrhoehe = scr->Height;
				umgebung.scrtiefe = scr->RastPort.BitMap->Depth;
				IIntuition->UnlockPubScreen(NULL, scr);
			}
		}

		ErstelleEnvSeiten();

		IExec->NewList(&envctlist);
		IExec->AddTail(&envctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0063, "Screen"), TNA_Number, 0, TAG_DONE));
		IExec->AddTail(&envctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0064, "Paths"), TNA_Number, 1, TAG_DONE));
		IExec->AddTail(&envctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0064A, "Control"), TNA_Number, 2, TAG_DONE));
		IExec->AddTail(&envctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0064B, "System"), TNA_Number, 3, TAG_DONE));

		envfensterobj = WindowObject,
			WA_PubScreen, hschirm,
			WA_Title, CAT(MSG_0065, "Environment Settings"),
			WA_Activate, TRUE,
			WA_DepthGadget, TRUE,
			WA_DragBar, TRUE,
			WA_SizeGadget, TRUE,
			WA_SizeBBottom, TRUE,
			WA_IDCMP, IDCMP_MENUPICK,
			WINDOW_ParentGroup, VLayoutObject,
				LAYOUT_SpaceOuter, TRUE,

				LAYOUT_AddChild, envgad[GAD_CLICKTAB] = (struct Gadget *)ClickTabObject,
					GA_ID, GAD_CLICKTAB, GA_RelVerify, TRUE,
					CLICKTAB_Labels, &envctlist,
					CLICKTAB_PageGroup, envgad[GAD_PAGE] = (struct Gadget *)PageObject,
						PAGE_Add, seitescreenmode,
						PAGE_Add, seitepfade,
						PAGE_Add, seitesteuerung,
						PAGE_Add, seitesystem,
						PAGE_Current, 0,
					End,
				End,

				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_BevelStyle, BVS_SBAR_VERT, LAYOUT_SpaceOuter, TRUE, LAYOUT_EvenSize, TRUE,
					LAYOUT_AddChild, ButtonObject, GA_ID, GAD_SPEICHERN, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0066, "Save"), End,
					CHILD_WeightedWidth, 0,
					LAYOUT_AddChild, ButtonObject, GA_ID, GAD_BENUTZEN, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0067, "Use"), End,
					CHILD_WeightedWidth, 0,
					LAYOUT_AddChild, ButtonObject, GA_ID, GAD_ABBRECHEN, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0068, "Cancel"), End,
					CHILD_WeightedWidth, 0,
				End,
				CHILD_WeightedHeight, 0,

			End,
		End;
	}	
	
	if (envfensterobj) {
		if (fenp[ENV].b > 0) {
			IIntuition->SetAttrs(envfensterobj,
				WA_Left, fenp[ENV].x, WA_Top, fenp[ENV].y,
				WA_InnerWidth, fenp[ENV].b, WA_InnerHeight, fenp[ENV].h,
				TAG_DONE);
		}

		ILayout->SetPageGadgetAttrs(envgad[GAD_WBSCREEN], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GA_Selected, umgebung.wbscreen,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_SCREENMODE], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GA_Disabled, umgebung.wbscreen,
			GETSCREENMODE_DisplayID, umgebung.screenmode,
			GETSCREENMODE_DisplayWidth, umgebung.scrbreite,
			GETSCREENMODE_DisplayHeight, umgebung.scrhoehe,
			GETSCREENMODE_DisplayDepth, umgebung.scrtiefe,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_BACKDROP], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GA_Disabled, umgebung.wbscreen,
			GA_Selected, umgebung.backdrop,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_PFADPROJ], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GETFILE_Drawer, umgebung.pfadproj,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_PFADSMF], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GETFILE_Drawer, umgebung.pfadsmf,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_PFADSYSEX], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GETFILE_Drawer, umgebung.pfadsysex,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_SPBENUTZEN], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GA_Selected, umgebung.startaktiv,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_PFADPHONOLITH], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GETFILE_FullFile, umgebung.pfadphonolith,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_STARTPROJ], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GA_Disabled, !umgebung.startaktiv,
			GETFILE_FullFile, umgebung.startproj,
			TAG_DONE);

		ILayout->SetPageGadgetAttrs(envgad[GAD_TASTATUR], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			RADIOBUTTON_Selected, umgebung.tastatur,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_MAUSRADTAUSCHEN], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			GA_Selected, umgebung.mausradtauschen,
			TAG_DONE);

		ILayout->SetPageGadgetAttrs(envgad[GAD_PLAYERPRI], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			INTEGER_Number, umgebung.playerPri,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_THRUPRI], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			INTEGER_Number, umgebung.thruPri,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(envgad[GAD_SYSEXPUFFER], (Object *)envgad[GAD_PAGE], envfenster, NULL,
			INTEGER_Number, umgebung.sysexpuffer,
			TAG_DONE);


		envfenster = (struct Window *)RA_OpenWindow(envfensterobj);
		IIntuition->SetMenuStrip(envfenster, minmenu);
	}
	scranders = FALSE;
}

void EntferneUmgebungsfenster(void) {
	struct Node *node;
	
	if (envfensterobj) {
		if (envfenster) {
			HoleFensterObjpos(envfensterobj, ENV);
			IIntuition->ClearMenuStrip(envfenster);
		}
		IIntuition->DisposeObject(envfensterobj);
		envfensterobj = NULL;
		envfenster = NULL;
		
		while ((node = IExec->RemTail(&envctlist))) {
			IClickTab->FreeClickTabNode(node);
		}
		while ((node = IExec->RemTail(&envkeyblist))) {
			IRadioButton->FreeRadioButtonNode(node);
		}
	}
}

void UmgebungsGadgetsLesen(void) {
	uint32 var;
	STRPTR str;

	// Bildschirm
	IIntuition->GetAttr(GA_Selected, (Object *)envgad[GAD_WBSCREEN], &var); umgebung.wbscreen = (BOOL)var;
	IIntuition->GetAttr(GETSCREENMODE_DisplayID, (Object *)envgad[GAD_SCREENMODE], &umgebung.screenmode);
	IIntuition->GetAttr(GETSCREENMODE_DisplayWidth, (Object *)envgad[GAD_SCREENMODE], &var); umgebung.scrbreite = (int16)var;
	IIntuition->GetAttr(GETSCREENMODE_DisplayHeight, (Object *)envgad[GAD_SCREENMODE], &var); umgebung.scrhoehe = (int16)var;
	IIntuition->GetAttr(GETSCREENMODE_DisplayDepth, (Object *)envgad[GAD_SCREENMODE], &var); umgebung.scrtiefe = (int16)var;
	IIntuition->GetAttr(GA_Selected, (Object *)envgad[GAD_BACKDROP], &var); umgebung.backdrop = (BOOL)var;

	// Pfade
	IIntuition->GetAttr(GETFILE_Drawer, (Object *)envgad[GAD_PFADPROJ], (uint32 *)&str); umgebung.pfadproj = String_Copy(umgebung.pfadproj, str);
	IIntuition->GetAttr(GETFILE_Drawer, (Object *)envgad[GAD_PFADSMF], (uint32 *)&str); umgebung.pfadsmf = String_Copy(umgebung.pfadsmf, str);
	IIntuition->GetAttr(GETFILE_Drawer, (Object *)envgad[GAD_PFADSYSEX], (uint32 *)&str); umgebung.pfadsysex = String_Copy(umgebung.pfadsysex, str);
	IIntuition->GetAttr(GETFILE_FullFile, (Object *)envgad[GAD_PFADPHONOLITH], (uint32 *)&str); umgebung.pfadphonolith = String_Copy(umgebung.pfadphonolith, str);
	IIntuition->GetAttr(GETFILE_FullFile, (Object *)envgad[GAD_STARTPROJ], (uint32 *)&str); umgebung.startproj = String_Copy(umgebung.startproj, str);
	IIntuition->GetAttr(GA_Selected, (Object *)envgad[GAD_SPBENUTZEN], &var); umgebung.startaktiv = (BOOL)var;

	// Steuerung
	IIntuition->GetAttr(RADIOBUTTON_Selected, (Object *)envgad[GAD_TASTATUR], &var); umgebung.tastatur = (uint8)var;
	IIntuition->GetAttr(GA_Selected, (Object *)envgad[GAD_MAUSRADTAUSCHEN], &var); umgebung.mausradtauschen = (BOOL)var;

	// System
	IIntuition->GetAttr(INTEGER_Number, (Object *)envgad[GAD_PLAYERPRI], &var); umgebung.playerPri = (int8)var;
	IIntuition->GetAttr(INTEGER_Number, (Object *)envgad[GAD_THRUPRI], &var); umgebung.thruPri = (int8)var;
	IIntuition->GetAttr(INTEGER_Number, (Object *)envgad[GAD_SYSEXPUFFER], &var); umgebung.sysexpuffer = (int16)var;
}

BOOL KontrolleUmgebungsfenster(void) {
	BOOL schliessen = FALSE;
	uint32 result;
	uint16 code;
	
	while ((result = RA_HandleInput(envfensterobj, &code)) != WMHI_LASTMSG) {
		switch (result & WMHI_CLASSMASK) {
			case WMHI_MENUPICK:
			MinMenuKontrolle(MinMenuPunkt(result & WMHI_MENUMASK));
			break;
			
			case WMHI_GADGETUP:
			switch (result & WMHI_GADGETMASK) {
				case GAD_WBSCREEN:
				scranders = TRUE;
				if (IIntuition->SetGadgetAttrs(envgad[GAD_SCREENMODE], envfenster, NULL, GA_Disabled, code, TAG_DONE)) {
					ILayout->RethinkLayout(envgad[GAD_SCREENMODE], envfenster, NULL, TRUE);
				}
				if (IIntuition->SetGadgetAttrs(envgad[GAD_BACKDROP], envfenster, NULL, GA_Disabled, code, TAG_DONE)) {
					ILayout->RethinkLayout(envgad[GAD_BACKDROP], envfenster, NULL, TRUE);
				}
				break;
				
				case GAD_SCREENMODE:
				RequestScreenMode((Object *)envgad[GAD_SCREENMODE], envfenster);
				scranders = TRUE;
				break;
				
				case GAD_BACKDROP:
				scranders = TRUE;
				break;
				
				case GAD_PFADPROJ:
				gfRequestDir((Object *)envgad[GAD_PFADPROJ], envfenster);
				break;

				case GAD_PFADSMF:
				gfRequestDir((Object *)envgad[GAD_PFADSMF], envfenster);
				break;

				case GAD_PFADSYSEX:
				gfRequestDir((Object *)envgad[GAD_PFADSYSEX], envfenster);
				break;

				case GAD_PFADPHONOLITH:
				gfRequestFile((Object *)envgad[GAD_PFADPHONOLITH], envfenster);
				break;

				case GAD_SPBENUTZEN:
				IIntuition->SetGadgetAttrs(envgad[GAD_STARTPROJ], envfenster, NULL, GA_Disabled, !code, TAG_DONE);
				break;

				case GAD_STARTPROJ:
				gfRequestFile((Object *)envgad[GAD_STARTPROJ], envfenster);
				break;
								
				case GAD_SPEICHERN:
				UmgebungsGadgetsLesen();
				SpeichereUmgebung();
				schliessen = TRUE;
				break;

				case GAD_BENUTZEN:
				UmgebungsGadgetsLesen();
				schliessen = TRUE;
				break;
				
				case GAD_ABBRECHEN:
				scranders = FALSE; schliessen = TRUE;
				break;			
			}
			break;
		}
	}
	if (schliessen) {
		HoleFensterObjpos(envfensterobj, ENV);
		IIntuition->ClearMenuStrip(envfenster);
		RA_CloseWindow(envfensterobj);
		envfenster = NULL;
		return(scranders);
	}
	return(FALSE);
}


void SpeichereUmgebung(void) {
	BPTR file;
	char zeile[1024];
	
	file = IDOS->Open("PROGDIR:System/environment.config", MODE_NEWFILE);
	if (file) {
		sprintf(zeile, "wb_screen=%d\n", umgebung.wbscreen); IDOS->FPuts(file, zeile);
		sprintf(zeile, "screen_mode=%ld\n", umgebung.screenmode); IDOS->FPuts(file, zeile);
		sprintf(zeile, "screen_width=%d\n", umgebung.scrbreite); IDOS->FPuts(file, zeile);
		sprintf(zeile, "screen_height=%d\n", umgebung.scrhoehe); IDOS->FPuts(file, zeile);
		sprintf(zeile, "screen_depth=%d\n", umgebung.scrtiefe); IDOS->FPuts(file, zeile);
		sprintf(zeile, "backdrop=%d\n", umgebung.backdrop); IDOS->FPuts(file, zeile);
		sprintf(zeile, "path_project=%s\n", umgebung.pfadproj); IDOS->FPuts(file, zeile);
		sprintf(zeile, "path_midi=%s\n", umgebung.pfadsmf); IDOS->FPuts(file, zeile);
		sprintf(zeile, "path_sysex=%s\n", umgebung.pfadsysex); IDOS->FPuts(file, zeile);
		sprintf(zeile, "path_phonolith=%s\n", umgebung.pfadphonolith); IDOS->FPuts(file, zeile);
		sprintf(zeile, "start_project=%s\n", umgebung.startproj); IDOS->FPuts(file, zeile);
		sprintf(zeile, "start_project_active=%d\n", umgebung.startaktiv); IDOS->FPuts(file, zeile);
		sprintf(zeile, "keyboard_type=%d\n", umgebung.tastatur); IDOS->FPuts(file, zeile);
		sprintf(zeile, "mouse_wheel_swap=%d\n", umgebung.mausradtauschen); IDOS->FPuts(file, zeile);
		sprintf(zeile, "player_priority=%d\n", umgebung.playerPri); IDOS->FPuts(file, zeile);
		sprintf(zeile, "thru_priority=%d\n", umgebung.thruPri); IDOS->FPuts(file, zeile);
		sprintf(zeile, "sysex_buffer_size=%d\n", umgebung.sysexpuffer); IDOS->FPuts(file, zeile);

		IDOS->Close(file);
	}
}

void SpeichereFensterPos(void) {
	BPTR file;
	char zeile[1024];
	
	file = IDOS->Open("PROGDIR:System/windows.config", MODE_NEWFILE);
	if (file) {
		sprintf(zeile, "window_main=%d/%d/%d/%d\n", fenp[HAUPT].x, fenp[HAUPT].y, fenp[HAUPT].b, fenp[HAUPT].h); IDOS->FPuts(file, zeile);
		sprintf(zeile, "window_set=%d/%d/%d/%d\n", fenp[SET].x, fenp[SET].y, fenp[SET].b, fenp[SET].h); IDOS->FPuts(file, zeile);
		sprintf(zeile, "window_env=%d/%d/%d/%d\n", fenp[ENV].x, fenp[ENV].y, fenp[ENV].b, fenp[ENV].h); IDOS->FPuts(file, zeile);
		sprintf(zeile, "window_ed=%d/%d/%d/%d\n", fenp[ED].x, fenp[ED].y, fenp[ED].b, fenp[ED].h); IDOS->FPuts(file, zeile);
		sprintf(zeile, "window_sex=%d/%d/%d/%d\n", fenp[SEX].x, fenp[SEX].y, fenp[SEX].b, fenp[SEX].h); IDOS->FPuts(file, zeile);
		sprintf(zeile, "window_mp=%d/%d/%d/%d\n", fenp[MISCHER].x, fenp[MISCHER].y, fenp[MISCHER].b, fenp[MISCHER].h); IDOS->FPuts(file, zeile);
		sprintf(zeile, "window_cc=%d/%d/%d/%d\n", fenp[CC].x, fenp[CC].y, fenp[CC].b, fenp[CC].h); IDOS->FPuts(file, zeile);

		IDOS->Close(file);
	}
}

void LadeUmgebung(void) {
	BPTR file;
	char zeile[1024];
	STRPTR wert;
	char *ende;
	
	file = IDOS->Open("PROGDIR:System/environment.config", MODE_OLDFILE);
	if (file) {
		while (IDOS->FGets(file, zeile, 1024)) {
			wert = strchr(zeile, '=');
			if (wert) {
				*wert++ = 0;
				
				ende = strchr(wert, '\n');
				if (ende) *ende = 0;
				
				if (strcmp(zeile, "wb_screen") == 0) umgebung.wbscreen = (BOOL)atoi(wert);
				else if (strcmp(zeile, "screen_mode") == 0) umgebung.screenmode = (uint32)atol(wert);
				else if (strcmp(zeile, "screen_width") == 0) umgebung.scrbreite = (int16)atoi(wert);
				else if (strcmp(zeile, "screen_height") == 0) umgebung.scrhoehe = (int16)atoi(wert);
				else if (strcmp(zeile, "screen_depth") == 0) umgebung.scrtiefe = (uint8)atoi(wert);
				else if (strcmp(zeile, "backdrop") == 0) umgebung.backdrop = (BOOL)atoi(wert);
				else if (strcmp(zeile, "path_project") == 0) umgebung.pfadproj = String_Copy(umgebung.pfadproj, wert);
				else if (strcmp(zeile, "path_midi") == 0) umgebung.pfadsmf = String_Copy(umgebung.pfadsmf, wert);
				else if (strcmp(zeile, "path_sysex") == 0) umgebung.pfadsysex = String_Copy(umgebung.pfadsysex, wert);
				else if (strcmp(zeile, "path_phonolith") == 0) umgebung.pfadphonolith = String_Copy(umgebung.pfadphonolith, wert);
				else if (strcmp(zeile, "start_project") == 0) umgebung.startproj = String_Copy(umgebung.startproj, wert);
				else if (strcmp(zeile, "start_project_active") == 0) umgebung.startaktiv = (BOOL)atoi(wert);
				else if (strcmp(zeile, "keyboard_type") == 0) umgebung.tastatur = (uint8)atoi(wert);
				else if (strcmp(zeile, "mouse_wheel_swap") == 0) umgebung.mausradtauschen = (BOOL)atoi(wert);

				else if (strcmp(zeile, "player_priority") == 0) umgebung.playerPri = (int8)atoi(wert);
				else if (strcmp(zeile, "thru_priority") == 0) umgebung.thruPri = (int8)atoi(wert);
				else if (strcmp(zeile, "sysex_buffer_size") == 0) umgebung.sysexpuffer = (int16)atoi(wert);
			}
		}
		IDOS->Close(file);
	}
}

void DekodiereFensterPos(uint8 fenid, STRPTR wert) {
	fenp[fenid].x = atoi(wert);
	wert = strchr(wert, '/');
	if (wert) {wert++; fenp[fenid].y = atoi(wert);}
	wert = strchr(wert, '/');
	if (wert) {wert++; fenp[fenid].b = atoi(wert);}
	wert = strchr(wert, '/');
	if (wert) {wert++; fenp[fenid].h = atoi(wert);}
}

void LadeFensterPos(void) {
	BPTR file;
	char zeile[1024];
	STRPTR wert;
	char *ende;
	
	file = IDOS->Open("PROGDIR:System/windows.config", MODE_OLDFILE);
	if (file) {
		while (IDOS->FGets(file, zeile, 1024)) {
			wert = strchr(zeile, '=');
			if (wert) {
				*wert++ = 0;
				
				ende = strchr(wert, '\n');
				if (ende) *ende = 0;
				
				if (strcmp(zeile, "window_main") == 0) DekodiereFensterPos(HAUPT, wert);
				else if (strcmp(zeile, "window_set") == 0) DekodiereFensterPos(SET, wert);
				else if (strcmp(zeile, "window_env") == 0) DekodiereFensterPos(ENV, wert);
				else if (strcmp(zeile, "window_ed") == 0) DekodiereFensterPos(ED, wert);
				else if (strcmp(zeile, "window_sex") == 0) DekodiereFensterPos(SEX, wert);
				else if (strcmp(zeile, "window_mp") == 0) DekodiereFensterPos(MISCHER, wert);
				else if (strcmp(zeile, "window_cc") == 0) DekodiereFensterPos(CC, wert);
			}
		}
		IDOS->Close(file);
	}
}
