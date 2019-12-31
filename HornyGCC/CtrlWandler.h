#include <exec/types.h>

struct CTRLCHANGE {
	char name[128];
	BYTE original;
	BYTE ziel;
	BOOL aktiv;
	struct CTRLCHANGE *next;
};

struct CTRLCHANGE *AddChangeCtrl(STRPTR name);
void EntferneChangeCtrl(struct CTRLCHANGE *cc);
void EntferneAlleChangeCtrl(void);
BYTE WandleController(BYTE data1);

void ErstelleChangeCtrlFenster(void);
void EntferneChangeCtrlFenster(void);
void KontrolleChangeCtrlFenster(void);
