#!/bin/bash

KEEPERS="(^stubgeneration.h$|^uutgeneration.h$|^switch.h$|^switchtest.h$|^clean.sh$)"
GARBAGE=$(ls | grep -Ev $KEEPERS)
echo "Removing $GARBAGE"
rm -i $GARBAGE
