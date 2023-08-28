# Makefile for !MidiMon

COMPONENT	= MidiMon
TARGET 		= !RunImage
OBJS 		= main preporter choices midi choiceswin messageswin pianowin songwin monitorwin iconbar
CINCLUDES 	= -Itbox:,C:,risc_oslib:
LIBS		= ${EVENTLIB} ${TBOXLIB} ${WIMPLIB} ${RLIB}
INSTDIR		?= <Install$Dir>
INSTAPP_FILES	= !Boot !Run ${TARGET} !Sprites !Sprites22 !Sprites11 !Help res Messages Modules.KeyEvent Modules.MIDIEvent

include CApp

# Override default option to not embed function names
C_NO_FNAMES = 

C_WARNINGS = -fa

clean_all:: clean
	${WIPE} ${INSTAPP} ${WFLAGS}

# Dynamic dependencies:o.main:	c.main

# Dynamic dependencies:
o.main:	c.main
o.main:	C:h.wimp
o.main:	C:h.toolbox
o.main:	C:h.kernel
o.main:	C:h.saveas
o.main:	C:h.window
o.main:	C:h.gadgets
o.main:	h.iconbar
o.main:	C:h.event
o.main:	C:h.menu
o.main:	C:h.wimplib
o.main:	C:h.kernel
o.main:	C:h.swis
o.main:	risc_oslib:h.msgs
o.main:	risc_oslib:h.msgtrans
o.main:	risc_oslib:h.werr
o.main:	h.preporter
o.main:	C:h.kernel
o.main:	h.common
o.main:	h.choices
o.main:	h.midi
o.main:	h.monitorwin
o.main:	h.choiceswin
o.main:	h.choices
o.main:	C:h.event
o.main:	h.pianowin
o.main:	C:h.toolbox
o.main:	h.messageswin
o.main:	h.songwin
o.main:	h.iconbar
o.preporter:	c.preporter
o.preporter:	C:h.kernel
o.preporter:	C:h.swis
o.preporter:	h.preporter
o.preporter:	C:h.kernel
o.choices:	c.choices
o.choices:	h.choices
o.choices:	h.preporter
o.choices:	C:h.kernel
o.choices:	h.common
o.choices:	h.choices
o.midi:	c.midi
o.midi:	risc_oslib:h.werr
o.midi:	C:h.kernel
o.midi:	C:h.swis
o.midi:	C:h.event
o.midi:	C:h.wimp
o.midi:	C:h.toolbox
o.midi:	h.common
o.midi:	h.choices
o.midi:	h.midi
o.midi:	h.preporter
o.midi:	C:h.kernel
o.midi:	h.monitorwin
o.choiceswin:	c.choiceswin
o.choiceswin:	C:h.kernel
o.choiceswin:	C:h.swis
o.choiceswin:	C:h.event
o.choiceswin:	C:h.wimp
o.choiceswin:	C:h.toolbox
o.choiceswin:	C:h.toolbox
o.choiceswin:	C:h.gadgets
o.choiceswin:	C:h.window
o.choiceswin:	risc_oslib:h.msgs
o.choiceswin:	risc_oslib:h.msgtrans
o.choiceswin:	h.choices
o.choiceswin:	h.preporter
o.choiceswin:	C:h.kernel
o.choiceswin:	h.common
o.choiceswin:	h.choices
o.choiceswin:	h.choiceswin
o.choiceswin:	h.choices
o.choiceswin:	C:h.event
o.choiceswin:	h.midi
o.messageswin:	c.messageswin
o.messageswin:	C:h.toolbox
o.messageswin:	C:h.kernel
o.messageswin:	C:h.wimp
o.messageswin:	C:h.gadgets
o.messageswin:	C:h.window
o.messageswin:	C:h.wimp
o.messageswin:	C:h.event
o.messageswin:	risc_oslib:h.msgs
o.messageswin:	risc_oslib:h.msgtrans
o.messageswin:	C:h.swis
o.messageswin:	h.messageswin
o.messageswin:	h.common
o.messageswin:	h.choices
o.messageswin:	h.midi
o.messageswin:	h.preporter
o.messageswin:	C:h.kernel
o.pianowin:	c.pianowin
o.pianowin:	C:h.wimp
o.pianowin:	C:h.wimplib
o.pianowin:	C:h.kernel
o.pianowin:	C:h.kernel
o.pianowin:	C:h.toolbox
o.pianowin:	C:h.gadgets
o.pianowin:	C:h.window
o.pianowin:	C:h.event
o.pianowin:	risc_oslib:h.msgs
o.pianowin:	risc_oslib:h.msgtrans
o.pianowin:	C:h.swis
o.pianowin:	h.common
o.pianowin:	h.choices
o.pianowin:	h.pianowin
o.pianowin:	C:h.toolbox
o.pianowin:	h.pianoconst
o.pianowin:	h.preporter
o.pianowin:	C:h.kernel
o.pianowin:	h.midi
o.songwin:	c.songwin
o.songwin:	C:h.toolbox
o.songwin:	C:h.kernel
o.songwin:	C:h.wimp
o.songwin:	C:h.gadgets
o.songwin:	C:h.window
o.songwin:	C:h.event
o.songwin:	risc_oslib:h.msgs
o.songwin:	risc_oslib:h.msgtrans
o.songwin:	C:h.swis
o.songwin:	h.common
o.songwin:	h.choices
o.songwin:	h.midi
o.songwin:	h.preporter
o.songwin:	C:h.kernel
o.songwin:	h.songwin
o.monitorwin:	c.monitorwin
o.monitorwin:	C:h.kernel
o.monitorwin:	C:h.swis
o.monitorwin:	C:h.wimp
o.monitorwin:	C:h.wimplib
o.monitorwin:	C:h.event
o.monitorwin:	C:h.toolbox
o.monitorwin:	C:h.toolbox
o.monitorwin:	h.iconbar
o.monitorwin:	C:h.menu
o.monitorwin:	C:h.ScrollList
o.monitorwin:	C:h.window
o.monitorwin:	C:h.gadgets
o.monitorwin:	C:h.saveas
o.monitorwin:	risc_oslib:h.msgs
o.monitorwin:	risc_oslib:h.msgtrans
o.monitorwin:	h.monitorwin
o.monitorwin:	h.common
o.monitorwin:	h.choices
o.monitorwin:	h.midi
o.monitorwin:	h.preporter
o.monitorwin:	C:h.kernel
o.iconbar:	c.iconbar
o.iconbar:	C:h.kernel
o.iconbar:	C:h.swis
o.iconbar:	C:h.toolbox
o.iconbar:	C:h.wimp
o.iconbar:	C:h.event
o.iconbar:	C:h.menu
o.iconbar:	h.common
o.iconbar:	h.choices
o.iconbar:	h.midi
o.iconbar:	h.preporter
o.iconbar:	C:h.kernel
o.iconbar:	h.monitorwin
o.iconbar:	h.choices
