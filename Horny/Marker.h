#include <exec/types.h>

struct MARKER *NeuerMarker(BYTE typ, LONG t, WORD d1, WORD d2);
void EntferneMarker(struct MARKER *mark);
void EntferneAlleMarker(void);
struct MARKER *TaktDirektMarker(LONG t);
struct MARKER *TaktMarker(struct MARKER *start, BYTE typ, LONG t);
struct MARKER *NextMarker(struct MARKER *akt);
void MarkerTauschen(struct MARKER *mark1, struct MARKER *mark2);
void MarkerSortieren(void);
void ErstelleGrundMarker(void);
void TaktWahlMark(LONG t);

LONG TaktZeit(LONG t);
LONG TaktSmpteTicks(LONG t);
LONG SmpteTicksTakt(LONG ticks);
void SmpteTicksAktualisieren(void);

void TakteAktualisieren(void);

LONG NextXMarkerTakt(LONG t);
LONG PrevXMarkerTakt(LONG t);
