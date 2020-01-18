#include <exec/types.h>

void InitAutokanaele(void);
void KanalSpurenBearbeitet(int8 p, int8 c);
void AutomationKopieren(int8 p, int8 c, int8 num);
void AutomationEinfuegen(int8 p, int8 c, int8 num);
void EntferneAutomationsKopie(void);
struct AUTOPUNKT *NeuerAutoPunkt(int8 p, int8 c, int8 num, int32 t, int8 wert);
void EntferneAutoPunkt(int8 p, int8 c, int8 num, struct AUTOPUNKT *punkt);
void EntferneAlleAutoPunkte(int8 p, int8 c, int8 num);
struct AUTOPUNKT *TaktAutoPunkt(int8 p, int8 c, int8 num, int32 t);
void AutomationVorbereiten(int8 p, int8 c, int32 t);
void LoopAutomationVorbereiten(int8 p, int8 c, int32 t);
void LoopAutomationResetten(int8 p, int8 c);
void SpieleAutomation(int8 p, int8 c, int32 t);
void KonvertiereContrZuAuto(int16 s);
void KonvertiereAutoZuContr(int16 s);
