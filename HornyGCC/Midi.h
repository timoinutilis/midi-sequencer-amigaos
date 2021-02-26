#include <exec/types.h>

void InitOutPortInstr(int8 n);
void ErstelleCamd(void);
void EntferneCamd(void);
void WarteStart(int16 s);
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
void SendeEvent(int16 s, uint8 status, uint8 data1, uint8 data2);
void SendeKanalEvent(int8 port, int8 channel, uint8 status, uint8 data1, uint8 data2);
BOOL AddEvent(int16 s, int32 t, uint8 status, uint8 data1, uint8 data2);
BOOL SpurAufnehmen(int16 s);
int32 vorgeschobenerPortTakt(int16 p, int32 zeit);
int32 verschobenerPortTakt(int16 p, int32 zeit);
void AbspielenVorbereiten(int16 s);
void LoopVorbereiten(int16 s);
void ResetteLoopZeit(void);
void SpurAbspielen(int16 s, int32 *nexteventtakt);
void KanalAbklingen(int8 port, int8 channel);
void SpurAbklingen(int16 s);
void SpieleMetronom(void);
void TesteMetronom(int8 taste);
void SetzeAktInstrument(int8 p, int8 c, int8 bank0, int8 bank32, int8 prog);
void SendeKanalInstrument(int8 p, int8 c, int8 bank0, int8 bank32, int8 prog);
void SendeInstrument(int16 s);
void Panik(void);
void ThruProcess(void);
void PlayerProcess(void);
