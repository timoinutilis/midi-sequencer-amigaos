#include <exec/types.h>
#include <stdio.h>
#include <string.h>

#define _KEINEEXTERN_
#include "Versionen.h"
#undef _KEINEEXTERN_

BOOL verLITE;
BYTE verOUTPORTS;
BYTE verINPORTS;
WORD verSPUREN;

char regname[50];

struct KEY {
	char revtxt[48];
	short sum;
	char nrmtxt[48];
};


STRPTR RegisterName(void) {
	return (STRPTR)regname;
}


short testReverse(char *z, char *q, short l) {
	short p;
	
	for (p = 0; p < l; p++) {
		if (z[p] != q[l - p - 1]) return 0;
	}
	return 1;
}

void decodeText(long *s, short numwords, long keyword) {
	short p;
	
	for (p = 0; p < numwords; p++) s[p] ^= keyword;
}

short addChecksum(unsigned char *s, short l, short cs) {
	short p;
	
	for (p = 0; p < l; p++) cs += s[p];
	return cs;
}

char loadKey(struct KEY *key, char *n) {
	FILE *fp;
	fp = fopen(n, "rb");
	if (fp) {
		fread(key, sizeof(struct KEY), 1, fp);
		fclose(fp);
		return 1;
	}
	return 0;
}


void TesteKey(void) {
	struct KEY key;
	short sum;
	char keyok = 0;
	char *end;

	if (loadKey(&key, "PROGDIR:System/horny.key")) {
		sum = addChecksum(key.revtxt, 48, 0);
		sum = addChecksum(key.nrmtxt, 48, sum);
		if (sum == key.sum) {
			decodeText((long *)key.revtxt, 48/sizeof(long), 0x92AB0F23);
			decodeText((long *)key.nrmtxt, 48/sizeof(long), 0x94837FF6);
			if (testReverse(key.revtxt, key.nrmtxt, 48)) {
				memcpy(regname, key.nrmtxt, 48);
				end = strchr(regname, '|');
				if (end) *end = 0;
				keyok = 1;
			}
		}
	}

	if (keyok) { // Vollversion
		verLITE = FALSE;
		verOUTPORTS = OUTPORTS;
		verINPORTS = INPORTS;
		verSPUREN = SPUREN;
	} else { // Lite
		verLITE = TRUE;
		verOUTPORTS = 1;
		verINPORTS = 1;
		verSPUREN = 16;
	}
};
