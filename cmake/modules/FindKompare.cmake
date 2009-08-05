# Try to find Kompare includes

find_path(KOMPARE_INCLUDES kompareinterface.h HINTS ${KDE4_INCLUDE_DIR}/kompare)

message("kde version: ${KDE_VERSION}")

#FIXME: Remove the version_greater once KDevelop has moved to 4.3
if(KOMPARE_INCLUDES AND ${KDE_VERSION} VERSION_LESS 4.3.61 )
    set(KOMPARE_INCLUDES KOMPARE_INCLUDES-NOTFOUND )
endif(KOMPARE_INCLUDES AND  ${KDE_VERSION} VERSION_LESS 4.3.61 )

message("include: ${KOMPARE_INCLUDES}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Kompare DEFAULT_MSG KOMPARE_INCLUDES)

message("KOMPARE found: ${KOMPARE_FOUND}")

MARK_AS_ADVANCED(KOMPARE_INCLUDES)

