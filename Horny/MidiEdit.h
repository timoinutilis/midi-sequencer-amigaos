#include <exec/types.h>

void AliaseAnpassen(struct SEQUENZ *original, WORD d);
void EventsVerschieben(struct SEQUENZ *seq, WORD d);
void OrdneEvents(struct SEQUENZ *seq);
void QuantisiereSequenz(struct SEQUENZ *seq, WORD quant);
void MarkSequenzenQuantisieren(WORD quant);
void PrintEvents(struct SEQUENZ *seq);
