#include <exec/types.h>

void AliaseAnpassen(struct SEQUENZ *original, WORD d);
void EventsVerschieben(struct SEQUENZ *seq, WORD d);
void OrdneEvents(struct SEQUENZ *seq);
void NotenEndenMarkieren(struct SEQUENZ *seq);
void QuantisiereSequenz(struct SEQUENZ *seq, WORD quant);
void MarkSequenzenQuantisieren(WORD quant);
void MarkEventsEntfernen(struct SEQUENZ *seq);
struct EVENT *EventEinfuegen(struct SEQUENZ *seq, LONG t, BYTE status, BYTE data1, BYTE data2, BOOL markieren);
void MarkEventsKopieren(struct SEQUENZ *seq);
void MarkNotenVerschieben(struct SEQUENZ *seq, LONG d, WORD h);
void MarkNotenEndenVerschieben(struct SEQUENZ *seq, LONG d);
void MarkEventsDynamik(struct SEQUENZ *seq, BYTE thresh, BYTE ratio, BYTE gain);
void MarkNotenQuantisieren(struct SEQUENZ *seq, WORD quant, BYTE modus, BOOL tripled);
void NotenMarkieren(struct SEQUENZ *seq, BYTE modus, UBYTE referenz);
struct EVENT *TaktContr(LONG t, BYTE contr, struct SEQUENZ *seq, struct EVENT **fcev);
struct EVENT *TaktNote(LONG t, UBYTE note, struct SEQUENZ *seq, BYTE *p);
void KeineEventsMarkieren(struct SEQUENZ *seq);
void NotenBereichMarkieren(struct SEQUENZ *seq, WORD vtast, WORD btast, LONG vt, LONG bt);
void ControllerMarkieren(struct SEQUENZ *seq, BYTE contr);
void MarkContrReduzieren(struct SEQUENZ *seq, WORD quant);
void MarkContrGlaetten(struct SEQUENZ *seq);
void RepariereNoten(struct SEQUENZ *seq);
BYTE ZerschneideSequenzNoten(struct SEQUENZ *seq, LONG t, BYTE trennart);
