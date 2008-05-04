/* KDevelop xUnit plugin
 *
 * Copyright 2006 systest.ch <qxrunner@systest.ch>
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*!
 * \file  qxrunner_global.h
 *
 * \brief Declares global functions, macros, symbols and the like.
 */

#ifndef QXRUNNER_GLOBAL_H
#define QXRUNNER_GLOBAL_H

#include <Qt/qglobal.h>

/*!
 * Macro to export symbols to DLL with VC++:
 *
 * - QXRUNNER_DLL_BUILD must be defined when building the DLL.
 * - QXRUNNER_DLL must be defined if linking against the DLL.
 * - If none of the above are defined then you are building or
 *   linking against the static library.
 */

#if defined(QXRUNNER_DLL_BUILD)
#  define QXRUNNER_EXPORT Q_DECL_EXPORT
#elif defined (QXRUNNER_DLL)
#  define QXRUNNER_EXPORT Q_DECL_IMPORT
#else
#  define QXRUNNER_EXPORT
#endif

#define QXRUNNER_VERSION_STR   "0.9.2"

namespace QxRunner
{

/*!
 * Returns the version number of QxRunner at run-time as a string
 * (for example, "1.0.0"). This may be a different version than the
 * version the application was compiled against.
 */
const char* version();

/*!
 * Result types handled by QxRunner.
 *
 * \sa \ref result_types
 */
enum RunnerResult {
    NoResult     = 0,  //!< No result available, item not run yet.
    RunSuccess   = 1,  //!< Item completed successfully.
    RunInfo      = 2,  //!< Item completed successfully and has an info.
    RunWarning   = 4,  //!< Item completed with a warning.
    RunError     = 8,  //!< Item completed with an error.
    RunFatal     = 16,  //!< Item completed with a fatal error.
    RunException = 32  //!< Item not completed due to an unhandled error.
};

/*!
 * This constant defines all reasonable results returned by runner
 * items, provided for convenience.
 */
const int AllResults = RunSuccess | RunInfo | RunWarning | RunError | RunFatal;

} // namespace

#endif // QXRUNNER_GLOBAL_H
