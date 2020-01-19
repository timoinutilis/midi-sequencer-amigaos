#include <exec/types.h>

void ErstelleAslReqs(void);
void EntferneAslReqs(void);
BOOL AslProjLaden(void);
BOOL AslProjSpeichern(void);
int32 Frage(STRPTR text, STRPTR knopf);
void Meldung(STRPTR text);
BOOL AslSMFLaden(void);
BOOL AslSMFSpeichern(void);
BOOL AslSysExLaden(void);
BOOL AslSysExSpeichern(void);
