#!/bin/bash

USEDOCKER=1



# docker run -u 30035: --volume /home/deniz/gem5-interrupt:/gem5 -w /gem5 --rm gcr.io/gem5-test/ubuntu-22.04_all-dependencies:v22-1 build/X86/gem5.opt --debug-flags=Faults configs/deprecated/example/se.py --cpu-type=O3CPU --caches -c tests/test-progs/custom/spamsig


CURR_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")

cd $SCRIPT_DIR
GEM5_ROOT=$(realpath ..)


DOCKER="docker run -u $UID:$GID --volume ${GEM5_ROOT}:/gem5 -w /gem5 --rm gcr.io/gem5-test/ubuntu-22.04_all-dependencies:v22-1"
RUN_SIM=""
if [ $USEDOCKER -eq 1 ]
then
        RUN_SIM="${DOCKER} /gem5/build/X86/gem5.opt"
        CONFIG="/gem5/configs/deprecated/example/se.py"
else 
        RUN_SIM="$(realpath ../build/X86/gem5.opt)"
        CONFIG=$(realpath ../configs/example/se.py)
fi

cd $CURR_DIR
$RUN_SIM $CONFIG --cpu-type=O3CPU --caches -c $@