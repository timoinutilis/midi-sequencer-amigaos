#include <exec/types.h>

void InitAutokanaele(void);
void KanalSpurenBearbeitet(BYTE p, BYTE c);
void AutomationKopieren(BYTE p, BYTE c, BYTE num);
void AutomationEinfuegen(BYTE p, BYTE c, BYTE num);
void EntferneAutomationsKopie(void);
struct AUTOPUNKT *NeuerAutoPunkt(BYTE p, BYTE c, BYTE num, LONG t, BYTE wert);
void EntferneAutoPunkt(BYTE p, BYTE c, BYTE num, struct AUTOPUNKT *punkt);
void EntferneAlleAutoPunkte(BYTE p, BYTE c, BYTE num);
struct AUTOPUNKT *TaktAutoPunkt(BYTE p, BYTE c, BYTE num, LONG t);
void AutomationVorbereiten(BYTE p, BYTE c, LONG t);
void LoopAutomationVorbereiten(BYTE p, BYTE c, LONG t);
void LoopAutomationResetten(BYTE p, BYTE c);
void SpieleAutomation(BYTE p, BYTE c, LONG t);
void KonvertiereContrZuAuto(WORD s);
void KonvertiereAutoZuContr(WORD s);
