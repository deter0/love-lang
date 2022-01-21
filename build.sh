#!/usr/bin/sh
CC="gcc"
FLAGS="-Wall -Wextra -ggdb"
LIBS=""
OLANG=$LANG
LANG="eng"

if $CC $FLAGS -o love $LIBS src/main.c; then
	echo "Compiled to build/love"
	mv love bin/love
	tests/run-tests.sh
else
	echo "Failed compiling"
fi

LANG=$OLANG