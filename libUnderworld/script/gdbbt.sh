#!/bin/sh

#---------------------------------------------------------------------
# borrowed from https://wiki.ubuntu.com/Backtrace
#
# usage:
# gdbbt.sh applicationPID
#---------------------------------------------------------------------
test "x${*}" = "x" && die 'no process given'

LOG="bt-${1}.txt"
echo "outputting trace to '${LOG}'"

exec gdb -batch-silent \
    -ex 'set logging overwrite on' \
    -ex "set logging file ${LOG}" \
    -ex 'set logging on' \
    -ex 'handle SIG33 pass nostop noprint' \
    -ex 'set pagination 0' \
    -ex 'backtrace full' \
    -ex 'info registers' \
    -ex 'x/16i $pc' \
    -ex 'thread apply all backtrace' \
    -ex 'quit' \
    -p ${*} \
    < /dev/null
