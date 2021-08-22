#!/bin/sh
echo "./runklee.sh [source file]"
if [ $# -ne 1 ]
then
    target='test.bc'
else
    target=$1
fi
echo "------> testing $target " 

klee --libc=uclibc --posix-runtime --external-calls=all -link-llvm-lib=lib/libm.a -link-llvm-lib=lib/libcrypt.a \
    -link-llvm-lib=/home/users/phli/lib/libnsl.a -link-llvm-lib=/home/users/phli/lib/libresolv.a  \
     -link-llvm-lib=/home/users/phli/lib/libutil.a  -link-llvm-lib=/home/users/phli/lib/librt.a \
    -write-smt2s -use-constant-arrays=false  \
    $target
