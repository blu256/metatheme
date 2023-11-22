include $(top_srcdir)/Makefile.base

EXTRA_DIST = metatheme/Makefile.am metatheme/Makefile.in metatheme/*.java genconsts

INCLUDES += -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

bin_SCRIPTS = mt-java

toolkitdir = $(JAVA_HOME)/jre/lib/i386
toolkit_LTLIBRARIES = libmt_java.la

libmt_java_la_SOURCES = MetaThemeNative.c
libmt_java_la_LDFLAGS = -avoid-version -module
libmt_java_la_LIBADD = $(CAIRO_LIBS) -L$(top_srcdir)/metatheme -lmetatheme

MetaThemeNative.c: MetaThemeNative.h

MetaThemeNative.h: metatheme.jar
	$(JAVA_HOME)/bin/javah -jni -o MetaThemeNative.h metatheme.MetaThemeNative

CLEANFILES = MetaThemeNative.h metatheme.jar metatheme/MT.java

javadir = $(JAVA_HOME)/jre/lib/ext
java_DATA = metatheme.jar

JAVA_SRCS = $(wildcard metatheme/*.java)

metatheme/MT.java: ../../metatheme/metatheme.h
	./genconsts < $< > $@

metatheme.jar: metatheme/MT.java $(JAVA_SRCS)
	$(JAVA_HOME)/bin/javac -deprecation $(JAVA_SRCS)
	$(JAVA_HOME)/bin/jar -cf $@ metatheme/*.class

clean-local:
	rm -f metatheme/*.class

SWING_PROPS = $(DESTDIR)$(JAVA_HOME)/jre/lib/swing.properties

install-data-local:
	if ! grep "^swing.defaultlaf=metatheme.MetaThemeLookAndFeel" $(SWING_PROPS) >/dev/null 2>&1; then \
		echo "swing.defaultlaf=metatheme.MetaThemeLookAndFeel" >> $(SWING_PROPS); \
	fi
