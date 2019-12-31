#include <exec/types.h>

void InitGuiWerte(void);
void LoescheLinksOben(void);
void ZeichneUebersicht(void);
LONG TestePunktUebersicht(WORD x);
void ZeichneSpurAutomation(WORD s);
void ZeichneSpurSpalte(WORD s, BOOL aktiv);
void ZeichneSpuren(BOOL spalte, BOOL events);
void SpurenEinpassen(void);
WORD PunktSpur(WORD y);
BYTE PunktAutoWert(WORD spur, WORD y);
void ZeichneSequenzen(WORD s, BOOL events);
void ZeichneSequenzRahmen(WORD s);
LONG PunktPosition(WORD x);
void ZeichneZeitleiste(BOOL zahlen);
void ZeichneSmpteLeiste(void);
void ZeichneMarkerleisten(void);
void ZeichneMarkerleiste(BYTE typ);
void ZeichneInfobox(UBYTE sparten);
BYTE TestePunktInfo(WORD x, WORD y);
void ZeichneAnzeigen(BOOL tt);
BYTE TestePunktBereich(WORD x, WORD y);
