##
## Do NOT remove the comments that start with "kdevelop:"
## They are actually directives to the kdevelop plugin system
##

#kdevelop: ADA_BUILDTOOL = ada
if include_ada
ADA_BUILDTOOL = ada
else
ADA_BUILDTOOL =
endif

#kdevelop: ANT_BUILDTOOL = ant
if include_ant
ANT_BUILDTOOL = ant
else
ANT_BUILDTOOL =
endif

#kdevelop: AUTOTOOLS_BUILDTOOL = autotools
if include_autoproject
AUTOTOOLS_BUILDTOOL = autotools
else
AUTOTOOL_BUILDTOOL =
endif

#kdevelop: CUSTOMMAKEFILES_BUILDTOOL = CUSTOMMAKEFILES
if include_customproject
CUSTOMMAKEFILES_BUILDTOOL = custommakefiles
else
CUSTOMMAKEFILES_BUILDTOOL =
endif

#kdevelop: GENERIC_BUILDTOOL = generic
if include_genericproject
GENERIC_BUILDTOOL = generic
else
GENERIC_BUILDTOOL =
endif

#kdevelop: HASKELL_BUILDTOOL = haskell
if include_haskell
HASKELL_BUILDTOOL = haskell
else
HASKELL_BUILDTOOL =
endif

#kdevelop: PASCAL_BUILDTOOL = pascal
if include_pascal
PASCAL_BUILDTOOL = pascal
else
PASCAL_BUILDTOOL =
endif

#kdevelop: QMAKE_BUILDTOOL = qmake
if include_trollproject
QMAKE_BUILDTOOL = qmake
else
QMAKE_BUILDTOOL =
endif

#kdevelop: SCRIPT_BUILDTOOL = script
if include_scriptproject
SCRIPT_BUILDTOOL = script
else
SCRIPT_BUILDTOOL =
endif

SUBDIRS = $(ADA_BUILDTOOL) $(ANT_BUILDTOOL) $(AUTOTOOLS_BUILDTOOL) $(CUSTOMMAKEFILES_BUILDTOOL) $(GENERIC_BUILDTOOL) $(HASKELL_BUILDTOOL) $(PASCAL_BUILDTOOL) $(QMAKE_BUILDTOOL) $(SCRIPT_BUILDTOOL)
