#include <exec/types.h>

void AddEdUndo(struct SEQUENZ *seq, STRPTR aktion);
BOOL EdUndo(struct SEQUENZ *seq);
BOOL EdRedo(struct SEQUENZ *seq);
void EntferneAlleEdUndo();
STRPTR EdUndoAktion();
STRPTR EdRedoAktion();
