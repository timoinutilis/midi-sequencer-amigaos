#include <exec/types.h>

void InitSpur(int16 s);

void AktualisiereSpuren(BOOL spalten);
BOOL SpurInSicht(int16 s);
void SpurAktivieren(int16 s);
void SpurScroll(int16 ds);
void NeueSpur(void);
void SpurLoeschen(int16 s);
void SpurVerschieben(int16 s1, int16 s2);
void SpurDuplizieren(int16 s);

void SpurMuteSchalter(int16 s);
void SpurSolo(int16 s);
void SpurenMutesAus(void);
