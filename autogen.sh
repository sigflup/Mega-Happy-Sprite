#!/bin/sh

make clean
rm -rf autom4te.cache
rm -rf configure config.h.in config.log config.h Makefile.windows Makefile config.status gui/Makefile gui/Makefile.windows
autoconf
autoheader
