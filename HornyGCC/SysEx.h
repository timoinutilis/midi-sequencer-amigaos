#include <exec/types.h>

void AktualisierePortChooserListe(void);

struct SYSEXMSG *NeuesSysEx(struct SYSEXUNIT *unit, STRPTR name, uint32 len, uint8 *data);
void EntferneSysEx(struct SYSEXUNIT *unit, struct SYSEXMSG *sysex);
void EntferneAlleSysEx(struct SYSEXUNIT *unit);
void SysExAufnehmen(void);
void SendeAlleSysEx(void);

struct SYSEXUNIT *NeueSysExUnit(STRPTR name);
void EntferneSysExUnit(struct SYSEXUNIT *unit);
void EntferneAlleSysExUnits(void);

void AktualisiereSysExGruppenListe(void);
void AktualisiereSysExMsgListe(void);
void ErstelleSysExFenster(void);
void EntferneSysExFenster(void);
BOOL KontrolleSysExFenster(void);
