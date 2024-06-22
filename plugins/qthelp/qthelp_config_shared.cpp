/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelp_config_shared.h"

#include <KSharedConfig>
#include <KConfigGroup>

void qtHelpReadConfig(QStringList& iconList, QStringList& nameList,
                      QStringList& pathList, QStringList& ghnsList,
                      QString& searchDir, bool& loadQtDoc)
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("QtHelp Documentation"));
    iconList = cg.readEntry("iconList", QStringList());
    nameList = cg.readEntry("nameList", QStringList());
    pathList = cg.readEntry("pathList", QStringList());
    ghnsList = cg.readEntry("ghnsList", QStringList());
    searchDir = cg.readEntry("searchDir", QString());
    loadQtDoc = cg.readEntry("loadQtDocs", true);
}

void qtHelpWriteConfig(const QStringList& iconList, const QStringList& nameList,
                       const QStringList& pathList, const QStringList& ghnsList,
                       const QString& searchDir, const bool loadQtDoc)
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("QtHelp Documentation"));
    cg.writeEntry("iconList", iconList);
    cg.writeEntry("nameList", nameList);
    cg.writeEntry("pathList", pathList);
    cg.writeEntry("ghnsList", ghnsList);
    cg.writeEntry("searchDir", searchDir);
    cg.writeEntry("loadQtDocs", loadQtDoc);
}
