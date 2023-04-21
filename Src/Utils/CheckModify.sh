#!/usr/bin/env bash

# if some files in this directory were modified,then
# return 1 and continue to compile,otherwise return 0

# CALL_NAME=$0
# FILE_NAME=$(basename ${CALL_NAME})
# REAL_PATH=$(realpath ${CALL_NAME})
# WORK_PATH=${REAL_PATH%${FILE_NAME}}

WORK_PATH=$1

if ! [[ -n ${WORK_PATH} ]]; then
    exit 255
fi
# check if the first parameter is valid

ROOT_PATH=$(realpath ${WORK_PATH})
LOCK_FILE=${WORK_PATH}/.Proj.status_lock
OUTPUT_FILE=${WORK_PATH}/.Proj.status_log
# get some basic info before start main code

export CurrentCode
export HistoryCode

function getCurrentProjectSha256 {
    CurrentCode=$(LANG= git diff ${ROOT_PATH} | sha256sum | awk '{print $1}')
}

function getHistoryProjectSha256 {
    if [[ -f ${OUTPUT_FILE} ]]; then
        source ${OUTPUT_FILE}
    else
        return 1
    fi
}

function updateHistoryProjectSha256 {
    echo "#!/usr/bin/env bash" >${OUTPUT_FILE}
    echo "export HistoryCode=${CurrentCode}" >>${OUTPUT_FILE}
}

function isLocked {
    if [[ -f ${LOCK_FILE} ]]; then
        return 0
    fi

    return 1
}
# the lock file is used to mark if the last process which compiled this module was done.
# If the process was not finished successfully,the lock file will prevent this script from running.
# At the end of the process,the lock file will be removed

function setLock {
    echo >${LOCK_FILE}
}

if isLocked; then
    exit 1
    # return and continue to compile modules till the compiling task is finished
fi

getCurrentProjectSha256

if ! getHistoryProjectSha256; then
    updateHistoryProjectSha256
fi

setLock

if [[ ${CurrentCode} == ${HistoryCode} ]]; then
    exit 0
else
    updateHistoryProjectSha256
    exit 1
    # if they are not the same value,then update and exit 1,
    # that means some files in this directory may be modified.
fi
