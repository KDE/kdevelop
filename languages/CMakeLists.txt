
ADD_SUBDIRECTORY(lib)

OPTION(WITH_ADA "Compile support for the Ada language" ON)
OPTION(WITH_BASH "Compile support for shell script language" ON)
OPTION(WITH_CPP "Compile support for the C/C++ language" ON)
OPTION(WITH_FORTRAN "Compile support for the Fortran language" ON)
OPTION(WITH_HASKELL "Compile support for the Haskell language" ON)
OPTION(WITH_JAVA    "Compile support for the Java language" ON)
OPTION(WITH_PASCAL  "Compile support for the Pascal language" ON)
OPTION(WITH_PERL    "Compile support for the Perl language" ON)
OPTION(WITH_PHP     "Compile support for the PHP language" ON)
OPTION(WITH_PYTHON  "Compile support for the Python language" ON)
OPTION(WITH_RUBY    "Compile support for the Ruby language" ON)
OPTION(WITH_SQL     "Compile support for the SQL language" ON)

IF (WITH_ADA)
   ADD_SUBDIRECTORY(ada)
ENDIF(WITH_ADA)

IF (WITH_BASH)
   ADD_SUBDIRECTORY( bash)
ENDIF(WITH_BASH)

IF (WITH_CPP)
   ADD_SUBDIRECTORY( cpp)
ENDIF(WITH_CPP)

IF (WITH_FORTRAN)
   ADD_SUBDIRECTORY( fortran)
ENDIF(WITH_FORTRAN)

IF (WITH_HASKELL)
   ADD_SUBDIRECTORY(haskell)
ENDIF(WITH_HASKELL)

IF (WITH_JAVA)
   ADD_SUBDIRECTORY(java)
ENDIF(WITH_JAVA)

IF (WITH_PASCAL)
   ADD_SUBDIRECTORY( pascal)
ENDIF(WITH_PASCAL)

IF (WITH_PERL)
   ADD_SUBDIRECTORY(perl)
ENDIF(WITH_PERL)

IF (WITH_PHP)
   ADD_SUBDIRECTORY( php)
ENDIF(WITH_PHP)

IF (WITH_PYTHON)
   ADD_SUBDIRECTORY(python)
ENDIF(WITH_PYTHON)

IF (WITH_RUBY)
   ADD_SUBDIRECTORY(ruby)
ENDIF(WITH_RUBY)

IF (WITH_SQL)
   ADD_SUBDIRECTORY(sql)
ENDIF(WITH_SQL)

#original Makefile.am contents follow:

###
### Do NOT remove the comments that start with "kdevelop:"
### They are actually directives to the kdevelop plugin system
###
### The include_xxxx variables are controlled by configure.in.in
###
#
#if include_ada
#ADA_LANGUAGE = ada
#endif
#
#if include_bash
#BASH_LANGUAGE = bash
#endif
#
#if include_cpp
#CPP_LANGUAGE = cpp
#endif
#
#if include_fortran
#FORTRAN_LANGUAGE = fortran
#endif
#
#if include_haskell
#HASKELL_LANGUAGE = haskell
#endif
#
#if include_java
#JAVA_LANGUAGE = java
#endif
#
#if include_pascal
#PASCAL_LANGUAGE = pascal
#endif
#
#if include_perl
#PERL_LANGUAGE = perl
#endif
#
#if include_php
#PHP_LANGUAGE = php
#endif
#
#if include_python
#PYTHON_LANGUAGE = python
#endif
#
#if include_ruby
#RUBY_LANGUAGE = ruby
#endif
#
#if include_sql
#SQL_LANGUAGE = sql
#endif
#
##kdevelop: ADA_LANGUAGE = ada
##kdevelop: BASH_LANGUAGE = bash
##kdevelop: CPP_LANGUAGE = cpp
##kdevelop: FORTRAN_LANGUAGE = fortran
##kdevelop: HASKELL_LANGUAGE = haskell
##kdevelop: JAVA_LANGUAGE = java
##kdevelop: PASCAL_LANGUAGE = pascal
##kdevelop: PERL_LANGUAGE = perl
##kdevelop: PHP_LANGUAGE = php
##kdevelop: PYTHON_LANGUAGE = python
##kdevelop: RUBY_LANGUAGE = ruby
##kdevelop: SQL_LANGUAGE = sql
#
#SUBDIRS = lib $(ADA_LANGUAGE) $(BASH_LANGUAGE) $(CPP_LANGUAGE) \
#	$(FORTRAN_LANGUAGE) $(HASKELL_LANGUAGE) $(JAVA_LANGUAGE) $(PASCAL_LANGUAGE) \
#	$(PERL_LANGUAGE) $(PHP_LANGUAGE) $(PYTHON_LANGUAGE) $(RUBY_LANGUAGE) $(SQL_LANGUAGE)
#
#DOXYGEN_EMPTY = YES
#include ../Doxyfile.am
