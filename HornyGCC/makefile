ifneq ($(shell uname), AmigaOS)
	CC 		= ppc-amigaos-gcc
	STRIP = ppc-amigaos-strip
	AMIGADATE = $(shell date +"%-d.%-m.%Y")
else
	CC 		= gcc
	STRIP = strip
	AMIGADATE = $(shell date LFORMAT "%-d.%-m.%Y")
endif 

CFLAGS = -Wall -Werror -Wwrite-strings -gstabs

OBJS = 		AppWindow.o \
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
			oca.o \
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


HornyOS4:  $(OBJS) makefile 
	$(CC) -o $@.debug  $(OBJS)
	ppc-amigaos-strip $@.debug -o $@

%.o : %.c
	$(CC) -o $@ -c $< $(CFLAGS) 
	
clean: 
	$(RM) $(OBJS) HornyOS4.debug HornyOS4
