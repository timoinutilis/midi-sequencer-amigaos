#include <exec/types.h>

void InitMPKanaele(void);
void SammleMPKanaele(void);
void ErstelleMPFenster(void);
void EntferneMPFenster(void);
void KontrolleMischpultFenster(void);

void SetzeMeter(uint8 p, uint8 c, int8 velo);
void ErniedrigeMeter(int8 wert);
void ZeichneMeter(void);
void SendeMischpult(void);
void AktualisiereMischpult(void);
void AutoUpdateMischpult(void);
void ControllerAnpassen(int8 p, int8 c, int8 data1, int8 data2);
