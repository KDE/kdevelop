#!/bin/bash

if [[ $1 == "clean" ]]; then
    make -f moneytest.make clean
    make -f banktest.make clean
    rm banktest moneytest banktest.make moneytest.make
else
    qmake -o moneytest.make moneytest.pro
    make -f moneytest.make
    qmake -o banktest.make banktest.pro
    make -f banktest.make
fi
