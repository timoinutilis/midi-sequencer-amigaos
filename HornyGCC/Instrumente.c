#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/camd.h>
#include <clib/alib_protos.h>

#include <proto/window.h>
#include <proto/layout.h>
#include <proto/button.h>
#include <proto/chooser.h>
#include <proto/listbrowser.h>
#include <proto/integer.h>
#include <proto/checkbox.h>
#include <proto/label.h>

#include <intuition/classes.h>
#include <midi/camd.h>
#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/chooser.h>
#include <gadgets/listbrowser.h>
#include <gadgets/integer.h>
#include <gadgets/checkbox.h>
#include <images/label.h>

#include "locale.h"

#include "Strukturen.h"
#include "Midi.h"
#include "Versionen.h"
#include "Dynamic_Strings.h"
#include "Requester.h"

struct INSTRUMENT *rootinstrument = NULL;
extern struct Screen *hschirm;
extern struct OUTPORT outport[];
struct ColumnInfo contrcolinfo[] = {{10, NULL, 0},{10, NULL, 0},{-1, NULL, 0}};
BOOL contralle = FALSE;



struct INSTRCONTR *LadeInstrContr(STRPTR datei) {
	BPTR file;
	char zeile[256];
	struct INSTRCONTR *neu;
	int16 n;
	int16 lz;

	file = IDOS->Open(datei, MODE_OLDFILE);
	if (file) {
		IDOS->FGets(file, zeile, 256);
		if (strcmp(zeile, "HORNY INSTRUMENT CONTROLLER\n") != 0) {
			IDOS->Close(file);
			return(NULL);
		}

		neu = IExec->AllocVecTags(sizeof(struct INSTRCONTR), TAG_END);
		if (neu) {
			for (n = 0; n < 128; n++) {
				sprintf(neu->name[n], "(Contr. %d)", n);
				neu->flags[n] = 0;
			}
			
			while (IDOS->FGets(file, zeile, 256)) {
				lz = -1;
				for (n = 0; n < 6; n++) {
					if (zeile[n] == 0) break;
					if (zeile[n] == ' ') {lz = n; break;}
				}
				if (lz > 0) {
					zeile[strlen(zeile) - 1] = 0;
					n = atoi(zeile);
					neu->flags[n] = CONTR_SET;
					if (zeile[lz - 1] == '+') neu->flags[n] = (neu->flags[n] | CONTR_MITTE);
					if (zeile[lz - 1] == '-') neu->flags[n] = (neu->flags[n] | CONTR_ONOFF);
					strncpy(neu->name[n], &zeile[lz + 1], 128);
				}
			}
		}
		IDOS->Close(file);
		return(neu);
	}
	return(NULL);
}


void NeuesProgramm(struct KATEGORIE *kat, int8 bank0, int8 bank32, int8 prog, STRPTR name) {
	struct PROGRAMM *neu;
	struct PROGRAMM *akt;
	
	neu = IExec->AllocVecTags(sizeof(struct PROGRAMM), TAG_END);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->bank0 = bank0;
		neu->bank32 = bank32;
		neu->prog = prog;
		neu->next = NULL;
		
		if (!kat->programm) {
			kat->programm = neu;
		} else {
			akt = kat->programm;
			while (akt->next) akt = akt->next;
			akt->next = neu;
		}
	}
}

struct KATEGORIE *NeueKategorie(struct INSTRUMENT *instr, STRPTR name, int8 chanvon, int8 chanbis) {
	struct KATEGORIE *neu;
	struct KATEGORIE *akt;
	
	neu = IExec->AllocVecTags(sizeof(struct KATEGORIE), TAG_END);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->programm = NULL;
		neu->chanvon = chanvon;
		neu->chanbis = chanbis;
		neu->next = NULL;
		
		if (!instr->kategorie) {
			instr->kategorie = neu;
		} else {
			akt = instr->kategorie;
			while (akt->next) akt = akt->next;
			akt->next = neu;
		}
	}
	return(neu);
}

struct INSTRUMENT *LadeInstrument(STRPTR name) {
	BPTR file;
	char datei[512];
	char zeile[256];
	struct INSTRUMENT *neu;
	struct INSTRUMENT *akt;
	struct KATEGORIE *kat = NULL;
	int8 prog = 0;
	int8 bank0 = -1;
	int8 bank32 = -1;
	int8 chanvon = 0, chanbis = 15;
	int16 schwanz;
	char *leer;
	int16 von, bis;
	int16 n;
	STRPTR numname;
	char progname[128];
	
	neu = IExec->AllocVecTags(sizeof(struct INSTRUMENT), TAG_END);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->kategorie = NULL;
		neu->next = NULL;
		
		
		snprintf(datei, sizeof(datei), "PROGDIR:System/Instruments/%s", name);
		
		file = IDOS->Open(datei, MODE_OLDFILE);
		if (file) {
			IDOS->FGets(file, zeile, 256);
			if (strcmp(zeile, "HORNY INSTRUMENT\n") != 0) {
				IDOS->Close(file);
				IExec->FreeVec(neu);
				return(NULL);
			}

			while (IDOS->FGets(file, zeile, 256)) {
				schwanz = strlen(zeile) - 1;
				if (zeile[schwanz] == '\n') zeile[schwanz] = 0;
				
				switch (zeile[0]) {
					case 0: break;
					
					case '*':
					kat = NeueKategorie(neu, &zeile[2], chanvon, chanbis);
					break;
					
					case '#':
					leer = strchr(zeile, ' ');
					if ((zeile[1] == 'B') && (zeile[5] != '3')) bank0 = (int8)atoi(leer); // #BANK0 x
					if ((zeile[1] == 'B') && (zeile[5] == '3')) bank32 = (int8)atoi(leer); // #BANK32 x
					if (zeile[1] == 'P') prog = (int8)atoi(leer); // #PROG x
					if (zeile[1] == 'C') { // #CHAN x x
						chanvon = atol(leer++) - 1;
						leer = strchr(leer, ' ');
						chanbis = atol(leer) - 1;
					}
					if (zeile[1] == 'N') { // #NUM s x x
						leer++;
						numname = leer;
						leer = strchr(leer, ' ');
						*leer++ = 0;
						von = atoi(leer++);
						leer = strchr(leer, ' ');
						bis = atoi(leer);
						for (n = von; n <= bis; n++) {
							sprintf(progname, "%s %d", numname, n);
							NeuesProgramm(kat, bank0, bank32, prog++, progname);
						}
					}
					break;
					
					default:
					if (kat) NeuesProgramm(kat, bank0, bank32, prog++, zeile);
				}
			}
	
			IDOS->Close(file);
			
			snprintf(datei, sizeof(datei), "PROGDIR:System/Instruments/%s.controller", name);
			
			neu->contr = LadeInstrContr(datei);
			if (!neu->contr) neu->contr = LadeInstrContr((STRPTR)"PROGDIR:System/Instruments/GM.controller");

			if (!rootinstrument) {
				rootinstrument = neu;
			} else {
				akt = rootinstrument;
				while (akt->next) akt = akt->next;
				akt->next = neu;
			}
		} else {
			IExec->FreeVec(neu);
		}
	}
	return(neu);
}

void ErstelleVorgabeInstrument(void) {
	struct KATEGORIE *kat;
	int16 prog;
	char progname[128];
	
	rootinstrument = IExec->AllocVecTags(sizeof(struct INSTRUMENT), TAG_END);
	if (rootinstrument) {
		strcpy(rootinstrument->name, "???");
		rootinstrument->kategorie = NULL;
		rootinstrument->contr = LadeInstrContr((STRPTR)"PROGDIR:System/Instruments/GM.controller");
		rootinstrument->next = NULL;
		
		kat = NeueKategorie(rootinstrument, CAT(MSG_0470, "Standard"), 0, 15);
		if (kat) {
			for (prog = 0; prog < 128; prog++) {
				sprintf(progname, CAT(MSG_0471, "Program %d"), prog);
				NeuesProgramm(kat, -1, -1, prog, progname);
			}
		}
	}
}

void LadeAlleInstrumente(void) {
	APTR context;

	struct ExamineData *xd = NULL;

	ErstelleVorgabeInstrument();
	context = IDOS->ObtainDirContextTags(EX_StringNameInput, "PROGDIR:System/Instruments/", TAG_END);
	if (context) {
		while ((xd = IDOS->ExamineDir(context))) {
				if (EXD_IS_FILE(xd)){
					LadeInstrument(xd->Name);
				}
		}
		IDOS->ReleaseDirContext(context);
	}
}

void EntferneAlleInstrumente(void) {
	struct INSTRUMENT *instr;
	struct KATEGORIE *kat;
	struct PROGRAMM *prog;
	struct INSTRUMENT *nextinstr;
	struct KATEGORIE *nextkat;
	struct PROGRAMM *nextprog;
	
	instr = rootinstrument;
	while (instr) {
		nextinstr = instr->next;
		
		kat = instr->kategorie;
		while (kat) {
			nextkat = kat->next;
			
			prog = kat->programm;
			while (prog) {
				nextprog = prog->next;
				IExec->FreeVec(prog);
				prog = nextprog;
			}
			
			IExec->FreeVec(kat);
			kat = nextkat;
		}
		
		if (instr->contr) IExec->FreeVec(instr->contr);
		
		IExec->FreeVec(instr);
		instr = nextinstr;
	}
	rootinstrument = NULL;
}

struct INSTRUMENT *SucheInstrument(STRPTR name) {
	struct INSTRUMENT *instr;
	
	instr = rootinstrument;
	while (instr) {
		if (strcmp(instr->name, name) == 0) return(instr);
		instr = instr->next;
	}
	return(NULL);
}

int16 SucheInstrumentNum(STRPTR name) {
	struct INSTRUMENT *instr;
	int16 n;
	
	instr = rootinstrument; n = 0;
	while (instr) {
		if (strcmp(instr->name, name) == 0) return(n);
		instr = instr->next; n++;
	}
	return(-1);
}

struct INSTRUMENT *NtesInstrument(int16 n) {
	struct INSTRUMENT *instr;
	int16 z;
	
	instr = rootinstrument;
	for (z = 0; z < n; z++) {
		if (instr->next) instr = instr->next;
	}
	return(instr);
}

BOOL SucheProgrammNum(struct INSTRUMENT *instr, int8 channel, int8 bank0, int8 bank32, int8 midiprog, int16 *katnum, int16 *prognum) {
	struct KATEGORIE *kat;
	struct PROGRAMM *prog;
	int16 zkat;
	int16 zprog;
	
	if (midiprog >= 0) {
		kat = instr->kategorie; zkat = 0;
		while (kat) {
			if ((channel >= kat->chanvon) && (channel <= kat->chanbis)) {
				prog = kat->programm; zprog = 0;
				while (prog) {
					if ((prog->bank0 == bank0) && (prog->bank32 == bank32) && (prog->prog == midiprog)) {
						*katnum = zkat;
						*prognum = zprog;
						return(TRUE);
					}
					prog = prog->next;
					zprog++;
				}
			}
			kat = kat->next;
			zkat++;
		}
	}
	return(FALSE);
}

struct INSTRUMENT *SucheChannelInstrument(uint8 port, int8 channel) {
	int8 n;
	STRPTR instrname;
	struct INSTRUMENT *instr;
	struct OUTPORT *outp;
	
	outp = &outport[port];
	for(n = 0; n < 4; n++) {
		if (channel >= outp->outinstr[n].unten) instrname = outp->outinstr[n].name;
		else break;
	}
	
	instr = SucheInstrument(instrname);
	if (!instr) instr = rootinstrument;
	return(instr);
}

void TesteInstrumente(void) {
	int8 n;
	int8 p;
	struct INSTRUMENT *i;
	STRPTR meldung = NULL;
	char instr[130];
	
	for (p = 0; p < verOUTPORTS; p++) {
		if (!outport[p].name[0]) break;
		for (n = 0; n < 4; n++) {
			if (outport[p].outinstr[n].unten < 16) {
				i = SucheInstrument(outport[p].outinstr[n].name);
				if (!i) {
					if (!meldung) meldung = String_Copy(meldung, CAT(MSG_0472, "Following instrument files\nare missing for this project:\n"));
					sprintf(instr, "\n%s", outport[p].outinstr[n].name);
					meldung = String_Cat(meldung, instr);
				}
			}
		}
	}
	if (meldung) {
		Meldung(meldung);
		String_Free(meldung);
	}
}


//==================


void ErstelleKategorieListe(struct List *katlist, struct INSTRUMENT *instr, int8 channel) {
	struct KATEGORIE *kat;
	struct Node *node;
	uint32 n = 0;
	
	kat = instr->kategorie;
	while (kat) {
		if ((channel >= kat->chanvon) && (channel <= kat->chanbis)) {
			node = IChooser->AllocChooserNode(CNA_Text, kat->name, CNA_UserData, (uint32 *)n, TAG_DONE);
			if (node) IExec->AddTail(katlist, node);
		}
		n++;
		kat = kat->next;
	}
}

int16 HoleChooserUserData(struct List *list, int16 num) {
	int32 userdata = 0;
	int16 n;
	struct Node *node;
	
	if (!IsListEmpty(list)) {
		node = list->lh_Head;
		for (n = 0; n < num; n++) node = node->ln_Succ;
		IChooser->GetChooserNodeAttrs(node, CNA_UserData, &userdata, TAG_DONE);
	}
	return((int16)userdata);
}

int16 WoIstChooserUserData(struct List *list, int16 userdata) {
	struct Node *node;
	int32 ud;
	int16 n = 0;
	
	node = list->lh_Head;
	while (node) {
		IChooser->GetChooserNodeAttrs(node, CNA_UserData, &ud, TAG_DONE);
		if (ud == userdata) return(n);
		n++; node = node->ln_Succ;
	}
	return(0);
}

void ErstelleProgrammListe(struct List *proglist, struct KATEGORIE *kat) {
	struct PROGRAMM *prog;
	struct Node *node;
	
	prog = kat->programm;
	while (prog) {
		node = IListBrowser->AllocListBrowserNode(1, LBNCA_Text, prog->name, TAG_DONE);
		if (node)
		{
			IExec->AddTail(proglist, node);
		}
		prog = prog->next;
	}
}

void EntferneChooserListe(struct List *list) {
	struct Node *node;
	
	while ((node = IExec->RemTail(list)))
	{
		IChooser->FreeChooserNode(node);
	}
}

void EntferneListBrowserListe(struct List *list) {
	struct Node *node;
	
	while ((node = IExec->RemTail(list)))
	{
		IListBrowser->FreeListBrowserNode(node);
	}
}

void InstrumentenFenster2(struct Window *fen, int8 channel, uint8 port, int8 *bank0, int8 *bank32, int8 *prog) {
	Object *fensterobj;
	struct Window *fenster;
	struct Gadget *gadkat;
	struct Gadget *gadprog;
	struct Gadget *gadintbank0;
	struct Gadget *gadintbank32;
	struct Gadget *gadintprog;
	BOOL schliessen = FALSE;
	uint32 result;
	uint16 code;
	uint32 var;
	struct List katlist;
	struct List proglist;
	struct INSTRUMENT *instr = NULL;
	struct KATEGORIE *kat = NULL;
	struct PROGRAMM *progr = NULL;
	int16 n;
	int16 katnum = 0;
	int16 prognum = -1;
	char fenstername[300];
	int8 b0, b32, pr;
	
	IExec->NewList(&katlist);
	IExec->NewList(&proglist);
	
	instr = SucheChannelInstrument(port, channel);
	
	ErstelleKategorieListe(&katlist, instr, channel);

	if (!SucheProgrammNum(instr, channel, *bank0, *bank32, *prog, &katnum, &prognum))
		katnum = HoleChooserUserData(&katlist, 0);
	kat = instr->kategorie;
	for (n = 0; n < katnum; n++) kat = kat->next;
	ErstelleProgrammListe(&proglist, kat);

	sprintf(fenstername, CAT(MSG_0474, "Program Choice(%s)"), instr->name);
	fensterobj = WindowObject,
		WA_PubScreen, hschirm,
		WA_Title, fenstername,
		WA_Activate, TRUE,
		WA_DragBar, TRUE,
		WA_SizeGadget, TRUE,
		WA_SizeBBottom, TRUE,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Height, 300,
		WINDOW_ParentGroup, VLayoutObject,
			LAYOUT_SpaceOuter, TRUE,
			
			LAYOUT_AddChild, gadkat = (struct Gadget *)ChooserObject,
				GA_ID, 0, GA_RelVerify, TRUE,
				CHOOSER_PopUp, TRUE,
				CHOOSER_Labels, &katlist,
				CHOOSER_MaxLabels, 40,
			End,				
			CHILD_WeightedHeight, 0,
			
			LAYOUT_AddChild, gadprog = (struct Gadget *)ListBrowserObject,
				GA_ID, 1, GA_RelVerify, TRUE,
				LISTBROWSER_Labels, &proglist,
				LISTBROWSER_ShowSelected, TRUE,
			End,
			
			LAYOUT_AddChild, VLayoutObject,

				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, gadintbank0 = (struct Gadget *)IntegerObject,
						GA_ID, 2, GA_RelVerify, TRUE,
						INTEGER_Number, *bank0,
						INTEGER_Minimum, -1,
						INTEGER_Maximum, 127,
					End,
					CHILD_Label, LabelObject, LABEL_Text, "Bank", End,
		
					LAYOUT_AddChild, gadintbank32 = (struct Gadget *)IntegerObject,
						GA_ID, 3, GA_RelVerify, TRUE,
						INTEGER_Number, *bank32,
						INTEGER_Minimum, -1,
						INTEGER_Maximum, 127,
					End,
				End,
	
				LAYOUT_AddChild, gadintprog = (struct Gadget *)IntegerObject,
					GA_ID, 4, GA_RelVerify, TRUE,
					INTEGER_Number, *prog,
					INTEGER_Minimum, 0,
					INTEGER_Maximum, 127,
				End,
				CHILD_Label, LabelObject, LABEL_Text, "Prog", End,
				
				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, 5, GA_Text, CAT(MSG_0477, "Cancel"), End,
					LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, 6, GA_Text, CAT(MSG_0478, "Nothing"), End,
				End,
	
				LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, 7, GA_Text, CAT(MSG_0479, "Accept"), End,
			End,
			CHILD_WeightedHeight, 0,
		End,
	End;
	
	if (fensterobj) {
		fenster = (struct Window *)RA_OpenWindow(fensterobj);
		if (fenster) {
			if (prognum >= 0) {
				IIntuition->SetGadgetAttrs(gadkat, fenster, NULL, CHOOSER_Selected, WoIstChooserUserData(&katlist, katnum), TAG_DONE);
				IIntuition->SetGadgetAttrs(gadprog, fenster, NULL,
					LISTBROWSER_Selected, prognum,
					LISTBROWSER_MakeVisible, prognum,
					TAG_DONE);
			}
			
			do {
				IExec->WaitPort(fenster->UserPort);
				while ((result = RA_HandleInput(fensterobj, &code)) != WMHI_LASTMSG) {
					switch (result & WMHI_CLASSMASK) {
						case WMHI_INACTIVE:
						schliessen = TRUE;
						break;
			
						case WMHI_GADGETUP:
						switch (result & WMHI_GADGETMASK) {
							case 0: // Kategorie
							katnum = HoleChooserUserData(&katlist, code);
							kat = instr->kategorie;
							for (n = 0; n < katnum; n++) kat = kat->next;
							IIntuition->SetGadgetAttrs(gadprog, fenster, NULL, LISTBROWSER_Labels, NULL, TAG_DONE);
							EntferneListBrowserListe(&proglist);
							ErstelleProgrammListe(&proglist, kat);
							IIntuition->SetGadgetAttrs(gadprog, fenster, NULL, LISTBROWSER_Labels, &proglist, TAG_DONE);
							break;
							
							case 1: // Programm
							progr = kat->programm;
							for (n = 0; n < code; n++) progr = progr->next;
							IIntuition->SetGadgetAttrs(gadintbank0, fenster, NULL, INTEGER_Number, progr->bank0, TAG_DONE);
							IIntuition->SetGadgetAttrs(gadintbank32, fenster, NULL, INTEGER_Number, progr->bank32, TAG_DONE);
							IIntuition->SetGadgetAttrs(gadintprog, fenster, NULL, INTEGER_Number, progr->prog, TAG_DONE);
							SendeKanalInstrument(port, channel, progr->bank0, progr->bank32, progr->prog);
							break;
							
							case 4:
							IIntuition->GetAttr(INTEGER_Number, (Object *)gadintbank0, &var); b0 = (int8)var;
							IIntuition->GetAttr(INTEGER_Number, (Object *)gadintbank32, &var); b32 = (int8)var;
							IIntuition->GetAttr(INTEGER_Number, (Object *)gadintprog, &var); pr = (int8)var;
							SendeKanalInstrument(port, channel, b0, b32, pr);
							break;
							
							case 5: // Abbruch
							schliessen = TRUE;
							break;

							case 6: // Nichts wählen
							*bank0 = -1; *bank32 = -1; *prog = -1;
							schliessen = TRUE;
							break;
							
							case 7: // Okay
							IIntuition->GetAttr(INTEGER_Number, (Object *)gadintbank0, &var); *bank0 = (int8)var;
							IIntuition->GetAttr(INTEGER_Number, (Object *)gadintbank32, &var); *bank32 = (int8)var;
							IIntuition->GetAttr(INTEGER_Number, (Object *)gadintprog, &var); *prog = (int8)var;
							schliessen = TRUE;
							break;
						}
					}
				}
			} while (!schliessen);
			if (progr) SetzeAktInstrument(port, channel, progr->bank0, progr->bank32, progr->prog);
		}

		IIntuition->DisposeObject(fensterobj);
	}

	EntferneListBrowserListe(&proglist);
	EntferneChooserListe(&katlist);
}

//-----------

void ErstelleControllerListe(struct List *contrlist, struct INSTRCONTR *instrcontr, BOOL extra, BOOL alle) {
	struct Node *node;
	uint32 n;
	char puf[5];
	
	if (extra) {
		node = IListBrowser->AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_Justification, LCJ_RIGHT,
			LBNCA_Text, "-",
			LBNA_Column, 1,
			LBNCA_Text, "Poly Press",
			LBNA_UserData, (APTR)-5,
			TAG_DONE);
		if (node) IExec->AddTail(contrlist, node);
		node = IListBrowser->AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_Justification, LCJ_RIGHT,
			LBNCA_Text, "-",
			LBNA_Column, 1,
			LBNCA_Text, "Chan Press",
			LBNA_UserData, (APTR)-2,
			TAG_DONE);
		if (node) IExec->AddTail(contrlist, node);
		node = IListBrowser->AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_Justification, LCJ_RIGHT,
			LBNCA_Text, "-",
			LBNA_Column, 1,
			LBNCA_Text, "Pitch Bend",
			LBNA_UserData, (APTR)-1,
			TAG_DONE);
		if (node) IExec->AddTail(contrlist, node);
	}
	for (n = 0; n < 128; n++) {
		node = NULL;
		sprintf(puf, "%ld", n);
		if (instrcontr->flags[n] & CONTR_SET) {
			node = IListBrowser->AllocListBrowserNode(2,
				LBNA_Column, 0,
				LBNCA_Justification, LCJ_RIGHT,
				LBNCA_CopyText, TRUE,
				LBNCA_Text, puf,
				LBNA_Column, 1,
				LBNCA_Text, instrcontr->name[n],
				LBNA_UserData, (APTR)n,
				TAG_DONE);
		} else if (alle) {
			node = IListBrowser->AllocListBrowserNode(2,
				LBNA_Column, 0,
				LBNCA_Justification, LCJ_RIGHT,
				LBNCA_CopyText, TRUE,
				LBNCA_Text, puf,
				LBNA_Column, 1,
				LBNA_Flags, LBFLG_CUSTOMPENS,
				LBNCA_FGPen, 2,
				LBNCA_Text, instrcontr->name[n],
				LBNA_UserData, (APTR)n,
				TAG_DONE);
		}
		if (node) IExec->AddTail(contrlist, node);
	}
}

int SucheControllerPosition(struct INSTRUMENT *instr, int contr, BOOL extra)
{
	int n;
	int wahl = extra ? 3 : 0;

	if (extra && contr < 0)
	{
		if (contr == -5) {
			return 0;
		} else if (contr == -2) {
			return 1;
		} else if (contr == -1) {
			return 2;
		}
	}

	for (n = 0; n < contr; n++) {
		if (instr->contr->flags[n] & CONTR_SET || contralle) {
			wahl++;
		}
	}
	return wahl;
}

int8 InstrControllerFenster(struct Window *fen, int8 channel, uint8 port, int8 vorwahl) {
	Object *fensterobj;
	struct Window *fenster;
	struct Gadget *gadliste;
	struct Gadget *gadabbruch;
	BOOL schliessen = FALSE;
	uint32 result;
	uint16 code;
	int32 var;
	struct List contrlist;
	struct INSTRUMENT *instr;
	int8 contr = -128;
	struct Node *node;
	char fenstername[300];

	instr = SucheChannelInstrument(port, channel);

	if (vorwahl >= 0 && !(instr->contr->flags[vorwahl] & CONTR_SET)) {
		contralle = TRUE;
	}

	IExec->NewList(&contrlist);
	ErstelleControllerListe(&contrlist, instr->contr, (vorwahl == -128), contralle);

	sprintf(fenstername, CAT(MSG_0483, "Controller Choice(%s)"), instr->name);
	fensterobj = WindowObject,
		WA_PubScreen, hschirm,
		WA_Title, fenstername,
		WA_Activate, TRUE,
		WA_DragBar, TRUE,
		WA_SizeGadget, TRUE,
		WA_SizeBBottom, TRUE,
		WA_Left, fen->LeftEdge + fen->MouseX,
		WA_Top, fen->TopEdge + fen->MouseY,
		WA_Width, 150,
		WA_Height, 280,
		WINDOW_ParentGroup, VLayoutObject,
			LAYOUT_SpaceOuter, TRUE,
			
			LAYOUT_AddChild, gadliste = (struct Gadget *)ListBrowserObject,
				GA_ID, 0, GA_RelVerify, TRUE,
				LISTBROWSER_Labels, &contrlist,
				LISTBROWSER_ColumnInfo, contrcolinfo,
				LISTBROWSER_AutoFit, TRUE,
				LISTBROWSER_ShowSelected, TRUE,
			End,
			
			LAYOUT_AddChild, CheckBoxObject,
				GA_ID, 3, GA_RelVerify, TRUE,
				GA_Text, CAT(MSG_0486, "Show All"),
				GA_Selected, contralle,
			End,
			CHILD_WeightedHeight, 0,
			
			LAYOUT_AddChild, HLayoutObject,
				LAYOUT_AddChild, gadabbruch = (struct Gadget *)ButtonObject, GA_RelVerify, TRUE, GA_ID, 1, GA_Text, CAT(MSG_0477, "Cancel"), End,
				LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, 2, GA_Text, CAT(MSG_0485, "Okay"), End,
			End,
			CHILD_WeightedHeight, 0,
		End,
	End;
	
	if (fensterobj) {
		fenster = (struct Window *)RA_OpenWindow(fensterobj);
		if (fenster) {

			if (vorwahl != -128) IIntuition->SetGadgetAttrs(gadabbruch, fenster, NULL, GA_Text, CAT(MSG_0478, "Nothing"), TAG_DONE);

			if (vorwahl >= 0) {
				int wahl = SucheControllerPosition(instr, vorwahl, FALSE);
				IIntuition->SetGadgetAttrs(gadliste, fenster, NULL,
					LISTBROWSER_Selected, wahl,
					LISTBROWSER_MakeVisible, wahl,
					TAG_DONE);
			}
			do {
				IExec->WaitPort(fenster->UserPort);
				while ((result = RA_HandleInput(fensterobj, &code)) != WMHI_LASTMSG) {
					switch (result & WMHI_CLASSMASK) {
						case WMHI_INACTIVE:
						schliessen = TRUE;
						if (vorwahl >= 0) contr = vorwahl;
						break;
			
						case WMHI_GADGETUP:
						switch (result & WMHI_GADGETMASK) {
							case 1:
							contr = -128;
							schliessen = TRUE;
							break;

							case 2:
							IIntuition->GetAttr(LISTBROWSER_SelectedNode, (Object *)gadliste, (uint32 *)&node);
							IListBrowser->GetListBrowserNodeAttrs(node, LBNA_UserData, &var, TAG_DONE);
							contr = (int8)var;
							schliessen = TRUE;
							break;
							
							case 3: {
								int wahl;
								int8 c;
								IIntuition->GetAttr(LISTBROWSER_SelectedNode, (Object *)gadliste, (uint32 *)&node);
								IListBrowser->GetListBrowserNodeAttrs(node, LBNA_UserData, &var, TAG_DONE);
								c = (int8)var;

								contralle = (BOOL)code;
								IIntuition->SetGadgetAttrs(gadliste, fenster, NULL, LISTBROWSER_Labels, NULL, TAG_DONE);
								EntferneListBrowserListe(&contrlist);
								ErstelleControllerListe(&contrlist, instr->contr, (vorwahl == -128), contralle);
								wahl = SucheControllerPosition(instr, c, (vorwahl == -128));
								IIntuition->SetGadgetAttrs(gadliste, fenster, NULL,
									LISTBROWSER_Labels, &contrlist,
									LISTBROWSER_AutoFit, TRUE,
									TAG_DONE);
								IIntuition->SetGadgetAttrs(gadliste, fenster, NULL,
									LISTBROWSER_Selected, wahl,
									LISTBROWSER_MakeVisible, wahl,
									TAG_DONE);

							} break;
						}
					}
				}
			} while (!schliessen);
		}

		IIntuition->DisposeObject(fensterobj);
	}
	
	EntferneListBrowserListe(&contrlist);
	return(contr);
}
