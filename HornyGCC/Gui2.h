#include <exec/types.h>

void InitGuiWerte(void);
void LoescheLinksOben(void);
void ZeichneUebersicht(void);
int32 TestePunktUebersicht(int16 x);
void ZeichneSpurAutomation(int16 s);
void ZeichneSpurSpalte(int16 s, BOOL aktiv);
void ZeichneSpuren(BOOL spalte, BOOL events);
void SpurenEinpassen(void);
int16 PunktSpur(int16 y);
int8 PunktAutoWert(int16 spur, int16 y);
void ZeichneSequenzen(int16 s, BOOL events);
void ZeichneSequenzRahmen(int16 s);
int32 PunktPosition(int16 x);
void ZeichneZeitleiste(BOOL zahlen);
void ZeichneSmpteLeiste(void);
void ZeichneMarkerleisten(void);
void ZeichneMarkerleiste(int8 typ);
void ZeichneInfobox(uint8 sparten);
int8 TestePunktInfo(int16 x, int16 y);
void ZeichneAnzeigen(BOOL tt);
int8 TestePunktBereich(int16 x, int16 y);
