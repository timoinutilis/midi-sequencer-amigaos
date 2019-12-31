#include <proto/gadtools.h>
#include <proto/intuition.h>
#include <proto/exec.h>

#include "locale.h"

#include "Versionen.h"
#include "Menu.h"
#include "Requester.h"

struct Library *GadToolsBase = NULL;

#ifdef __amigaos4__
struct GadToolsIFace *IGadTools;
#endif

struct Menu *menu = NULL;
struct Menu *minmenu = NULL;
struct Menu *edmenu = NULL;
APTR vi = NULL;
UWORD liteignore;

extern struct Window *hfenster;
extern struct Window *edfenster;
extern struct Window *setfenster;
extern struct Window *envfenster;

void ErstelleMenuLibs(void) {
	if (!(GadToolsBase = OpenLibrary("gadtools.library", 0))) Meldung("gadtools.library nicht geöffnet");

#ifdef __amigaos4__
	IGadTools = (struct GadToolsIFace *)GetInterface(GadToolsBase, "main", 1, NULL);
#endif

	if (verLITE) liteignore = NM_ITEMDISABLED;
	else liteignore = 0;
}

void EntferneMenuLibs(void) {
#ifdef __amigaos4__
	DropInterface(IGadTools);
#endif
	CloseLibrary(GadToolsBase);
}

void ErstelleMenu(void) {
	struct NewMenu newmenu[] = {
		{NM_TITLE,	CAT(MSG_0247, "Program"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0248, "About..."), 0, 0, 0, (void *)MENU_UEBER},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0249, "Environment..."), "U", 0, 0, (void *)MENU_UMGEBUNG},
		{NM_ITEM,	CAT(MSG_0251, "Snapshot All Windows"), 0, 0, 0, (void *)MENU_FENSTERFIX},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0252, "Quit"), "Q", 0, 0, (void *)MENU_ENDE},
	
		{NM_TITLE,	CAT(MSG_0254, "Project"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0255, "New"), "N", 0, 0, (void *)MENU_NEU},
		{NM_ITEM,	CAT(MSG_0257, "Load..."), "L", 0, 0, (void *)MENU_LADEN},
		{NM_ITEM,	CAT(MSG_0259, "Import SMF..."), 0, 0, 0, (void *)MENU_IMPORTSMF},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0260, "Save"), "S", 0, 0, (void *)MENU_SPEICHERN},
		{NM_ITEM,	CAT(MSG_0262, "Save as..."), 0, 0, 0, (void *)MENU_SPEICHERNALS},
		{NM_ITEM,	CAT(MSG_0261, "Save as Start Project"), 0, 0, 0, (void *)MENU_SPEICHERNALSAUTOLOAD},
		{NM_ITEM,	CAT(MSG_0263, "Export SMF..."), 0, 0, 0, (void *)MENU_EXPORTSMF},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0264, "Settings..."), "E", 0, 0, (void *)MENU_PROJEINST},
		{NM_ITEM,	CAT(MSG_0266, "SysEx Manager..."), "Y", 0, 0, (void *)MENU_SYSEX},
		{NM_ITEM,	CAT(MSG_0268, "Mixer..."), "X", 0, 0, (void *)MENU_MISCHPULT},
		{NM_ITEM,	CAT(MSG_0269, "Controller Transformer..."), 0, 0, 0, (void *)MENU_CTRLCHANGE},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0270, "Send Programs"), 0, 0, 0, (void *)MENU_PROGSENDEN},
		{NM_ITEM,	CAT(MSG_0271, "Send All SysEx"), 0, 0, 0, (void *)MENU_SYSEXSENDEN},
		{NM_ITEM,	CAT(MSG_0272, "Send Mix"), 0, 0, 0, (void *)MENU_MIXSENDEN},
		{NM_ITEM,	CAT(MSG_0273, "Send All Notes Off"), "-", 0, 0, (void *)MENU_ALLOFFSENDEN},
		
		{NM_TITLE,	CAT(MSG_0275, "Track"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0276, "Add"), "H", 0, 0, (void *)MENU_SPR_HINZU},
		{NM_ITEM,	CAT(MSG_0278, "Duplicate"), "P", 0, 0, (void *)MENU_SPR_DUPLIZIEREN},
		{NM_ITEM,	CAT(MSG_0280, "Delete"), "D", 0, 0, (void *)MENU_SPR_LOESCHEN},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0282, "Mute On/Off"), "M", 0, 0, (void *)MENU_SPR_MUTEN},
		{NM_ITEM,	CAT(MSG_0284, "Solo"), ",", 0, 0, (void *)MENU_SPR_SOLO},
		{NM_ITEM,	CAT(MSG_0286, "All Mutes Off"), ".", 0, 0, (void *)MENU_SPR_MUTESAUS},

		{NM_TITLE,	CAT(MSG_0288, "Track Automation"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0289, "Volume"), "1", 0, 0, (void *)MENU_SPR_AUTO_VOL},
		{NM_ITEM,	CAT(MSG_0290, "Panorama"), "2", 0, 0, (void *)MENU_SPR_AUTO_PAN},
		{NM_ITEM,	CAT(MSG_0291, "Controller 1"), "3", 0, 0, (void *)MENU_SPR_AUTO_CTRL1},
		{NM_ITEM,	CAT(MSG_0292, "Controller 2"), "4", 0, 0, (void *)MENU_SPR_AUTO_CTRL2},
		{NM_ITEM,	CAT(MSG_0293, "Controller 3"), "5", 0, 0, (void *)MENU_SPR_AUTO_CTRL3},
		{NM_ITEM,	CAT(MSG_0294, "Controller 4"), "6", liteignore, 0, (void *)MENU_SPR_AUTO_CTRL4},
		{NM_ITEM,	CAT(MSG_0295, "Controller 5"), "7", liteignore, 0, (void *)MENU_SPR_AUTO_CTRL5},
		{NM_ITEM,	CAT(MSG_0296, "Controller 6"), "8", liteignore, 0, (void *)MENU_SPR_AUTO_CTRL6},
		{NM_ITEM,	CAT(MSG_0297, "Hide Automation"), "0", 0, 0, (void *)MENU_SPR_AUTO_HIDE},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0304, "Copy"), 0, 0, 0, (void *)MENU_SPR_AUTO_COPY},
		{NM_ITEM,	CAT(MSG_0306, "Paste"), 0, 0, 0, (void *)MENU_SPR_AUTO_PASTE},
		{NM_ITEM,	CAT(MSG_0308, "Delete"), 0, 0, 0, (void *)MENU_SPR_AUTO_DEL},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0298A, "Convert to Sequence"), 0, 0, 0, (void *)MENU_SPR_AUTO_A2C},
		{NM_ITEM,	CAT(MSG_0298B, "Get from Sequences"), 0, 0, 0, (void *)MENU_SPR_AUTO_C2A},

		{NM_TITLE,	CAT(MSG_0299, "Sequence(s)"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0300, "Mark"), 0, 0, 0, 0},
		{NM_SUB,		CAT(MSG_0301, "All in Active Track"), 0, 0, 0, (void *)MENU_SEQ_MARK_ALLESPUR},
		{NM_SUB,		CAT(MSG_0302, "All in Song"), 0, 0, 0, (void *)MENU_SEQ_MARK_ALLELIED},
		{NM_SUB,		CAT(MSG_0303, "All From Edit Position"), 0, 0, 0, (void *)MENU_SEQ_MARK_EDITPOS},
		{NM_ITEM,	CAT(MSG_0303A, "Set Loop around Sequence(s)"), "A", 0, 0, (void *)MENU_SEQ_SETZELOOP},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0304, "Copy"), "C", 0, 0, (void *)MENU_SEQ_COPY},
		{NM_ITEM,	CAT(MSG_0306, "Paste"), "V", 0, 0, (void *)MENU_SEQ_PASTE},
		{NM_ITEM,	CAT(MSG_0308, "Delete"), "Del", NM_COMMANDSTRING, 0, (void *)MENU_SEQ_LOESCHEN},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0310, "Quantize..."), "O", 0, 0, (void *)MENU_SEQ_QUANT},
		{NM_ITEM,	CAT(MSG_0312, "Split"), "I", 0, 0, (void *)MENU_SEQ_SCHNEIDEN},
		{NM_ITEM,	CAT(MSG_0314, "Subdivide"), "T", 0, 0, (void *)MENU_SEQ_UNTERTEILEN},
		{NM_ITEM,	CAT(MSG_0316, "Merge"), "J", 0, 0, (void *)MENU_SEQ_VERBINDEN},
		{NM_ITEM,	CAT(MSG_0318, "Alias to Real"), "R", 0, 0, (void *)MENU_SEQ_ALIASREAL},
	
		{NM_END, 0, 0, 0, 0, 0}
	};

	struct NewMenu newminmenu[] = {
		{NM_TITLE,	CAT(MSG_0247, "Program"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0248, "About..."), 0, 0, 0, (void *)MENU_UEBER},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0249, "Environment..."), "U", 0, 0, (void *)MENU_UMGEBUNG},
		{NM_ITEM,	CAT(MSG_0251, "Snapshot All Windows"), 0, 0, 0, (void *)MENU_FENSTERFIX},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0252, "Quit"), "Q", 0, 0, (void *)MENU_ENDE},
	
		{NM_TITLE,	CAT(MSG_0254, "Project"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0255, "New"), "N", 0, 0, (void *)MENU_NEU},
		{NM_ITEM,	CAT(MSG_0257, "Load..."), "L", 0, 0, (void *)MENU_LADEN},
		{NM_ITEM,	CAT(MSG_0259, "Import SMF..."), 0, 0, 0, (void *)MENU_IMPORTSMF},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0260, "Save"), "S", 0, 0, (void *)MENU_SPEICHERN},
		{NM_ITEM,	CAT(MSG_0262, "Save as..."), 0, 0, 0, (void *)MENU_SPEICHERNALS},
		{NM_ITEM,	CAT(MSG_0261, "Save as Start Project"), 0, 0, 0, (void *)MENU_SPEICHERNALSAUTOLOAD},
		{NM_ITEM,	CAT(MSG_0263, "Export SMF..."), 0, 0, 0, (void *)MENU_EXPORTSMF},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0264, "Settings..."), "E", 0, 0, (void *)MENU_PROJEINST},
		{NM_ITEM,	CAT(MSG_0266, "SysEx Manager..."), "Y", 0, 0, (void *)MENU_SYSEX},
		{NM_ITEM,	CAT(MSG_0268, "Mixer..."), "X", 0, 0, (void *)MENU_MISCHPULT},
		{NM_ITEM,	CAT(MSG_0269, "Controller Transformer..."), 0, 0, 0, (void *)MENU_CTRLCHANGE},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0270, "Send Programs"), 0, 0, 0, (void *)MENU_PROGSENDEN},
		{NM_ITEM,	CAT(MSG_0271, "Send All SysEx"), 0, 0, 0, (void *)MENU_SYSEXSENDEN},
		{NM_ITEM,	CAT(MSG_0272, "Send Mix"), 0, 0, 0, (void *)MENU_MIXSENDEN},
		{NM_ITEM,	CAT(MSG_0273, "Send All Notes Off"), "-", 0, 0, (void *)MENU_ALLOFFSENDEN},
	
		{NM_END, 0, 0, 0, 0, 0}
	};

	struct NewMenu newedmenu[] = {
		{NM_TITLE,	CAT(MSG_0247, "Program"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0248, "About..."), 0, 0, 0, (void *)MENU_UEBER},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0249, "Environment..."), "U", 0, 0, (void *)MENU_UMGEBUNG},
		{NM_ITEM,	CAT(MSG_0251, "Snapshot All Windows"), 0, 0, 0, (void *)MENU_FENSTERFIX},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0252, "Quit"), "Q", 0, 0, (void *)MENU_ENDE},
	
		{NM_TITLE,	CAT(MSG_0254, "Project"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0255, "New"), "N", 0, 0, (void *)MENU_NEU},
		{NM_ITEM,	CAT(MSG_0257, "Load..."), "L", 0, 0, (void *)MENU_LADEN},
		{NM_ITEM,	CAT(MSG_0259, "Import SMF..."), 0, 0, 0, (void *)MENU_IMPORTSMF},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0260, "Save"), "S", 0, 0, (void *)MENU_SPEICHERN},
		{NM_ITEM,	CAT(MSG_0262, "Save as..."), 0, 0, 0, (void *)MENU_SPEICHERNALS},
		{NM_ITEM,	CAT(MSG_0261, "Save as Start Project"), 0, 0, 0, (void *)MENU_SPEICHERNALSAUTOLOAD},
		{NM_ITEM,	CAT(MSG_0263, "Export SMF..."), 0, 0, 0, (void *)MENU_EXPORTSMF},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0264, "Settings..."), "E", 0, 0, (void *)MENU_PROJEINST},
		{NM_ITEM,	CAT(MSG_0266, "SysEx Manager..."), "Y", 0, 0, (void *)MENU_SYSEX},
		{NM_ITEM,	CAT(MSG_0268, "Mixer..."), "X", 0, 0, (void *)MENU_MISCHPULT},
		{NM_ITEM,	CAT(MSG_0269, "Controller Transformer..."), 0, 0, 0, (void *)MENU_CTRLCHANGE},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0270, "Send Programs"), 0, 0, 0, (void *)MENU_PROGSENDEN},
		{NM_ITEM,	CAT(MSG_0271, "Send All SysEx"), 0, 0, 0, (void *)MENU_SYSEXSENDEN},
		{NM_ITEM,	CAT(MSG_0272, "Send Mix"), 0, 0, 0, (void *)MENU_MIXSENDEN},
		{NM_ITEM,	CAT(MSG_0273, "Send All Notes Off"), "-", 0, 0, (void *)MENU_ALLOFFSENDEN},
	
		{NM_TITLE,	CAT(MSG_0376, "Edit"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0377, "Undo"), "Z", 0, 0, (void *)MENU_EDIT_UNDO},
		{NM_ITEM,	CAT(MSG_0379, "Redo"), 0, 0, 0, (void *)MENU_EDIT_REDO},
	
		{NM_TITLE,	CAT(MSG_0380, "Note(s)"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0381, "Mark"), 0, 0, 0, 0},
		{NM_SUB,		CAT(MSG_0382, "All"), "M", 0, 0, (void *)MENU_EV_MARK_ALLE},
		{NM_SUB,		CAT(MSG_0383, "Higher and same Notes"), 0, 0, 0, (void *)MENU_EV_MARK_HOEHERE},
		{NM_SUB,		CAT(MSG_0384, "Lower and same Notes"), 0, 0, 0, (void *)MENU_EV_MARK_TIEFERE},
		{NM_SUB,		CAT(MSG_0385, "Quieter Notes"), 0, 0, 0, (void *)MENU_EV_MARK_ANSCHLAEGE},
		{NM_SUB,		CAT(MSG_0386, "All with same Channel"), 0, 0, 0, (void *)MENU_EV_MARK_KANAL},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0387, "Delete"), "Del", NM_COMMANDSTRING, 0, (void *)MENU_EV_LOESCHEN},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0389, "Quantize"), 0, 0, 0, 0},
		{NM_SUB,		CAT(MSG_0390, "Starts"), "O", 0, 0, (void *)MENU_EV_QUANT_ON},
		{NM_SUB,		CAT(MSG_0392, "Ends"), 0, 0, 0, (void *)MENU_EV_QUANT_OFF},
		{NM_SUB,		CAT(MSG_0393, "Starts Nearer"), "9", 0, 0, (void *)MENU_EV_QUANT_ONNEAR},
		{NM_ITEM,	CAT(MSG_0395, "Velocity Compressor..."), "A", 0, 0, (void *)MENU_EV_DYNAMIK},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0397, "Repair Notes"), "P", 0, 0, (void *)MENU_EV_REPARIEREN},
		
		{NM_TITLE,	CAT(MSG_0399, "Controller"), 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0400, "Nothing Marked"), "I", 0, 0, (void *)MENU_EV_MARK_NICHTS},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0402, "Add..."), "H", 0, 0, (void *)MENU_EV_CONTRHINZU},
		{NM_ITEM,	CAT(MSG_0404, "Delete"), "Del", NM_COMMANDSTRING, 0, (void *)MENU_EV_LOESCHEN},
		{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
		{NM_ITEM,	CAT(MSG_0406, "Reduce..."), "R", 0, 0, (void *)MENU_EV_REDUZIEREN},
		{NM_ITEM,	CAT(MSG_0408, "Compressor..."), "K", 0, 0, (void *)MENU_EV_DYNAMIK},
		{NM_ITEM,	CAT(MSG_0410, "Smooth"), "G", 0, 0, (void *)MENU_EV_GLAETTEN},

		{NM_END, 0, 0, 0, 0, 0}
	};


	vi = GetVisualInfoA(hfenster->WScreen, NULL);
	menu = CreateMenus(newmenu, GTMN_FullMenu, TAG_DONE);
	LayoutMenus(menu, vi, GTMN_NewLookMenus, GTMN_FrontPen, 1, TAG_DONE);
	SetMenuStrip(hfenster, menu);

	minmenu = CreateMenus(newminmenu, GTMN_FullMenu, TAG_DONE);
	LayoutMenus(minmenu, vi, GTMN_NewLookMenus, GTMN_FrontPen, 1, TAG_DONE);

	edmenu = CreateMenus(newedmenu, GTMN_FullMenu, TAG_DONE);
	LayoutMenus(edmenu, vi, GTMN_NewLookMenus, GTMN_FrontPen, 1, TAG_DONE);
}

void EntferneMenu(void) {
	ClearMenuStrip(hfenster);
	FreeMenus(menu);
	FreeMenus(minmenu);
	FreeMenus(edmenu);
	FreeVisualInfo(vi);
}

ULONG MenuPunkt(ULONG code) {
	struct MenuItem *item;

	item = ItemAddress(menu, code);
	if (item) {
		return((ULONG)GTMENUITEM_USERDATA(item));
	} else return(~0);
}

void MenuDeaktivieren(BYTE typ, BYTE num, BOOL status) {
	struct Menu *m = NULL;
	BYTE n;
	
	if (typ == 0) m = menu; else m = edmenu;
	if (m) {
		for (n = 0; n < num; n++) m = m->NextMenu;
		if (status) m->Flags &= !MENUENABLED;
		else m->Flags |= MENUENABLED;
	}
}

void MenuItemDeaktivieren(BYTE typ, UWORD code, BOOL status) {
	struct Menu *m = NULL;
	struct MenuItem *item;
	
	if (typ == 0) m = menu; else m = edmenu;
	if (m) {
		item = ItemAddress(m, code);
		if (item) {
//			if (status) item->Flags &= !ITEMENABLED;
//			else item->Flags |= ITEMENABLED;
		}
	}
}

ULONG EdMenuPunkt(ULONG code) {
	struct MenuItem *item;

	item = ItemAddress(edmenu, code);
	if (item) {
		return((ULONG)GTMENUITEM_USERDATA(item));
	} else return(~0);
}

ULONG MinMenuPunkt(ULONG code) {
	struct MenuItem *item;

	item = ItemAddress(minmenu, code);
	if (item) {
		return((ULONG)GTMENUITEM_USERDATA(item));
	} else return(~0);
}

