#!/bin/bash

# Get an identifier for our operating system.
OS=`uname`
OS=${OS:0:3}

if [ $OS != "Dar" -a  $OS != "Lin"  -a $OS != "MIN" ]
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

if [ $OS = "Lin" ]
    then
        $MAKE
fi

if [ $OS = "Dar" ]
    then
        $MAKE
fi

if [ $OS = "MIN" ]
    then
        $MAKE
fi

STOP=`date`
echo "Compile start: $START"
echo "Compile   end: $STOP"
