#!/usr/bin/env bash

WORK_PATH=$1

if ! [[ -n ${WORK_PATH} ]]; then
	exit 255
fi
# check if the first parameter is valid

ROOT_PATH=$(realpath ${WORK_PATH})
LOCK_FILE=${WORK_PATH}/.Proj.status_lock
OUTPUT_FILE=${WORK_PATH}/.Proj.status_log
# get some basic info before start main code

function isLocked {
	if [[ -f ${LOCK_FILE} ]]; then
		return 0
	fi

	return 1
}
# the lock file is used to mark if the last process which compiled this module was done.
# If the process was not finished successfully,the lock file will prevent this script from running.
# At the end of the process,the lock file will be removed

if isLocked; then
	rm -f ${LOCK_FILE}
fi
