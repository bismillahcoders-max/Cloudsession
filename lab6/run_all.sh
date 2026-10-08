#!/bin/sh
# Builds the program and runs it once for each of the five grammars.
# Usage: ./run_all.sh        (prints every result to the terminal)
set -e
cd "$(dirname "$0")"
make -s first_follow
for g in 1 2 3 4 5; do
    echo
    echo "################ Grammar $g ################"
    printf '%s\n' "$g" | ./first_follow | sed -n '/^Grammar [0-9]:$/,$p'
done
