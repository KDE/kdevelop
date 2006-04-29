TEMPLATE = lib
SOURCES += WeaverInterface.cpp \
	ThreadWeaver.cpp WeaverImpl.cpp \
	DebuggingAids.cpp Thread.cpp Job.cpp State.cpp \
	InConstructionState.cpp WorkingHardState.cpp \
	SuspendingState.cpp SuspendedState.cpp \
	ShuttingDownState.cpp DestructedState.cpp \
        WeaverObserver.cpp \
	JobCollection.cpp \
	JobSequence.cpp

HEADERS += WeaverInterface.h \
	ThreadWeaver.h WeaverImpl.h \
	DebuggingAids.h Thread.h Job.h State.h \
	InConstructionState.h WorkingHardState.h \
	SuspendingState.h SuspendedState.h \
        ShuttingDownState.h DestructedState.h \
        WeaverObserver.h \
	JobCollection.h \
	JobSequence.h \
        JobCollection_p.h

CONFIG += dll warn_on thread release debug
VERSION = 0.4.0
TARGET = ThreadWeaver

# CONFIG(debug, debug|release) {
# 	DEFINES += DEBUG
# } else {
#         DEFINES += RELEASE
# }

