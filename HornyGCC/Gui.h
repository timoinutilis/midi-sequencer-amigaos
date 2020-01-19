#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/classusr.h>

#define GAD_T_PREV 0
#define GAD_T_NEXT 1
#define GAD_T_STOP 2
#define GAD_T_PLAY 3
#define GAD_T_REC 4
#define GAD_T_MARKER 5
#define GAD_S_H 6
#define GAD_S_V 7
#define GAD_Z_H 8
#define GAD_Z_V 9
#define GAD_F_MREC 10
#define GAD_F_MPLAY 11
#define GAD_F_LOOP 12
#define GAD_F_FOLLOW 13
#define GAD_F_THRU 14
#define GAD_F_SYNC 15
#define GADANZ 16

void OeffneFont(void);
void EntferneFont(void);
void ErstelleGuiClasses(void);
void EntferneGuiClasses(void);
void ErstelleBildschirm(void);
void ZeigeBildschirm(void);
void ErstelleHauptfenster(void);
void EntferneHauptfenster(void);
void EntferneBildschirm(void);
void FensterTitel(STRPTR projekt);
void SetzeFont();
void Fett(BOOL f);
void EntferneSprungliste(void);
void AktualisiereSprungliste(void);
void ErstelleGadgets(void);
void EntferneGadgets(void);
void PositioniereGadgets(void);
void AktualisiereGadgets(void);
void AktualisiereFunctGadgets(void);
void HoleFensterWinpos(struct Window *fenster, uint8 fenid);
void HoleFensterObjpos(Object *fensterobj, uint8 fenid);
void HoleAlleFensterpos(void);
void BildFrei(void);
void Schreibe(uint8 f, int16 x, int16 y, STRPTR t, int16 xe);
void SchreibeSys(uint8 f, int16 x, int16 y, STRPTR t);
void SchreibeZahl(uint8 f, int16 x, int16 y, int16 z);
#define STIL_DH 1
#define STIL_HD 2
#define STIL_ND 3
#define STIL_DN 4
void Gradient(uint8 f, uint8 stil, int16 x1, int16 y1, int16 x2, int16 y2);
void Balken(uint8 f, int16 x1, int16 y1, int16 x2, int16 y2);
void RahmenEin(int16 f, uint8 stil, int16 x1, int16 y1, int16 x2, int16 y2);
void RahmenAus(int16 f, uint8 stil, int16 x1, int16 y1, int16 x2, int16 y2);
void RahmenRundungEin(int16 x1, int16 y1, int16 x2, int16 y2);
void RahmenRundungAus(int16 x1, int16 y1, int16 x2, int16 y2);
void Linie(uint8 f, int16 x1, int16 y1, int16 x2, int16 y2);
void PunktLinie(uint8 f, int16 x1, int16 y1, int16 x2, int16 y2);
void Farbe(uint8 f);
void Punkt(int16 x, int16 y);
void ZeichnePosition(BOOL edit);
void KeinePosition(void);
void ZeichneProjektion(int32 start, int32 ende);
void KeineProjektion(void);
void Lasso(int16 x1, int16 y1, int16 x2, int16 y2);
BOOL LassoWahl(int16 mx1, int16 my1, int16 *mx2, int16 *my2);
void SchleifeUpdate(struct TagItem *tags, int16 *id, int16 *code);
