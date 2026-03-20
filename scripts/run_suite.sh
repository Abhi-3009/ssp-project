#!/bin/bash
# Usage: ./run_suite.sh <MB_SIZE> <NAME_OF_RUN>

SIZE=$1
TAG=$2

# 1. Clear OS Caches to force Major Faults if memory is tight
sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

# 2. Run with perf and capture to CSV-style format
# -e: events, -x,: comma separated output
perf stat -x, -e minor-faults,major-faults \
    ./bin/workload $SIZE 0 2> data/${TAG}_stats.csv

echo "Run $TAG complete. Data saved to data/${TAG}_stats.csv"
