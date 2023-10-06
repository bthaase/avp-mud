#!/bin/bash
ulimit -c unlimited
cd area
gdb --eval-command=r --args ../src/avp 7676
