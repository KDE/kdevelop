/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTS_TEMPORARY_FILE_HELPERS_H
#define KDEVPLATFORM_TESTS_TEMPORARY_FILE_HELPERS_H

#include "testsexport.h"

#include <testhelpermacros.h>

class QStringView;
class QTemporaryFile;

namespace KDevelop {
/**
 * Open a given temporary file, write a given contents to it and close the file.
 *
 * Call RETURN_IF_TEST_FAILED() after this function or use the
 * wrapper macro OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE() instead.
 *
 * @warning This function may be called only from a test function that is invoked by the Qt Test framework.
 */
KDEVPLATFORMTESTS_EXPORT void openWriteAndCloseTemporaryFile(QTemporaryFile& file, QStringView fileContents);

#define OPEN_WRITE_AND_CLOSE_TEMPORARY_FILE(file, fileContents)                                                        \
    do {                                                                                                               \
        KDevelop::openWriteAndCloseTemporaryFile(file, fileContents);                                                  \
        RETURN_IF_TEST_FAILED();                                                                                       \
    } while (false)

} // namespace KDevelop

#endif // KDEVPLATFORM_TESTS_TEMPORARY_FILE_HELPERS_H
