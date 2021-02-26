#include <exec/types.h>

struct MARKER *NeuerMarker(int8 typ, int32 t, int16 d1, int16 d2);
void EntferneMarker(struct MARKER *mark);
void EntferneAlleMarker(void);
struct MARKER *TaktDirektMarker(int32 t);
struct MARKER *TaktMarker(struct MARKER *start, int8 typ, int32 t);
struct MARKER *NextMarker(struct MARKER *akt);
void MarkerTauschen(struct MARKER *mark1, struct MARKER *mark2);
void MarkerSortieren(void);
void ErstelleGrundMarker(void);
void TaktWahlMark(int32 t);

int32 TaktZeit(int32 t);
int32 TaktSmpteTicks(int32 t);
int32 SmpteTicksTakt(int32 ticks);
void SmpteTicksAktualisieren(void);

void TakteAktualisieren(void);

int32 NextXMarkerTakt(int32 t);
int32 PrevXMarkerTakt(int32 t);
