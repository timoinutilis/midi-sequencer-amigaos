#include <stdio.h>
#include <string.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/camd.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <proto/window.h>
#include <proto/layout.h>
#include <proto/button.h>
#include <proto/chooser.h>
#include <proto/listbrowser.h>
#include <proto/string.h>

#include <intuition/classes.h>
#include <midi/camd.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/chooser.h>
#include <gadgets/listbrowser.h>
#include <gadgets/string.h>

#include "locale.h"

#include "Strukturen.h"
#include "GuiFenster.h"
#include "Versionen.h"
#include "Midi.h"
#include "Fenster.h"
#include "Gui.h"
#include "Menu.h"
#include "Projekt.h"
#include "Requester.h"


#define GAD_UNITSLIST 0
#define GAD_UNITNAME 1
#define GAD_UNITNEU 2
#define GAD_UNITDEL 3
#define GAD_UNITPORT 4
#define GAD_UNITSENDEN 5
#define GAD_ALLESENDEN 6
#define GAD_MSGLIST 7
#define GAD_MSGNAME 8
#define GAD_MSGDEL 9
#define GAD_MSGSENDEN 10
#define GAD_MSGLADEN 11
#define GAD_MSGSPEICHERN 12
#define GAD_MSGREC 13

extern struct CamdIFace *ICamd;

extern struct Screen *hschirm;
extern struct Menu *minmenu;
extern struct FENSTERPOS fenp[];

struct Window *sexfenster = NULL;
Object *sexfensterobj = NULL;
struct Gadget *sexgad[12];
struct List sexportlist = {NULL};

struct ColumnInfo msgcolinfo[] = {{65, "", 0}, {35, "", 0}, {-1, NULL, 0}};
struct List sexunitlist = {NULL};
struct List sexmsglist = {NULL};

struct SYSEXUNIT *rootsexunit = NULL;
BOOL sysexrec = FALSE;
struct SYSEXUNIT *wahlsexunit = NULL;

extern struct MidiNode *midi;
extern struct MidiLink *midiout[];
extern struct OUTPORT outport[];

extern char sysexdatei[];

int16 sysexnummer = 1;

void EntfernePortChooserListe(void) {
	struct Node *node;

	IIntuition->SetGadgetAttrs(sexgad[GAD_UNITPORT], sexfenster, NULL, CHOOSER_Labels, NULL, TAG_DONE);
	while ((node = IExec->RemTail(&sexportlist))) {
		IChooser->FreeChooserNode(node);
	}
}

void ErstellePortChooserListe(void) {
	struct Node *node;
	uint8 n;

	IExec->NewList(&sexportlist);
	for (n = 0; n < verOUTPORTS; n++) {
		if (outport[n].name[0]) {
			node = IChooser->AllocChooserNode(CNA_Text, outport[n].name, TAG_DONE);
			IExec->AddTail(&sexportlist, node);
		}
	}
	IIntuition->SetGadgetAttrs(sexgad[GAD_UNITPORT], sexfenster, NULL, CHOOSER_Labels, &sexportlist, TAG_DONE);
}

void AktualisierePortChooserListe(void) {
	if (sexfenster) {
		EntfernePortChooserListe();
		ErstellePortChooserListe();
	}
}



struct SYSEXMSG *NeuesSysEx(struct SYSEXUNIT *unit, STRPTR name, uint32 len, uint8 *data) {
	struct SYSEXMSG *neu;
	struct SYSEXMSG *akt;
	
	neu = IExec->AllocVecTags(sizeof(struct SYSEXMSG), TAG_END);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->len = len;
		neu->data = data;
		neu->prev = NULL;
		neu->next = NULL;
		
		if (!unit->sysex) {
			unit->sysex = neu;
		} else {
			akt = unit->sysex;
			while (akt->next) akt = akt->next;
			akt->next = neu;
			neu->prev = akt;
		}
	}
	return(neu);
}

void EntferneSysEx(struct SYSEXUNIT *unit, struct SYSEXMSG *sysex) {
	if (sysex != unit->sysex) {
		sysex->prev->next = sysex->next;
		if (sysex->next) sysex->next->prev=sysex->prev;
	} else {
		if (sysex->next) sysex->next->prev = NULL;
		unit->sysex = sysex->next;
	}
	IExec->FreeVec(sysex->data);
	IExec->FreeVec(sysex);
}

void EntferneAlleSysEx(struct SYSEXUNIT *unit) {
	struct SYSEXMSG *akt;
	struct SYSEXMSG *next;
	
	akt = unit->sysex;
	while (akt) {
		next = akt->next;
		IExec->FreeVec(akt->data);
		IExec->FreeVec(akt);
		akt = next;
	}
	unit->sysex = NULL;
}

void SysExAufnehmen(void) {
	uint32 len;
	uint8 *data;
	char puffer[140];
	
	len = ICamd->QuerySysEx(midi);
	if (len) {
		data = IExec->AllocVecTags(len, TAG_END);
		if (data) {
			if (ICamd->GetSysEx(midi, (uint8 *)data, len)) {
				if ((*data == 0xF0) || (*data == 0xF7)) {
					sprintf(puffer, "%s %d", wahlsexunit->name, sysexnummer++);
					puffer[127] = 0;
					NeuesSysEx(wahlsexunit, puffer, len, data);
				} else {
					IExec->FreeVec(data);
					Meldung(CAT(MSG_0525, "Incorrect SysEx message received"));
				}
			}
		} else {
			ICamd->SkipSysEx(midi);
			Meldung(CAT(MSG_0526, "Not enough memory for SysEx\n<SysEx.c>"));
		}
	}
}

void SendeSysExUnit(void) {
	struct SYSEXMSG *sysex;
	
	if (wahlsexunit) {
		if (midiout[wahlsexunit->port]) {
			sysex = wahlsexunit->sysex;
			while (sysex) {
				ICamd->PutSysEx(midiout[wahlsexunit->port], sysex->data);
				sysex = sysex->next;
			}
		}
	}
}

void SendeAlleSysEx(void) {
	struct SYSEXUNIT *unit;
	struct SYSEXMSG *sysex;
	
	unit = rootsexunit;
	while (unit) {
		if (midiout[unit->port] && !unit->gesperrt) {
			sysex = unit->sysex;
			while (sysex) {
				ICamd->PutSysEx(midiout[unit->port], sysex->data);
				sysex = sysex->next;
			}
		}
		unit = unit->next;
	}
}


struct SYSEXUNIT *NeueSysExUnit(STRPTR name) {
	struct SYSEXUNIT *neu;
	struct SYSEXUNIT *akt;
	
	neu = IExec->AllocVecTags(sizeof(struct SYSEXMSG), TAG_END);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->port = 0;
		neu->gesperrt = FALSE;
		neu->sysex = NULL;
		neu->prev = NULL;
		neu->next = NULL;
		
		if (!rootsexunit) {
			rootsexunit = neu;
		} else {
			akt = rootsexunit;
			while (akt->next) akt = akt->next;
			akt->next = neu;
			neu->prev = akt;
		}
	}
	return(neu);
}

void EntferneSysExUnit(struct SYSEXUNIT *unit) {
	if (unit != rootsexunit) {
		unit->prev->next = unit->next;
		if (unit->next) unit->next->prev = unit->prev;
	} else {
		if (rootsexunit->next) rootsexunit->next->prev = NULL;
		rootsexunit = rootsexunit->next;
	}
	EntferneAlleSysEx(unit);
	IExec->FreeVec(unit);
}

void EntferneAlleSysExUnits(void) {
	struct SYSEXUNIT *akt;
	struct SYSEXUNIT *next;
	
	akt = rootsexunit;
	while (akt) {
		next = akt->next;
		EntferneAlleSysEx(akt);
		IExec->FreeVec(akt);
		akt = next;
	}
	rootsexunit = NULL;
}

int16 zaehleSysEx(struct SYSEXUNIT *unit) {
	struct SYSEXMSG *sysex;
	int16 anz = 0;
	
	sysex = unit->sysex;
	while (sysex) {
		anz++;
		sysex = sysex->next;
	}
	return anz;
}

//------------------------------------------------------------------


void LadeSysEx(void) {
	BPTR file;
	uint8 *data;
	uint32 len;
	uint8 *start;
	uint8 *pos;
	uint8 *sysexdata;
	uint32 sysexlen;
	STRPTR name;
	char puffer[140];
	
	if (AslSysExLaden()) {
		file = IDOS->Open(sysexdatei, MODE_OLDFILE);
		if (file) {
//			Seek(file, 0, OFFSET_END);
			IDOS->ChangeFilePosition(file, 0, OFFSET_END);
			len = IDOS->ChangeFilePosition(file, 0, OFFSET_BEGINNING);
			
			name = (STRPTR)IDOS->FilePart(sysexdatei);
			
			data = IExec->AllocVecTags(len, TAG_END);
			if (data) {
				IDOS->Read(file, data, len);
				
				start = data;
				pos = data;
				sysexnummer = 1;
				
				do {
					if (*pos != 0xF0) {
						Meldung(CAT(MSG_0525A, "Error in SysEx file"));
						break;
					}
					pos++;
					while (*pos != 0xF7) {
						if ((uint32)pos < (uint32)data + len)
							pos++;
						else 
							break;
					}
					if (*pos != 0xF7) {
						Meldung(CAT(MSG_0525A, "Error in SysEx file"));
						break;
					}
					
					sysexlen = (uint32)pos - (uint32)start + 1;
					sysexdata = IExec->AllocVecTags(sysexlen, TAG_END);
					if (sysexdata) {
						memcpy(sysexdata, start, sysexlen);
						sprintf(puffer, "%s %d", name, sysexnummer++);
						puffer[127] = 0;
						NeuesSysEx(wahlsexunit, puffer, sysexlen, sysexdata);
					}
					
					pos++;
					start = pos;
				} while ((uint32)pos < (uint32)data + len);
				
				IExec->FreeVec(data);
			}
			
			IDOS->Close(file);
		}
	}
}

void SpeichereMarkSysEx(void) {
	struct Node *node;
	uint32 sel = FALSE;
	struct SYSEXMSG *sysex = NULL;
	BPTR file;
	BOOL ok = FALSE;
	
	if (!IsListEmpty(&sexmsglist)) {
		node = sexmsglist.lh_Head;
		while (node) {
			IListBrowser->GetListBrowserNodeAttrs(node,
				LBNA_Selected, &sel,
				LBNA_UserData, &sysex,
				TAG_DONE);
			if (sel && sysex) ok = TRUE;
			if (node == sexmsglist.lh_TailPred) break;
			node = node->ln_Succ;
		}
	}

	if (ok) {
		if (AslSysExSpeichern()) {
			file = IDOS->Open(sysexdatei, MODE_NEWFILE);
			if (file) {
				if (!IsListEmpty(&sexmsglist)) {
					node = sexmsglist.lh_Head;
					while (node) {
						IListBrowser->GetListBrowserNodeAttrs(node,
							LBNA_Selected, &sel,
							LBNA_UserData, &sysex,
							TAG_DONE);
						if (sel && sysex) {
							if (midiout[wahlsexunit->port]) ICamd->PutSysEx(midiout[wahlsexunit->port], sysex->data);
							IDOS->Write(file, sysex->data, sysex->len);
						}
						if (node == sexmsglist.lh_TailPred) break;
						node = node->ln_Succ;
					}
				}
				IDOS->Close(file);
			}
		}
	} else Meldung(CAT(MSG_0525B, "At least one SysEx message must be selected for saving"));
}

void SendeMarkSysEx(void) {
	struct Node *node;
	uint32 sel = FALSE;
	struct SYSEXMSG *sysex = NULL;
	
	if (!IsListEmpty(&sexmsglist)) {
		node = sexmsglist.lh_Head;
		while (node) {
			IListBrowser->GetListBrowserNodeAttrs(node,
				LBNA_Selected, &sel,
				LBNA_UserData, &sysex,
				TAG_DONE);
			if (sel && sysex) {
				if (midiout[wahlsexunit->port]) ICamd->PutSysEx(midiout[wahlsexunit->port], sysex->data);
			}
			if (node == sexmsglist.lh_TailPred) break;
			node = node->ln_Succ;
		}
	}
}

void EntferneMarkSysEx(void) {
	struct Node *node;
	uint32 sel = FALSE;
	struct SYSEXMSG *sysex = NULL;
	
	if (!IsListEmpty(&sexmsglist)) {
		node = sexmsglist.lh_Head;
		while (node) {
			IListBrowser->GetListBrowserNodeAttrs(node,
				LBNA_Selected, &sel,
				LBNA_UserData, &sysex,
				TAG_DONE);
			if (sel && sysex) EntferneSysEx(wahlsexunit, sysex);
			if (node == sexmsglist.lh_TailPred) break;
			node = node->ln_Succ;
		}
	}
}

void BenenneMarkSysEx(STRPTR name) {
	struct Node *node;
	uint32 sel = FALSE;
	struct SYSEXMSG *sysex = NULL;
	
	if (!IsListEmpty(&sexmsglist)) {
		node = sexmsglist.lh_Head;
		while (node) {
			IListBrowser->GetListBrowserNodeAttrs(node,
				LBNA_Selected, &sel,
				LBNA_UserData, &sysex,
				TAG_DONE);
			if (sel && sysex) strncpy(sysex->name, name, 128);
			if (node == sexmsglist.lh_TailPred) break;
			node = node->ln_Succ;
		}
	}
}


void EntferneSysExGruppenListe(void) {
	struct Node *node;
	
	if (!sexunitlist.lh_Head) {
		IExec->NewList(&sexunitlist);
	}
	while ((node = IExec->RemTail(&sexunitlist))) {
		IListBrowser->FreeListBrowserNode(node);
	}
}

void EntferneSysExMsgListe(void) {
	struct Node *node;
	
	if (!sexmsglist.lh_Head) {
		IExec->NewList(&sexmsglist);
	}
	while ((node = IExec->RemTail(&sexmsglist))) {
		IListBrowser->FreeListBrowserNode(node);
	}
}

void AktualisiereSysExGruppenListe(void) {
	struct SYSEXUNIT *unit;
	struct Node *node;
	char puffer[140];
	int16 anz;
	
	if (sexfenster)
		IIntuition->SetGadgetAttrs(sexgad[GAD_UNITSLIST], sexfenster, NULL, LISTBROWSER_Labels, NULL, TAG_DONE);

	EntferneSysExGruppenListe();

	unit = rootsexunit;
	while (unit) {
		anz = zaehleSysEx(unit);
		if (anz > 0) {
			sprintf(puffer, "%s  (%d)", unit->name, anz);
		} else {
			strncpy(puffer, unit->name, 140);
		}
		node = IListBrowser->AllocListBrowserNode(1,
			LBNA_UserData, (APTR)unit,
			LBNA_CheckBox, TRUE,
			LBNA_Checked, !unit->gesperrt,
			LBNCA_CopyText, TRUE,
			LBNCA_Text, puffer,
			TAG_DONE);
		if (node) IExec->AddTail(&sexunitlist, node);
		unit = unit->next;
	}

	if (sexfenster)
		IIntuition->SetGadgetAttrs(sexgad[GAD_UNITSLIST], sexfenster, NULL, LISTBROWSER_Labels, &sexunitlist, TAG_DONE);
}

void AktualisiereSysExMsgListe(void) {
	struct SYSEXMSG *sysex;
	struct Node *node;
	char lentxt[10];
	
	if (sexfenster)
		IIntuition->SetGadgetAttrs(sexgad[GAD_MSGLIST], sexfenster, NULL, LISTBROWSER_Labels, NULL, TAG_DONE);

	EntferneSysExMsgListe();

	if (wahlsexunit) {
		sysex = wahlsexunit->sysex;
		while (sysex) {
			sprintf(lentxt, "%ld", sysex->len);
			node = IListBrowser->AllocListBrowserNode(2,
				LBNA_UserData, (APTR)sysex,
				LBNA_Column, 0,
				LBNCA_CopyText, TRUE,
				LBNCA_Text, sysex->name,
				LBNA_Column, 1,
				LBNCA_CopyText, TRUE,
				LBNCA_Text, lentxt,
				LBNCA_Justification, LCJ_RIGHT,
				TAG_DONE);
			if (node) IExec->AddTail(&sexmsglist, node);
			sysex = sysex->next;
		}
	}

	if (sexfenster)
		IIntuition->SetGadgetAttrs(sexgad[GAD_MSGLIST], sexfenster, NULL,
			LISTBROWSER_Labels, &sexmsglist,
			LISTBROWSER_AutoFit, TRUE,
			TAG_DONE);
}

void ErstelleSysExFenster(void) {
	if (!sexfensterobj) {
	
		IExec->NewList(&sexunitlist);
		IExec->NewList(&sexmsglist);
		AktualisiereSysExGruppenListe();
		AktualisiereSysExMsgListe();

		sexfensterobj = WindowObject,
			WA_PubScreen, hschirm,
			WA_Title, CAT(MSG_0527, "SysEx Manager"),
			WA_Activate, TRUE,
			WA_CloseGadget, TRUE,
			WA_DepthGadget, TRUE,
			WA_DragBar, TRUE,
			WA_SizeGadget, TRUE,
			WA_SizeBBottom, TRUE,
			WA_IDCMP, IDCMP_MENUPICK,
			WINDOW_ParentGroup, HLayoutObject,
				LAYOUT_SpaceOuter, TRUE,
				
				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_Label, CAT(MSG_0528, "Groups"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
					
					LAYOUT_AddChild, sexgad[GAD_UNITSLIST] = (struct Gadget *)ListBrowserObject,
						GA_ID, GAD_UNITSLIST, GA_RelVerify, TRUE,
						LISTBROWSER_Labels, &sexunitlist,
						LISTBROWSER_ShowSelected, TRUE,
						LISTBROWSER_Selected, 0,
					End,
					
					LAYOUT_AddChild, sexgad[GAD_UNITNAME] = (struct Gadget *)StringObject,
						GA_ID, GAD_UNITNAME, GA_RelVerify, TRUE,
						STRINGA_MaxChars, 127,
						STRINGA_TextVal, wahlsexunit->name,
					End,
					CHILD_WeightedHeight, 0,
					
					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_UNITNEU, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0529, "New"), End,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_UNITDEL, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0530, "Delete"), End,
					End,
					CHILD_WeightedHeight, 0,

					LAYOUT_AddChild, sexgad[GAD_UNITPORT] = (struct Gadget *)ChooserObject,
						GA_ID, GAD_UNITPORT, GA_RelVerify, TRUE,
						CHOOSER_PopUp, TRUE,
						CHOOSER_Labels, NULL,
						CHOOSER_MaxLabels, verOUTPORTS,
						CHOOSER_Selected, wahlsexunit->port,
					End,				
					CHILD_WeightedHeight, 0,
					
					LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, GAD_UNITSENDEN, GA_Text, CAT(MSG_0531, "Send Group"), End,
					CHILD_WeightedHeight, 0,

					LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, GAD_ALLESENDEN, GA_Text, CAT(MSG_0532, "Send All Groups"), End,
					CHILD_WeightedHeight, 0,
				End,

				LAYOUT_AddChild, VLayoutObject,
					LAYOUT_Label, CAT(MSG_0533, "SysEx Messages"), LAYOUT_BevelStyle, BVS_GROUP, LAYOUT_SpaceOuter, TRUE,
					
					LAYOUT_AddChild, sexgad[GAD_MSGLIST] = (struct Gadget *)ListBrowserObject,
						GA_ID, GAD_MSGLIST, GA_RelVerify, TRUE,
						LISTBROWSER_Labels, &sexmsglist,
						LISTBROWSER_ColumnInfo, msgcolinfo,
						LISTBROWSER_AutoFit, TRUE,
						LISTBROWSER_Separators, FALSE,
						LISTBROWSER_MultiSelect, TRUE,
						LISTBROWSER_ShowSelected, TRUE,
						LISTBROWSER_Editable, TRUE,
					End,
					
					LAYOUT_AddChild, sexgad[GAD_MSGNAME] = (struct Gadget *)StringObject,
						GA_ID, GAD_MSGNAME, GA_RelVerify, TRUE,
						STRINGA_MaxChars, 127,
					End,
					CHILD_WeightedHeight, 0,
					
					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_MSGDEL, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0530, "Delete"), End,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_MSGSENDEN, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0535, "Send"), End,
					End,
					CHILD_WeightedHeight, 0,

					LAYOUT_AddChild, HLayoutObject,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_MSGLADEN, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0535A, "Load..."), End,
						LAYOUT_AddChild, ButtonObject, GA_ID, GAD_MSGSPEICHERN, GA_RelVerify, TRUE, GA_Text, CAT(MSG_0535B, "Save..."), End,
					End,
					CHILD_WeightedHeight, 0,

					LAYOUT_AddChild, sexgad[GAD_MSGREC] = (struct Gadget *)ButtonObject, 
						GA_RelVerify, TRUE,
						GA_ID, GAD_MSGREC,
						GA_Text, CAT(MSG_0536, "Record SysEx"),
						BUTTON_PushButton, TRUE,
					End,
					CHILD_WeightedHeight, 0,
				End,

			End,
		End;
	}
	
	if (sexfensterobj) {
		if (fenp[ENV].b>0) {
			IIntuition->SetAttrs(sexfensterobj,
				WA_Left, fenp[SEX].x, WA_Top, fenp[SEX].y,
				WA_InnerWidth, fenp[SEX].b, WA_InnerHeight, fenp[SEX].h,
				TAG_DONE);
		}

		sexfenster = (struct Window *)RA_OpenWindow(sexfensterobj);
		IIntuition->SetMenuStrip(sexfenster, minmenu);
		ErstellePortChooserListe();
	}
}

void EntferneSysExFenster(void) {
	if (sexfensterobj) {
		if (sexfenster) {
			HoleFensterObjpos(sexfensterobj, SEX);
			IIntuition->ClearMenuStrip(sexfenster);
		}
		IIntuition->DisposeObject(sexfensterobj);
		EntferneSysExGruppenListe();
		EntferneSysExMsgListe();
		sexfensterobj = NULL;
		sexfenster = NULL;
	}
}

void SysExRecAus(void) {
	if (sysexrec) {
		sysexrec = FALSE;
		IIntuition->SetGadgetAttrs(sexgad[GAD_MSGREC], sexfenster, NULL, GA_Selected, FALSE, TAG_DONE);
	}
}


void KontrolleSysExFenster(void) {
	BOOL schliessen=FALSE;
	uint32 result;
	uint16 code;
	struct Node *node = NULL;
	struct SYSEXMSG *sysex;
	int16 n;
	STRPTR name = NULL;
	int32 var;
	
	while ((result = RA_HandleInput(sexfensterobj, &code)) != WMHI_LASTMSG) {
		switch (result & WMHI_CLASSMASK) {
			case WMHI_CLOSEWINDOW:
			schliessen = TRUE;
			break;

			case WMHI_MENUPICK:
			MinMenuKontrolle(MinMenuPunkt(result & WMHI_MENUMASK));
			break;
			
			case WMHI_GADGETUP:
			switch (result & WMHI_GADGETMASK) {
				case GAD_UNITSLIST:
				IIntuition->GetAttr(LISTBROWSER_SelectedNode, (Object *)sexgad[GAD_UNITSLIST], (uint32 *)&node);
				IListBrowser->GetListBrowserNodeAttrs(node, LBNA_UserData, (uint32 *)&wahlsexunit, TAG_DONE);

				IListBrowser->GetListBrowserNodeAttrs(node, LBNA_Checked, &var, TAG_DONE);
				wahlsexunit->gesperrt = (BOOL)!var;
				
				AktualisiereSysExMsgListe();
				IIntuition->SetGadgetAttrs(sexgad[GAD_UNITNAME], sexfenster, NULL, STRINGA_TextVal, wahlsexunit->name, TAG_DONE);
				IIntuition->SetGadgetAttrs(sexgad[GAD_UNITPORT], sexfenster, NULL, CHOOSER_Selected, wahlsexunit->port, TAG_DONE);
				break;
				
				case GAD_UNITNAME:
				IIntuition->GetAttr(STRINGA_TextVal, (Object *)sexgad[GAD_UNITNAME], (uint32 *)&name);
				strncpy(wahlsexunit->name, name, 128);
				AktualisiereSysExGruppenListe();
				break;
				
				
				case GAD_UNITNEU:
				NeueSysExUnit(CAT(MSG_0529, "New"));
				AktualisiereSysExGruppenListe();
				break;
				
				case GAD_UNITDEL:
				IIntuition->GetAttr(LISTBROWSER_SelectedNode, (Object *)sexgad[GAD_UNITSLIST], (uint32 *)&node);
				IListBrowser->GetListBrowserNodeAttrs(node, LBNA_UserData, (uint32 *)&wahlsexunit, TAG_DONE);
				EntferneSysExUnit(wahlsexunit);
				if (!rootsexunit) NeueSysExUnit(CAT(MSG_0538, "Standard"));
				wahlsexunit = rootsexunit;
				AktualisiereSysExGruppenListe();
				AktualisiereSysExMsgListe();
				break;
				
				case GAD_UNITPORT:
				wahlsexunit->port = code;
				break;
				
				case GAD_UNITSENDEN:
				SysExRecAus();
				SendeSysExUnit();
				break;
				
				case GAD_ALLESENDEN:
				SysExRecAus();
				SendeAlleSysEx();
				break;
				
				case GAD_MSGLIST:
				sysex = wahlsexunit->sysex;
				for (n = 0; n < code; n++) sysex = sysex->next;
				IIntuition->SetGadgetAttrs(sexgad[GAD_MSGNAME], sexfenster, NULL, STRINGA_TextVal, sysex->name, TAG_DONE);
				break;

				case GAD_MSGNAME:
				IIntuition->GetAttr(STRINGA_TextVal, (Object *)sexgad[GAD_MSGNAME], (uint32 *)&name);
				BenenneMarkSysEx(name);
				AktualisiereSysExMsgListe();
				break;
				
				case GAD_MSGLADEN:
				LadeSysEx();
				AktualisiereSysExMsgListe();
				AktualisiereSysExGruppenListe();
				break;
				
				case GAD_MSGSPEICHERN:
				SpeichereMarkSysEx();
				break;
				
				case GAD_MSGDEL:
				EntferneMarkSysEx();
				AktualisiereSysExMsgListe();
				AktualisiereSysExGruppenListe();
				break;
				
				case GAD_MSGSENDEN:
				SysExRecAus();
				SendeMarkSysEx();
				break;
				
				case GAD_MSGREC:
				sysexnummer = 1;
				sysexrec = (BOOL)code;
				break;			
			}
			break;
		}
	}
	if (schliessen) {
		HoleFensterObjpos(sexfensterobj, SEX);
		SysExRecAus();
		IIntuition->ClearMenuStrip(sexfenster);
		EntfernePortChooserListe();
		RA_CloseWindow(sexfensterobj);
		sexfenster = NULL;
	}
}
