#!/bin/sh
aclocal
autoheader
automake -a -c --foreign
autoconf
