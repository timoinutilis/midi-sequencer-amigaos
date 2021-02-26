#include <string.h>

#include <proto/exec.h>

STRPTR String_Copy(STRPTR dest, STRPTR src) {
	STRPTR newstr;

	newstr = (STRPTR)IExec->AllocVecTags(strlen(src) + 1, TAG_END);
	if (newstr) {
		strcpy(newstr, src);
		if (dest) IExec->FreeVec(dest);
		return(newstr);
	} else return(dest);
}

STRPTR String_Cat(STRPTR dest, STRPTR src) {
	STRPTR newstr;
	
	if (dest) {
		newstr = (STRPTR)IExec->AllocVecTags(strlen(dest) + strlen(src) + 1, TAG_END);
		if (newstr) {
			strcpy(newstr, dest);
			strcat(newstr, src);
			IExec->FreeVec(dest);
			return(newstr);
		} else return(dest);
	} else return(String_Copy(dest, src));
}

void String_Free(STRPTR str) {
	if (str) IExec->FreeVec(str);
}
