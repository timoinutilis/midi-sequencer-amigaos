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
void HoleFensterWinpos(struct Window *fenster, UBYTE fenid);
void HoleFensterObjpos(Object *fensterobj, UBYTE fenid);
void HoleAlleFensterpos(void);
void BildFrei(void);
void Schreibe(UBYTE f, WORD x, WORD y, STRPTR t, WORD xe);
void SchreibeSys(UBYTE f, WORD x, WORD y, STRPTR t);
void SchreibeZahl(UBYTE f, WORD x, WORD y, WORD z);
#define STIL_DH 1
#define STIL_HD 2
#define STIL_ND 3
#define STIL_DN 4
#ifdef __amigaos4__
void Gradient(UBYTE f, UBYTE stil, WORD x1, WORD y1, WORD x2, WORD y2);
#endif
void Balken(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void RahmenEin(WORD f, UBYTE stil, WORD x1, WORD y1, WORD x2, WORD y2);
void RahmenAus(WORD f, UBYTE stil, WORD x1, WORD y1, WORD x2, WORD y2);
void RahmenRundungEin(WORD x1, WORD y1, WORD x2, WORD y2);
void RahmenRundungAus(WORD x1, WORD y1, WORD x2, WORD y2);
void Linie(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void PunktLinie(UBYTE f, WORD x1, WORD y1, WORD x2, WORD y2);
void Farbe(UBYTE f);
void Punkt(WORD x, WORD y);
void ZeichnePosition(BOOL edit);
void KeinePosition(void);
void ZeichneProjektion(LONG start, LONG ende);
void KeineProjektion(void);
void Lasso(WORD x1, WORD y1, WORD x2, WORD y2);
BOOL LassoWahl(WORD mx1, WORD my1, WORD *mx2, WORD *my2);
void SchleifeUpdate(struct TagItem *tags, WORD *id, WORD *code);
