include(MacroBoolTo01)
macro_bool_to_01(BERKELEY_DB_FOUND HAVE_BDB)

check_include_files(valgrind/valgrind.h   HAVE_VALGRIND_H)
