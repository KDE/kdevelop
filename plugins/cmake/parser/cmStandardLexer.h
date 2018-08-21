/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmStandardLexer_h
#define cmStandardLexer_h

#include <QtGlobal>

/* Disable some warnings.  */
#if defined(_MSC_VER)
#pragma warning(disable : 4018)
#pragma warning(disable : 4127)
#pragma warning(disable : 4131)
#pragma warning(disable : 4244)
#pragma warning(disable : 4251)
#pragma warning(disable : 4267)
#pragma warning(disable : 4305)
#pragma warning(disable : 4309)
#pragma warning(disable : 4706)
#pragma warning(disable : 4786)
#endif

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 403
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#endif

/* Make sure isatty is available. */
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <io.h>
#if defined(_MSC_VER)
#define isatty _isatty
#endif
#else
#include <unistd.h> /* IWYU pragma: export */
#endif

/* Make sure malloc and free are available on QNX.  */
#ifdef __QNX__
#include <malloc.h>
#endif

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1
#define YY_NO_INPUT 1
#define YY_NO_UNPUT 1
#define ECHO

typedef qint8 flex_int8_t;
typedef quint8 flex_uint8_t;
typedef qint16 flex_int16_t;
typedef quint16 flex_uint16_t;
typedef qint32 flex_int32_t;
typedef quint32 flex_uint32_t;

#endif
