#!/bin/sh
dir=bin/

if [ ! -d "$dir" ]; then
    mkdir $dir
fi

gcc bil.c -Wall -Wextra -flto -O3 -fno-ident -pipe -fno-ident -z noexecstack -DNDEBUG -o $dir/bil
strip -s -R .comment -R .note $dir/bil