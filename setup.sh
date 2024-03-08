#!/bin/sh
gcc bil.c -flto -O3 -fno-ident -pipe -fno-ident -z noexecstack -DNDEBUG -o bil
strip -s -R .comment -R .note bil