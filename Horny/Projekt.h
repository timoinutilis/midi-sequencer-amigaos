#include <exec/types.h>

void EntferneLied(void);
void InitLied(void);
void NeuesLied(void);

void StartProjekt(STRPTR startdatei);

void ProjektLadenKomplett(void);
void MinMenuKontrolle(ULONG item);
void Stoppen(void);
void StoppenZero(void);
void WiedergabeStarten(BOOL remote);
void AufnahmeStarten(void);
void SpringeTakt(LONG t);
void Springe(BYTE n);
void TransportKontrolle(UBYTE taste);
void TransportKontrolleRaw(UBYTE taste);
