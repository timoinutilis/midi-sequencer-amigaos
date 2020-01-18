#include <exec/types.h>

void EntferneLied(void);
void InitLied(void);
void NeuesLied(void);

void StartProjekt(STRPTR startdatei);

void ProjektLadenKomplett(void);
void MinMenuKontrolle(uint32 item);
void Stoppen(void);
void StoppenZero(void);
void WiedergabeStarten(BOOL remote);
void AufnahmeStarten(void);
void SpringeTakt(int32 t);
void Springe(int8 n);
void TransportKontrolle(uint8 taste);
void TransportKontrolleRaw(uint8 taste);
