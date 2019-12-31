#include <stdio.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/camd.h>
#include <clib/alib_protos.h>

#include <proto/window.h>
#include <proto/layout.h>
#include <proto/label.h>
#include <proto/button.h>
#include <proto/chooser.h>
#include <proto/string.h>
#include <proto/integer.h>
#include <proto/slider.h>
#include <proto/listbrowser.h>
#include <proto/clicktab.h>
#include <proto/radiobutton.h>
#include <proto/getfile.h>
#include <proto/checkbox.h>

#include <intuition/classes.h>
#include <midi/camd.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <images/label.h>
#include <gadgets/button.h>
#include <gadgets/chooser.h>
#include <gadgets/string.h>
#include <gadgets/integer.h>
#include <gadgets/slider.h>
#include <gadgets/listbrowser.h>
#include <gadgets/clicktab.h>
#include <gadgets/radiobutton.h>
#include <gadgets/getfile.h>
#include <gadgets/checkbox.h>

#include "Strukturen.h"
#include "GuiFenster.h"
#include "Versionen.h"
#include "Midi.h"
#include "Fenster.h"
#include "Requester.h"
#include "Gui.h"
#include "Gui2.h"
#include "Menu.h"
#include "Projekt.h"
#include "SysEx.h"
#include "Instrumente.h"
#include "Smpte.h"
#include "Marker.h"

#include "locale.h"


#define GAD_CLICKTAB 0
#define GAD_PAGE 1

#define GAD_OPLIST 2
#define GAD_OPANDERS 3
#define GAD_OPHINZU 4
#define GAD_OPENTF 5
#define GAD_OPLATENZ 6
#define GAD_OUTINSTR1 7
#define GAD_OUTINSTR2 8
#define GAD_OUTINSTR3 9
#define GAD_OUTINSTR4 10
#define GAD_OUTINSTRCH2 11
#define GAD_OUTINSTRCH3 12
#define GAD_OUTINSTRCH4 13
#define GAD_IPLIST 14
#define GAD_IPHINZU 15
#define GAD_IPENTF 16
#define GAD_PHONOLITHPROJ 17

#define GAD_MKANAL 18
#define GAD_MTASTE1 19
#define GAD_MTASTE2 20
#define GAD_MVELO1 21
#define GAD_MVELO2 22
#define GAD_MRASTER 23

#define GAD_SHH 24
#define GAD_SMM 25
#define GAD_SSS 26
#define GAD_SFF 27
#define GAD_SLOESCHEN 28
#define GAD_SUEBERNEHMEN 29
#define GAD_SFORMAT 30

#define GAD_NUM 31

extern struct Screen *hschirm;
extern struct Window *hfenster;
extern struct Menu *minmenu;
extern struct FENSTERPOS fenp[];

struct Window *setfenster = NULL;
Object *setfensterobj = NULL;
struct Gadget *setgad[GAD_NUM];
struct List setctlist;

Object *seitemidiports = NULL;
struct List clusterlist;
struct List outportlist;
struct List setinstrlist;
struct ColumnInfo opcolinfo[] = {{10, "Port", 0},{10, "Midi Cluster", 0}, {-1, NULL, 0}};

extern struct LIED lied;
extern struct OUTPORT outport[];
struct List inportlist;
extern struct INPORT inport[];
extern struct INSTRUMENT *rootinstrument;

Object *seitemetronom = NULL;
extern struct METRONOM metro;
struct List rasterlist;
char porttext[128];

Object *seitesmpte = NULL;
extern struct SMPTE smpte;
struct List fpslist;

void AktualisiereOutPortListe(void) {
	struct Node *node;
	BYTE n;
	char zahl[4];
	
	if (outportlist.lh_Head) {
		SetPageGadgetAttrs(setgad[GAD_OPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			LISTBROWSER_Labels, NULL,
			TAG_DONE);
		while (node = RemTail(&outportlist)) FreeListBrowserNode(node);
	}

	for (n = 0; n < verOUTPORTS; n++) {
		if (!outport[n].name[0]) break;
		sprintf(zahl, "%d", n + 1);
		node = AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, zahl,
			LBNA_Column, 1,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, outport[n].name,
			TAG_DONE);
		if (node) AddTail(&outportlist, node);
	}
	SetPageGadgetAttrs(setgad[GAD_OPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		LISTBROWSER_Labels, &outportlist,
		LISTBROWSER_Selected, 0,
		TAG_DONE);

    SetPageGadgetAttrs(setgad[GAD_OPLATENZ], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, outport[0].latenz,
		TAG_DONE);

}

void AktualisiereInPortListe(void) {
	struct Node *node;
	BYTE n;
	
	if (inportlist.lh_Head) {
		SetPageGadgetAttrs(setgad[GAD_IPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			LISTBROWSER_Labels, NULL,
			TAG_DONE);
		while (node = RemTail(&inportlist)) FreeListBrowserNode(node);
	}

	for (n = 0; n < verINPORTS; n++) {
		if (!inport[n].name[0]) break;
		node = AllocListBrowserNode(1,
                LBNCA_CopyText, TRUE,
				LBNCA_Text, inport[n].name,
				TAG_DONE);
		if (node) AddTail(&inportlist, node);
	}
	SetPageGadgetAttrs(setgad[GAD_IPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		LISTBROWSER_Labels, &inportlist,
		LISTBROWSER_Selected, 0,
		TAG_DONE);
}

void OPHinzu(STRPTR name) {
	BYTE n;
	
	for (n = 0;  n < verOUTPORTS; n++) {
		if (!outport[n].name[0]) break;
	}
	if (n < verOUTPORTS) {
		strncpy(outport[n].name, name, 128);
		InitOutPortInstr(n);
	}
}

void IPHinzu(STRPTR name) {
	BYTE n;
	
	for (n = 0; n < verINPORTS; n++) {
		if (!inport[n].name[0]) break;
	}
	if (n < verINPORTS) strncpy(inport[n].name, name, 128);
}

void OPEntfernen(BYTE p) {
	BYTE n;
	
	for (n = p; n < verOUTPORTS - 1; n++) memcpy(&outport[n], &outport[n + 1], sizeof(struct OUTPORT));
	outport[verOUTPORTS - 1].name[0]=0;
}

void IPEntfernen(BYTE p) {
	BYTE n;
	
	for (n = p; n < verINPORTS - 1; n++) memcpy(&inport[n], &inport[n + 1], sizeof(struct INPORT));
	inport[verINPORTS - 1].name[0] = 0;
}

void PortText(void) {
	sprintf(porttext, CAT(MSG_0017, "%s: Channel %d"), outport[metro.port].name, metro.channel + 1);
}

void AktualisiereInstrGadgets(void) {
	WORD n;
	ULONG port;

	GetAttr(LISTBROWSER_Selected, setgad[GAD_OPLIST], &port);
	if (port >= 0) {
		n = SucheInstrumentNum(outport[port].outinstr[0].name);
		if (n >= 0) {
			SetPageGadgetAttrs(setgad[GAD_OUTINSTR1], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, FALSE,
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		n = SucheInstrumentNum(outport[port].outinstr[1].name);
		if (n >= 0) {
			SetPageGadgetAttrs(setgad[GAD_OUTINSTR2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, (outport[port].outinstr[1].unten == 16),
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		SetPageGadgetAttrs(setgad[GAD_OUTINSTRCH2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			INTEGER_Number, outport[port].outinstr[1].unten + 1,
			TAG_DONE);
		n = SucheInstrumentNum(outport[port].outinstr[2].name);
		if (n >= 0) {
			SetPageGadgetAttrs(setgad[GAD_OUTINSTR3], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, (outport[port].outinstr[2].unten == 16),
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		SetPageGadgetAttrs(setgad[GAD_OUTINSTRCH3], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			INTEGER_Number, outport[port].outinstr[2].unten + 1,
			TAG_DONE);
		n = SucheInstrumentNum(outport[port].outinstr[3].name);
		if (n >= 0) {
			SetPageGadgetAttrs(setgad[GAD_OUTINSTR4], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, (outport[port].outinstr[3].unten == 16),
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		SetPageGadgetAttrs(setgad[GAD_OUTINSTRCH4], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			INTEGER_Number, outport[port].outinstr[3].unten + 1,
			TAG_DONE);
	}
}

void AktualisiereMetronomGadgets(void) {
	PortText();
	SetPageGadgetAttrs(setgad[GAD_MKANAL], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		GA_Text, porttext,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_MRASTER], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Selected, metro.raster - VIERTEL + 1,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_MTASTE1], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.taste1,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_MVELO1], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.velo1,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_MTASTE2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.taste2,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_MVELO2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.velo2,
		TAG_DONE);
}

void AktualisiereSmpteGadgets(void) {
	BYTE fmax[] = {23, 24, 29, 29};
	
	SetPageGadgetAttrs(setgad[GAD_SHH], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, Ticks2hh(smpte.startticks),
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_SMM], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, Ticks2mm(smpte.startticks),
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_SSS], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, Ticks2ss(smpte.startticks),
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_SFF], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Maximum, fmax[smpte.format],
		INTEGER_Number, Ticks2ff(smpte.startticks),
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_SFORMAT], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		RADIOBUTTON_Selected, smpte.format,
		TAG_DONE);
}

void AktualisiereClusterListe(void) {
	struct Node *node;
	APTR camdlock;
	STRPTR name;
	struct MidiCluster *cluster;
	
	if (clusterlist.lh_Head) {
		SetPageGadgetAttrs(setgad[GAD_OPANDERS], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			CHOOSER_Labels, NULL,
			TAG_DONE);
		SetPageGadgetAttrs(setgad[GAD_OPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			CHOOSER_Labels, NULL,
			TAG_DONE);
		SetPageGadgetAttrs(setgad[GAD_IPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			CHOOSER_Labels, NULL,
			TAG_DONE);
		while (node = RemTail(&clusterlist)) FreeChooserNode(node);
	}

	cluster = NULL;
	camdlock = LockCAMD(CD_Linkages);
	name = NULL;
	while (cluster = NextCluster(cluster)) {
		name = cluster->mcl_Node.ln_Name;
		node = AllocChooserNode(CNA_Text, name, TAG_DONE);
		if (node) AddTail(&clusterlist, node);
	}
	UnlockCAMD(camdlock);
	if (!name) {
		node = AllocChooserNode(CNA_Text, CAT(MSG_0016, "<no ports available>"), CNA_ReadOnly, TRUE, TAG_DONE);
		if (node) AddTail(&clusterlist, node);
	}

	SetPageGadgetAttrs(setgad[GAD_OPANDERS], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Labels, &clusterlist,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_OPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Labels, &clusterlist,
		TAG_DONE);
	SetPageGadgetAttrs(setgad[GAD_IPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Labels, &clusterlist,
		TAG_DONE);
}


void ErstelleSeiten(void) {
	struct INSTRUMENT *instr;
	struct Node *node;

	//MidiPorts Seite

	NewList(&outportlist);
	NewList(&inportlist);
	NewList(&clusterlist);
	
	NewList(&setinstrlist);
	instr = rootinstrument;
	while (instr) {
		node = AllocChooserNode(CNA_Text, instr->name, TAG_DONE);
		if (node) AddTail(&setinstrlist, node);
		instr = instr->next;
	}

	seitemidiports = VLayoutObject,
		LAYOUT_SpaceOuter, TRUE,

		LAYOUT_AddChild, HLayoutObject,
				
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0018, "Outputs"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
					
				LAYOUT_AddChild, setgad[GAD_OPLIST] = ListBrowserObject,
					GA_ID, GAD_OPLIST, GA_RelVerify, TRUE,
					LISTBROWSER_Labels, NULL,
					LISTBROWSER_ColumnInfo, opcolinfo,
					LISTBROWSER_ColumnTitles, TRUE,
					LISTBROWSER_AutoFit, TRUE,
					LISTBROWSER_ShowSelected, TRUE,
				End,
										
				LAYOUT_AddChild, setgad[GAD_OPANDERS] = ChooserObject,
					GA_ID, GAD_OPANDERS, GA_RelVerify, TRUE,
					CHOOSER_Title, CAT(MSG_0019, "Change Active"),
					CHOOSER_DropDown, TRUE,
					CHOOSER_Labels, NULL,
					CHOOSER_MaxLabels, 64,
				End,				
					
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, setgad[GAD_OPHINZU] = ChooserObject,
						GA_ID, GAD_OPHINZU, GA_RelVerify, TRUE,
						GA_Disabled, verLITE,
						CHOOSER_Title, CAT(MSG_0020, "Add"),
						CHOOSER_DropDown, TRUE,
						CHOOSER_Labels, NULL,
						CHOOSER_AutoFit, TRUE,
						CHOOSER_MaxLabels, 64,
					End,				
					LAYOUT_AddChild, ButtonObject,
						GA_ID, GAD_OPENTF, GA_RelVerify, TRUE,
						GA_Text, CAT(MSG_0021, "Remove"),
						GA_Disabled, verLITE,
					End,
				End,
				CHILD_WeightedHeight, 0,

				LAYOUT_AddChild, setgad[GAD_OPLATENZ] = IntegerObject,
					GA_ID, GAD_OPLATENZ, GA_RelVerify, TRUE,
					INTEGER_Minimum, 0,
					INTEGER_Maximum, 1000,
				End,
				Label("Port Latency (ms)"),

			End,

			LAYOUT_AddChild, VLayoutObject,

				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_Label, CAT(MSG_0022, "Output Instruments"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

					LAYOUT_AddChild, setgad[GAD_OUTINSTR1] = ChooserObject,
						GA_ID, GAD_OUTINSTR1, GA_RelVerify, TRUE, GA_Disabled, TRUE,
						CHOOSER_PopUp, TRUE,
						CHOOSER_Labels, &setinstrlist,
						CHOOSER_MaxLabels, 24,
					End,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_OUTINSTR2] = ChooserObject,
							GA_ID, GAD_OUTINSTR2, GA_RelVerify, TRUE, GA_Disabled, TRUE,
							CHOOSER_PopUp, TRUE,
							CHOOSER_Labels, &setinstrlist,
							CHOOSER_MaxLabels, 24,
						End,
						LAYOUT_AddChild, setgad[GAD_OUTINSTRCH2] = IntegerObject,
							GA_ID, GAD_OUTINSTRCH2, GA_RelVerify, TRUE,
							INTEGER_Minimum, 1,
							INTEGER_Maximum, 17,
							INTEGER_MinVisible, 3,
						End,
						CHILD_WeightedWidth, 0,
					End,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_OUTINSTR3] = ChooserObject,
							GA_ID, GAD_OUTINSTR3, GA_RelVerify, TRUE, GA_Disabled, TRUE,
							CHOOSER_PopUp, TRUE,
							CHOOSER_Labels, &setinstrlist,
							CHOOSER_MaxLabels, 24,
						End,
						LAYOUT_AddChild, setgad[GAD_OUTINSTRCH3] = IntegerObject,
							GA_ID, GAD_OUTINSTRCH3, GA_RelVerify, TRUE, GA_Width, 40,
							INTEGER_Minimum, 1,
							INTEGER_Maximum, 17,
							INTEGER_MinVisible, 3,
						End,
						CHILD_WeightedWidth, 0,
					End,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_OUTINSTR4] = ChooserObject,
							GA_ID, GAD_OUTINSTR4, GA_RelVerify, TRUE, GA_Disabled, TRUE,
							CHOOSER_PopUp, TRUE,
							CHOOSER_Labels, &setinstrlist,
							CHOOSER_MaxLabels, 24,
						End,
						LAYOUT_AddChild, setgad[GAD_OUTINSTRCH4] = IntegerObject,
							GA_ID, GAD_OUTINSTRCH4, GA_RelVerify, TRUE, GA_Width, 40,
							INTEGER_Minimum, 1,
							INTEGER_Maximum, 17,
							INTEGER_MinVisible, 3,
						End,
						CHILD_WeightedWidth, 0,
					End,

				End,
				CHILD_WeightedHeight, 0,
		
		
				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_Label, CAT(MSG_0023, "Inputs"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
						
					LAYOUT_AddChild, setgad[GAD_IPLIST] = ListBrowserObject,
						GA_ID, GAD_IPLIST, GA_RelVerify, TRUE,
						LISTBROWSER_Labels, NULL,
						LISTBROWSER_AutoFit, TRUE,
						LISTBROWSER_ShowSelected, TRUE,
					End,
		
					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_IPHINZU] = ChooserObject,
							GA_ID, GAD_IPHINZU, GA_RelVerify, TRUE,
							CHOOSER_Title, CAT(MSG_0020, "Add"),
							CHOOSER_DropDown, TRUE,
							CHOOSER_Labels, NULL,
							CHOOSER_AutoFit, TRUE,
						End,				
						LAYOUT_AddChild, ButtonObject,
							GA_ID, GAD_IPENTF, GA_RelVerify, TRUE,
							GA_Text, CAT(MSG_0021, "Remove"),
						End,
					End,
					CHILD_WeightedHeight, 0,
				End,
			End,
		End,


	    #ifdef __amigaos4__
		LAYOUT_AddChild, HLayoutObject,
			LAYOUT_Label, "Phonolith", LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

			LAYOUT_AddChild, setgad[GAD_PHONOLITHPROJ] = GetFileObject,
				GA_ID, GAD_PHONOLITHPROJ, GA_RelVerify, TRUE,
				GETFILE_Pattern, "#?.pproj",
			End,
			Label("Project File"),
		End,
		CHILD_WeightedHeight, 0,
		#endif

	End;
	
	
	//Metronom Seite
	NewList(&rasterlist);
	node = AllocChooserNode(CNA_Text, CAT(MSG_0026, "1/8 Notes"), TAG_DONE);
	if (node) AddTail(&rasterlist, node);
	node = AllocChooserNode(CNA_Text, CAT(MSG_0027, "1/4 Notes"), TAG_DONE);
	if (node) AddTail(&rasterlist, node);
	node = AllocChooserNode(CNA_Text, CAT(MSG_0028, "1/2 Notes"), TAG_DONE);
	if (node) AddTail(&rasterlist, node);
	node = AllocChooserNode(CNA_Text, CAT(MSG_0029, "Bars"), TAG_DONE);
	if (node) AddTail(&rasterlist, node);
	
	seitemetronom = VLayoutObject,
		LAYOUT_SpaceOuter, TRUE,
		
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_AddChild, setgad[GAD_MKANAL] = ButtonObject,
				GA_ID, GAD_MKANAL, GA_RelVerify, TRUE,
				BUTTON_Justification, BCJ_LEFT,
			End,
			Label(CAT(MSG_0030, "Channel")),
			
			LAYOUT_AddChild, setgad[GAD_MRASTER] = ChooserObject,
				GA_ID, GAD_MRASTER, GA_RelVerify, TRUE,
				CHOOSER_PopUp, TRUE,
				CHOOSER_Labels, &rasterlist,
			End,
			Label(CAT(MSG_0031, "Grid")),
	
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0032, "Main Beat"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
	
				LAYOUT_AddChild, setgad[GAD_MTASTE1] = SliderObject,
					GA_ID, GAD_MTASTE1, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
#ifdef __amigaos4__
					SLIDER_LevelFormat, "%ld",
#endif
				End,
				Label(CAT(MSG_0033, "Key")),
	
				LAYOUT_AddChild, setgad[GAD_MVELO1] = SliderObject,
					GA_ID, GAD_MVELO1, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
#ifdef __amigaos4__
					SLIDER_LevelFormat, "%ld",
#endif
				End,
				Label(CAT(MSG_0034, "Velocity")),
			End,
	
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0035, "Side Beat"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
	
				LAYOUT_AddChild, setgad[GAD_MTASTE2] = SliderObject,
					GA_ID, GAD_MTASTE2, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
#ifdef __amigaos4__
					SLIDER_LevelFormat, "%ld",
#endif
				End,
				Label(CAT(MSG_0033, "Key")),
	
				LAYOUT_AddChild, setgad[GAD_MVELO2] = SliderObject,
					GA_ID, GAD_MVELO2, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
#ifdef __amigaos4__
					SLIDER_LevelFormat, "%ld",
#endif
				End,
				Label(CAT(MSG_0034, "Velocity")),
			End,
		End,
		CHILD_WeightedHeight, 0,
	End;

	// SMPTE Seite
	NewList(&fpslist);
	node = AllocRadioButtonNode(1, RBNA_Labels, "24 fps - Film", TAG_DONE);
	if (node) AddTail(&fpslist, node);
	node = AllocRadioButtonNode(1, RBNA_Labels, "25 fps - TV PAL (EBU)", TAG_DONE);
	if (node) AddTail(&fpslist, node);
	node = AllocRadioButtonNode(1, RBNA_Labels, "30 fps - TV NTSC (SMPTE)", TAG_DONE);
	if (node) AddTail(&fpslist, node);

	seitesmpte = VLayoutObject,
		LAYOUT_SpaceOuter, TRUE,
		
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0041, "Start Time   HH:MM:SS:FF"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
				
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, setgad[GAD_SHH] = IntegerObject,
						GA_ID, GAD_SHH, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_Maximum, 99, INTEGER_MinVisible, 3,
					End,
	
					LAYOUT_AddChild, setgad[GAD_SMM] = IntegerObject,
						GA_ID, GAD_SMM, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_Maximum, 59, INTEGER_MinVisible, 3,
					End,
		
					LAYOUT_AddChild, setgad[GAD_SSS] = IntegerObject,
						GA_ID, GAD_SSS, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_Maximum, 59, INTEGER_MinVisible, 3,
					End,
		
					LAYOUT_AddChild, setgad[GAD_SFF] = IntegerObject,
						GA_ID, GAD_SFF, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_MinVisible, 3,
					End,
				End,
				
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, LayoutObject, End,
					LAYOUT_AddChild, Button(CAT(MSG_0042, "Delete"), GAD_SLOESCHEN), CHILD_WeightedWidth, 0,
					LAYOUT_AddChild, Button(CAT(MSG_0043, "Accept"), GAD_SUEBERNEHMEN), CHILD_WeightedWidth, 0,
				End,
			End,
			
			LAYOUT_AddChild, setgad[GAD_SFORMAT] = RadioButtonObject,
				GA_ID, GAD_SFORMAT, GA_RelVerify, TRUE,
				RADIOBUTTON_Labels, &fpslist,
			End,
		End,
		CHILD_WeightedHeight, 0,
	End;

}

void EntferneListen(void) {
	struct Node *node;
	
	while (node = RemTail(&setinstrlist)) FreeChooserNode(node);
	while (node = RemTail(&setctlist)) FreeClickTabNode(node);
	while (node = RemTail(&clusterlist)) FreeChooserNode(node);
	while (node = RemTail(&outportlist)) FreeListBrowserNode(node);
	while (node = RemTail(&inportlist)) FreeListBrowserNode(node);
	while (node = RemTail(&rasterlist)) FreeChooserNode(node);
	while (node = RemTail(&fpslist)) FreeRadioButtonNode(node);
}


void ErstelleEinstellungsfenster(void) {
	if (!setfensterobj) {
		ErstelleSeiten();

		NewList(&setctlist);
		AddTail(&setctlist, AllocClickTabNode(TNA_Text, CAT(MSG_0044, "Midi Ports"), TNA_Number, 0, TAG_DONE));
		AddTail(&setctlist, AllocClickTabNode(TNA_Text, CAT(MSG_0045, "Metronome"), TNA_Number, 1, TAG_DONE));
		AddTail(&setctlist, AllocClickTabNode(TNA_Text, CAT(MSG_0046, "SMPTE"), TNA_Number, 2, TAG_DONE));
	
		setfensterobj=WindowObject,
			WA_PubScreen, hschirm,
			WA_Title, CAT(MSG_0047, "Project Settings"),
			WA_Activate, TRUE,
			WA_DepthGadget, TRUE,
			WA_DragBar, TRUE,
			WA_CloseGadget, TRUE,
			WA_SizeGadget, TRUE,
			WA_SizeBBottom, TRUE,
			WA_IDCMP, IDCMP_MENUPICK,
			WINDOW_ParentGroup, VLayoutObject,
				LAYOUT_SpaceOuter, TRUE,
				LAYOUT_AddChild, ClickTabObject,
					GA_ID, GAD_CLICKTAB, GA_RelVerify, TRUE,
					CLICKTAB_Labels, &setctlist,
					CLICKTAB_PageGroup, setgad[GAD_PAGE] = PageObject,
						PAGE_Add, seitemidiports,
						PAGE_Add, seitemetronom,
						PAGE_Add, seitesmpte,
						PAGE_Current, 0,
					End,
				End,
			End,
		End;
	}	

	if (setfensterobj) {
		if (fenp[SET].b > 0) {
			SetAttrs(setfensterobj,
				WA_Left, fenp[SET].x, WA_Top, fenp[SET].y,
				WA_InnerWidth, fenp[SET].b, WA_InnerHeight, fenp[SET].h,
				TAG_DONE);
		}
#if __amigaos4__
		SetAttrs(setgad[GAD_PHONOLITHPROJ],
				GETFILE_FullFile, lied.phonolithprojekt,
			    TAG_DONE);
#endif

		setfenster = (struct Window *)RA_OpenWindow(setfensterobj);
		SetMenuStrip(setfenster, minmenu);

		AktualisiereClusterListe();
		AktualisiereOutPortListe();
		AktualisiereInPortListe();
		AktualisiereInstrGadgets();
		AktualisiereMetronomGadgets();
		AktualisiereSmpteGadgets();

	}
}

void EntferneEinstellungsfenster(void) {
	if (setfensterobj) {
		if (setfenster) {
			HoleFensterObjpos(setfensterobj, SET);
			ClearMenuStrip(setfenster);
		}
		DisposeObject(setfensterobj);
		setfensterobj = NULL;
		setfenster = NULL;
	
		EntferneListen();
	}
}

void KontrolleEinstellungsfenster(void) {
	BOOL schliessen = FALSE;
	ULONG result;
	UWORD code;
	WORD n;
	struct Node *node;
	STRPTR text;
	LONG var, var2, var3, var4;
	struct INSTRUMENT *instr;
	
	while ((result = RA_HandleInput(setfensterobj, &code)) != WMHI_LASTMSG) {
		switch (result & WMHI_CLASSMASK) {
			case WMHI_CLOSEWINDOW:
			schliessen = TRUE;
			break;

			case WMHI_MENUPICK:
			MinMenuKontrolle(MinMenuPunkt(result & WMHI_MENUMASK));
			break;

			case WMHI_GADGETUP:
			switch (result & WMHI_GADGETMASK) {
				
				// MidiPorts Seite
				
				case GAD_OPLIST:
				AktualisiereInstrGadgets();
				SetGadgetAttrs(setgad[GAD_OPLATENZ], setfenster, NULL, INTEGER_Number, outport[code].latenz, TAG_DONE);
				break;
				
				case GAD_OPHINZU:
				node = clusterlist.lh_Head;
				for (n = 0; n < code; n++) {
					node = node->ln_Succ;
				}
				GetChooserNodeAttrs(node, CNA_Text, &text, TAG_DONE);
				OPHinzu(text); AktualisiereOutPortListe();
				break;

				case GAD_IPHINZU:
				node = clusterlist.lh_Head;
				for (n = 0; n < code; n++) {
					node = node->ln_Succ;
				}
				GetChooserNodeAttrs(node, CNA_Text, &text, TAG_DONE);
				IPHinzu(text); AktualisiereInPortListe();
				break;

				case GAD_OPANDERS:
				node = clusterlist.lh_Head;
				for (n = 0; n < code; n++) {
					node = node->ln_Succ;
				}
				GetChooserNodeAttrs(node, CNA_Text, &text, TAG_DONE);
				GetAttr(LISTBROWSER_Selected, setgad[GAD_OPLIST], (ULONG *)&var);
				strncpy(outport[var].name, text, 128); AktualisiereOutPortListe();
				break;
				
				case GAD_OPENTF:
				GetAttr(LISTBROWSER_Selected, setgad[GAD_OPLIST], (ULONG *)&var);
				if (var >= 0) {
					OPEntfernen((BYTE)var); AktualisiereOutPortListe();
				}
				break;

			    case GAD_OPLATENZ:
				GetAttr(LISTBROWSER_Selected, setgad[GAD_OPLIST], (ULONG *)&var);
				if (var >= 0) {
					outport[var].latenz = code;
				}
				break;

				case GAD_OUTINSTR1:
				case GAD_OUTINSTR2:
				case GAD_OUTINSTR3:
				case GAD_OUTINSTR4:
				GetAttr(LISTBROWSER_Selected, setgad[GAD_OPLIST], (ULONG *)&var);
				if (var >= 0) {
					instr = NtesInstrument(code);
					strncpy(outport[var].outinstr[(result & WMHI_GADGETMASK) - GAD_OUTINSTR1].name, instr->name, 128);
				}
				break;

				case GAD_OUTINSTRCH2:
				case GAD_OUTINSTRCH3:
				case GAD_OUTINSTRCH4:
				GetAttr(LISTBROWSER_Selected, setgad[GAD_OPLIST], (ULONG *)&var);
				if (var >= 0) {
					n = (result & WMHI_GADGETMASK) - GAD_OUTINSTRCH2 + 1;
					outport[var].outinstr[n].unten = code - 1;
					SetGadgetAttrs(setgad[n + GAD_OUTINSTR1], setfenster, NULL, GA_Disabled, (outport[var].outinstr[n].unten == 16), TAG_DONE);
				}
				break;

				case GAD_IPENTF:
				GetAttr(LISTBROWSER_Selected, setgad[GAD_IPLIST], (ULONG *)&var);
				if (var >= 0) {
					IPEntfernen((BYTE)var); AktualisiereInPortListe();
				}
				break;
				
				#ifdef __amigaos4__
				case GAD_PHONOLITHPROJ:
				gfRequestFile((Object *)setgad[GAD_PHONOLITHPROJ], setfenster);
				break;
				#endif

				// Metronom Seite
				
				case GAD_MKANAL:
				ChannelPortFenster(hfenster, &metro.channel, &metro.port);
				PortText();
				SetGadgetAttrs(setgad[GAD_MKANAL], setfenster, NULL, GA_Text, porttext, TAG_DONE);
				break;
				
				case GAD_MTASTE1: metro.taste1 = code; TesteMetronom(1); break;
				case GAD_MTASTE2: metro.taste2 = code; TesteMetronom(2); break;
				case GAD_MVELO1: metro.velo1 = code; TesteMetronom(1); break;
				case GAD_MVELO2: metro.velo2 = code; TesteMetronom(2); break;
				case GAD_MRASTER: metro.raster = code + VIERTEL - 1; break;
				
				// Smpte Seite
				
				case GAD_SLOESCHEN:
				smpte.startticks = 0;
				SmpteTicksAktualisieren();
				AktualisiereSmpteGadgets();
				ZeichneSmpteLeiste();
				break;
				
				case GAD_SUEBERNEHMEN:
				GetAttr(INTEGER_Number, setgad[GAD_SHH], (ULONG *)&var);
				GetAttr(INTEGER_Number, setgad[GAD_SMM], (ULONG *)&var2);
				GetAttr(INTEGER_Number, setgad[GAD_SSS], (ULONG *)&var3);
				GetAttr(INTEGER_Number, setgad[GAD_SFF], (ULONG *)&var4);
				smpte.startticks = Smpte2Ticks((BYTE)var, (BYTE)var2, (BYTE)var3, (BYTE)var4);
				SmpteTicksAktualisieren();
				ZeichneSmpteLeiste();
				break;
				
				case GAD_SFORMAT:
				smpte.format = code;
				ZeichneSmpteLeiste();
				AktualisiereSmpteGadgets();
				break;
			}
			break;
		}
	}
	if (schliessen) {
		#ifdef __amigaos4__
		STRPTR str = NULL;
		GetAttr(GETFILE_FullFile, (Object *)setgad[GAD_PHONOLITHPROJ], (ULONG *)&str);
		strncpy(lied.phonolithprojekt, str, 1024);
		#endif

		ErneuereLinks();
		HoleFensterObjpos(setfensterobj, SET);
		ClearMenuStrip(setfenster);
		RA_CloseWindow(setfensterobj);
		setfenster = NULL;
		AktualisierePortChooserListe();
	}
}
