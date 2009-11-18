#!/bin/bash

if [ ! -z "$1" ];
then
	if [ ! -s source/buildtype.h ];
	then
		echo "#define $1" > source/buildtype.h
	fi
else
	if [[ ! -f source/buildtype.h || -s source/buildtype.h ]];
	then
		cp /dev/null source/buildtype.h
	fi 
fi 
