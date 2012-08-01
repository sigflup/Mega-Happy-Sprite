#!/bin/sh

make clean
rm -rf configure config.h.in config.log config.h Makefile config.status gui/Makefile
autoconf
autoheader
