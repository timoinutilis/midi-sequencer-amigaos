void LadeAlleInstrumente(void);
void EntferneAlleInstrumente(void);
struct INSTRUMENT *SucheInstrument(STRPTR name);
WORD SucheInstrumentNum(STRPTR name);
struct INSTRUMENT *NtesInstrument(WORD n);
struct INSTRUMENT *SucheChannelInstrument(UBYTE port, BYTE channel);
void TesteInstrumente(void);

void InstrumentenFenster2(struct Window *fen, BYTE channel, UBYTE port, BYTE *bank0, BYTE *bank32, BYTE *prog);
BYTE InstrControllerFenster(struct Window *fen, BYTE channel, UBYTE port, BYTE vorwahl);
