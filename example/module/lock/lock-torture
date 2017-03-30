#!/bin/bash

TOPDIR=/sys/kernel/debug/mystack

for ((i=0;j<10000;j++)) ; do
    echo 0 >${TOPDIR}/push &
    cat ${TOPDIR}/show >/dev/null &
    cat ${TOPDIR}/pop >/dev/null &
done
