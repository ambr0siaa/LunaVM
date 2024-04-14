#!/bin/sh
dir=./bin

if [ ! -d "$dir" ]; then
    echo "[Info] Start making dirictory $dir"
    mkdir $dir
    echo "[Info] $dir has made"
fi

gcc bil.c -flto -O3 -fno-ident -pipe -fno-ident -z noexecstack -DNDEBUG -o $dir/bil
strip -s -R .comment -R .note $dir/bil