TEMPLATE = app
TARGET = test_parser
SOURCES += test_pool.cpp 

include(../../rxx.pri)

CONFIG += qtestlib

OBJECTS_DIR     = tmp
MOC_DIR         = tmp
