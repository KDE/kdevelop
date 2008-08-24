#!/bin/bash
# starts the qtest standalonerunner with kdevelops suite loaded

ROOT="/home/nix/KdeDev/kdevelop"
QTEST="$ROOT/plugins/xtest/qtest/tests"
EXE="$ROOT/build/plugins/xtest/qtest/tests/qtestrunner"
REG_XML="$QTEST/testsuite_stress.xml"
BUILD_DIR="$QTEST/stress/build/"

CMD="$EXE $REG_XML $BUILD_DIR"
export LD_LIBRARY_PATH=$BUILD_DIR/lib:$LD_LIBRARY_PATH
echo "<<==============================================================>>"
echo " EXECUTING $CMD"
echo "<<==============================================================>>"
$CMD
