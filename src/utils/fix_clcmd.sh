#!/usr/bin/env bash

# a script to strip off some arguments which are not supported by clangd
# ref : https://stackoverflow.com/questions/70819007/can-not-use-clangd-to-read-linux-kernel-code

TARGET=$1

declare list=(
    "-maccumulate-outgoing-args"
    "-mno-direct-extern-access"
    "-fconserve-stack"
    "-fno-allow-store-data-races"
    "-mfunction-return=thunk-extern"
    "-mindirect-branch-cs-prefix"
    "-mindirect-branch-register"
    "-mindirect-branch=thunk-extern"
    "-mskip-rax-setup"
    "-mpreferred-stack-boundary=3"
    "-mno-fp-ret-in-387"
)

for arg in ${list} ; do
    sed -i "/"$arg"/d" ${TARGET}
done

