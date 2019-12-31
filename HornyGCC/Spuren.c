#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "locale.h"

#include "Strukturen.h"
#include "Versionen.h"
#include "Requester.h"
#include "Midi.h"
#include "Sequenzen.h"
#include "Gui.h"
#include "Gui2.h"

extern WORD snum;
extern struct LIED lied;
extern struct SPUR spur[];
extern struct SPURTEMP sp[];
extern struct GUI gui;

void InitSpur(WORD s) {
	sprintf(spur[s].name, CAT(MSG_0007, "Track %d"), s + 1);
	spur[s].port = 0;
	spur[s].channel = 0;
	spur[s].bank0 = -1;
	spur[s].bank32 = -1;
	spur[s].prog = -1;
	spur[s].shift = 0;
	spur[s].mute = FALSE;
	spur[s].seq = NULL;
	spur[s].aktseq = NULL;
	spur[s].aktevbl = NULL;
	spur[s].aktevnum = 0;
	spur[s].autostatus = 0;
}

void AktualisiereSpuren(BOOL spalten) {
	WORD n;
	WORD s;
	
	KeinePosition();
	for (n = 0; n < gui.spsicht; n++) {
		s = n + gui.spur;
		if (sp[s].anders == 1) ZeichneSequenzRahmen(s);
		else if (sp[s].anders == 2) {
			ZeichneSequenzen(s, TRUE);
			if (spalten) ZeichneSpurSpalte(s, s == snum);
		}
		else if (sp[s].anders == 3) {
			if (spalten) ZeichneSpurSpalte(s, s == snum);
		}
		sp[s].anders = 0;
	}
	ZeichnePosition(TRUE);
}

BOOL SpurInSicht(WORD s) {
	if (s == gui.spur - 1) {gui.spur--; return(TRUE);}
	if (s == gui.spur + gui.spsicht) {gui.spur++; return(TRUE);}
	if ((s < gui.spur) || (s >= gui.spur + gui.spsicht)) {
		gui.spur = s - (gui.spsicht / 2);
		if (gui.spur < 0) gui.spur = 0;
		return(TRUE);
	}
	return(FALSE);
}

void SpurAktivieren(WORD s) {
	if (SpurInSicht(s)) {
		snum = s;
		KeinePosition(); ZeichneSpuren(TRUE, TRUE); ZeichnePosition(TRUE);
		AktualisiereGadgets();
	} else {
		ZeichneSpurSpalte(snum, FALSE);
		snum = s;
		ZeichneSpurSpalte(snum, TRUE);
	}
	ZeichneInfobox(4); SendeInstrument(snum);
	AktualisiereFunctGadgets();
}

void SpurScroll(WORD ds) {
	if (ds < 0) {
		if (gui.spur > 0) {
			gui.spur--;
			KeinePosition(); ZeichneSpuren(TRUE, TRUE); ZeichnePosition(TRUE);
		}
	} else if (ds > 0) {
		if (gui.spur + gui.spsicht <= lied.spuranz) {
			gui.spur++;
			KeinePosition(); ZeichneSpuren(TRUE, TRUE); ZeichnePosition(TRUE);
		}
	}
	AktualisiereGadgets();
}

void FolgenderName(STRPTR name, WORD spur) {
	WORD i;
	WORD len = strlen(name);
	char puf[256];
	WORD z;
	
	for (i = len - 1; i >= 0; i--) {
		if (name[i] < '0' || name[i] > '9') break;
	}
	if (i == len - 1) { //keine Zahl
		sprintf(puf, CAT(MSG_0007, "Track %d"), spur + 1);
	} else {
		z = atoi(&name[i + 1]) + 1;
		if (strlen(name) + 1 > 126 && (z - 1) / 10 != z / 10)
			name[i] = 0;
		else
			name[i + 1] = 0;
		sprintf(puf, "%s%d", name, z);
	}
	strncpy(name, puf, 128);
}

void NeueSpur(void) {
	if (lied.spuranz < verSPUREN) {
		ZeichneSpurSpalte(snum, FALSE);
		lied.spuranz++; snum = lied.spuranz - 1;
		spur[snum].channel = spur[snum - 1].channel + 1;
		if (spur[snum].channel > 15) spur[snum].channel = 0;
		spur[snum].port = spur[snum - 1].port;
		
		strncpy(spur[snum].name, spur[snum - 1].name, 128);
		FolgenderName(spur[snum].name, snum);
		
		if (SpurInSicht(snum)) ZeichneSpuren(TRUE, TRUE); else ZeichneSpurSpalte(snum, TRUE);
		ZeichneInfobox(4);
		AktualisiereGadgets();
	} else {
		if (verLITE) Meldung(CAT(MSG_0009, "In Lite version, it's not possible\nto create more than 16 tracks"));
		else Meldung(CAT(MSG_0010, "It's not possible to create more tracks"));
	}
}

void SpurLoeschen(WORD s) {
	struct SEQUENZ *seq;
	struct SEQUENZ *zseq;
	WORD n;

	seq = spur[s].seq;
	while (seq) {
		if (seq->aliasanz) {

			zseq = spur[s].seq; n = 0;
			while (zseq) {
				if (zseq->aliasorig == seq) n++;
				zseq = zseq->next;
			}

			if (n == seq->aliasanz) seq = NULL;
			break;
		}
		seq = seq->next;
	}
	if (!seq) {
		seq = spur[s].seq;
		while (seq) {
			if (seq->aliasorig) seq->aliasorig->aliasanz--;
			seq = seq->next;
		}
		SpurSequenzenEntfernen(s);

		for (n = s; n < lied.spuranz - 1; n++) {
			memcpy(&spur[n], &spur[n + 1], sizeof(struct SPUR));
			SequenzenSpuren(n);
		}
		InitSpur(n);
		if (lied.spuranz > 1) lied.spuranz--;

		if (snum >= lied.spuranz) snum = lied.spuranz - 1;

		KeinePosition();
		ZeichneSpuren(TRUE, TRUE);
		ZeichnePosition(TRUE);
		SpurAktivieren(snum);
	} else {
		Meldung(CAT(MSG_0011, "Track contains original sequence(s) of aliases"));
	}
}

void SpurVerschieben(WORD s1, WORD s2) {
	struct SPUR ksp;
	WORD n;
	
	memcpy(&ksp, &spur[s1], sizeof(struct SPUR));
	KeinePosition();
	if (s1 < s2) {
		for (n = s1; n < s2; n++) {
			memcpy(&spur[n], &spur[n + 1], sizeof(struct SPUR));
			SequenzenSpuren(n);
			ZeichneSpurSpalte(n, n == snum); ZeichneSequenzen(n, TRUE);
		}
	}
	if (s1 > s2) {
		for (n = s1; n > s2; n--) {
			memcpy(&spur[n], &spur[n - 1], sizeof(struct SPUR));
			SequenzenSpuren(n);
			ZeichneSpurSpalte(n, n == snum); ZeichneSequenzen(n, TRUE);
		}
	}
	memcpy(&spur[s2], &ksp, sizeof(struct SPUR));
	SequenzenSpuren(s2);
	ZeichneSpurSpalte(s2, s2 == snum); ZeichneSequenzen(s2, TRUE);
	ZeichnePosition(TRUE);
}

void SpurDuplizieren(WORD s) {
	WORD neus;
	
	if (lied.spuranz < verSPUREN) {
		lied.spuranz++; neus = lied.spuranz - 1;
		strncpy(spur[neus].name, spur[s].name, 128);
		memcpy(&spur[neus], &spur[s], sizeof(struct SPUR));
		spur[neus].seq = NULL;
		spur[neus].aktseq = NULL;
		spur[neus].aktevbl = NULL;
		spur[neus].aktevnum = 0;
		snum = s + 1;
		SpurVerschieben(neus, snum);
		ZeichneSpurSpalte(s, FALSE);
		ZeichneInfobox(4);
		AktualisiereGadgets();
	} else {
		if (verLITE) Meldung(CAT(MSG_0009, "In Lite version, it's not possible\nto create more than 8 tracks"));
		else Meldung(CAT(MSG_0010, "It's not possible to create more tracks"));
	}
}


void SpurMuteSchalter(WORD s) {
	spur[s].mute = !spur[s].mute;
	if (spur[s].mute) SpurAbklingen(s);
	ZeichneSpurSpalte(s, s == snum);
}

void SpurSolo(WORD s) {
	WORD n;
	
	spur[s].mute = FALSE; ZeichneSpurSpalte(s, s == snum);
	for (n = 0; n < lied.spuranz; n++) {
		if (n != s) {
			spur[n].mute = TRUE; ZeichneSpurSpalte(n, n == snum);
			SpurAbklingen(n);
		}
	}
}

void SpurenMutesAus(void) {
	WORD s;
	
	for (s = 0; s < lied.spuranz; s++) {
		if (spur[s].mute) {
			spur[s].mute = FALSE;
			ZeichneSpurSpalte(s, s == snum);
		}
	}
}
