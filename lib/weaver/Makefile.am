
INCLUDES = $(all_includes)

lib_LTLIBRARIES = libthreadweaver.la

libthreadweaver_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 4:0
libthreadweaver_la_LIBADD = $(LIB_QTCORE)

METASOURCES = AUTO

libthreadweaver_la_SOURCES = \
	DestructedState.cpp \
	WeaverImpl.cpp \
	Job.cpp \
	InConstructionState.cpp \
	State.cpp \
	ShuttingDownState.cpp \
	WeaverInterface.cpp \
	WeaverObserver.cpp \
	SuspendedState.cpp \
	WorkingHardState.cpp \
	DebuggingAids.cpp \
	ThreadWeaver.cpp \
	Thread.cpp \
	SuspendingState.cpp
