include $(top_srcdir)/Makefile.base

EXTRA_DIST = themerc 

themedir = $(METATHEMEDIR)/themes/flat
theme_DATA = themerc
theme_LTLIBRARIES = libmt_flat.la

libmt_flat_la_SOURCES = flat.c
libmt_flat_la_LDFLAGS = -avoid-version -module
