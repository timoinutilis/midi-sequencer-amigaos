#include <dos/dos.h>
#include <proto/dos.h>
#include <workbench/startup.h>

void haupt(void);

char *ver="$VER: Inutilis Horny Version 0.2";

void wbmain(struct WBStartup *argmsg) {
	if (argmsg->sm_ArgList->wa_Lock) CurrentDir(argmsg->sm_ArgList->wa_Lock);
	haupt();
}

void main(void) {
	haupt();
}
