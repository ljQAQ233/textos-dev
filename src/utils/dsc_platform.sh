#!/usr/bin/env bash

# Date   : 20.12.2022
# Author : Maouai233@outlook.com

# Basic form :
# PLATFORM_NAME = SigmaBootPkg

# Pass the args
DSC=$1

# If DSC is empty or no that file,return error.
if ! [[ -n ${DSC} && -f ${DSC} ]];then
    exit 1
fi

# Get platform name from the .dsc file.
cat ${DSC} | grep "PLATFORM_NAME" | awk '{print $3}'
