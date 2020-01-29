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

extern struct CamdIFace *ICamd;

void AktualisiereOutPortListe(void) {
	struct Node *node;
	int8 n;
	char zahl[4];
	
	if (outportlist.lh_Head) {
		ILayout->SetPageGadgetAttrs(setgad[GAD_OPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			LISTBROWSER_Labels, NULL,
			TAG_DONE);
		while ((node = IExec->RemTail(&outportlist))) { 
			IListBrowser->FreeListBrowserNode(node);
		}
	}

	for (n = 0; n < verOUTPORTS; n++) {
		if (!outport[n].name[0]) break;
		sprintf(zahl, "%d", n + 1);
		node = IListBrowser->AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, zahl,
			LBNA_Column, 1,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, outport[n].name,
			TAG_DONE);
		if (node) IExec->AddTail(&outportlist, node);
	}
	ILayout->SetPageGadgetAttrs(setgad[GAD_OPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		LISTBROWSER_Labels, &outportlist,
		LISTBROWSER_Selected, 0,
		TAG_DONE);

    ILayout->SetPageGadgetAttrs(setgad[GAD_OPLATENZ], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, outport[0].latenz,
		TAG_DONE);

}

void AktualisiereInPortListe(void) {
	struct Node *node;
	int8 n;
	
	if (inportlist.lh_Head) {
		ILayout->SetPageGadgetAttrs(setgad[GAD_IPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			LISTBROWSER_Labels, NULL,
			TAG_DONE);
		while ((node = IExec->RemTail(&inportlist))) { 
			IListBrowser->FreeListBrowserNode(node);
		}
	}

	for (n = 0; n < verINPORTS; n++) {
		if (!inport[n].name[0]) break;
		node = IListBrowser->AllocListBrowserNode(1,
                LBNCA_CopyText, TRUE,
				LBNCA_Text, inport[n].name,
				TAG_DONE);
		if (node) IExec->AddTail(&inportlist, node);
	}
	ILayout->SetPageGadgetAttrs(setgad[GAD_IPLIST], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		LISTBROWSER_Labels, &inportlist,
		LISTBROWSER_Selected, 0,
		TAG_DONE);
}

void OPHinzu(STRPTR name) {
	int8 n;
	
	for (n = 0;  n < verOUTPORTS; n++) {
		if (!outport[n].name[0]) break;
	}
	if (n < verOUTPORTS) {
		strncpy(outport[n].name, name, 128);
		InitOutPortInstr(n);
	}
}

void IPHinzu(STRPTR name) {
	int8 n;
	
	for (n = 0; n < verINPORTS; n++) {
		if (!inport[n].name[0]) break;
	}
	if (n < verINPORTS) strncpy(inport[n].name, name, 128);
}

void OPEntfernen(int8 p) {
	int8 n;
	
	for (n = p; n < verOUTPORTS - 1; n++) memcpy(&outport[n], &outport[n + 1], sizeof(struct OUTPORT));
	outport[verOUTPORTS - 1].name[0]=0;
}

void IPEntfernen(int8 p) {
	int8 n;
	
	for (n = p; n < verINPORTS - 1; n++) memcpy(&inport[n], &inport[n + 1], sizeof(struct INPORT));
	inport[verINPORTS - 1].name[0] = 0;
}

void PortText(void) {
	sprintf(porttext, CAT(MSG_0017, "%s: Channel %d"), outport[metro.port].name, metro.channel + 1);
}

void AktualisiereInstrGadgets(void) {
	int16 n;
	uint32 port;

	IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_OPLIST], &port);
	if (port >= 0) {
		n = SucheInstrumentNum(outport[port].outinstr[0].name);
		if (n >= 0) {
			ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTR1], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, FALSE,
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		n = SucheInstrumentNum(outport[port].outinstr[1].name);
		if (n >= 0) {
			ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTR2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, (outport[port].outinstr[1].unten == 16),
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTRCH2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			INTEGER_Number, outport[port].outinstr[1].unten + 1,
			TAG_DONE);
		n = SucheInstrumentNum(outport[port].outinstr[2].name);
		if (n >= 0) {
			ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTR3], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, (outport[port].outinstr[2].unten == 16),
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTRCH3], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			INTEGER_Number, outport[port].outinstr[2].unten + 1,
			TAG_DONE);
		n = SucheInstrumentNum(outport[port].outinstr[3].name);
		if (n >= 0) {
			ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTR4], (Object *)setgad[GAD_PAGE], setfenster, NULL,
				GA_Disabled, (outport[port].outinstr[3].unten == 16),
				CHOOSER_Selected, n,
				TAG_DONE);
		}
		ILayout->SetPageGadgetAttrs(setgad[GAD_OUTINSTRCH4], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			INTEGER_Number, outport[port].outinstr[3].unten + 1,
			TAG_DONE);
	}
}

void AktualisiereMetronomGadgets(void) {
	PortText();
	ILayout->SetPageGadgetAttrs(setgad[GAD_MKANAL], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		GA_Text, porttext,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_MRASTER], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Selected, metro.raster - VIERTEL + 1,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_MTASTE1], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.taste1,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_MVELO1], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.velo1,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_MTASTE2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.taste2,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_MVELO2], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		SLIDER_Level, metro.velo2,
		TAG_DONE);
}

void AktualisiereSmpteGadgets(void) {
	int8 fmax[] = {23, 24, 29, 29};
	
	ILayout->SetPageGadgetAttrs(setgad[GAD_SHH], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, Ticks2hh(smpte.startticks),
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_SMM], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, Ticks2mm(smpte.startticks),
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_SSS], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Number, Ticks2ss(smpte.startticks),
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_SFF], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		INTEGER_Maximum, fmax[smpte.format],
		INTEGER_Number, Ticks2ff(smpte.startticks),
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_SFORMAT], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		RADIOBUTTON_Selected, smpte.format,
		TAG_DONE);
}

void AktualisiereClusterListe(void) {
	struct Node *node;
	APTR camdlock;
	STRPTR name;
	struct MidiCluster *cluster;
	
	if (clusterlist.lh_Head) {
		ILayout->SetPageGadgetAttrs(setgad[GAD_OPANDERS], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			CHOOSER_Labels, NULL,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(setgad[GAD_OPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			CHOOSER_Labels, NULL,
			TAG_DONE);
		ILayout->SetPageGadgetAttrs(setgad[GAD_IPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
			CHOOSER_Labels, NULL,
			TAG_DONE);
		while ((node = IExec->RemTail(&clusterlist))) {
			IChooser->FreeChooserNode(node);
		}
	}

	cluster = NULL;
	camdlock = ICamd->LockCAMD(CD_Linkages);
	name = NULL;
	while ((cluster = ICamd->NextCluster(cluster))) {
		name = cluster->mcl_Node.ln_Name;
		node = IChooser->AllocChooserNode(CNA_Text, name, TAG_DONE);
		if (node) IExec->AddTail(&clusterlist, node);
	}
	ICamd->UnlockCAMD(camdlock);
	if (!name) {
		node = IChooser->AllocChooserNode(CNA_Text, CAT(MSG_0016, "<no ports available>"), CNA_ReadOnly, TRUE, TAG_DONE);
		if (node) IExec->AddTail(&clusterlist, node);
	}

	ILayout->SetPageGadgetAttrs(setgad[GAD_OPANDERS], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Labels, &clusterlist,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_OPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Labels, &clusterlist,
		TAG_DONE);
	ILayout->SetPageGadgetAttrs(setgad[GAD_IPHINZU], (Object *)setgad[GAD_PAGE], setfenster, NULL,
		CHOOSER_Labels, &clusterlist,
		TAG_DONE);
}


void ErstelleSeiten(void) {
	struct INSTRUMENT *instr;
	struct Node *node;

	//MidiPorts Seite

	IExec->NewList(&outportlist);
	IExec->NewList(&inportlist);
	IExec->NewList(&clusterlist);
	
	IExec->NewList(&setinstrlist);
	instr = rootinstrument;
	while (instr) {
		node = IChooser->AllocChooserNode(CNA_Text, instr->name, TAG_DONE);
		if (node) IExec->AddTail(&setinstrlist, node);
		instr = instr->next;
	}

	seitemidiports = VLayoutObject,
		LAYOUT_SpaceOuter, TRUE,

		LAYOUT_AddChild, HLayoutObject,
				
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0018, "Outputs"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
					
				LAYOUT_AddChild, setgad[GAD_OPLIST] = (struct Gadget *)ListBrowserObject,
					GA_ID, GAD_OPLIST, GA_RelVerify, TRUE,
					LISTBROWSER_Labels, NULL,
					LISTBROWSER_ColumnInfo, opcolinfo,
					LISTBROWSER_ColumnTitles, TRUE,
					LISTBROWSER_AutoFit, TRUE,
					LISTBROWSER_ShowSelected, TRUE,
				End,
										
				LAYOUT_AddChild, setgad[GAD_OPANDERS] = (struct Gadget *)ChooserObject,
					GA_ID, GAD_OPANDERS, GA_RelVerify, TRUE,
					CHOOSER_Title, CAT(MSG_0019, "Change Active"),
					CHOOSER_DropDown, TRUE,
					CHOOSER_Labels, NULL,
					CHOOSER_MaxLabels, 64,
				End,				
					
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, setgad[GAD_OPHINZU] = (struct Gadget *)ChooserObject,
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

				LAYOUT_AddChild, setgad[GAD_OPLATENZ] = (struct Gadget *)IntegerObject,
					GA_ID, GAD_OPLATENZ, GA_RelVerify, TRUE,
					INTEGER_Minimum, 0,
					INTEGER_Maximum, 1000,
				End,
				Label("Port Latency (ms)"),

			End,

			LAYOUT_AddChild, VLayoutObject,

				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_Label, CAT(MSG_0022, "Output Instruments"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

					LAYOUT_AddChild, setgad[GAD_OUTINSTR1] = (struct Gadget *)ChooserObject,
						GA_ID, GAD_OUTINSTR1, GA_RelVerify, TRUE, GA_Disabled, TRUE,
						CHOOSER_PopUp, TRUE,
						CHOOSER_Labels, &setinstrlist,
						CHOOSER_MaxLabels, 24,
					End,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_OUTINSTR2] = (struct Gadget *)ChooserObject,
							GA_ID, GAD_OUTINSTR2, GA_RelVerify, TRUE, GA_Disabled, TRUE,
							CHOOSER_PopUp, TRUE,
							CHOOSER_Labels, &setinstrlist,
							CHOOSER_MaxLabels, 24,
						End,
						LAYOUT_AddChild, setgad[GAD_OUTINSTRCH2] = (struct Gadget *)IntegerObject,
							GA_ID, GAD_OUTINSTRCH2, GA_RelVerify, TRUE,
							INTEGER_Minimum, 1,
							INTEGER_Maximum, 17,
							INTEGER_MinVisible, 3,
						End,
						CHILD_WeightedWidth, 0,
					End,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_OUTINSTR3] = (struct Gadget *)ChooserObject,
							GA_ID, GAD_OUTINSTR3, GA_RelVerify, TRUE, GA_Disabled, TRUE,
							CHOOSER_PopUp, TRUE,
							CHOOSER_Labels, &setinstrlist,
							CHOOSER_MaxLabels, 24,
						End,
						LAYOUT_AddChild, setgad[GAD_OUTINSTRCH3] = (struct Gadget *)IntegerObject,
							GA_ID, GAD_OUTINSTRCH3, GA_RelVerify, TRUE, GA_Width, 40,
							INTEGER_Minimum, 1,
							INTEGER_Maximum, 17,
							INTEGER_MinVisible, 3,
						End,
						CHILD_WeightedWidth, 0,
					End,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_OUTINSTR4] = (struct Gadget *)ChooserObject,
							GA_ID, GAD_OUTINSTR4, GA_RelVerify, TRUE, GA_Disabled, TRUE,
							CHOOSER_PopUp, TRUE,
							CHOOSER_Labels, &setinstrlist,
							CHOOSER_MaxLabels, 24,
						End,
						LAYOUT_AddChild, setgad[GAD_OUTINSTRCH4] = (struct Gadget *)IntegerObject,
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
						
					LAYOUT_AddChild, setgad[GAD_IPLIST] = (struct Gadget *)ListBrowserObject,
						GA_ID, GAD_IPLIST, GA_RelVerify, TRUE,
						LISTBROWSER_Labels, NULL,
						LISTBROWSER_AutoFit, TRUE,
						LISTBROWSER_ShowSelected, TRUE,
					End,
		
					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, setgad[GAD_IPHINZU] = (struct Gadget *)ChooserObject,
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


		LAYOUT_AddChild, HLayoutObject,
			LAYOUT_Label, "Phonolith", LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,

			LAYOUT_AddChild, setgad[GAD_PHONOLITHPROJ] = (struct Gadget *)GetFileObject,
				GA_ID, GAD_PHONOLITHPROJ, GA_RelVerify, TRUE,
				GETFILE_Pattern, "#?.pproj",
			End,
			Label("Project File"),
		End,
		CHILD_WeightedHeight, 0,

	End;
	
	
	//Metronom Seite
	IExec->NewList(&rasterlist);
	node = IChooser->AllocChooserNode(CNA_Text, CAT(MSG_0026, "1/8 Notes"), TAG_DONE);
	if (node) IExec->AddTail(&rasterlist, node);
	node = IChooser->AllocChooserNode(CNA_Text, CAT(MSG_0027, "1/4 Notes"), TAG_DONE);
	if (node) IExec->AddTail(&rasterlist, node);
	node = IChooser->AllocChooserNode(CNA_Text, CAT(MSG_0028, "1/2 Notes"), TAG_DONE);
	if (node) IExec->AddTail(&rasterlist, node);
	node = IChooser->AllocChooserNode(CNA_Text, CAT(MSG_0029, "Bars"), TAG_DONE);
	if (node) IExec->AddTail(&rasterlist, node);
	
	seitemetronom = VLayoutObject,
		LAYOUT_SpaceOuter, TRUE,
		
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_AddChild, setgad[GAD_MKANAL] = (struct Gadget *)ButtonObject,
				GA_ID, GAD_MKANAL, GA_RelVerify, TRUE,
				BUTTON_Justification, BCJ_LEFT,
			End,
			Label(CAT(MSG_0030, "Channel")),
			
			LAYOUT_AddChild, setgad[GAD_MRASTER] = (struct Gadget *)ChooserObject,
				GA_ID, GAD_MRASTER, GA_RelVerify, TRUE,
				CHOOSER_PopUp, TRUE,
				CHOOSER_Labels, &rasterlist,
			End,
			Label(CAT(MSG_0031, "Grid")),
	
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0032, "Main Beat"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
	
				LAYOUT_AddChild, setgad[GAD_MTASTE1] = (struct Gadget *)SliderObject,
					GA_ID, GAD_MTASTE1, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
					SLIDER_LevelFormat, "%ld",
				End,
				Label(CAT(MSG_0033, "Key")),
	
				LAYOUT_AddChild, setgad[GAD_MVELO1] = (struct Gadget *)SliderObject,
					GA_ID, GAD_MVELO1, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
					SLIDER_LevelFormat, "%ld",
				End,
				Label(CAT(MSG_0034, "Velocity")),
			End,
	
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0035, "Side Beat"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
	
				LAYOUT_AddChild, setgad[GAD_MTASTE2] = (struct Gadget *)SliderObject,
					GA_ID, GAD_MTASTE2, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
					SLIDER_LevelFormat, "%ld",
				End,
				Label(CAT(MSG_0033, "Key")),
	
				LAYOUT_AddChild, setgad[GAD_MVELO2] = (struct Gadget *)SliderObject,
					GA_ID, GAD_MVELO2, GA_RelVerify, TRUE, GA_Immediate, TRUE,
					SLIDER_Min, 0, SLIDER_Max, 127,
					SLIDER_Orientation, SORIENT_HORIZ,
					SLIDER_LevelFormat, "%ld",
				End,
				Label(CAT(MSG_0034, "Velocity")),
			End,
		End,
		CHILD_WeightedHeight, 0,
	End;

	// SMPTE Seite
	IExec->NewList(&fpslist);
	node = IRadioButton->AllocRadioButtonNode(1, RBNA_Label, "24 fps - Film", TAG_DONE);
	if (node) IExec->AddTail(&fpslist, node);
	node = IRadioButton->AllocRadioButtonNode(1, RBNA_Label, "25 fps - TV PAL (EBU)", TAG_DONE);
	if (node) IExec->AddTail(&fpslist, node);
	node = IRadioButton->AllocRadioButtonNode(1, RBNA_Label, "30 fps - TV NTSC (SMPTE)", TAG_DONE);
	if (node) IExec->AddTail(&fpslist, node);

	seitesmpte = VLayoutObject,
		LAYOUT_SpaceOuter, TRUE,
		
		LAYOUT_AddChild, VLayoutObject,
			LAYOUT_AddChild, VLayoutObject,
				LAYOUT_Label, CAT(MSG_0041, "Start Time   HH:MM:SS:FF"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
				
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, setgad[GAD_SHH] = (struct Gadget *)IntegerObject,
						GA_ID, GAD_SHH, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_Maximum, 99, INTEGER_MinVisible, 3,
					End,
	
					LAYOUT_AddChild, setgad[GAD_SMM] = (struct Gadget *)IntegerObject,
						GA_ID, GAD_SMM, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_Maximum, 59, INTEGER_MinVisible, 3,
					End,
		
					LAYOUT_AddChild, setgad[GAD_SSS] = (struct Gadget *)IntegerObject,
						GA_ID, GAD_SSS, GA_RelVerify, TRUE,
						GA_TabCycle, TRUE,
						INTEGER_Minimum, 0, INTEGER_Maximum, 59, INTEGER_MinVisible, 3,
					End,
		
					LAYOUT_AddChild, setgad[GAD_SFF] = (struct Gadget *)IntegerObject,
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
			
			LAYOUT_AddChild, setgad[GAD_SFORMAT] = (struct Gadget *)RadioButtonObject,
				GA_ID, GAD_SFORMAT, GA_RelVerify, TRUE,
				RADIOBUTTON_Labels, &fpslist,
			End,
		End,
		CHILD_WeightedHeight, 0,
	End;

}

void EntferneListen(void) {
	struct Node *node;
	
	while ((node = IExec->RemTail(&setinstrlist))) {
		IChooser->FreeChooserNode(node);
	}
	while ((node = IExec->RemTail(&setctlist))) {
		IClickTab->FreeClickTabNode(node);
	}
	while ((node = IExec->RemTail(&clusterlist))) {
		IChooser->FreeChooserNode(node);
	}
	while ((node = IExec->RemTail(&outportlist))) {
		IListBrowser->FreeListBrowserNode(node);
	}
	while ((node = IExec->RemTail(&inportlist))) {
		IListBrowser->FreeListBrowserNode(node);
	}
	while ((node = IExec->RemTail(&rasterlist))) {
		IChooser->FreeChooserNode(node);
	}
	while ((node = IExec->RemTail(&fpslist))) {
		IRadioButton->FreeRadioButtonNode(node);
	}
}


void ErstelleEinstellungsfenster(void) {
	if (!setfensterobj) {
		ErstelleSeiten();

		IExec->NewList(&setctlist);
		IExec->AddTail(&setctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0044, "Midi Ports"), TNA_Number, 0, TAG_DONE));
		IExec->AddTail(&setctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0045, "Metronome"), TNA_Number, 1, TAG_DONE));
		IExec->AddTail(&setctlist, IClickTab->AllocClickTabNode(TNA_Text, CAT(MSG_0046, "SMPTE"), TNA_Number, 2, TAG_DONE));
	
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
					CLICKTAB_PageGroup, setgad[GAD_PAGE] = (struct Gadget *)PageObject,
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
			IIntuition->SetAttrs(setfensterobj,
				WA_Left, fenp[SET].x, WA_Top, fenp[SET].y,
				WA_InnerWidth, fenp[SET].b, WA_InnerHeight, fenp[SET].h,
				TAG_DONE);
		}
#if __amigaos4__
		IIntuition->SetAttrs(setgad[GAD_PHONOLITHPROJ],
				GETFILE_FullFile, lied.phonolithprojekt,
			    TAG_DONE);
#endif

		setfenster = (struct Window *)RA_OpenWindow(setfensterobj);
		IIntuition->SetMenuStrip(setfenster, minmenu);

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
			IIntuition->ClearMenuStrip(setfenster);
		}
		IIntuition->DisposeObject(setfensterobj);
		setfensterobj = NULL;
		setfenster = NULL;
	
		EntferneListen();
	}
}

void KontrolleEinstellungsfenster(void) {
	BOOL schliessen = FALSE;
	uint32 result;
	uint16 code;
	int16 n;
	struct Node *node;
	STRPTR text;
	int32 var, var2, var3, var4;
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
				IIntuition->SetGadgetAttrs(setgad[GAD_OPLATENZ], setfenster, NULL, INTEGER_Number, outport[code].latenz, TAG_DONE);
				break;
				
				case GAD_OPHINZU:
				node = clusterlist.lh_Head;
				for (n = 0; n < code; n++) {
					node = node->ln_Succ;
				}
				IChooser->GetChooserNodeAttrs(node, CNA_Text, &text, TAG_DONE);
				OPHinzu(text); AktualisiereOutPortListe();
				break;

				case GAD_IPHINZU:
				node = clusterlist.lh_Head;
				for (n = 0; n < code; n++) {
					node = node->ln_Succ;
				}
				IChooser->GetChooserNodeAttrs(node, CNA_Text, &text, TAG_DONE);
				IPHinzu(text); AktualisiereInPortListe();
				break;

				case GAD_OPANDERS:
				node = clusterlist.lh_Head;
				for (n = 0; n < code; n++) {
					node = node->ln_Succ;
				}
				IChooser->GetChooserNodeAttrs(node, CNA_Text, &text, TAG_DONE);
				IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_OPLIST], (uint32 *)&var);
				if(text) snprintf(outport[var].name,sizeof(outport[var].name), "%s", text);
				AktualisiereOutPortListe();
				break;
				
				case GAD_OPENTF:
				IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_OPLIST], (uint32 *)&var);
				if (var >= 0) {
					OPEntfernen((int8)var); AktualisiereOutPortListe();
				}
				break;

			    case GAD_OPLATENZ:
				IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_OPLIST], (uint32 *)&var);
				if (var >= 0) {
					outport[var].latenz = code;
				}
				break;

				case GAD_OUTINSTR1:
				case GAD_OUTINSTR2:
				case GAD_OUTINSTR3:
				case GAD_OUTINSTR4:
				IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_OPLIST], (uint32 *)&var);
				if (var >= 0) {
					instr = NtesInstrument(code);
					strncpy(outport[var].outinstr[(result & WMHI_GADGETMASK) - GAD_OUTINSTR1].name, instr->name, 128);
				}
				break;

				case GAD_OUTINSTRCH2:
				case GAD_OUTINSTRCH3:
				case GAD_OUTINSTRCH4:
				IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_OPLIST], (uint32 *)&var);
				if (var >= 0) {
					n = (result & WMHI_GADGETMASK) - GAD_OUTINSTRCH2 + 1;
					outport[var].outinstr[n].unten = code - 1;
					IIntuition->SetGadgetAttrs(setgad[n + GAD_OUTINSTR1], setfenster, NULL, GA_Disabled, (outport[var].outinstr[n].unten == 16), TAG_DONE);
				}
				break;

				case GAD_IPENTF:
				IIntuition->GetAttr(LISTBROWSER_Selected, (Object *)setgad[GAD_IPLIST], (uint32 *)&var);
				if (var >= 0) {
					IPEntfernen((int8)var); AktualisiereInPortListe();
				}
				break;
				
				case GAD_PHONOLITHPROJ:
				gfRequestFile((Object *)setgad[GAD_PHONOLITHPROJ], setfenster);
				break;

				// Metronom Seite
				
				case GAD_MKANAL:
				ChannelPortFenster(hfenster, &metro.channel, &metro.port);
				PortText();
				IIntuition->SetGadgetAttrs(setgad[GAD_MKANAL], setfenster, NULL, GA_Text, porttext, TAG_DONE);
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
				IIntuition->GetAttr(INTEGER_Number, (Object *)setgad[GAD_SHH], (uint32 *)&var);
				IIntuition->GetAttr(INTEGER_Number, (Object *)setgad[GAD_SMM], (uint32 *)&var2);
				IIntuition->GetAttr(INTEGER_Number, (Object *)setgad[GAD_SSS], (uint32 *)&var3);
				IIntuition->GetAttr(INTEGER_Number, (Object *)setgad[GAD_SFF], (uint32 *)&var4);
				smpte.startticks = Smpte2Ticks((int8)var, (int8)var2, (int8)var3, (int8)var4);
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
		STRPTR str = NULL;
		IIntuition->GetAttr(GETFILE_FullFile, (Object *)setgad[GAD_PHONOLITHPROJ], (uint32 *)&str);
		if (str) snprintf(lied.phonolithprojekt, sizeof(lied.phonolithprojekt), "%s", str);

		ErneuereLinks();
		HoleFensterObjpos(setfensterobj, SET);
		IIntuition->ClearMenuStrip(setfenster);
		RA_CloseWindow(setfensterobj);
		setfenster = NULL;
		AktualisierePortChooserListe();
	}
}
