/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef CONFIGCONSTANTS_H
#define CONFIGCONSTANTS_H

class QString;

namespace ConfigConstants
{
    QString buildConfigPrefix();
    QString buildDirKey();
    QString currentConfigKey();
    QString toolGroupPrefix();
    QString toolArguments();
    QString toolExecutable();
    QString toolEnvironment();
    QString toolType();
    QString toolEnabled();
    QString projectPathPrefix();
    QString projectPathKey();
    QString customBuildSystemGroup();
    QString configTitleKey();
}

#endif
