include $(top_srcdir)/Makefile.base

EXTRA_DIST = themerc b3rc

noinst_HEADERS = highcolor.h bitmaps.h

themedir = $(METATHEMEDIR)/themes/highcolor
theme_DATA = themerc
theme_LTLIBRARIES = libmt_highcolor.la

libmt_highcolor_la_SOURCES = highcolor.cpp
libmt_highcolor_la_LDFLAGS = -L$(top_srcdir)/metatheme -lstdc++ -avoid-version -module

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(METATHEMEDIR)/themes/b3
	$(INSTALL_DATA) b3rc $(DESTDIR)$(METATHEMEDIR)/themes/b3/themerc

uninstall-local:
	rm -f $(DESTDIR)$(METATHEMEDIR)/themes/b3/themerc
	-rmdir $(DESTDIR)$(METATHEMEDIR)/themes/b3
