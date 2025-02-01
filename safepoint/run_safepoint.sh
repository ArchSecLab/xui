#!/bin/bash


MAIN_DIR="$(dirname "$(realpath "$0")")"
result_path="$MAIN_DIR/result/safepoint/"

mkdir -p $result_path

echo "First kill all existing gem5 processes:"
pkill -KILL gem5

TRY=13

run() {
    app=$1
    value=$2
    mode=$3
    quantum=$4

    # Run the command $TRY times
    for ((i = 1; i <= $TRY; i++)); do
        outp="$result_path/$app/$value/$mode/$quantum/$i"
        mkdir -p "$outp"
        $MAIN_DIR/build/X86/gem5.fast -d "$outp" $MAIN_DIR/configs/experiment/safepoint.py $mode $quantum $app $value > "$outp/run.out" 2>&1 &
        pid=$!
        echo "Running $app ($mode, $quantum us) with PID $pid"
        sleep 1
    done
}


run_experiments() {
    app=$1
    value=$2
    mode=$3
    shift 3  # Remove first three arguments
    quanta=("$@")  # Remaining arguments are the quanta array

    for q in "${quanta[@]}"; do
        run $app $value $mode $q
    done
}

# check_gem5_process() {
#     local max_time=1200  # Maximum sleep time in seconds (20 minutes)
#     local interval=30   # Check interval in seconds
#     local elapsed_time=0

#     while ps -A | grep -q "[g]em5"; do
#         if (( elapsed_time >= max_time )); then
#             echo "Max sleep time reached. Exiting."
#             pkill gem5
#             return
#         fi
#         sleep $interval
#         (( elapsed_time += interval ))
#     done

#     echo "gem5 process no longer exists. Exiting."
# }

check_gem5_process() {
    local max_time=1200  # Maximum sleep time in seconds (20 minutes)
    local interval=15    # Check interval in seconds
    local elapsed_time=0
    local prev_low_count=false  # Track if the last check had ≤2 processes

    while true; do
        local count=$(pgrep -c gem5)  # Count gem5 processes

        if (( count == 0 )); then
            echo "No gem5 process found. Exiting."
            return
        fi

        if (( count <= 2 )); then
            if $prev_low_count; then
                echo "Detected ≤2 gem5 processes for two consecutive checks. Killing gem5..."
                pkill -KILL gem5  # Force kill.
                echo "gem5 process terminated."
                return
            fi
            prev_low_count=true
        else
            prev_low_count=false
        fi

        if (( elapsed_time >= max_time )); then
            echo "Max sleep time reached. Exiting."
            pkill -KILL gem5  # Force kill.
            return
        fi

        # echo "gem5 process count: $count. Sleeping for $interval seconds..."
        sleep $interval
        (( elapsed_time += interval ))
    done
}

QUANTA1=(100000000 200 100)
QUANTA2=(50 30 20)
QUANTA3=(10 5 2)
MODES=('concord' 'safepoint' 'uintr')

# First run base64.
BASE64=base64
BASE64_VALUE=1
run_experiments $BASE64 $BASE64_VALUE base 100000000
check_gem5_process
for mode in "${MODES[@]}"; do
    run_experiments $BASE64 $BASE64_VALUE $mode ${QUANTA1[@]}
    check_gem5_process
    run_experiments $BASE64 $BASE64_VALUE $mode ${QUANTA2[@]}
    check_gem5_process
    run_experiments $BASE64 $BASE64_VALUE $mode ${QUANTA3[@]}
    check_gem5_process
done


# Then run matmul.
MATMUL=matmul
MATMUL_VALUE=100
run_experiments $MATMUL $MATMUL_VALUE base 100000000
check_gem5_process
for mode in "${MODES[@]}"; do
    run_experiments $MATMUL $MATMUL_VALUE $mode ${QUANTA1[@]}
    check_gem5_process
    run_experiments $MATMUL $MATMUL_VALUE $mode ${QUANTA2[@]}
    check_gem5_process
    run_experiments $MATMUL $MATMUL_VALUE $mode ${QUANTA3[@]}
    check_gem5_process
done


python3 $MAIN_DIR/plot_safepoint.py
