CC = gcc
CFLAGS = -D __USE_INLINE__

../Horny/HornyOS4: \
		AppWindow.o \
		Automation.o \
		Clipboard.o \
		CtrlWandler.o \
		DiskHornyLaden.o \
		DiskHornySpeichern.o\
		DiskSMF.o \
		DTGrafik.o \
		Dynamic_Strings.o\
		EditorContrGui.o \
		EditorNoten.o\
		EditorNotenGui.o \
		Einstellungen.o\
		Fenster.o \
		Gui.o \
		Gui2.o \
		Instrumente.o \
		Main.o \
		Marker.o \
		Menu.o \
		Midi.o \
		MidiEdit.o \
		Mischpult.o \
		Projekt.o \
		Requester.o \
		Sequenzen.o \
		Smpte.o \
		Spuren.o \
		Start.o \
		SysEx.o \
		Umgebung.o \
		Undo.o \
		Versionen.o
	$(CC) -o $@ $^ -lauto -lraauto -lm -lamiga

AppWindow.o: AppWindow.c
	$(CC) $(CFLAGS) -c $<

Automation.o:	Automation.c \
				Strukturen.h \
				Versionen.h \
				Midi.h \
				Sequenzen.h \
				Requester.h \
				locale.h \
				catalogids.h 
	$(CC) $(CFLAGS) -c $<

Clipboard.o:	Clipboard.c \
				Strukturen.h \
				Versionen.h \
				Sequenzen.h
	$(CC) $(CFLAGS) -c $<

CtrlWandler.o:	CtrlWandler.c \
				Strukturen.h \
				Gui.h \
				GuiFenster.h \
				Menu.h \
				Projekt.h \
				Requester.h \
				Instrumente.h \
				CtrlWandler.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

DiskHornyLaden.o:	DiskHornyLaden.c \
				Strukturen.h \
				Projekt.h \
				Requester.h \
				Marker.h \
				dateiformat.h \
				Sequenzen.h \
				Midi.h \
				SysEx.h \
				Versionen.h \
				Automation.h \
				locale.h \
				catalogids.h \
				CtrlWandler.h
	$(CC) $(CFLAGS) -c $<

DiskHornySpeichern.o:	DiskHornySpeichern.c \
				Strukturen.h \
				Requester.h \
				dateiformat.h \
				Versionen.h \
				locale.h \
				catalogids.h \
				CtrlWandler.h
	$(CC) $(CFLAGS) -c $<

DiskSMF.o:		DiskSMF.c \
				Strukturen.h \
				Midi.h \
				Sequenzen.h \
				Requester.h \
				Marker.h \
				Projekt.h \
				SysEx.h \
				Versionen.h \
				locale.h \
				catalogids.h \
				Smpte.h
	$(CC) $(CFLAGS) -c $<

DTGrafik.o:		DTGrafik.c \
				DTGrafik.h \
				Gui.h \
				Versionen.h
	$(CC) $(CFLAGS) -c $<

Dynamic_Strings.o:	Dynamic_Strings.c
	$(CC) $(CFLAGS) -c $<

EditorContrGui.o:	EditorContrGui.c \
				Strukturen.h \
				Gui.h \
				Instrumente.h \
				Gui2.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

EditorNoten.o:	EditorNoten.c \
				Strukturen.h \
				EditorNotenGui.h \
				Gui.h \
				Gui2.h \
				Sequenzen.h \
				Requester.h \
				MidiEdit.h \
				Menu.h \
				Midi.h \
				Projekt.h \
				Fenster.h \
				EditorContrGui.h \
				Instrumente.h \
				Marker.h \
				Undo.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

EditorNotenGui.o:	EditorNotenGui.c \
				Strukturen.h \
				Gui.h \
				Marker.h \
				Menu.h \
				Gui2.h \
				DTGrafik.h \
				Undo.h \
				locale.h \
				catalogids.h \
				GuiFenster.h
	$(CC) $(CFLAGS) -c $<

Einstellungen.o:	Einstellungen.c \
				Strukturen.h \
				Midi.h \
				Fenster.h \
				Gui.h \
				Menu.h \
				Projekt.h \
				SysEx.h \
				Instrumente.h \
				Versionen.h \
				Gui2.h \
				Smpte.h \
				Marker.h \
				Requester.h \
				locale.h \
				catalogids.h \
				GuiFenster.h
	$(CC) $(CFLAGS) -c $<

Fenster.o:		Fenster.c \
				Strukturen.h \
				Gui.h \
				Versionen.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Gui.o:			Gui.c \
				Strukturen.h \
				Versionen.h \
				Requester.h \
				Images.h \
				Gui.h \
				Marker.h \
				DTGrafik.h \
				catalogids.h \
				GuiFenster.h
	$(CC) $(CFLAGS) -c $<

Gui2.o:			Gui2.c \
				Strukturen.h \
				Gui.h \
				Marker.h \
				Smpte.h \
				Versionen.h \
				Instrumente.h \
				DTGrafik.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Instrumente.o:	Instrumente.c \
				Strukturen.h \
				Midi.h \
				locale.h \
				catalogids.h \
				Versionen.h \
				Dynamic_Strings.h \
				Requester.h
	$(CC) $(CFLAGS) -c $<

Main.o:			Main.c \
				Midi.h \
				Gui.h \
				Strukturen.h \
				Fenster.h \
				MidiEdit.h \
				Gui2.h \
				Menu.h \
				Sequenzen.h \
				Requester.h \
				Marker.h \
				Einstellungen.h \
				Spuren.h \
				EditorNoten.h \
				Projekt.h \
				Umgebung.h \
				SysEx.h \
				Instrumente.h \
				Clipboard.h \
				Mischpult.h \
				DTGrafik.h \
				Automation.h \
				locale.h \
				catalogids.h \
				AppWindow.h \
				CtrlWandler.h
	$(CC) $(CFLAGS) -c $<

Marker.o:		Marker.c \
				Strukturen.h \
				Requester.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Menu.o:			Menu.c \
				Menu.h \
				Requester.h \
				locale.h \
				catalogids.h \
				Versionen.h
	$(CC) $(CFLAGS) -c $<

Midi.o:			Midi.c \
				Strukturen.h \
				Requester.h \
				Marker.h \
				Spuren.h \
				Sequenzen.h \
				SysEx.h \
				Versionen.h \
				Mischpult.h \
				Automation.h \
				Gui.h \
				locale.h \
				catalogids.h \
				CtrlWandler.h
	$(CC) $(CFLAGS) -c $<

MidiEdit.o:		MidiEdit.c \
				Strukturen.h \
				Sequenzen.h \
				Requester.h \
				Undo.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Mischpult.o:	Mischpult.c \
				Strukturen.h \
				Gui.h \
				Menu.h \
				Dynamic_Strings.h \
				Versionen.h \
				Midi.h \
				Projekt.h \
				Instrumente.h \
				DTGrafik.h \
				locale.h \
				catalogids.h \
				GuiFenster.h
	$(CC) $(CFLAGS) -c $<

Projekt.o:		Projekt.c \
				Strukturen.h \
				Midi.h \
				Spuren.h \
				Sequenzen.h \
				MidiEdit.h \
				Marker.h \
				Gui.h \
				Gui2.h \
				Menu.h \
				Fenster.h \
				Requester.h \
				Einstellungen.h \
				EditorNoten.h \
				Umgebung.h \
				SysEx.h \
				DiskHorny.h \
				Versionen.h \
				DiskSMF.h \
				Clipboard.h \
				Mischpult.h \
				DTGrafik.h \
				Automation.h \
				locale.h \
				catalogids.h \
				CtrlWandler.h \
				Instrumente.h
	$(CC) $(CFLAGS) -c $<

Requester.o:	Requester.c \
				Strukturen.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Sequenzen.o:	Sequenzen.c \
				Strukturen.h \
				Requester.h \
				MidiEdit.h \
				EditorNotenGui.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Smpte.o:		Smpte.c \
				Strukturen.h
	$(CC) $(CFLAGS) -c $<

Spuren.o:		Spuren.c \
				Strukturen.h \
				Midi.h \
				Sequenzen.h \
				Gui.h \
				Gui2.h \
				Requester.h \
				Versionen.h \
				locale.h \
				catalogids.h
	$(CC) $(CFLAGS) -c $<

Start.o:		Start.c \
				Versionen.h \
				Dynamic_Strings.h \
				Strukturen.h
	$(CC) $(CFLAGS) -c $<

SysEx.o:		SysEx.c \
				Strukturen.h \
				Midi.h \
				Fenster.h \
				Gui.h \
				Menu.h \
				Projekt.h \
				Requester.h \
				Versionen.h \
				locale.h \
				catalogids.h \
				GuiFenster.h
	$(CC) $(CFLAGS) -c $<

Umgebung.o:		Umgebung.c \
				Strukturen.h \
				Gui.h \
				Menu.h \
				Projekt.h \
				Requester.h \
				Dynamic_Strings.h \
				locale.h \
				catalogids.h \
				GuiFenster.h
	$(CC) $(CFLAGS) -c $<

Undo.o:			Undo.c \
				Undo.h \
				MidiEdit.h \
				Strukturen.h \
				Sequenzen.h \
				EditorNotenGui.h
	$(CC) $(CFLAGS) -c $<

Versionen.o:		Versionen.c \
				Versionen.h
	$(CC) $(CFLAGS) -c $<
