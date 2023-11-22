include $(top_srcdir)/Makefile.base

INCLUDES += $(QT_CFLAGS)

noinst_HEADERS = mt_qt.h

toolkitdir = $(QT_STYLES_DIR)
toolkit_LTLIBRARIES = metatheme.la

metatheme_la_SOURCES = mt_qt.cpp
metatheme_la_LDFLAGS = -avoid-version $(QT_CFLAGS) -module
metatheme_la_LIBADD = $(QT_LIBS) $(CAIRO_LIBS) -L$(top_srcdir)/metatheme -lmetatheme

CLEANFILES = mt_qt.moc

mt_qt.cpp: mt_qt.moc

mt_qt.moc: mt_qt.h
	$(QT_MOC) mt_qt.h -o mt_qt.moc
