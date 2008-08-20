#!/bin/bash
# starts the qtest standalonerunner with kdevplatforms suite loaded

ROOT="/home/nix/KdeDev"
KDEVELOP="$ROOT/kdevelop"
KDEVPLATFORM="$ROOT/kdevplatform"
EXE="$KDEVELOP/build/plugins/xtest/qtest/tests/qtestrunner"
REG_XML="$KDEVELOP/plugins/xtest/qtest/tests/testsuite_kdevplatform.xml"
BUILD_DIR="$KDEVPLATFORM/build/"

CMD="$EXE $REG_XML $BUILD_DIR"
export LD_LIBRARY_PATH=$BUILD_DIR/lib:$LD_LIBRARY_PATH
echo "<<==============================================================>>"
echo " EXECUTING $CMD"
echo "<<==============================================================>>"
$CMD
