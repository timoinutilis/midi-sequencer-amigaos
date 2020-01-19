#ifndef OCA_H
#define OCA_H

#include <proto/intuition.h>

#define MAJ_REV 53

// Libraries
extern struct AslIFace *IAsl;
extern struct DiskfontIFace *IDiskfont;
extern struct GraphicsIFace *IGraphics;
extern struct IconIFace *IIcon;
extern struct IntuitionIFace *IIntuition;
extern struct LocaleIFace *ILocale;
extern struct UtilityIFace *IUtility;
extern struct WorkbenchIFace *IWorkbench;

// Gadget Classes
extern struct ClassLibrary *BitmapBase;
extern Class *BitmapClass;

//extern struct ClassLibrary *ButtonBase;
extern Class *ButtonClass;

//extern struct ClassLibrary *CheckBoxBase;
extern Class *CheckBoxClass;
extern struct CheckBoxIFace *ICheckBox;

//extern struct ClassLibrary *ChooserBase;
extern Class *ChooserClass;
extern struct ChooserIFace *IChooser;

extern struct ClassLibrary *ClickTabBase;
extern Class *ClickTabClass;
extern struct ClickTabIFace *IClickTab;

extern struct ClassLibrary *GetFileBase;
extern Class *GetFileClass;
extern struct GetFileIFace *IGetFile;

extern struct ClassLibrary *GetScreenModeBase;
extern Class *GetScreenModeClass;
extern struct GetScreenModeIFace *IGetScreenMode;

//extern struct ClassLibrary *IntegerBase;
extern Class *IntegerClass;
extern struct IntegerIFace *IInteger;

extern struct ClassLibrary *LabelBase;
extern Class *LabelClass;

extern struct ClassLibrary *LayoutBase;
extern Class *LayoutClass;
extern struct LayoutIFace *ILayout;

extern struct ClassLibrary *ListBrowserBase;
extern Class *ListBrowserClass;
extern struct ListBrowserIFace *IListBrowser;

extern struct ClassLibrary *RadioButtonBase;
extern Class *RadioButtonClass;
extern struct RadioButtonIFace *IRadioButton;

//extern struct ClassLibrary *ScrollerBase;
extern Class *ScrollerClass;
extern struct ScrollerIFace *IScroller;

//extern struct ClassLibrary *SliderBase;
extern Class *SliderClass;
extern struct SliderIFace *ISlider;

extern struct ClassLibrary *SpaceBase;
extern Class *SpaceClass;

//extern struct ClassLibrary *StringBase;
extern Class *StringClass;
extern struct StringIFace *IString;

extern struct ClassLibrary *WindowBase;
extern Class *WindowClass;
extern struct WindowIFace *IWindow;


//
// Opens all libraries, classes, devices
// reads all args and tooltypes
// returns RETURN_OK, else RETURN_FAIL
//
int32 openAll(int argc, char **argv);

// always safe to call, handles any intermediate states
// displays any given reason by errMessage()
// ALWAYS returns RETURN_FAIL
int32 closeAll(CONST_STRPTR reason);

// use any available output, or create one if needed
void errMessage(CONST_STRPTR reason);

// easy OpenLibrary & GetInterface in one
// no need to store LibraryBase, as long as closeInterface is used to cleanup
// displays error details by errMessage if needed
APTR openInterface (CONST_STRPTR lib_name, int32 lib_vers,
CONST_STRPTR int_name, int32 int_vers);
void closeInterface (APTR interface);

// IIntuition->OpenClass() with error text output
APTR openClass(CONST_STRPTR className, uint32 classVers, Class **classPtr);

#endif