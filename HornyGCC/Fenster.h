void ChannelPortFenster(struct Window *fen, uint8 *ch, uint8 *po);
void StringFenster(struct Window *fen, STRPTR t, int16 l);
int16 QuantisierungsFenster(struct Window *fen, BOOL fein);
int16 IntegerFenster(struct Window *fen, int16 w, int16 min, int16 max);
void InstrumentenFenster(struct Window *fen, int8 *bank0, int8 *bank32, int8 *prog);
BOOL DynamikFenster(struct Window *fen, int8 *threshold, int8 *ratio, int8 *gain);
