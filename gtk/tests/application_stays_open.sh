#!/bin/sh
set -eu

timeout_command=$1
xvfb_run=$2
application=$3

set +e
"$timeout_command" 3s "$xvfb_run" -a "$application"
status=$?
set -e

# timeout(1) returns 124 only when the application remained alive for the
# complete smoke-test interval. Any earlier exit is a failed lifecycle test.
if [ "$status" -ne 124 ]; then
    echo "Application exited early with status $status" >&2
    exit 1
fi
