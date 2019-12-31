#include <exec/types.h>

void InitSpur(WORD s);

void AktualisiereSpuren(BOOL spalten);
BOOL SpurInSicht(WORD s);
void SpurAktivieren(WORD s);
void SpurScroll(WORD ds);
void NeueSpur(void);
void SpurLoeschen(WORD s);
void SpurVerschieben(WORD s1, WORD s2);
void SpurDuplizieren(WORD s);

void SpurMuteSchalter(WORD s);
void SpurSolo(WORD s);
void SpurenMutesAus(void);
