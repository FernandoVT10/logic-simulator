#!/bin/bash

set -xe

CFLAGS="-Wall -Werror -Wextra"
FILES="src/main.c src/simulation.c src/visual.c"
RAYLIB="-I./raylib-5.5/include -L./raylib-5.5/lib/ -l:libraylib.a"

gcc -o main $FILES $CFLAGS $RAYLIB -lm
