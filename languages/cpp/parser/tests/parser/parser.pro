TEMPLATE = app
TARGET = test_parser
SOURCES += test_parser.cpp

include(../../rxx.pri)

CONFIG += qtestlib

OBJECTS_DIR     = tmp
MOC_DIR         = tmp
