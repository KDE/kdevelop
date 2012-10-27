macro_optional_find_package( LibOktetaKasten )

macro_log_feature( LIBOKTETAKASTEN_FOUND
  "liboktetakasten" "Okteta Kasten libraries" "kdesdk/okteta/kasten" FALSE
  "" "Required for building Okteta KDevelop plugin."
)

if( LIBOKTETAKASTEN_FOUND )
set( LIBOKTETA_FIND_VERSION ${LIBOKTETAKASTEN_NEEDS_OKTETA_VERSION} )
set( LIBKASTEN_FIND_VERSION ${LIBOKTETAKASTEN_NEEDS_KASTEN_VERSION} )
macro_optional_find_package( LibOkteta )
macro_optional_find_package( LibKasten )

macro_log_feature( LIBOKTETA_FOUND
  "libokteta v${LIBOKTETA_FIND_VERSION}" "Okteta libraries" "kdesdk/okteta" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
macro_log_feature( LIBKASTEN_FOUND
  "libkasten v${LIBKASTEN_FIND_VERSION}" "Kasten libraries" "kdesk/okteta/libs/kasten" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
endif( LIBOKTETAKASTEN_FOUND )


if( LIBOKTETAKASTEN_FOUND AND LIBKASTEN_FOUND AND LIBOKTETA_FOUND )
    add_subdirectory(okteta)
endif( LIBOKTETAKASTEN_FOUND AND LIBKASTEN_FOUND AND LIBOKTETA_FOUND )
