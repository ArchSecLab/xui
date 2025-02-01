#!/bin/bash

# read ordered args
DEBUG_FLAGS="$2"
DEBUG_FILE="$3"

# start constructing COMMAND
CURR_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")

cd $SCRIPT_DIR
GEM5_ROOT=$(realpath ..)

COMMAND="cd /gem5; mkdir -p resources/local_mnt; mount -o loop,offset=$((2048*512)) resources/x86-ubuntu-18.04-img resources/local_mnt; "
COMMAND+="cp /gem5/mybuild/mybuildout/$1 resources/local_mnt/home/gem5/; "
COMMAND+="ls -lsh resources/local_mnt/home/gem5/; "
COMMAND+="umount resources/local_mnt/; "

COMMAND+="./build/X86/gem5.opt " 

# only use nonempty args
if [ -n "$DEBUG_FLAGS" ]; then
    COMMAND+="--debug-flags=$DEBUG_FLAGS "
fi

if [ -n "$DEBUG_FILE" ]; then
    COMMAND+="--debug-file=$DEBUG_FILE "
fi

COMMAND+="./configs/example/gem5_library/x86-ubuntu-run-file-kvm.py $1; "

echo "$COMMAND" > "${GEM5_ROOT}/script.sh"
chmod +x "${GEM5_ROOT}/script.sh"

DOCKER="docker run --privileged=true --volume ${GEM5_ROOT}:/gem5 -w /gem5 --rm mktaram/gem5-interrupt bash script.sh"

$DOCKER
