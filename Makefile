# Makefile for !MidiMon

COMPONENT	= MidiMon
TARGET 		= !RunImage
OBJS 		= main preporter choices midi choiceswin messageswin pianowin songwin monitorwin ibar infowin
CINCLUDES 	= -Itbox:,C:,risc_oslib:
LIBS		= ${EVENTLIB} ${TBOXLIB} ${WIMPLIB} ${RLIB}
INSTDIR		?= <Install$Dir>
INSTAPP_FILES	= !Boot !Run ${TARGET} !Sprites !Sprites22 !Sprites11 !Help res Messages Modules.MIDIEvent LICENSE Docs.CHANGELOG
CFLAGS = -Wp # suppress a warning involving RISC_OSLib

# CFLAGS = -DREPORTER_DEBUG

include CApp

# Override default option to not embed function names
C_NO_FNAMES = 

C_WARNINGS = -fa

clean_all:: clean
	${WIPE} ${INSTAPP} ${WFLAGS}

# Dynamic dependencies:o.main:	c.main
