#!/bin/bash

#for gdb scripting tutorial https://sdimitro.github.io/post/scripting-gdb/
#and https://blog.oddbit.com/post/2019-01-22-debugging-attiny-code-pt-2/
cleanup() {
        sudo pkill st-util
}

trap "cleanup" INT QUIT TERM EXIT [...]

sudo -u root ${ST_DIR}/st-util &
sleep 2
echo "st-util up and running"
sleep 2

cd wait_test
rm gdb.output
arm-none-eabi-gdb -x gdb_script || echo "Prevent fail from gdb exit" #from the testbench folder

python check_gdb_out.py 
