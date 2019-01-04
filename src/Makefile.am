#AM_CFLAGS = @X_CFLAGS@ @TN5250_CFLAGS@ -DDATADIR=\"$(datadir)\"
AM_CFLAGS = @X_CFLAGS@ -DDATADIR=\"$(datadir)\"

bin_PROGRAMS =		x5250

x5250_SOURCES = 	alloccolors.c\
			debug.c\
			dialog.c\
			help.c\
			mouse.c\
			key.c\
			resources.c\
			x5250.c\
			x5250term.c

noinst_HEADERS = 	debug.h\
			dialog.h\
			help.h\
			resources.h\
			x5250.h\
			x5250term.h

EXTRA_DIST =		icccm.r4

x5250_LDFLAGS = $(TN5250_LIB)

INCLUDES = -DSYSCONFDIR=\"$(sysconfdir)\" -I$(top_srcdir)/bitmaps

CLEANFILES = *~
