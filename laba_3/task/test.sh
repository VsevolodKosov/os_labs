#!/bin/bash

function print_message {
    echo "--------------------------------------------------"
    echo "$1"
    echo "--------------------------------------------------"
}


print_message "Start program..."
make
if [ $? -ne 0 ]; then
    echo "Error: make failed!"
    exit 1
fi

input_file=$(mktemp)
output_file=$(mktemp)

echo "HELLO   WORLD" > $input_file

strace ./out/parent < $input_file > $output_file

actual_output=$(cat $output_file)

print_message "$actual_output"

rm -f $input_file $output_file

make clean