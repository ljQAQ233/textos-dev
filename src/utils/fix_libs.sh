#!/usr/bin/env bash

grep -oP '#pragma\s+lib\("\K[^"]+' $1 2>/dev/null | tr '\n' ' ' | sort | uniq
