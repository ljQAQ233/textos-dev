#!/usr/bin/env bash

_PREFIX_GITHUB=""

function udk_setup
{
    echo -n "Downloading UdkDebugger..."
    if ! curl -o Udk.tar.xz "${_PREFIX_GITHUB}https://raw.githubusercontent.com/ljQAQ233/raw/master/Udk.tar.xz"; then
        print_error "Downloaded files error"
        exit 1
    fi

    if ! echo ${PASSWORD} | sudo -S mkdir /opt/intel/udkdebugger; then
        print_error "MkDir failed , please check the path \"/opt/intel/udkdebugger\" is available"
        exit 1
    fi
    tar xvf Udk.tar.xz
    sudo mv Udk /opt/intel/udkdebugger
    sudo chmod a+rw /opt/intel/udkdebugger/*  --recursive
    rm Udk.tar.xz

    echo -n "Copying udkdebugger.conf to /etc/ ..."
    if ! echo ${PASSWORD} | sudo -S cp /opt/intel/udkdebugger/udkdebugger.conf /etc/udkdebugger.conf;then
        print_error "Please exec this again! Exiting..." 
        exit 1
    fi
    print_success "done"

    _callback+="udk_profile"
}

# It also can not be executed after all initialization completed,
# BECAUSE we will load the environment configs in launch.json
function udk_profile
{
    echo -n "Adding PYTHONPATH into /etc/profile..."
    sudo chmod 0777 /etc/profile
    sudo echo -e "\nexport PYTHONPATH=\${PYTHONPATH}:$(realpath /opt/intel/udkdebugger/script)\n" >> /etc/profile
    print_success "done"
}

