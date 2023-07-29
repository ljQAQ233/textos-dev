#!/usr/bin/env bash

_times=0;

function _repo_setup
{
    echo "Try to complete repo($_times)..."

    git submodule init > /dev/null

    if ! git submodule update;then
        return 1
    fi

    return 0
}

function repo_setup
{
    while ! _repo_setup ;do
        ((times++))
        if [[ ${times} -ge 5 ]];then
            return 1
        fi
    done

    print_success "The repo was up-to-date"

    return 0
}

