#include <exec/types.h>

void ErstelleEditorNotenFenster(void);
void EntferneEditorNotenFenster(void);
void EdFensterTitel(void);
void PositioniereEdGadgets(void);
void AktualisiereEdGadgets(void);
void AktualisiereEdRasterGadgets(void);
void AktualisiereEdNeuLenGadgets(void);
void AktualisiereEdModus(void);

void ZeichneTastatur(void);
void ZeichneEdZeitleiste(void);
void ZeichneNote(struct EVENT *sevent, struct EVENT *eevent, BOOL vorschau);
void ZeichneNotenfeld(BOOL hg, BOOL vorschau, BOOL anders);
void ZeichneNotenVelos(void);
void ZeichneNotenanzeige(void);
void ZeichneEdInfobox(void);

BYTE PunktTaste(WORD y);
LONG EdPunktPosition(WORD x);
BYTE TesteEdPunktBereich(WORD x, WORD y);
