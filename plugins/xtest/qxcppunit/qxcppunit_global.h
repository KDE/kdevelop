/*!
 * \file  qxcppunit_global.h
 *
 * \brief Declares global functions, macros, symbols and the like.
 */

#ifndef QXCPPUNIT_GLOBAL_H
#define QXCPPUNIT_GLOBAL_H

#include <Qt/qglobal.h>

/*!
 * Macro to export symbols to DLL with VC++:
 *
 * - QXCPPUNIT_DLL_BUILD must be defined when building the DLL.
 * - QXCPPUNIT_DLL must be defined if linking against the DLL.
 * - If none of the above are defined then you are building or
 *   linking against the static library.
 */

#if defined(QXCPPUNIT_DLL_BUILD)
#  define QXCPPUNIT_EXPORT Q_DECL_EXPORT
#elif defined (QXCPPUNIT_DLL)
#  define QXCPPUNIT_EXPORT Q_DECL_IMPORT
#else
#  define QXCPPUNIT_EXPORT
#endif

#endif // QXCPPUNIT_GLOBAL_H
