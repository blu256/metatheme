include $(top_srcdir)/Makefile.base

INCLUDES += $(GLIB_CFLAGS) $(GTK2_CFLAGS)

EXTRA_DIST = themerc

noinst_HEADERS = msstyle-ini.h msstyle-loader.h structures.h

themedir = $(METATHEMEDIR)/themes/msstyle
theme_DATA = themerc
theme_LTLIBRARIES = libmt_msstyle.la

libmt_msstyle_la_SOURCES = msstyle-loader.c msstyle.c msstyle-ini.c
libmt_msstyle_la_LDFLAGS = -avoid-version -module $(GLIB_LIBS)

bin_PROGRAMS = msstyle-config
bindir = $(METATHEMEDIR)/themes/msstyle

msstyle_config_SOURCES = msstyle-config.c
msstyle_config_LDADD = $(GTK2_LIBS) $(CAIRO_LIBS) -L$(top_srcdir)/metatheme -lmetatheme
