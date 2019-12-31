#include <proto/wb.h>
#include <proto/dos.h>

#include <workbench/startup.h>

struct AppWindow *appwindow = NULL;
extern struct Screen *hschirm;
extern struct Window *hfenster;


void AppWindowAnmelden(void) {
	appwindow = AddAppWindowA(0, 0, hfenster, hfenster->UserPort, NULL);
}

void AppWindowAbmelden(void) {
	if (appwindow) {
		RemoveAppWindow(appwindow);
		appwindow = NULL;
	}
}

STRPTR HoleAppMessageDatei(struct IntuiMessage *msg) {
	struct AppMessage *amsg;
	
	amsg = (struct AppMessage *)msg;
	CurrentDir(amsg->am_ArgList->wa_Lock);
	return(amsg->am_ArgList->wa_Name);
}
