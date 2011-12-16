#!/bin/bash

# Get an identifier for our operating system.
OS=`uname`

if [ $OS != "Darwin" -a  $OS != "Linux"  -a $OS != "MINGW32_NT-6.1" ]
    then
        echo "This script is not meant to be used on this OS."
        exit
fi


# Get the architecture
which arch >/dev/null
if [ "$?" = "0" ] ; then
    ARCHITECTURE=`arch`
else
    which uname >/dev/null
    if [ "$?" = "0" ] ; then
        ARCHITECTURE=`uname -m`
    fi
fi

if [ "$ARCHITECTURE" = "" ] ; then
    echo "could not get architecture"
    exit 1
fi

echo "architecture=$ARCHITECTURE"

MAKE="make"

START=`date`

if [ $OS = "Linux" ]
    then
        $MAKE
fi

if [ $OS = "Darwin" ]
    then
        $MAKE
fi

if [ $OS = "MINGW32_NT-6.1" ]
    then
        $MAKE
fi

STOP=`date`
echo "Compile start: $START"
echo "Compile   end: $STOP"
