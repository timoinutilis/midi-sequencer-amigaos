#include <exec/types.h>

void InitMPKanaele(void);
void SammleMPKanaele(void);
void ErstelleMPFenster(void);
void EntferneMPFenster(void);
void KontrolleMischpultFenster(void);

void SetzeMeter(UBYTE p, UBYTE c, BYTE velo);
void ErniedrigeMeter(BYTE wert);
void ZeichneMeter(void);
void SendeMischpult(void);
void AktualisiereMischpult(void);
void AutoUpdateMischpult(void);
void ControllerAnpassen(BYTE p, BYTE c, BYTE data1, BYTE data2);
