#!/bin/bash

make && \
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib && \
./moneytest

