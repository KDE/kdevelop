TEMPLATE	= lib
CONFIG		= qt dll qtopia warn_on release zecke-libqpe zecke-libopie opie-input
HEADERS		= %{APPNAMELC}.h
SOURCES		= %{APPNAMELC}.cpp
TARGET		= %{APPNAMELC}


QUICK_SPEC_FOO = $$(OPIE_BUILD_QUICK_APP)
contains( TEMPLATE, quick-template ){
        contains( QUICK_SPEC_FOO, quick-app-lib ){
                message( "foo" )
                system( rm $$TARGET )
                system( ln -s $$(OPIEDIR)/bin/quicklauncher $$TARGET)
		DEFINES += OPIE_APP_INTERFACE
                TEMPLATE = lib
                CONFIG += dll
        }else{
                LI = $$join( TARGET, "lib", "lib", ".so*" )
                system( rm $$LI )
                TEMPLATE = app
        }
}


QMAKE_LIBDIR= $$(OPIEDIR)/lib
OPIE-SPEC=6

zecke-libqpe {
	INCLUDEPATH += $$(OPIEDIR)/include
	LIBS += -lqpe
	DEPENDPATH += $$(OPIEDIR)/include
}

zecke-libopie {
	INCLUDEPATH += $$(OPIEDIR)/include
	DEPENDPATH += $$(OPIEDIR)/include
	LIBS += -lopie
}
