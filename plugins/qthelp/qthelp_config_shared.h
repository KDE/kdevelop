/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QTHELP_CONFIG_SHARED_H
#define QTHELP_CONFIG_SHARED_H

#include <QStringList>

void qtHelpReadConfig(QStringList& iconList,
                      QStringList& nameList,
                      QStringList& pathList,
                      QStringList& ghnsList,
                      QString& searchDir,
                      bool& loadQtDoc);

void qtHelpWriteConfig(const QStringList& iconList,
                       const QStringList& nameList,
                       const QStringList& pathList,
                       const QStringList& ghnsList,
                       const QString& searchDir,
                       const bool loadQtDoc);

void qtHelpResetConfigToDefaults();

#endif // QTHELP_CONFIG_SHARED_H
