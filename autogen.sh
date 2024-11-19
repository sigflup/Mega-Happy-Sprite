#!/bin/sh

export AUTOCONF_VERSION=2.69

gmake clean
rm -rf autom4te.cache
rm -rf configure config.h.in config.log config.h Makefile.windows Makefile config.status gui/Makefile gui/Makefile.windows
autoconf
autoheader
