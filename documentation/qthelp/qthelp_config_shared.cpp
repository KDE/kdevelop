/*  This file is part of KDevelop

    Copyright 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qthelp_config_shared.h"

#include <KSharedConfig>
#include <KGlobal>
#include <KConfigGroup>

void qtHelpReadConfig(QStringList& iconList, QStringList& nameList,
                      QStringList& pathList, QStringList& ghnsList,
                      QString& searchDir, bool& loadQtDoc)
{
    KConfigGroup cg(KGlobal::config(), "QtHelp Documentation");
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
    KConfigGroup cg(KGlobal::config(), "QtHelp Documentation");
    cg.writeEntry("iconList", iconList);
    cg.writeEntry("nameList", nameList);
    cg.writeEntry("pathList", pathList);
    cg.writeEntry("ghnsList", ghnsList);
    cg.writeEntry("searchDir", searchDir);
    cg.writeEntry("loadQtDocs", loadQtDoc);
}
