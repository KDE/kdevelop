#if include_ada
SET(ADA_BUILDTOOL ada)
#endif
#
##kdevelop: ANT_BUILDTOOL = ant
#if include_ant
SET(ANT_BUILDTOOL ant)
#endif
#
##kdevelop: AUTOTOOLS_BUILDTOOL = autotools
#if include_autoproject
SET(AUTOTOOLS_BUILDTOOL autotools)
#endif
#
##kdevelop: CUSTOMMAKEFILES_BUILDTOOL = custommakefiles
#if include_customproject
SET(CUSTOMMAKEFILES_BUILDTOOL custommakefiles)
#endif
#
##kdevelop: HASKELL_BUILDTOOL = haskell
#if include_haskell
SET(HASKELL_BUILDTOOL haskell)
#endif
#
##kdevelop: PASCAL_BUILDTOOL = pascal
#if include_pascal
SET(PASCAL_BUILDTOOL pascal)
#endif
#
##kdevelop: QMAKE_BUILDTOOL = qmake
#if include_trollproject
SET(QMAKE_BUILDTOOL qmake)
#endif
#
##kdevelop: SCRIPT_BUILDTOOL = script
#if include_scriptproject
SET(SCRIPT_BUILDTOOL script)
#endif


SUBDIRS( lib projectmanager ${ADA_BUILDTOOL} ${ANT_BUILDTOOL}  	${AUTOTOOLS_BUILDTOOL} ${CUSTOMMAKEFILES_BUILDTOOL} ${HASKELL_BUILDTOOL} ${PASCAL_BUILDTOOL} ${QMAKE_BUILDTOOL} ${SCRIPT_BUILDTOOL} )



#original Makefile.am contents follow:

###
### Do NOT remove the comments that start with "kdevelop:"
### They are actually directives to the kdevelop plugin system
###
### The include_xxxx variables are controlled by configure.in.in
###
#
##kdevelop: ADA_BUILDTOOL = ada
#if include_ada
#ADA_BUILDTOOL = ada
#endif
#
##kdevelop: ANT_BUILDTOOL = ant
#if include_ant
#ANT_BUILDTOOL = ant
#endif
#
##kdevelop: AUTOTOOLS_BUILDTOOL = autotools
#if include_autoproject
#AUTOTOOLS_BUILDTOOL = autotools
#endif
#
##kdevelop: CUSTOMMAKEFILES_BUILDTOOL = custommakefiles
#if include_customproject
#CUSTOMMAKEFILES_BUILDTOOL = custommakefiles
#endif
#
##kdevelop: HASKELL_BUILDTOOL = haskell
#if include_haskell
#HASKELL_BUILDTOOL = haskell
#endif
#
##kdevelop: PASCAL_BUILDTOOL = pascal
#if include_pascal
#PASCAL_BUILDTOOL = pascal
#endif
#
##kdevelop: QMAKE_BUILDTOOL = qmake
#if include_trollproject
#QMAKE_BUILDTOOL = qmake
#endif
#
##kdevelop: SCRIPT_BUILDTOOL = script
#if include_scriptproject
#SCRIPT_BUILDTOOL = script
#endif
#
#SUBDIRS = lib projectmanager $(ADA_BUILDTOOL) $(ANT_BUILDTOOL) \
#	$(AUTOTOOLS_BUILDTOOL) $(CUSTOMMAKEFILES_BUILDTOOL) $(HASKELL_BUILDTOOL) $(PASCAL_BUILDTOOL) \
#	$(QMAKE_BUILDTOOL) $(SCRIPT_BUILDTOOL)
#
#DOXYGEN_EMPTY = YES
#include ../Doxyfile.am
