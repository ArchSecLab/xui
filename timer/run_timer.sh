#!/bin/bash
TRIAL=9
THREADS=24

MAIN_DIR="$(dirname "$(realpath "$0")")"
RESULT_DIR="$MAIN_DIR/result/"

mkdir -p $RESULT_DIR

cd $MAIN_DIR

for i in {0..39}
do
echo "2000000" | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_min_freq
echo "2000000" | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_max_freq
done

# Run baseline:
make clean
make timer=no

Tus=(100)
folder="$RESULT_DIR/base"
if [ ! -d "$folder" ]; then
    mkdir $folder
fi 
for ((i=0; i<${#Tus[@]}; i++)); do 
    tus=${Tus[i]}
    usfolder="$folder/$tus"
    if [ ! -d "$usfolder" ]; then
        mkdir $usfolder
    fi 

    for ((t=1; t<=1; t++)); do
        tfolder="$usfolder/$t"
        if [ ! -d "$tfolder" ]; then
            mkdir $tfolder
        fi 

        echo Running baseline: $t threads, $tus us
            
        for ((j=1; j<=$TRIAL; j++)); do
            ./timer $t $tus >$tfolder/$j 2>&1            
        done 
    done
done 



# Run nanosleep:
make clean
make timer=sleep

Tus=(100 50)
folder="$RESULT_DIR/sleep"
if [ ! -d "$folder" ]; then
    mkdir $folder
fi 
for ((i=0; i<${#Tus[@]}; i++)); do 
    tus=${Tus[i]}
    usfolder="$folder/$tus"
    if [ ! -d "$usfolder" ]; then
        mkdir $usfolder
    fi 

    for ((t=1; t<=$THREADS; t++)); do
        tfolder="$usfolder/$t"
        if [ ! -d "$tfolder" ]; then
            mkdir $tfolder
        fi 

        echo Running sleep: $t threads, $tus us
            
        for ((j=1; j<=$TRIAL; j++)); do
            ./timer $t $tus >$tfolder/$j 2>&1            
        done 
    done
done 

# Run itimer:
make clean
make timer=itimer

Tus=(100 50 20 10 5 3)
folder="$RESULT_DIR/itimer"
if [ ! -d "$folder" ]; then
    mkdir $folder
fi 
for ((i=0; i<${#Tus[@]}; i++)); do 
    tus=${Tus[i]}
    usfolder="$folder/$tus"
    if [ ! -d "$usfolder" ]; then
        mkdir $usfolder
    fi 

    for ((t=1; t<=$THREADS; t++)); do
        if [[ $tus -eq 3 && $t -gt 8 ]]; then
            continue
        fi

        tfolder="$usfolder/$t"
        if [ ! -d "$tfolder" ]; then
            mkdir $tfolder
        fi 

        echo Running itimer: $t threads, $tus us
            
        for ((j=1; j<=$TRIAL; j++)); do
            ./timer $t $tus >$tfolder/$j 2>&1            
        done 
    done
done 


for i in {0..39}
do
echo "800000" | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_min_freq
echo "3900000" | sudo tee /sys/devices/system/cpu/cpu$i/cpufreq/scaling_max_freq
done
python3 $MAIN_DIR/plot_timer.py
