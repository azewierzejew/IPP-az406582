#!/usr/bin/env bash

if [[ $# < 1 ]]
then
    echo "Specify input file."
    exit 1
fi

parameters=("$@")
file=$1
numbers=${parameters[@]:1}

test -f $file
if (( $? != 0))
then
    echo "File does not exist."
    exit 1
fi

function split () {
    first_part=$(echo $1 | cut -d";" -f1)
    second_part=$(echo $1 | cut -s -d";" -f2-)
}
positiveNumberRegex="^([1-9]([0-9])*|0)$"
for id in $numbers
do
    if ! [[ $id =~ $positiveNumberRegex ]]
    then
        echo "Parameters after file should be numbers positive or zero written in decimal."
        exit 1
    fi

    if (( $id <= 0 || $id >= 1000 ))
    then
        echo "Number should be between 1 and 999."
        exit 1
    fi

    description=$(grep "^$id;.*$" $file)
    if [[ "$description" == "" ]]
    then
        echo ""
        continue
    fi

    length=0
    i=1

    while ! [[ "$description" == "" ]]
    do
        split $description
        param=$first_part
        description=$second_part

        if (( $i % 3 == 0 ))
        then
            length=$(( $length + $param ))
        fi
        i=$(( $i + 1 ))
    done

    echo "$id;$length"
done
