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
	WORD n;
	WORD lz;

	file = Open(datei, MODE_OLDFILE);
	if (file) {
		FGets(file, zeile, 256);
		if (strcmp(zeile, "HORNY INSTRUMENT CONTROLLER\n") != 0) {
			Close(file);
			return(NULL);
		}

		neu = AllocVec(sizeof(struct INSTRCONTR), 0);
		if (neu) {
			for (n = 0; n < 128; n++) {
				sprintf(neu->name[n], "(Contr. %d)", n);
				neu->flags[n] = 0;
			}
			
			while (FGets(file, zeile, 256)) {
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
		Close(file);
		return(neu);
	}
	return(NULL);
}


void NeuesProgramm(struct KATEGORIE *kat, BYTE bank0, BYTE bank32, BYTE prog, STRPTR name) {
	struct PROGRAMM *neu;
	struct PROGRAMM *akt;
	
	neu = AllocVec(sizeof(struct PROGRAMM), 0);
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

struct KATEGORIE *NeueKategorie(struct INSTRUMENT *instr, STRPTR name, BYTE chanvon, BYTE chanbis) {
	struct KATEGORIE *neu;
	struct KATEGORIE *akt;
	
	neu = AllocVec(sizeof(struct KATEGORIE), 0);
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
	BYTE prog = 0;
	BYTE bank0 = -1;
	BYTE bank32 = -1;
	BYTE chanvon = 0, chanbis = 15;
	WORD schwanz;
	char *leer;
	WORD von, bis;
	WORD n;
	STRPTR numname;
	char progname[128];
	
	neu = AllocVec(sizeof(struct INSTRUMENT), 0);
	if (neu) {
		strncpy(neu->name, name, 128);
		neu->kategorie = NULL;
		neu->next = NULL;
		
		
		strcpy(datei, "PROGDIR:System/Instruments/");
		strncat(datei, name, 512);
		file = Open(datei, MODE_OLDFILE);
		if (file) {
			FGets(file, zeile, 256);
			if (strcmp(zeile, "HORNY INSTRUMENT\n") != 0) {
				Close(file);
				FreeVec(neu);
				return(NULL);
			}

			while (FGets(file, zeile, 256)) {
				schwanz = strlen(zeile) - 1;
				if (zeile[schwanz] == '\n') zeile[schwanz] = 0;
				
				switch (zeile[0]) {
					case 0: break;
					
					case '*':
					kat = NeueKategorie(neu, &zeile[2], chanvon, chanbis);
					break;
					
					case '#':
					leer = strchr(zeile, ' ');
					if ((zeile[1] == 'B') && (zeile[5] != '3')) bank0 = (BYTE)atoi(leer); // #BANK0 x
					if ((zeile[1] == 'B') && (zeile[5] == '3')) bank32 = (BYTE)atoi(leer); // #BANK32 x
					if (zeile[1] == 'P') prog = (BYTE)atoi(leer); // #PROG x
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
	
			Close(file);
			
			strncat(datei, ".controller", 512);
			neu->contr = LadeInstrContr(datei);
			if (!neu->contr) neu->contr = LadeInstrContr("PROGDIR:System/Instruments/GM.controller");

			if (!rootinstrument) {
				rootinstrument = neu;
			} else {
				akt = rootinstrument;
				while (akt->next) akt = akt->next;
				akt->next = neu;
			}
		} else {
			FreeVec(neu);
		}
	}
	return(neu);
}

void ErstelleVorgabeInstrument(void) {
	struct KATEGORIE *kat;
	WORD prog;
	char progname[128];
	
	rootinstrument = AllocVec(sizeof(struct INSTRUMENT), 0);
	if (rootinstrument) {
		strcpy(rootinstrument->name, "???");
		rootinstrument->kategorie = NULL;
		rootinstrument->contr = LadeInstrContr("PROGDIR:System/Instruments/GM.controller");
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
	struct FileInfoBlock *fib = NULL;
	BPTR lock;

	fib = AllocVec(sizeof(struct FileInfoBlock), 0);
	if (fib)
	{
		ErstelleVorgabeInstrument();
		lock = Lock("PROGDIR:System/Instruments/", ACCESS_READ);
		if (lock) {
			if (Examine(lock, fib)) {
				while (ExNext(lock, fib)) {
					if (strchr(fib->fib_FileName, '.') == NULL) {
						LadeInstrument(fib->fib_FileName);
					}
				}
			}
			UnLock(lock);
		}
		FreeVec(fib);
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
				FreeVec(prog);
				prog = nextprog;
			}
			
			FreeVec(kat);
			kat = nextkat;
		}
		
		if (instr->contr) FreeVec(instr->contr);
		
		FreeVec(instr);
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

WORD SucheInstrumentNum(STRPTR name) {
	struct INSTRUMENT *instr;
	WORD n;
	
	instr = rootinstrument; n = 0;
	while (instr) {
		if (strcmp(instr->name, name) == 0) return(n);
		instr = instr->next; n++;
	}
	return(-1);
}

struct INSTRUMENT *NtesInstrument(WORD n) {
	struct INSTRUMENT *instr;
	WORD z;
	
	instr = rootinstrument;
	for (z = 0; z < n; z++) {
		if (instr->next) instr = instr->next;
	}
	return(instr);
}

BOOL SucheProgrammNum(struct INSTRUMENT *instr, BYTE channel, BYTE bank0, BYTE bank32, BYTE midiprog, WORD *katnum, WORD *prognum) {
	struct KATEGORIE *kat;
	struct PROGRAMM *prog;
	WORD zkat;
	WORD zprog;
	
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

struct INSTRUMENT *SucheChannelInstrument(UBYTE port, BYTE channel) {
	BYTE n;
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
	BYTE n;
	BYTE p;
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


void ErstelleKategorieListe(struct List *katlist, struct INSTRUMENT *instr, BYTE channel) {
	struct KATEGORIE *kat;
	struct Node *node;
	WORD n = 0;
	
	kat = instr->kategorie;
	while (kat) {
		if ((channel >= kat->chanvon) && (channel <= kat->chanbis)) {
			node = AllocChooserNode(CNA_Text, kat->name, CNA_UserData, (ULONG *)n, TAG_DONE);
			if (node) AddTail(katlist, node);
		}
		n++;
		kat = kat->next;
	}
}

WORD HoleChooserUserData(struct List *list, WORD num) {
	LONG userdata = 0;
	WORD n;
	struct Node *node;
	
	if (!IsListEmpty(list)) {
		node = list->lh_Head;
		for (n = 0; n < num; n++) node = node->ln_Succ;
		GetChooserNodeAttrs(node, CNA_UserData, &userdata, TAG_DONE);
	}
	return((WORD)userdata);
}

WORD WoIstChooserUserData(struct List *list, WORD userdata) {
	struct Node *node;
	LONG ud;
	WORD n = 0;
	
	node = list->lh_Head;
	while (node) {
		GetChooserNodeAttrs(node, CNA_UserData, &ud, TAG_DONE);
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
		node = AllocListBrowserNode(1, LBNCA_Text, prog->name, TAG_DONE);
		if (node) AddTail(proglist, node);
		prog = prog->next;
	}
}

void EntferneChooserListe(struct List *list) {
	struct Node *node;
	
	while (node = RemTail(list)) FreeChooserNode(node);
}

void EntferneListBrowserListe(struct List *list) {
	struct Node *node;
	
	while (node = RemTail(list)) FreeListBrowserNode(node);
}

void InstrumentenFenster2(struct Window *fen, BYTE channel, UBYTE port, BYTE *bank0, BYTE *bank32, BYTE *prog) {
	Object *fensterobj;
	struct Window *fenster;
	struct Gadget *gadkat;
	struct Gadget *gadprog;
	struct Gadget *gadintbank0;
	struct Gadget *gadintbank32;
	struct Gadget *gadintprog;
	BOOL schliessen = FALSE;
	ULONG result;
	UWORD code;
	ULONG var;
	struct List katlist;
	struct List proglist;
	struct INSTRUMENT *instr = NULL;
	struct KATEGORIE *kat = NULL;
	struct PROGRAMM *progr = NULL;
	WORD n;
	WORD katnum = 0;
	WORD prognum = -1;
	char fenstername[300];
	BYTE b0, b32, pr;
	
	NewList(&katlist);
	NewList(&proglist);
	
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
			
			LAYOUT_AddChild, gadkat = ChooserObject,
				GA_ID, 0, GA_RelVerify, TRUE,
				CHOOSER_PopUp, TRUE,
				CHOOSER_Labels, &katlist,
				CHOOSER_MaxLabels, 40,
			End,				
			CHILD_WeightedHeight, 0,
			
			LAYOUT_AddChild, gadprog = ListBrowserObject,
				GA_ID, 1, GA_RelVerify, TRUE,
				LISTBROWSER_Labels, &proglist,
				LISTBROWSER_ShowSelected, TRUE,
			End,
			
			LAYOUT_AddChild, VLayoutObject,

				LAYOUT_AddChild, HLayoutObject,
					LAYOUT_AddChild, gadintbank0 = IntegerObject,
						GA_ID, 2, GA_RelVerify, TRUE,
						INTEGER_Number, *bank0,
						INTEGER_Minimum, -1,
						INTEGER_Maximum, 127,
					End,
					CHILD_Label, LabelObject, LABEL_Text, "Bank", End,
		
					LAYOUT_AddChild, gadintbank32 = IntegerObject,
						GA_ID, 3, GA_RelVerify, TRUE,
						INTEGER_Number, *bank32,
						INTEGER_Minimum, -1,
						INTEGER_Maximum, 127,
					End,
				End,
	
				LAYOUT_AddChild, gadintprog = IntegerObject,
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
				SetGadgetAttrs(gadkat, fenster, NULL, CHOOSER_Selected, WoIstChooserUserData(&katlist, katnum), TAG_DONE);
				SetGadgetAttrs(gadprog, fenster, NULL,
					LISTBROWSER_Selected, prognum,
					LISTBROWSER_MakeVisible, prognum,
					TAG_DONE);
			}
			
			do {
				WaitPort(fenster->UserPort);
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
							SetGadgetAttrs(gadprog, fenster, NULL, LISTBROWSER_Labels, NULL, TAG_DONE);
							EntferneListBrowserListe(&proglist);
							ErstelleProgrammListe(&proglist, kat);
							SetGadgetAttrs(gadprog, fenster, NULL, LISTBROWSER_Labels, &proglist, TAG_DONE);
							break;
							
							case 1: // Programm
							progr = kat->programm;
							for (n = 0; n < code; n++) progr = progr->next;
							SetGadgetAttrs(gadintbank0, fenster, NULL, INTEGER_Number, progr->bank0, TAG_DONE);
							SetGadgetAttrs(gadintbank32, fenster, NULL, INTEGER_Number, progr->bank32, TAG_DONE);
							SetGadgetAttrs(gadintprog, fenster, NULL, INTEGER_Number, progr->prog, TAG_DONE);
							SendeKanalInstrument(port, channel, progr->bank0, progr->bank32, progr->prog);
							break;
							
							case 4:
							GetAttr(INTEGER_Number, gadintbank0, &var); b0 = (BYTE)var;
							GetAttr(INTEGER_Number, gadintbank32, &var); b32 = (BYTE)var;
							GetAttr(INTEGER_Number, gadintprog, &var); pr = (BYTE)var;
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
							GetAttr(INTEGER_Number, gadintbank0, &var); *bank0 = (BYTE)var;
							GetAttr(INTEGER_Number, gadintbank32, &var); *bank32 = (BYTE)var;
							GetAttr(INTEGER_Number, gadintprog, &var); *prog = (BYTE)var;
							schliessen = TRUE;
							break;
						}
					}
				}
			} while (!schliessen);
			if (progr) SetzeAktInstrument(port, channel, progr->bank0, progr->bank32, progr->prog);
		}

		DisposeObject(fensterobj);
	}

	EntferneListBrowserListe(&proglist);
	EntferneChooserListe(&katlist);
}

//-----------

void ErstelleControllerListe(struct List *contrlist, struct INSTRCONTR *instrcontr, BOOL extra, BOOL alle) {
	struct Node *node;
	WORD n;
	char puf[5];
	
	if (extra) {
		node = AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_Justification, LCJ_RIGHT,
			LBNCA_Text, "-",
			LBNA_Column, 1,
			LBNCA_Text, "Poly Press",
			LBNA_UserData, (APTR)-5,
			TAG_DONE);
		if (node) AddTail(contrlist, node);
		node = AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_Justification, LCJ_RIGHT,
			LBNCA_Text, "-",
			LBNA_Column, 1,
			LBNCA_Text, "Chan Press",
			LBNA_UserData, (APTR)-2,
			TAG_DONE);
		if (node) AddTail(contrlist, node);
		node = AllocListBrowserNode(2,
			LBNA_Column, 0,
			LBNCA_Justification, LCJ_RIGHT,
			LBNCA_Text, "-",
			LBNA_Column, 1,
			LBNCA_Text, "Pitch Bend",
			LBNA_UserData, (APTR)-1,
			TAG_DONE);
		if (node) AddTail(contrlist, node);
	}
	for (n = 0; n < 128; n++) {
		node = NULL;
		sprintf(puf, "%d", n);
		if (instrcontr->flags[n] & CONTR_SET) {
			node = AllocListBrowserNode(2,
				LBNA_Column, 0,
				LBNCA_Justification, LCJ_RIGHT,
				LBNCA_CopyText, TRUE,
				LBNCA_Text, puf,
				LBNA_Column, 1,
				LBNCA_Text, instrcontr->name[n],
				LBNA_UserData, (APTR)n,
				TAG_DONE);
		} else if (alle) {
			node = AllocListBrowserNode(2,
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
		if (node) AddTail(contrlist, node);
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

BYTE InstrControllerFenster(struct Window *fen, BYTE channel, UBYTE port, BYTE vorwahl) {
	Object *fensterobj;
	struct Window *fenster;
	struct Gadget *gadliste;
	struct Gadget *gadabbruch;
	BOOL schliessen = FALSE;
	ULONG result;
	UWORD code;
	LONG var;
	struct List contrlist;
	struct INSTRUMENT *instr;
	BYTE contr = -128;
	struct Node *node;
	char fenstername[300];

	instr = SucheChannelInstrument(port, channel);

	if (vorwahl >= 0 && !(instr->contr->flags[vorwahl] & CONTR_SET)) {
		contralle = TRUE;
	}

	NewList(&contrlist);
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
			
			LAYOUT_AddChild, gadliste = ListBrowserObject,
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
				LAYOUT_AddChild, gadabbruch = ButtonObject, GA_RelVerify, TRUE, GA_ID, 1, GA_Text, CAT(MSG_0477, "Cancel"), End,
				LAYOUT_AddChild, ButtonObject, GA_RelVerify, TRUE, GA_ID, 2, GA_Text, CAT(MSG_0485, "Okay"), End,
			End,
			CHILD_WeightedHeight, 0,
		End,
	End;
	
	if (fensterobj) {
		fenster = (struct Window *)RA_OpenWindow(fensterobj);
		if (fenster) {

			if (vorwahl != -128) SetGadgetAttrs(gadabbruch, fenster, NULL, GA_Text, CAT(MSG_0478, "Nothing"), TAG_DONE);

			if (vorwahl >= 0) {
				int wahl = SucheControllerPosition(instr, vorwahl, FALSE);
				SetGadgetAttrs(gadliste, fenster, NULL,
					LISTBROWSER_Selected, wahl,
					LISTBROWSER_MakeVisible, wahl,
					TAG_DONE);
			}
			do {
				WaitPort(fenster->UserPort);
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
							GetAttr(LISTBROWSER_SelectedNode, gadliste, (ULONG *)&node);
							GetListBrowserNodeAttrs(node, LBNA_UserData, &var, TAG_DONE);
							contr = (BYTE)var;
							schliessen = TRUE;
							break;
							
							case 3: {
								int wahl;
								BYTE c;
								GetAttr(LISTBROWSER_SelectedNode, gadliste, (ULONG *)&node);
								GetListBrowserNodeAttrs(node, LBNA_UserData, &var, TAG_DONE);
								c = (BYTE)var;

								contralle = (BOOL)code;
								SetGadgetAttrs(gadliste, fenster, NULL, LISTBROWSER_Labels, NULL, TAG_DONE);
								EntferneListBrowserListe(&contrlist);
								ErstelleControllerListe(&contrlist, instr->contr, (vorwahl == -128), contralle);
								wahl = SucheControllerPosition(instr, c, (vorwahl == -128));
								SetGadgetAttrs(gadliste, fenster, NULL,
									LISTBROWSER_Labels, &contrlist,
									LISTBROWSER_AutoFit, TRUE,
									TAG_DONE);
								SetGadgetAttrs(gadliste, fenster, NULL,
									LISTBROWSER_Selected, wahl,
									LISTBROWSER_MakeVisible, wahl,
									TAG_DONE);

							} break;
						}
					}
				}
			} while (!schliessen);
		}

		DisposeObject(fensterobj);
	}
	
	EntferneListBrowserListe(&contrlist);
	return(contr);
}
