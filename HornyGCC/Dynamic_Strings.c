#include <string.h>

#include <proto/exec.h>

STRPTR String_Copy(STRPTR dest, STRPTR src) {
	STRPTR newstr;

	newstr = (STRPTR)AllocVec(strlen(src) + 1, 0);
	if (newstr) {
		strcpy(newstr, src);
		if (dest) FreeVec(dest);
		return(newstr);
	} else return(dest);
}

STRPTR String_Cat(STRPTR dest, STRPTR src) {
	STRPTR newstr;
	
	if (dest) {
		newstr = (STRPTR)AllocVec(strlen(dest) + strlen(src) + 1, 0);
		if (newstr) {
			strcpy(newstr, dest);
			strcat(newstr, src);
			FreeVec(dest);
			return(newstr);
		} else return(dest);
	} else return(String_Copy(dest, src));
}

void String_Free(STRPTR str) {
	if (str) FreeVec(str);
}
