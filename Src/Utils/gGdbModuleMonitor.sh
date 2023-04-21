#!/usr/bin/env bash

sleep 6

while ps aux | grep -E 'gdb' | grep -v grep >/dev/null; do
	sleep 3
done

kill -9 $(ps aux | grep qemu-system- | grep -v grep | grep -v while | awk '{print $2}') >/dev/null 2>&1
