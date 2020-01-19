void LadeAlleInstrumente(void);
void EntferneAlleInstrumente(void);
struct INSTRUMENT *SucheInstrument(STRPTR name);
int16 SucheInstrumentNum(STRPTR name);
struct INSTRUMENT *NtesInstrument(int16 n);
struct INSTRUMENT *SucheChannelInstrument(uint8 port, int8 channel);
void TesteInstrumente(void);

void InstrumentenFenster2(struct Window *fen, int8 channel, uint8 port, int8 *bank0, int8 *bank32, int8 *prog);
int8 InstrControllerFenster(struct Window *fen, int8 channel, uint8 port, int8 vorwahl);
