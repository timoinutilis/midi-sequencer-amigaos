void ChannelPortFenster(struct Window *fen, UBYTE *ch, UBYTE *po);
void StringFenster(struct Window *fen, STRPTR t, WORD l);
WORD QuantisierungsFenster(struct Window *fen, BOOL fein);
WORD IntegerFenster(struct Window *fen, WORD w, WORD min, WORD max);
void InstrumentenFenster(struct Window *fen, BYTE *bank0, BYTE *bank32, BYTE *prog);
BOOL DynamikFenster(struct Window *fen, BYTE *threshold, BYTE *ratio, BYTE *gain);
