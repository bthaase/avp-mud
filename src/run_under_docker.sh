#!/bin/bash
ulimit -c unlimited
#!/bin/bash

if [[ $VALGRIND = "TRUE" ]]; then
    echo "Running under valgrind"
	/usr/bin/valgrind --log-file="../log/valgrind.log" --track-origins=yes --show-leak-kinds=all --leak-check=full $1 $2
elif [[ $GDBSERVER = "TRUE" ]]; then
    echo "Running under gdbserver"
	/usr/bin/gdbserver localhost:4444 $1 $2
elif [[ $NOGDB = "TRUE" ]]; then
    echo "Running without gdb"
    $1 $2
else
    echo "Running under gdb"
    /usr/bin/gdb -ex=r -ex=bt --args $1 $2
fi
