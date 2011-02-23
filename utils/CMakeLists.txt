macro_optional_find_package( LibOktetaKasten )
macro_optional_find_package( LibOkteta )
macro_optional_find_package( LibKasten )

macro_log_feature( LIBOKTETAKASTEN_FOUND
  "liboktetakasten" "Okteta Kasten libraries" "kdeutils/okteta/kasten" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
macro_log_feature( LIBOKTETA_FOUND
  "libokteta" "Okteta libraries" "kdeutils/okteta" FALSE
  "" "Required for building Okteta KDevelop plugin."
)
macro_log_feature( LIBKASTEN_FOUND
  "libkasten" "Kasten libraries" "kdeutils/okteta/libs" FALSE
  "" "Required for building Okteta KDevelop plugin."
)

if( LIBOKTETA_FOUND AND LIBOKTETAKASTEN_FOUND AND LIBKASTEN_FOUND AND KDE_VERSION VERSION_GREATER 4.4.50)
    add_subdirectory(okteta)
endif( LIBOKTETA_FOUND AND LIBOKTETAKASTEN_FOUND AND LIBKASTEN_FOUND AND KDE_VERSION VERSION_GREATER 4.4.50)
