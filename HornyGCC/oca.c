// oca.c
// Open Close All
// Inits globals, opens classes and devices
// Closes everything when needed.

#include <proto/dos.h>
#include <proto/diskfont.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <proto/utility.h>
#include <proto/intuition.h>
#include <proto/application.h>

#include "oca.h"

	// here's all the global stuff
	// all inits as NULL so we can tell if we got there yet.

// Libraries
struct AslIFace         *IAsl         = NULL;
struct DataTypesIFace   *IDataTypes   = NULL;
struct DiskfontIFace    *IDiskfont    = NULL;
struct GraphicsIFace    *IGraphics    = NULL;
struct IconIFace        *IIcon        = NULL;
struct IntuitionIFace   *IIntuition   = NULL;
struct LocaleIFace      *ILocale      = NULL;
struct UtilityIFace     *IUtility     = NULL;
struct WorkbenchIFace   *IWorkbench   = NULL;

// Classes
struct ClassLibrary *BitmapBase       = NULL;
Class *BitmapClass;

struct ClassLibrary *ButtonBase       = NULL;
Class *ButtonClass;

struct ClassLibrary *CheckBoxBase     = NULL;
Class *CheckBoxClass;
struct CheckBoxIFace     *ICheckBox   = NULL;

struct ClassLibrary *ChooserBase      = NULL;
Class *ChooserClass;
struct ChooserIFace     *IChooser     = NULL;

struct ClassLibrary *ClickTabBase     = NULL;
Class *ClickTabClass;
struct ClickTabIFace     *IClickTab   = NULL;

struct ClassLibrary *GetFileBase      = NULL;
Class *GetFileClass;
struct GetFileIFace     *IGetFile     = NULL;

struct ClassLibrary *GetScreenModeBase = NULL;
Class *GetScreenModeClass;
struct GetScreenModeIFace *IGetScreenMode = NULL;

struct ClassLibrary *IntegerBase      = NULL;
Class *IntegerClass;
struct IntegerIFace *IInteger         = NULL;

struct ClassLibrary *LabelBase        = NULL;
Class *LabelClass;
struct LabelIFace *ILabel             = NULL;

struct ClassLibrary *LayoutBase       = NULL;
Class  *LayoutClass;
struct LayoutIFace      *ILayout      = NULL;

struct ClassLibrary *ListBrowserBase  = NULL;
Class *ListBrowserClass;
struct ListBrowserIFace *IListBrowser = NULL;

struct ClassLibrary *RadioButtonBase  = NULL;
Class *RadioButtonClass;
struct RadioButtonIFace *IRadioButton = NULL;

struct ClassLibrary *ScrollerBase     = NULL;
Class *ScrollerClass;
struct ScrollerIFace *IScroller       = NULL;

struct ClassLibrary *SliderBase       = NULL;
Class *SliderClass;
struct SliderIFace *ISlider           = NULL;

struct ClassLibrary *StringBase       = NULL;
Class *StringClass;
struct StringIFace      *IString      = NULL;

struct ClassLibrary *WindowBase       = NULL;
Class *WindowClass;
struct WindowIFace      *IWindow      = NULL;




/*
 *	Opens all libraries, classes, devices
 *   returns RETURN_OK, else RETURN_FAIL
 */
int32 openAll(int argc, char **argv)
{

	if(
		(!(IAsl = openInterface("asl.library",
			MAJ_REV, "main", 1L)))										||
		(!(IDataTypes = openInterface("datatypes.library",
			MAJ_REV, "main", 1L)))										||
		(!(IDiskfont = openInterface("diskfont.library",
			MAJ_REV, "main", 1L)))										||
		(!(IGraphics = openInterface("graphics.library", 
			MAJ_REV, "main", 1L)))										||
		(!(IIcon = openInterface("icon.library", 
			MAJ_REV, "main", 1L))) 										||
		(! (IIntuition = 	openInterface("intuition.library",
			MAJ_REV, "main", 1L)))										||
		(! (ILocale = 	openInterface("locale.library",
			MAJ_REV, "main", 1L)))										||
		(! (IUtility = openInterface("utility.library", 
			MAJ_REV, "main", 1L)))										||
		(! (IWorkbench = openInterface("workbench.library", 
			MAJ_REV, "main", 1L)))										||
		(!(BitmapBase = openClass("images/bitmap.image", 
			MAJ_REV, &BitmapClass)))                           ||
		(!(ButtonBase = openClass("gadgets/button.gadget", 
			MAJ_REV, &ButtonClass)))                           ||
		(!(ChooserBase = openClass("gadgets/chooser.gadget", 
			MAJ_REV, &ChooserClass)))                          ||
		(!(IChooser = (APTR)IExec->GetInterfaceTags((struct Library *)ChooserBase,"main",1,TAG_END))) ||
		(!(CheckBoxBase = openClass("gadgets/checkbox.gadget", 
			MAJ_REV, &CheckBoxClass)))                         ||
		(!(ICheckBox = (APTR)IExec->GetInterfaceTags((struct Library *)CheckBoxBase,"main",1,TAG_END))) ||
		(!(ClickTabBase = openClass("gadgets/clicktab.gadget", 
			MAJ_REV, &ClickTabClass)))                         ||
		(!(IClickTab = (APTR)IExec->GetInterfaceTags((struct Library *)ClickTabBase,"main",1,TAG_END))) ||
		(!(GetFileBase = openClass("gadgets/getfile.gadget", 
			MAJ_REV, &GetFileClass)))                          ||
		(!(IGetFile = (APTR)IExec->GetInterfaceTags((struct Library *)GetFileBase,"main",1,TAG_END))) ||
		(!(GetScreenModeBase = openClass("gadgets/getscreenmode.gadget", 
			MAJ_REV, &GetScreenModeClass)))                    ||
		(!(IGetScreenMode = (APTR)IExec->GetInterfaceTags((struct Library *)GetScreenModeBase,"main",1,TAG_END))) ||
		(!(IntegerBase = openClass("gadgets/integer.gadget", 
			MAJ_REV, &IntegerClass)))                          ||
		(!(IInteger = (APTR)IExec->GetInterfaceTags((struct Library *)IntegerBase,"main",1,TAG_END))) ||
		(!(LabelBase = openClass("images/label.image", 
			MAJ_REV, &LabelClass)))                            ||
		(!(ILabel = (APTR)IExec->GetInterfaceTags((struct Library *)LabelBase,"main",1,TAG_END))) ||
		(!(LayoutBase = openClass("gadgets/layout.gadget", 
			MAJ_REV, &LayoutClass)))                           ||
		(!(ILayout = (APTR)IExec->GetInterfaceTags((struct Library *)LayoutBase,"main",1,TAG_END))) ||
		(!(ListBrowserBase = openClass("gadgets/listbrowser.gadget", 
			MAJ_REV, &ListBrowserClass)))                      ||
		(!(IListBrowser = (APTR)IExec->GetInterfaceTags((struct Library *)ListBrowserBase,"main",1,TAG_END))) ||
		(!(RadioButtonBase = openClass("gadgets/radiobutton.gadget", 
			MAJ_REV, &RadioButtonClass)))                      ||
		(!(IRadioButton = (APTR)IExec->GetInterfaceTags((struct Library *)RadioButtonBase,"main",1,TAG_END))) ||
		(!(ScrollerBase = openClass("gadgets/scroller.gadget",
			MAJ_REV, &ScrollerClass)))                         ||
		(!(IScroller = (APTR)IExec->GetInterfaceTags((struct Library *)ScrollerBase,"main",1,TAG_END))) ||
		(!(SliderBase = openClass("gadgets/slider.gadget",
			MAJ_REV, &SliderClass)))                           ||
		(!(ISlider = (APTR)IExec->GetInterfaceTags((struct Library *)SliderBase,"main",1,TAG_END))) ||
		(!(StringBase = openClass("gadgets/string.gadget", 
			MAJ_REV, &StringClass)))                           ||
		(!(IString = (APTR)IExec->GetInterfaceTags((struct Library *)StringBase,"main",1,TAG_END))) ||
		(!(WindowBase = openClass("window.class", 
			MAJ_REV, &WindowClass)))                           ||
		(!(IWindow = (APTR)IExec->GetInterfaceTags((struct Library *)WindowBase,"main",1,TAG_END))) )
	{
		return(closeAll(NULL));
	}

	return(RETURN_OK);	// success!
}


// note: closeAll calls may be nested, 
//  so NULL what you have returned
// This function ALWAYS returns RETURN_FAIL.
int32 closeAll(CONST_STRPTR reason)
{

	errMessage(reason);

	IExec->DropInterface((struct Interface *)IWindow);
	IWindow = NULL;
	IExec->DropInterface((struct Interface *)IString);
	IString = NULL;
	IExec->DropInterface((struct Interface *)ISlider);
	ISlider = NULL;
	IExec->DropInterface((struct Interface *)IScroller);
	IScroller = NULL;
	IExec->DropInterface((struct Interface *)IRadioButton);
	IRadioButton = NULL;
	IExec->DropInterface((struct Interface *)IListBrowser);
	IListBrowser = NULL;
	IExec->DropInterface((struct Interface *)ILayout);
	ILayout = NULL;
	IExec->DropInterface((struct Interface *)IInteger);
	IInteger = NULL;
	IExec->DropInterface((struct Interface *)IGetScreenMode);
	IGetScreenMode = NULL;
	IExec->DropInterface((struct Interface *)IGetFile);
	IGetFile = NULL;
	IExec->DropInterface((struct Interface *)IClickTab);
	IClickTab = NULL;
	IExec->DropInterface((struct Interface *)ICheckBox);
	ICheckBox = NULL;
	IExec->DropInterface((struct Interface *)IChooser);
	IChooser = NULL;

	if(IIntuition)
	{
		IIntuition->CloseClass(WindowBase);
		WindowBase = NULL;
		IIntuition->CloseClass(StringBase);
		StringBase = NULL;
		IIntuition->CloseClass(SliderBase);
		SliderBase = NULL;
		IIntuition->CloseClass(ScrollerBase);
		ScrollerBase = NULL;
		IIntuition->CloseClass(RadioButtonBase);
		RadioButtonBase = NULL;
		IIntuition->CloseClass(ListBrowserBase);
		ListBrowserBase = NULL;
		IIntuition->CloseClass(LayoutBase);
		LayoutBase = NULL;
		IIntuition->CloseClass(LabelBase);
		LabelBase = NULL;
		IIntuition->CloseClass(IntegerBase);
		IntegerBase = NULL;
		IIntuition->CloseClass(GetScreenModeBase);
		GetScreenModeBase = NULL;
		IIntuition->CloseClass(GetFileBase);
		GetFileBase = NULL;
		IIntuition->CloseClass(ClickTabBase);
		ClickTabBase = NULL;
		IIntuition->CloseClass(CheckBoxBase);
		CheckBoxBase = NULL;
		IIntuition->CloseClass(ChooserBase);
		ChooserBase = NULL;
		IIntuition->CloseClass(ButtonBase);
		ButtonBase = NULL;
		IIntuition->CloseClass(BitmapBase);
		BitmapBase = NULL;
	}

	closeInterface(IWorkbench);
	IWorkbench = NULL;

	closeInterface(IUtility);
	IUtility   = NULL;

	closeInterface(ILocale);
	ILocale = NULL;

	closeInterface(IIntuition);
	IIntuition = NULL;

	closeInterface(IIcon);
	IIcon      = NULL;

	closeInterface(IGraphics);
	IGraphics  = NULL;

	closeInterface(IDiskfont);
	IDiskfont  = NULL;

	closeInterface(IDataTypes);
	IDataTypes = NULL;

	closeInterface(IAsl);
	IAsl = NULL;

	return(RETURN_FAIL);
}

// find a non-NIL: output handle, or open a CON:
// Write the given message to it.
void errMessage(CONST_STRPTR reason)
{
	if((!reason)||(0 == reason[0]))
	{
		return;
	}

	BPTR err_out = IDOS->ErrorOutput();
	if((err_out) && (IDOS->IsInteractive(err_out)))
	{
		IDOS->FPuts(err_out, reason);
	}
	else
	{
		err_out = IDOS->Output();
		if((err_out) && (IDOS->IsInteractive(err_out)))
		{
			IDOS->FPuts(err_out, reason);
		}
		else
		{
			if((err_out = IDOS->FOpen("Con:0/0/500/300/Error Output", MODE_NEWFILE, 256)))
			{
				IDOS->FPuts(err_out, reason);
				IDOS->Delay(200);
				IDOS->FClose(err_out);
			}
		}
	}
}

APTR openClass(CONST_STRPTR className, uint32 classVers, Class **classPtr)
{
	APTR result = NULL;
	char error[512];

	if((result = IIntuition->OpenClass(className, classVers, classPtr)))
	{
		return(result);
	}

	IUtility->Strlcpy(error, "Failed to open Class ", sizeof(error));
	IUtility->Strlcat(error, className, sizeof(error));
	IUtility->Strlcat(error, "\n", sizeof(error));
	errMessage(error);
	return(NULL);
}

APTR openInterface (CONST_STRPTR lib_name, int32 lib_vers,
   CONST_STRPTR int_name, int32 int_vers)
{
   struct Library   *library;
   struct Interface *interface;
   STRPTR buffer;

   library = IExec->OpenLibrary(lib_name, lib_vers);
   if (library)
   {
      interface = IExec->GetInterface(library, 
      	int_name, int_vers, NULL);
      if (interface)
      {
         return interface;
      }
      IExec->CloseLibrary(library);

		if((IUtility)&&((buffer = IUtility->ASPrintf(
			"Failed to get %s interface version %ld from %s\n",
			int_name, int_vers, lib_name))))
		{
			errMessage(buffer);
			IExec->FreeVec(buffer);
			return(NULL);
		}

   }

	if((IUtility)&&((buffer = IUtility->ASPrintf(
		"Failed to open %s version %ld\n",
		lib_name, lib_vers))))
	{
		errMessage(buffer);
		IExec->FreeVec(buffer);
		return(NULL);
	}

	errMessage("Failed to open a library");


   return NULL;
}

void closeInterface (APTR interface)
{
   if (interface)
   {
      struct Library *library = ((struct Interface *)
      	interface)->Data.LibBase;
      IExec->DropInterface(interface);
      IExec->CloseLibrary(library);
   }
}


