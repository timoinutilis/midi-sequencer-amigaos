#include <exec/types.h>

struct SEQUENZ *NeueSequenzEinordnen(WORD s);
void SpurSequenzenEntfernen(WORD s);
struct SEQUENZ *TaktSequenz(WORD s, LONG t, BYTE *p);
void MarkSequenzenVerschieben(WORD s, WORD sd, LONG d);
void MarkSequenzenEntfernen(void);
void MarkSequenzenAlias(void);
void MarkSequenzenKopieren(void);
void NichtsMarkieren(void);
void SpurenTauschen(WORD s1, WORD s2);
BOOL SpurLoeschen(WORD s);
void MarkSequenzenZerschneiden(LONG tp);
void MarkSequenzenVerbinden(void);
void MarkSequenzenStartVerschieben(WORD d);
void MarkSequenzenEndeVerschieben(WORD d);
