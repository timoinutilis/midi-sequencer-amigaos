#include <exec/types.h>

void InitOutPortInstr(BYTE n);
void ErstelleCamd(void);
void EntferneCamd(void);
void WarteStart(WORD s);
void WarteEnde(void);
void ErstelleLinks(void);
BOOL LinkVorhanden(STRPTR name);
void InitOutportLatenzen();
void ErneuereLinks(void);
void EntferneLinks(void);

void AktiviereExtreamSync(void);
void DeaktiviereExtreamSync(void);
BOOL IstExtreamSyncAktiv(void);
void KontrolleExtreamSync(void);
void StartExtreamSync(void);
void StopExtreamSync(void);
void LocateExtreamSync(void);

void ResetteLMarker(void);
void ResetteZeit(void);
void AktualisiereTakt(void);
void TesteMidiThru(void);
void SendeEvent(WORD s, UBYTE status, UBYTE data1, UBYTE data2);
void SendeKanalEvent(BYTE port, BYTE channel, UBYTE status, UBYTE data1, UBYTE data2);
BOOL AddEvent(WORD s, LONG t, UBYTE status, UBYTE data1, UBYTE data2);
BOOL SpurAufnehmen(WORD s);
LONG vorgeschobenerPortTakt(WORD p, LONG zeit);
LONG verschobenerPortTakt(WORD p, LONG zeit);
void AbspielenVorbereiten(WORD s);
void LoopVorbereiten(WORD s);
void ResetteLoopZeit(void);
void SpurAbspielen(WORD s, LONG *nexteventtakt);
void KanalAbklingen(BYTE port, BYTE channel);
void SpurAbklingen(WORD s);
void SpieleMetronom(void);
void TesteMetronom(BYTE taste);
void SetzeAktInstrument(BYTE p, BYTE c, BYTE bank0, BYTE bank32, BYTE prog);
void SendeKanalInstrument(BYTE p, BYTE c, BYTE bank0, BYTE bank32, BYTE prog);
void SendeInstrument(WORD s);
void Panik(void);
void ThruProcess(void);
void PlayerProcess(void);
