add_library(gfx_timsort INTERFACE)
add_library(gfx::timsort ALIAS gfx_timsort)
target_include_directories(gfx_timsort INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
