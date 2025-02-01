#!/bin/bash

MAIN_DIR="$(dirname "$(realpath "$0")")"
result_path="$MAIN_DIR/result/rocksdb/"

mkdir -p "$result_path"

process_mpps() {
    local mode=$1          # First argument: mode
    local quantum=$2       # Second argument: quantum (int)
    local start_mpps=$3    # Third argument: start_mpps (float)
    local target_mpps=$4   # Fourth argument: mpps (float)
    local incr=$5          # Fifth argument: incr (float)
    local try=$6           # Sixth argument: try (int)

    local current_mpps=$start_mpps

    outp="$result_path/$mode"
    if [ ! -d "$outp" ]; then
        mkdir -p "$outp"
    fi
    
    # Loop from start_mpps to mpps, increasing by incr
    while (( $(echo "$current_mpps <= $target_mpps" | bc -l) )); do
        formatted_mpps=$(printf "%.3f" "$current_mpps")
        
        outp="$result_path/$mode/${quantum}us"
        if [ ! -d "$outp" ]; then
            mkdir -p "$outp"
        fi

        outp="$result_path/$mode/${quantum}us/$formatted_mpps"
        if [ ! -d "$outp" ]; then
            mkdir -p "$outp"
        fi

        pidfile=$outp/pid
        if [ -e "$pidfile" ]; then
            rm "$pidfile"
        fi

        # Run the command $try times
        for ((i = 1; i <= $try; i++)); do
            echo "Running experiment for $formatted_mpps MPPS with ($mode, $quantum us)"
            run_outp="$outp/try_$i"   
            if [ ! -d "$run_outp" ]; then
                mkdir -p "$run_outp"
            fi
            $MAIN_DIR/build/X86/gem5.fast -d "$run_outp" $MAIN_DIR/configs/experiment/rocksdb/$mode.py "$quantum" "$formatted_mpps" > /dev/null 2>&1 &
            pid=$!
            echo $pid >> $pidfile
            sleep 5
        done

        current_mpps=$(echo "$current_mpps + $incr" | bc -l)
    done
}



check_gem5_background() {
    local interval=300  # 5 minutes in seconds

    while true; do
        local count=$(pgrep -c gem5)  # Count gem5 processes

        echo "$(date): $count gem5 processes remaining"

        if (( count < 10 )); then
            echo "$(date): It might be ready to run 'python3 plot_rocksdb.py'"
        fi
        
        sleep $interval  # Sleep for 5 minutes
    done
}

echo "First kill all existing gem5 processes."
pkill -KILL gem5


# Issue totally 65 gem5 instances:
# running baseline: [0.02, 0.04, 0.06, 0.08, 0.13, 0.14, 0.15, 0.16, 0.17]
process_mpps base 100000000 0.02 0.08 0.02 1
process_mpps base 100000000 0.13 0.17 0.01 1

# running two interrupts, both in [0.02, 0.06, 0.08, 0.10, 0.12, 0.13, 0.14, 0.145, 0.15, 0.155, 0.16, 0.165]
process_mpps fast 5 0.02 0.02 0.02 1
process_mpps fast 5 0.06 0.10 0.02 1
process_mpps fast 5 0.12 0.13 0.01 1
process_mpps fast 5 0.14 0.155 0.005 5 # Run 5 times for cirtical datapoints.
process_mpps fast 5 0.16 0.165 0.005 1

process_mpps uintr 5 0.02 0.02 0.02 1
process_mpps uintr 5 0.06 0.10 0.02 1
process_mpps uintr 5 0.12 0.13 0.01 1
process_mpps uintr 5 0.14 0.155 0.005 5 # Run 5 times for cirtical datapoints.
process_mpps uintr 5 0.16 0.165 0.005 1

check_gem5_background