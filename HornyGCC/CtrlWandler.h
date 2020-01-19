#include <exec/types.h>

struct CTRLCHANGE {
	char name[128];
	int8 original;
	int8 ziel;
	BOOL aktiv;
	struct CTRLCHANGE *next;
};

struct CTRLCHANGE *AddChangeCtrl(STRPTR name);
void EntferneChangeCtrl(struct CTRLCHANGE *cc);
void EntferneAlleChangeCtrl(void);
int8 WandleController(int8 data1);

void ErstelleChangeCtrlFenster(void);
void EntferneChangeCtrlFenster(void);
void KontrolleChangeCtrlFenster(void);
