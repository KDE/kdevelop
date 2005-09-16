TEMPLATE = app
TARGET = test_parser
SOURCES += test_parser.cpp

include(../../rxx.pri)

!contains(CONFIG, qttest_p4) {
    INCLUDEPATH += $$(KDEDIR)/include
    LIBS += -L$$(KDEDIR)/lib -lqttestlib

    OBJECTS_DIR     = tmp
    MOC_DIR         = tmp
}
