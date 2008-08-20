#!/bin/bash
# starts the qtest standalonerunner with kdevelops suite loaded

ROOT="/home/nix/KdeDev"
KDEVELOP="$ROOT/kdevelop"
EXE="$KDEVELOP/build/plugins/xtest/qtest/tests/qtestrunner"
REG_XML="$KDEVELOP/plugins/xtest/qtest/tests/testsuite_kdevelop.xml"
BUILD_DIR="$KDEVELOP/build/"

CMD="$EXE $REG_XML $BUILD_DIR"
export LD_LIBRARY_PATH=$BUILD_DIR/lib:$LD_LIBRARY_PATH
echo "<<==============================================================>>"
echo " EXECUTING $CMD"
echo "<<==============================================================>>"
$CMD
