#!/bin/sh

lht=10
rht=10

if [ $lht -ne $rht ]
then
    echo "not equal"
elif [ $lht -eq $rht ]
then
    echo "euqal"
else
    echo "Condition not met"
fi
