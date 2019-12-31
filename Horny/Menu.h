#include <exec/types.h>

#define MENU_UEBER 0
#define MENU_ENDE 1
#define MENU_NEU 10
#define MENU_LADEN 11
#define MENU_SPEICHERN 12
#define MENU_SPEICHERNALS 13
#define MENU_SEQ_LOESCHEN 20
#define MENU_SEQ_QUANT 21
#define MENU_SEQ_SCHNEIDEN 22
#define MENU_SEQ_VERBINDEN 23
#define MENU_SPR_LOESCHEN 30

void ErstelleMenu();
void EntferneMenu();
ULONG MenuPunkt(ULONG code);
