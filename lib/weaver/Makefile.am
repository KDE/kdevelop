# we deal with Job.moc and Thread.moc separately since KDE's build system can't
# cope with Q_OBJECT in .cpp files

METASOURCES = ThreadWeaver.moc WeaverImpl.moc WeaverInterface.moc \
	WeaverObserver.moc

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libThreadWeaver.la

libThreadWeaver_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 4:0
libThreadWeaver_la_LIBADD = $(LIB_QTCORE)


Thread.lo: Thread.moc
Job.lo: Job.moc

CLEANFILES = Thread.moc Job.moc

Thread.moc: $(srcdir)/Thread.h $(srcdir)/Thread.cpp
	${MOC} $(srcdir)/Thread.h > $@
	${MOC} $(srcdir)/Thread.cpp >> $@

Job.moc: $(srcdir)/Job.h $(srcdir)/Job.cpp
	${MOC} $(srcdir)/Job.h > $@
	${MOC} $(srcdir)/Job.cpp >> $@

libThreadWeaver_la_SOURCES = \
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
	SuspendingState.cpp \
	IncludeMocs.cpp

include ../../admin/Doxyfile.am

