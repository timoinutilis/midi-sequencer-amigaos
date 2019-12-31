#include <libraries/gadtools.h>
#include <intuition/intuition.h>

#include <proto/gadtools.h>
#include <proto/intuition.h>

#include "Menu.h"

struct Menu *menu=NULL;
APTR vi=NULL;

extern struct Window *hfenster;

struct NewMenu menue[]={
	{NM_TITLE,	"Programm", 0, 0, 0, 0},
	{NM_ITEM,	"Über...", 0, 0, 0, (void *)MENU_UEBER},
	{NM_ITEM,	"Beenden", 0, 0, 0, (void *)MENU_ENDE},

	{NM_TITLE,	"Projekt", 0, 0, 0, 0},
	{NM_ITEM,	"Neu", "N", 0, 0, (void *)MENU_NEU},
	{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
	{NM_ITEM,	"Laden...", "L", 0, 0, (void *)MENU_LADEN},
	{NM_ITEM,	"Speichern", "S", 0, 0, (void *)MENU_SPEICHERN},
	{NM_ITEM,	"Speichern als...", 0, 0, 0, (void *)MENU_SPEICHERNALS},

	{NM_TITLE,	"Spur", 0, 0, 0, 0},
	{NM_ITEM,	"Löschen", "D", 0, 0, (void *)MENU_SPR_LOESCHEN},

	{NM_TITLE,	"Sequenz(en)", 0, 0, 0, 0},
	{NM_ITEM,	"Löschen", "Del", NM_COMMANDSTRING, 0, (void *)MENU_SEQ_LOESCHEN},
	{NM_ITEM,	NM_BARLABEL, 0, 0, 0, 0},
	{NM_ITEM,	"Quantisieren...", "Q", 0, 0, (void *)MENU_SEQ_QUANT},
	{NM_ITEM,	"Zerschneiden", "C", 0, 0, (void *)MENU_SEQ_SCHNEIDEN},
	{NM_ITEM,	"Verbinden", "J", 0, 0, (void *)MENU_SEQ_VERBINDEN},

	{NM_END, 0, 0, 0, 0, 0}
};

void ErstelleMenu() {
	vi=GetVisualInfoA(hfenster->WScreen, NULL);
	menu=CreateMenus(menue, GTMN_FullMenu, TAG_DONE);
	LayoutMenus(menu, vi, GTMN_NewLookMenus, GTMN_FrontPen, 1, TAG_DONE);
	SetMenuStrip(hfenster, menu);
}

void EntferneMenu() {
	ClearMenuStrip(hfenster);
	FreeMenus(menu);
	FreeVisualInfo(vi);
}

ULONG MenuPunkt(ULONG code) {
	struct MenuItem *item;

	item=ItemAddress(menu, code);
	return((ULONG)GTMENUITEM_USERDATA(item));
}
