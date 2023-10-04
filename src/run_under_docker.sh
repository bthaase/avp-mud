#!/bin/bash
ulimit -c unlimited
cd area

# if NOGDB is set, then run without gdb
if [[ $NOGDB = "TRUE" ]]; then
    echo "Running without gdb"
    $1 $2
else
    echo "Running with gdb"
    /usr/bin/gdb -ex=r -ex=bt --args $1 $2
fi

