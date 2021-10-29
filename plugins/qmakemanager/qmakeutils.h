/*
    SPDX-FileCopyrightText: 2015 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef QMAKEUTILS_H
#define QMAKEUTILS_H

#include <QHash>

namespace KDevelop
{
class IProject;
}

namespace QMakeUtils {

/**
 * Checks whether there's a need to run qmake for the given project item
 * This is the case if no builddir has been specified, in which case
 * it asks for one.
 *
 * @returns true if configure should be run, false otherwise
 */
bool checkForNeedingConfigure(KDevelop::IProject* project);

QHash<QString,QString> queryQMake(KDevelop::IProject*);

}

#endif // QMAKEUTILS_H
