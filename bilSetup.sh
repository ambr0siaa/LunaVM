#!/bin/sh
dir=./build

if [ ! -d "$dir" ]; then
    echo "[Info] Start making dirictory $dir"
    mkdir $dir
    echo "[Info] $dir has made"
fi

gcc bil.c -flto -O3 -fno-ident -pipe -fno-ident -z noexecstack -DNDEBUG -o ./build/bil
strip -s -R .comment -R .note ./build/bil