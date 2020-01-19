#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include <proto/application.h>
#include <proto/wb.h>
#include <libraries/application.h>

#include <exec/exec.h>
#include <workbench/startup.h>

#include "Versionen.h"
#include "Dynamic_Strings.h"
#include "Strukturen.h"
#include "oca.h"

void haupt(STRPTR startdatei);


char *ver = (STRPTR)"$VER: Horny 1.4 (for OS4 PPC)";


//==================================================================================
// OS 4 Specials
//==================================================================================


// **** Application Library ****

uint32 start_appID = 0;
struct MsgPort *start_appPort = NULL;
uint32 start_appSigMask = 0;

struct Library           *ApplicationBase = NULL;
struct ApplicationIFace  *IApplication    = NULL;

BOOL start_newPhonolithProject = FALSE;
STRPTR start_phonolithProject = NULL;

extern struct UMGEBUNG umgebung;

void openOS4Lib()
{
	ApplicationBase = IExec->OpenLibrary("application.library", 50);
	if (ApplicationBase)
	{
		IApplication = (struct ApplicationIFace *) IExec->GetInterface(ApplicationBase, "application", 2, NULL);
	}
}

void closeOS4Lib()
{
	IExec->DropInterface((struct Interface *) IApplication);
	IApplication = NULL;
	IExec->CloseLibrary(ApplicationBase);
	ApplicationBase = NULL;
}

void registerApplication(struct WBStartup *wbstartup)
{
	if (IApplication)
	{
		start_appID = IApplication->RegisterApplication("Horny",
				REGAPP_URLIdentifier, "inutilis.de",
				REGAPP_WBStartup, wbstartup,
				REGAPP_LoadPrefs, TRUE,
				REGAPP_AppNotifications, TRUE,
				//REGAPP_NoIcon, TRUE,
				TAG_DONE);

		IApplication->GetApplicationAttrs(start_appID, APPATTR_Port, &start_appPort, TAG_DONE);
		start_appSigMask = 1L << start_appPort->mp_SigBit;
	}
}

void unregisterApplication()
{
	if (IApplication)
	{
		IApplication->UnregisterApplication(start_appID, NULL);
	}
}

void loadPhonolithProject(STRPTR datei) {
	if (IApplication) {
		uint32 appID = IApplication->FindApplication(FINDAPP_AppIdentifier, "Phonolith.inutilis.de", TAG_DONE);

		if (appID)
		{
			struct MsgPort *appPort = NULL;
			struct ApplicationOpenPrintDocMsg appMsg;

			IApplication->GetApplicationAttrs(appID, APPATTR_Port, &appPort, TAG_DONE);

			memset(&appMsg, 0, sizeof(struct ApplicationOpenPrintDocMsg));
			appMsg.fileName = datei;
			if (IApplication->SendApplicationMsg(start_appID, appID, (struct ApplicationMsg *)&appMsg, APPLIBMT_OpenDoc))
			{
				start_newPhonolithProject = FALSE;
			}
		}
	}
}

void newPhonolithProject(STRPTR datei)
{
	if (IApplication) {
		if (datei != NULL && strlen(datei) > 0)
		{
			start_newPhonolithProject = TRUE;
			start_phonolithProject = datei;
			loadPhonolithProject(start_phonolithProject);
			if (start_newPhonolithProject) //not yet loaded
			{
				//start Phonolith
				IWorkbench->OpenWorkbenchObject(umgebung.pfadphonolith, TAG_DONE);
			}
		}
	}
}

void checkPhonolithProject()
{
	if (start_newPhonolithProject)
	{
		loadPhonolithProject(start_phonolithProject);
	}
}


// **** AmiUpdate ****

void SetAmiUpdateENVVariable( char *varname )
{
  /* AmiUpdate support code */

  BPTR lock;
  APTR oldwin;

  /* obtain the lock to the home directory */
  if(( lock = IDOS->GetProgramDir() ))
  {
    TEXT progpath[2048];
    TEXT varpath[1024] = "AppPaths";

    /*
    get a unique name for the lock,
    this call uses device names,
    as there can be multiple volumes
    with the same name on the system
    */

	if( IDOS->DevNameFromLock( lock, progpath, sizeof(progpath), DN_FULLPATH ))
    {
	  /* stop any "Insert volume..." type requesters */
	  oldwin = IDOS->SetProcWindow((APTR)-1);

      /*
      finally set the variable to the
      path the executable was run from
      don't forget to supply the variable
      name to suit your application
      */

	  IDOS->AddPart( varpath, varname, 1024);
	  IDOS->SetVar( varpath, progpath, -1, GVF_GLOBAL_ONLY|GVF_SAVE_VAR );

      /* turn requesters back on */
	  IDOS->SetProcWindow( oldwin );
    }
  }
}


//==========================================================================

void wbmain(struct WBStartup *argmsg) {
	struct WBArg *wbarg;
	STRPTR wbdatei = NULL;
	BPTR lock = 0;

	openOS4Lib();
	SetAmiUpdateENVVariable((STRPTR)"Horny");
	registerApplication(argmsg);

	
	if (argmsg->sm_NumArgs > 1) {
		wbarg = argmsg->sm_ArgList;
		wbarg++;
		wbdatei = wbarg->wa_Name;
		lock = wbarg->wa_Lock;
		IDOS->SetCurrentDir(lock);
	}
	
	haupt(wbdatei);	

	unregisterApplication();
	closeOS4Lib();
	exit(0);
}

int main(int argc, char *argv[]) {

	if(RETURN_OK != openAll(argc, argv))
	{
		return(RETURN_FAIL);
	}
	if (argc == 0)
	{
		struct WBStartup *wbstartup = (struct WBStartup *) argv;
		wbmain(wbstartup);
	}
	else
	{
		SetAmiUpdateENVVariable((STRPTR)"Horny");
	}
	if (argc > 1) haupt(argv[1]);
	else haupt(NULL);
	
	closeAll(NULL);
	
	exit(0);
}
