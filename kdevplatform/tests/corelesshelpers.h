/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTS_CORELESS_HELPERS_H
#define KDEVPLATFORM_TESTS_CORELESS_HELPERS_H

#include <KLocalizedString>

#include <QStandardPaths>

namespace KDevelop {
/**
 * Initialize a unit test that does not call AutoTestShell::init()
 * and TestCore::initialize() but invokes KDevelop's @c i18n* code.
 *
 * AutoTestShell::init() and TestCore::initialize() perform this setup themselves,
 * so unit tests that call those two functions should not call this one.
 */
inline void initCorelessTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    // prevent kf.i18n warnings
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kdevelop"));
}

} // namespace KDevelop

#endif // KDEVPLATFORM_TESTS_CORELESS_HELPERS_H
