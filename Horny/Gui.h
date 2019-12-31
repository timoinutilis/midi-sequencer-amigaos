#include <exec/types.h>

void ErstelleHauptfenster(void);
void EntferneHauptfenster(void);
void SetzeFont();
void Fett(BOOL f);
void ErstelleGadgets(void);
void EntferneGadgets(void);
void AktualisiereGadgets(void);
void BildFrei(void);
void Schreibe(UBYTE f, WORD x, WORD y, STRPTR t, WORD xe);
void SchreibeZahl(UBYTE f, WORD x, WORD y, WORD z);
void RahmenEin(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void RahmenAus(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void Balken(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void Linie(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void Farbe(UBYTE f);
void Punkt(WORD x, WORD y);
void ZeichnePosition(LONG zeit);
void KeinePosition(void);
LONG PunktPosition(WORD x);
