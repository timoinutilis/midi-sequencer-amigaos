#include <exec/types.h>

void AliaseAnpassen(struct SEQUENZ *original, int16 d);
void EventsVerschieben(struct SEQUENZ *seq, int16 d);
void OrdneEvents(struct SEQUENZ *seq);
void NotenEndenMarkieren(struct SEQUENZ *seq);
void QuantisiereSequenz(struct SEQUENZ *seq, int16 quant);
void MarkSequenzenQuantisieren(int16 quant);
void MarkEventsEntfernen(struct SEQUENZ *seq);
struct EVENT *EventEinfuegen(struct SEQUENZ *seq, int32 t, int8 status, int8 data1, int8 data2, BOOL markieren);
void MarkEventsKopieren(struct SEQUENZ *seq);
void MarkNotenVerschieben(struct SEQUENZ *seq, int32 d, int16 h);
void MarkNotenEndenVerschieben(struct SEQUENZ *seq, int32 d);
void MarkEventsDynamik(struct SEQUENZ *seq, int8 thresh, int8 ratio, int8 gain);
void MarkNotenQuantisieren(struct SEQUENZ *seq, int16 quant, int8 modus, BOOL tripled);
void NotenMarkieren(struct SEQUENZ *seq, int8 modus, uint8 referenz);
struct EVENT *TaktContr(int32 t, int8 contr, struct SEQUENZ *seq, struct EVENT **fcev);
struct EVENT *TaktNote(int32 t, uint8 note, struct SEQUENZ *seq, int8 *p);
void KeineEventsMarkieren(struct SEQUENZ *seq);
void NotenBereichMarkieren(struct SEQUENZ *seq, int16 vtast, int16 btast, int32 vt, int32 bt);
void ControllerMarkieren(struct SEQUENZ *seq, int8 contr);
void MarkContrReduzieren(struct SEQUENZ *seq, int16 quant);
void MarkContrGlaetten(struct SEQUENZ *seq);
void RepariereNoten(struct SEQUENZ *seq);
int8 ZerschneideSequenzNoten(struct SEQUENZ *seq, int32 t, int8 trennart);
