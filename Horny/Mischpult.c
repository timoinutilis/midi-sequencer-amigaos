#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/button.h>
#include <proto/scroller.h>
#include <proto/slider.h>

#include <libraries/gadtools.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <gadgets/button.h>
#include <gadgets/scroller.h>
#include <gadgets/slider.h>
#include <midi/mididefs.h>

#include "locale.h"

#include "Strukturen.h"
#include "GuiFenster.h"
#include "Versionen.h"
#include "Gui.h"
#include "Menu.h"
#include "Midi.h"
#include "Projekt.h"
#include "Instrumente.h"
#include "Dynamic_Strings.h"
#include "DTGrafik.h"

#define QUALIFIER_SHIFT 0x03
#define QUALIFIER_ALT 0x30
#define QUALIFIER_CTRL 0x08

extern struct TextFont *font;
extern struct Menu *minmenu;

extern struct Hook *backfill;
extern struct Screen *hschirm;
extern struct Window *hfenster;
extern struct Window *aktfenster;
extern struct FENSTERPOS fenp[];

BYTE mpmauskanal = 0;
BYTE mpmausdata = 0;
BYTE mpmausaktion = 0;
#define MPAKTION_PAN 1
#define MPAKTION_POTI 2
WORD mpmausalty = 0;
WORD mpmausaltx = 0;

struct Window *mpfenster = NULL;

struct MPGUIKANAL {
	struct Gadget *contrgad[6];
	struct Gadget *fadergad;
	struct Gadget *mutegad;
};

#define GUIKANALANZ 20
struct MPGUIKANAL mpguikanal[GUIKANALANZ];
struct Gadget *mpscrollgad = NULL;
#define GAD_SCROLL 0
#define GAD_CONTR 1
#define GAD_FADER 10
#define GAD_MUTE 11
#define MPGAD_ANZ 12

extern struct LIED lied;
extern struct SPUR spur[];
extern struct OUTPORT outport[];

struct MPDATA mpdata = {
	0, // kanalanz
	0, // kanalerst
	0, // kanalsicht
};

struct MPKANAL mpkanal[OUTPORTS][16];
struct MPKANALNUM mpkanalnum[SPUREN];

struct TagItem mixmapfader[] = {SLIDER_Level, ICSPECIAL_CODE, TAG_DONE};
struct TextAttr mixfentextattr = {"helvetica.font", 11, FS_NORMAL, 0};

#define MUTE_SOLO 1
#define MUTE_OFF 2

void InitMPKanaele(void) {
	BYTE p, c, n;
	
	for (p = 0; p < verOUTPORTS; p++) {
		for (c = 0; c < 16; c++) {
			for (n = 0; n < 6; n++) {
				mpkanal[p][c].contr[n] = -1;
				mpkanal[p][c].contrwert[n] = 0;
			}
			mpkanal[p][c].contr[0] = 91;
			mpkanal[p][c].contr[1] = 93;

			mpkanal[p][c].pan = MCCenter;
			mpkanal[p][c].fader = 100;
			mpkanal[p][c].mute = FALSE;
			mpkanal[p][c].meter = 0;
			for (n = 0; n < 3; n++) mpkanal[p][c].bezspur[n] = -1;
			mpkanal[p][c].autoupdate = FALSE;
			mpkanal[p][c].updateflags = 0;
		}
	}
}

void SammleMPKanaele(void) {
	WORD s;
	BYTE p, c;
	BOOL benutzt[OUTPORTS][16];
	BYTE n;
	
	for (p = 0; p < verOUTPORTS; p++) {
		for (c = 0; c < 16; c++) {
			benutzt[p][c] = FALSE;
			for (n = 0; n < 3; n++) mpkanal[p][c].bezspur[n] = -1;
		}
	}

	for (s = 0; s < lied.spuranz; s++) {
		p = spur[s].port;
		c = spur[s].channel;
		if (c < 16) {
			benutzt[p][c] = TRUE;
			for (n = 0; n < 3; n++) {
				if (mpkanal[p][c].bezspur[n] == -1) {
					mpkanal[p][c].bezspur[n] = s;
					break;
				}
			}
		}
	}
	
	s = 0;
	for (p = 0; p < verOUTPORTS; p++)
		for (c = 0; c < 16; c++)
			if (benutzt[p][c]) {
				mpkanalnum[s].port = p;
				mpkanalnum[s].channel = c;
				s++;
			}
	mpdata.kanalanz = s;
}

void ErstelleMPKanalGadgets(UBYTE n) {
	WORD x, y;
	BYTE i;
	BYTE max;
	
	x = mpfenster->BorderLeft + (n * 60);
	y = mpfenster->BorderTop;
	
	// Controller...
	if (verLITE) max = 3;
	else max = 6;
	for (i = 0; i < max; i++) {
		mpguikanal[n].contrgad[i] = NewObject(BUTTON_GetClass(), NULL,
			GA_Top, y + 2 + (i * 16), GA_Left, x + 2,
			GA_Width, 40, GA_Height, 16, GA_RelVerify, TRUE,
			GA_ID, (MPGAD_ANZ * n) + GAD_CONTR + i, GA_UserData, n,
			GA_TextAttr, &mixfentextattr,
			GA_Text, "",
			TAG_DONE);
	}
	// Fader...
	mpguikanal[n].fadergad = NewObject(SLIDER_GetClass(), NULL,
		GA_BackFill, backfill,
		GA_Top, y + 150, GA_Left, x + 25,
		GA_Width, 30, GA_Height, 140,
		GA_RelVerify, TRUE,
		GA_ID, (MPGAD_ANZ * n) + GAD_FADER, GA_UserData, n,
		SLIDER_Min, 0, SLIDER_Max, 127,
		SLIDER_Level, 100,
		SLIDER_Orientation, SORIENT_VERT,
		SLIDER_Invert, TRUE,
#ifdef __amigaos4__
		SLIDER_LevelPlace, PLACETEXT_IN,
		SLIDER_LevelJustify, SLJ_CENTER,
		SLIDER_LevelFormat, "%ld",
#endif
		ICA_TARGET, ICTARGET_IDCMP,
		ICA_MAP, mixmapfader,
		TAG_DONE);
	// Mute...
	mpguikanal[n].mutegad = NewObject(BUTTON_GetClass(), NULL,
		GA_Top, y + 294, GA_Left, x + 6,
		GA_Width, 50, GA_Height, 16, GA_RelVerify, TRUE,
		GA_ID, (MPGAD_ANZ * n) + GAD_MUTE, GA_UserData, n,
		GA_TextAttr, &mixfentextattr,
		GA_Text, "MUTE", BUTTON_PushButton, TRUE,
		TAG_DONE);
	
}

void EntferneMPKanalGadgets(UBYTE n) {
	UBYTE i;
	BYTE max;
	
	if (verLITE) max = 3;
	else max = 6;
	
	for (i = 0; i < max; i++) DisposeObject(mpguikanal[n].contrgad[i]);
	DisposeObject(mpguikanal[n].fadergad);
	DisposeObject(mpguikanal[n].mutegad);
}

void MPKanaeleEinsetzen(void) {
	UBYTE i;
	WORD x;
	UBYTE n;
	BYTE max;
	
	aktfenster = mpfenster;
	
	if (verLITE) max = 3;
	else max = 6;
	
	mpdata.kanalsicht = (mpfenster->Width - mpfenster->BorderLeft - mpfenster->BorderRight) / 60;
	for (n = 0; n < mpdata.kanalsicht; n++) {
		x = n * 60;
		
		for (i = 0; i < max; i++) AddGadget(mpfenster, mpguikanal[n].contrgad[i], -1);

		AddGadget(mpfenster, mpguikanal[n].fadergad, -1);
		AddGadget(mpfenster, mpguikanal[n].mutegad, -1);
	
		RahmenAus(0, 0, x, 0, x + 59, 367);
		Linie(1, x + 1, 312, x + 58, 312);
		Linie(1, x + 1, 355, x + 58, 355);
		RahmenRundungAus(x + 1, 1, x + 58, 311);
		BlitteBitMap(BMAP_PAN_BG, 0, 0, x + 1, 126, 58, 19);
	}
	RefreshGadgets(mpguikanal[0].contrgad[0], mpfenster, NULL);
}

void MPKanaeleWegnehmen(void) {
	UBYTE i;
	UBYTE n;
	BYTE max;
	
	aktfenster = mpfenster;
	BildFrei();

	if (verLITE) max = 3;
	else max = 6;
	
	for (n = 0; n < mpdata.kanalsicht; n++) {
		for (i = 0; i < max; i++) RemoveGadget(mpfenster, mpguikanal[n].contrgad[i]);
		RemoveGadget(mpfenster, mpguikanal[n].fadergad);
		RemoveGadget(mpfenster, mpguikanal[n].mutegad);
	}
	mpdata.kanalsicht = 0;
}

void ZeichnePanorama(WORD x, BYTE pan) {
	BYTE y[] = {8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 0};
	WORD xp;
	
	aktfenster = mpfenster;
	BlitteBitMap(BMAP_PAN_BG, 0, 0, x + 1, 126, 58, 19);
	xp = (WORD)(pan * 0.37);
	if (xp < 23) BlitteBitMap(BMAP_PAN_BG_ACTIVE, xp + 7, 0, x + 7 + xp, 126, 23 - xp, 19);
	if (xp > 23) BlitteBitMap(BMAP_PAN_BG_ACTIVE, 29, 0, x + 30, 126, xp - 23, 19);
	BlitteBitMap(BMAP_PAN_POINTER, 0, 0, x + 5 + xp, 136 - y[abs(xp - 23)], 5, 9);
}

void BewegePanorama(BYTE guikanal, WORD delta) {
	BYTE p, c;
	WORD pan;
	
	p = mpkanalnum[guikanal + mpdata.kanalerst].port;
	c = mpkanalnum[guikanal + mpdata.kanalerst].channel;
	
	pan = mpkanal[p][c].pan; pan += delta;
	if (pan < 0) pan = 0;
	if (pan > 127) pan = 127;
	mpkanal[p][c].pan = pan;
	SendeKanalEvent(p, c, MS_Ctrl, MC_Pan, pan);
	ZeichnePanorama(guikanal * 60, pan);
}

void PanoramaMitte(BYTE guikanal) {
	BYTE p, c;
	
	p = mpkanalnum[guikanal + mpdata.kanalerst].port;
	c = mpkanalnum[guikanal + mpdata.kanalerst].channel;
	
	mpkanal[p][c].pan = MCCenter;
	SendeKanalEvent(p, c, MS_Ctrl, MC_Pan, MCCenter);
	ZeichnePanorama(guikanal * 60, mpkanal[p][c].pan);
}

void ZeichneContrPoti(WORD x, WORD y, BYTE wert) {
	FLOAT w;
	
	aktfenster = mpfenster;
	w = 3.1415926535 / 180.0 * (20 + ((FLOAT)wert * 2.5));
	BlitteBitMap(BMAP_POTI, 0, 0, x, y, 15, 15);
	Linie(1, x + 7, y + 7, x + 7 - (WORD)(sin(w) * 7), y + 7 + (WORD)(cos(w) * 7));
}

void BewegeContrPoti(BYTE guikanal, BYTE n, WORD delta) {
	BYTE p, c;
	WORD neuwert;
	
	p = mpkanalnum[guikanal + mpdata.kanalerst].port;
	c = mpkanalnum[guikanal + mpdata.kanalerst].channel;
	
	if (mpkanal[p][c].contr[n] != -1) {
		neuwert = mpkanal[p][c].contrwert[n]; neuwert += delta;
		if (neuwert < 0 ) neuwert = 0;
		if (neuwert > 127) neuwert = 127;
		mpkanal[p][c].contrwert[n] = neuwert;
		SendeKanalEvent(p, c, MS_Ctrl, mpkanal[p][c].contr[n], neuwert);
		ZeichneContrPoti(guikanal * 60 + 43, 2 + (n * 16), mpkanal[p][c].contrwert[n]);
	}
}

void ContrPotiReset(BYTE guikanal, BYTE n) {
	BYTE p, c;
	
	p = mpkanalnum[guikanal + mpdata.kanalerst].port;
	c = mpkanalnum[guikanal + mpdata.kanalerst].channel;
	
	if (mpkanal[p][c].contr[n] != -1) {
		mpkanal[p][c].contrwert[n] = 0;
		SendeKanalEvent(p, c, MS_Ctrl, mpkanal[p][c].contr[n], 0);
		ZeichneContrPoti(guikanal * 60 + 43, 2 + (n * 16), mpkanal[p][c].contrwert[n]);
	}
}

void MPKanalContrAktualisieren(UBYTE k) {
	BYTE p, c;
	BYTE guikanal;
	struct INSTRUMENT *instr;
	BYTE n;
	WORD x;
	BYTE max;
	
	p = mpkanalnum[k].port;
	c = mpkanalnum[k].channel;
	guikanal = k - mpdata.kanalerst;
	x = guikanal * 60;
	instr = SucheChannelInstrument(mpkanalnum[k].port, mpkanalnum[k].channel);

	if (verLITE) max = 3;
	else max = 6;
	
	for (n = 0; n < max; n++) {
		if (mpkanal[p][c].contr[n] != -1) {
			SetGadgetAttrs(mpguikanal[guikanal].contrgad[n], mpfenster, NULL,
				GA_Text, instr->contr->name[mpkanal[p][c].contr[n]], TAG_DONE);
			ZeichneContrPoti(x + 43, 2 + (n * 16), mpkanal[p][c].contrwert[n]);
		} else {
			SetGadgetAttrs(mpguikanal[guikanal].contrgad[n], mpfenster, NULL,
				GA_Text, "", TAG_DONE);
			Balken(0, x + 43, 2 + (n * 16), x + 57, 16 + (n * 16));
		}
	}
}

void MPKanalAktualisieren(UBYTE k) {
	BYTE p, c;
	WORD x;
	BYTE guikanal;
	BYTE n;
	
	aktfenster = mpfenster;
	
	guikanal = k - mpdata.kanalerst;
	x = guikanal * 60;
	if (k < mpdata.kanalanz) {
		p = mpkanalnum[k].port;
		c = mpkanalnum[k].channel;
		BlitteBitMap(BMAP_METER_OFF, 0, 0, x + 6, 150, 15, 140); // Meter
#ifdef __amigaos4__
		Gradient(26, STIL_ND, x + 1, 313, x + 58, 354);
#else
		Balken(26, x + 1, 313, x + 58, 354);
#endif
		
		for (n = 0; n < 3; n++) {
			if (mpkanal[p][c].bezspur[n] != -1)
				Schreibe(1, x + 3, 323 + (n * 11), spur[mpkanal[p][c].bezspur[n]].name, x + 58);
		}
		
#ifdef __amigaos4__
		Gradient(9 + c, STIL_DH, x + 1, 356, x + 58, 366);
#else
		Balken(9 + c, x + 1, 356, x + 58, 366);
#endif
		Fett(TRUE);
		SchreibeZahl(1, x + 3, 364, c + 1);
		Fett(FALSE);

		ZeichnePanorama(x, mpkanal[p][c].pan);
		SetGadgetAttrs(mpguikanal[guikanal].fadergad, mpfenster, NULL,
			SLIDER_Level, mpkanal[p][c].fader, TAG_DONE);
		SetGadgetAttrs(mpguikanal[guikanal].mutegad, mpfenster, NULL,
			GA_Selected, mpkanal[p][c].mute, TAG_DONE);
		MPKanalContrAktualisieren(k);
		
	} else {
		BlitteBitMap(BMAP_METER_INACTIVE, 0, 0, x + 6, 150, 15, 140); // Meter
		BlitteBitMap(BMAP_PAN_BG, 0, 0, x + 1, 126, 58, 19);
		Balken(0, x + 1, 313, x + 58, 354);
		Balken(0, x + 1, 356, x + 58, 366);

		SetGadgetAttrs(mpguikanal[guikanal].fadergad, mpfenster, NULL,
			SLIDER_Level, 0, TAG_DONE);
		SetGadgetAttrs(mpguikanal[guikanal].mutegad, mpfenster, NULL,
			GA_Selected, FALSE, TAG_DONE);
	}
}

void MPZeichnePorts(void) {
	WORD k;
	WORD x;
	BYTE p;
	BOOL zeige;
	
	aktfenster = mpfenster;
	
	for (k = mpdata.kanalsicht - 1; k >= 0; k--) {
		x = k * 60;
		if (k < mpdata.kanalanz) {
			p = mpkanalnum[k + mpdata.kanalerst].port;
#ifdef __amigaos4__
			Gradient(27 - (p % 2), STIL_DN, x, 369, x + 59, 382);
#else
			Balken(27 - (p % 2), x, 369, x + 59, 382);
#endif

			if (k == 0) zeige = TRUE;
			else if (p != mpkanalnum[k + mpdata.kanalerst - 1].port) zeige = TRUE;
			else zeige = FALSE;

			if (zeige) {
				Linie(1, x, 368, x, 382);
				Schreibe(1, x + 3, 378, outport[p].name, mpdata.kanalsicht * 60 - 1);
			}
		} else Balken(0, x, 369, x + 59, 382);
	}
}

void AktualisiereMischpult(void) {
	WORD k;
	
	SammleMPKanaele();
	if (mpfenster) {
		for (k = 0; k < mpdata.kanalsicht; k++) MPKanalAktualisieren(k + mpdata.kanalerst);
		MPZeichnePorts();
		SetGadgetAttrs(mpscrollgad, mpfenster, NULL,
			SCROLLER_Top, mpdata.kanalerst,
			SCROLLER_Visible, mpdata.kanalsicht,
			SCROLLER_Total, mpdata.kanalanz,
			TAG_DONE);
	}
}

void SetzeMeter(UBYTE p, UBYTE c, BYTE velo) {
	if (mpkanal[p][c].meter < ((WORD)velo * (WORD)mpkanal[p][c].fader / 128))
		mpkanal[p][c].meter = (BYTE)((WORD)velo * (WORD)mpkanal[p][c].fader / 128);
}

void ErniedrigeMeter(BYTE wert) {
	WORD n;
	BYTE p, c;
	
	for (n = 0; n < mpdata.kanalanz; n++) {
		p = mpkanalnum[n].port;
		c = mpkanalnum[n].channel;
		mpkanal[p][c].meter -= wert;
		if (mpkanal[p][c].meter < 0) mpkanal[p][c].meter = 0;
	}
}

void ZeichneMeter(void) {
	WORD x;
	WORD n;
	WORD k;
	BYTE p, c;
	BYTE m;
	WORD h;
	
	aktfenster = mpfenster;
	for (n = 0; n < mpdata.kanalsicht; n++) {
		x = n * 60;
		k = n + mpdata.kanalerst;
		if (k >= mpdata.kanalanz) break;
		p = mpkanalnum[k].port;
		c = mpkanalnum[k].channel;
		m = mpkanal[p][c].meter;
		
		h = (WORD)(m * 1.102362204724);
		BlitteBitMap(BMAP_METER_OFF, 0, 0, x + 6, 150, 15, 140 - h);
		if (m > 0) BlitteBitMap(BMAP_METER_ON, 0, 139 - h, x + 6, 289 - h, 15, h);
		
	}
}

void SendeMischpult(void) {
	WORD n;
	BYTE cn;
	BYTE p, c;
	BYTE max;
	
	if (verLITE) max = 3;
	else max = 6;
	
	for (n = 0; n < mpdata.kanalanz; n++) {
		p = mpkanalnum[n].port;
		c = mpkanalnum[n].channel;
		for (cn = 0; cn < max; cn++) {
			if (mpkanal[p][c].contr[cn] != -1)
				SendeKanalEvent(p, c, MS_Ctrl, mpkanal[p][c].contr[cn], mpkanal[p][c].contrwert[cn]);
		}
		SendeKanalEvent(p, c, MS_Ctrl, MC_Volume, mpkanal[p][c].fader);
		SendeKanalEvent(p, c, MS_Ctrl, MC_Pan, mpkanal[p][c].pan);
	}
}

void ErstelleMPFenster(void) {
	UBYTE n;
	WORD borderbr;

	if (hschirm) borderbr = hschirm->WBorLeft;
	else borderbr = hfenster->BorderLeft;
	
	if (fenp[MISCHER].y == -1) {
		fenp[MISCHER].x = hfenster->LeftEdge + 80;
		fenp[MISCHER].y = hfenster->TopEdge + 40;
		fenp[MISCHER].b = 8 * 60 + (2 * borderbr);
	}
	
	mpfenster = OpenWindowTags(NULL,
		WA_PubScreen, hschirm,
		WA_Left, fenp[MISCHER].x,
		WA_Top, fenp[MISCHER].y,
		WA_Width, fenp[MISCHER].b,
		WA_InnerHeight, 384,
		WA_MinWidth, 100,
		WA_MinHeight, 0,
		WA_MaxWidth, GUIKANALANZ * 60 + (2 * borderbr),
		WA_MaxHeight, 0,
		WA_IDCMP, IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_NEWSIZE |
					IDCMP_VANILLAKEY | IDCMP_RAWKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MENUPICK |
					IDCMP_IDCMPUPDATE | IDCMP_CLOSEWINDOW,
		WA_SizeGadget, TRUE,
		WA_SizeBBottom, TRUE,
		WA_DragBar, TRUE,
		WA_DepthGadget, TRUE,
		WA_CloseGadget, TRUE,
		WA_NewLookMenus, TRUE,
		WA_ReportMouse, TRUE,
		WA_SmartRefresh, TRUE,
		WA_Activate, TRUE,
		WA_Title, CAT(MSG_0006, "Mixer"),
		WA_BackFill, backfill,
		TAG_DONE);
	
	aktfenster = mpfenster;
	SetzeFont();
	SetMenuStrip(mpfenster, minmenu);
	
	mpscrollgad = NewObject(SCROLLER_GetClass(), NULL,
		GA_RelBottom, -(mpfenster->BorderBottom) + 3, GA_Left, mpfenster->BorderLeft - 1,
		GA_RelWidth, -23, GA_Height, mpfenster->BorderBottom - 4,
		GA_RelVerify, TRUE,
		GA_ID, GAD_SCROLL,
		GA_BottomBorder, TRUE,
		SCROLLER_Top, 0,
		SCROLLER_Visible, 1,
		SCROLLER_Total, 1,
		SCROLLER_Orientation, SORIENT_HORIZ,
		SCROLLER_Arrows, FALSE,
		TAG_DONE);
	AddGadget(mpfenster, mpscrollgad, -1);

	for (n = 0; n < GUIKANALANZ; n++) ErstelleMPKanalGadgets(n);
	MPKanaeleEinsetzen();
	AktualisiereMischpult();
}

void EntferneMPFenster(void) {
	UBYTE n;
	
	if (mpfenster) {
		HoleFensterWinpos(mpfenster, MISCHER);
		
		MPKanaeleWegnehmen();
		for (n = 0; n < GUIKANALANZ; n++) EntferneMPKanalGadgets(n);
		RemoveGadget(mpfenster, mpscrollgad);
		
		ClearMenuStrip(mpfenster);
		CloseWindow(mpfenster);
		mpfenster = NULL;
	}
}

void SetzeAlleFader(BYTE wert) {
	WORD k;
	BYTE p, c;
	WORD guikanal;
	
	for (k = 0; k < mpdata.kanalanz; k++) {
		p = mpkanalnum[k].port;
		c = mpkanalnum[k].channel;
		mpkanal[p][c].fader = wert;
		SendeKanalEvent(p, c, MS_Ctrl, MC_Volume, wert);
		if ((k >= mpdata.kanalerst) && (k < mpdata.kanalerst + mpdata.kanalsicht)) {
			guikanal = k - mpdata.kanalerst;
			SetGadgetAttrs(mpguikanal[guikanal].fadergad, mpfenster, NULL,
				SLIDER_Level, wert, TAG_DONE);
		}
	}
}

void SetzeAlleMutes(BYTE wp, BYTE wc, BYTE modus) {
	WORD k;
	BYTE p, c;
	WORD guikanal;
	BOOL mute;

	for (k = 0; k < mpdata.kanalanz; k++) {
		p = mpkanalnum[k].port;
		c = mpkanalnum[k].channel;

		switch (modus) {
		case MUTE_SOLO:
			if ((wp != p) || (wc != c)) mute = TRUE;
			else mute = FALSE;
			break;
		case MUTE_OFF:
			mute = FALSE;
			break;
		}
		
		if (!mpkanal[p][c].mute && mute) KanalAbklingen(p, c);
		mpkanal[p][c].mute = mute;

		if ((k >= mpdata.kanalerst) && (k < mpdata.kanalerst + mpdata.kanalsicht)) {
			guikanal = k - mpdata.kanalerst;
			SetGadgetAttrs(mpguikanal[guikanal].mutegad, mpfenster, NULL,
				GA_Selected, mute, TAG_DONE);
		}
	}
}

void KontrolleMischpultFenster(void) {
	struct IntuiMessage *mes;
	struct Gadget *gad;
	WORD gadid;
	UBYTE kanal;
	BYTE p, c;
	BOOL schliessen = FALSE;
	WORD updateid, updatecode;
	WORD xmaus, ymaus;
	WORD yrand;

	if (verLITE) yrand = 50;
	else yrand = 98;

	while (mes = (struct IntuiMessage *)GetMsg(mpfenster->UserPort)) {
		
		switch (mes->Class) {
			case IDCMP_CLOSEWINDOW: schliessen = TRUE; break;

			case IDCMP_NEWSIZE:
			if (mpdata.kanalsicht != (mpfenster->Width - mpfenster->BorderLeft - mpfenster->BorderRight) / 60) {
				MPKanaeleWegnehmen();
				RefreshWindowFrame(mpfenster);
				MPKanaeleEinsetzen();
				AktualisiereMischpult();
			}
			break;

			case IDCMP_MENUPICK:
			MinMenuKontrolle(MinMenuPunkt(mes->Code));
			break;

			case IDCMP_RAWKEY:
			TransportKontrolleRaw(mes->Code);
			break;

			case IDCMP_VANILLAKEY:
			TransportKontrolle(mes->Code);
			break;
			
			case IDCMP_MOUSEBUTTONS:
			xmaus = mes->MouseX - mpfenster->BorderLeft; ymaus = mes->MouseY - mpfenster->BorderTop;
			mpmauskanal = xmaus / 60;
			if ((mes->Code == 104) && (mpmauskanal - mpdata.kanalerst < mpdata.kanalanz)) {
				if ((ymaus > 126) && (ymaus < 146)) { // Panorama
					mpmausaktion = MPAKTION_PAN;
					mpmausaltx = mes->MouseX;
					if (mes->Qualifier & QUALIFIER_ALT) PanoramaMitte(mpmauskanal);
				}
				if ((ymaus >= 2) && (ymaus <= yrand)) { // Controller
					mpmausaktion = MPAKTION_POTI;
					mpmausdata = (ymaus - 2) / 16;
					mpmausalty = mes->MouseY;
					if (mes->Qualifier & QUALIFIER_ALT) ContrPotiReset(mpmauskanal, mpmausdata);
				}
			}
			if (mes->Code == 232) mpmausaktion = 0;
			break;
			
			case IDCMP_MOUSEMOVE:
			if (mpmausaktion == MPAKTION_PAN) {
				BewegePanorama(mpmauskanal, mes->MouseX - mpmausaltx);
				mpmausaltx = mes->MouseX;
			}
			if (mpmausaktion == MPAKTION_POTI) {
				BewegeContrPoti(mpmauskanal, mpmausdata, mpmausalty - mes->MouseY);
				mpmausalty = mes->MouseY;
			}
			break;
			
			case IDCMP_IDCMPUPDATE:
			SchleifeUpdate((struct TagItem *)mes->IAddress, &updateid, &updatecode);
			gadid = updateid % MPGAD_ANZ;
			kanal = (updateid / MPGAD_ANZ) + mpdata.kanalerst;
			if (kanal < mpdata.kanalanz) {
				if (gadid == GAD_FADER) {
					p = mpkanalnum[kanal].port;
					c = mpkanalnum[kanal].channel;
					mpkanal[p][c].fader = (BYTE)mes->Code;
					SendeKanalEvent(p, c, MS_Ctrl, MC_Volume, (BYTE)mes->Code);
				}
			}
			break;
		
			case IDCMP_GADGETUP:
			gad = (struct Gadget *)mes->IAddress;
			gadid = gad->GadgetID % MPGAD_ANZ;
			kanal = (gad->GadgetID / MPGAD_ANZ) + mpdata.kanalerst;
			if (gadid == GAD_SCROLL) {
				mpdata.kanalerst = mes->Code;
				AktualisiereMischpult();
			}
			if (kanal < mpdata.kanalanz) {
				p = mpkanalnum[kanal].port;
				c = mpkanalnum[kanal].channel;
				switch (gadid) {
					case GAD_FADER:
					if (mes->Qualifier & QUALIFIER_SHIFT) {
						SetzeAlleFader((BYTE)mes->Code);
					} else {
						mpkanal[p][c].fader = (BYTE)mes->Code;
						SendeKanalEvent(p, c, MS_Ctrl, MC_Volume, (BYTE)mes->Code);
					}
					break;
	
					case GAD_MUTE:
					if (mes->Qualifier & QUALIFIER_SHIFT)
						SetzeAlleMutes(p, c, MUTE_SOLO);
					else if (mes->Qualifier & QUALIFIER_ALT)
						SetzeAlleMutes(p, c, MUTE_OFF);
					else {
						mpkanal[p][c].mute = (BOOL)mes->Code;
						if (mes->Code) KanalAbklingen(p, c);
					}
					break;
				}
				if ((gadid >= GAD_CONTR) && (gadid < GAD_CONTR + 6)) {
					mpkanal[p][c].contr[gadid - GAD_CONTR] = InstrControllerFenster(mpfenster, c, p, mpkanal[p][c].contr[gadid - GAD_CONTR]);
					if (mpkanal[p][c].contr[gadid - GAD_CONTR] == -128) mpkanal[p][c].contr[gadid - GAD_CONTR] = -1;
					MPKanalContrAktualisieren(kanal);
				}
			}
			break;

		}
		ReplyMsg((struct Message *)mes);
	}
	
	if (schliessen) EntferneMPFenster();
}

void AutoUpdateMischpult(void) {
	WORD k;
	BYTE p, c;
	UBYTE flags;
	UBYTE guikanal;
	WORD x;
	BYTE n;
	BYTE max;
	
	if (verLITE) max = 3;
	else max = 6;
	
	for (k = 0; k < mpdata.kanalanz; k++) {
		p = mpkanalnum[k].port;
		c = mpkanalnum[k].channel;
		if (mpkanal[p][c].autoupdate) {
			if ((k >= mpdata.kanalerst) && (k < mpdata.kanalerst + mpdata.kanalsicht)) {
				guikanal = k - mpdata.kanalerst;
				x = guikanal * 60;
				flags = mpkanal[p][c].updateflags;
				
				if (flags & 0x01) { // Fader
					SetGadgetAttrs(mpguikanal[guikanal].fadergad, mpfenster, NULL,
						SLIDER_Level, mpkanal[p][c].fader, TAG_DONE);
				}
				if (flags & 0x02) ZeichnePanorama(x, mpkanal[p][c].pan); // Panorama
				for (n = 0; n < max; n++) { // Controller
					if (flags & (0x04 << n)) ZeichneContrPoti(x + 43, 2 + (n * 16), mpkanal[p][c].contrwert[n]);
				}
			}
			mpkanal[p][c].autoupdate = FALSE;
			mpkanal[p][c].updateflags = 0;
		}
	}
}

void ControllerAnpassen(BYTE p, BYTE c, BYTE data1, BYTE data2) {
	BYTE n;
	BYTE max;
		
	if (data1 == MC_Volume) {
		if (data2 != mpkanal[p][c].fader) {
			mpkanal[p][c].fader = data2;
			mpkanal[p][c].autoupdate = TRUE;
			mpkanal[p][c].updateflags |= 0x01;
		}
	}
	if (data1 == MC_Pan) {
		if (data2 != mpkanal[p][c].pan) {
			mpkanal[p][c].pan = data2;
			mpkanal[p][c].autoupdate = TRUE;
			mpkanal[p][c].updateflags |= 0x02;
		}
	}
	
	if (verLITE) max = 3;
	else max = 6;
	for (n = 0; n < max; n++) {
		if (data1 == mpkanal[p][c].contr[n]) {
			if (data2 != mpkanal[p][c].contrwert[n]) {
				mpkanal[p][c].contrwert[n] = data2;
				mpkanal[p][c].autoupdate = TRUE;
				mpkanal[p][c].updateflags |= (0x04 << n);
			}
		}
	}
}
