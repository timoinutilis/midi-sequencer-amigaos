#include <exec/types.h>

void InitGuiWerte(void);
void ZeichneSpuren(WORD snum, BOOL spa, BOOL seq);
WORD PunktSpur(WORD y);
void ZeichneSequenzen(WORD s);
void ZeichneZeitleiste(void);
void ZeichneInfobox(WORD snum, struct SEQUENZ *seq);
BYTE TestePunktInfo(WORD x, WORD y);
BYTE TestePunktBereich(WORD x, WORD y);
