/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinstance.h>
#include <klocale.h>

#ifndef INDEXER
#include "doctreeviewfactory.h"
#endif
#include "misc.h"
#include "../../config.h"


static KConfig *instanceConfig()
{
#ifdef INDEXER
    return KGlobal::config();
#else
    return DocTreeViewFactory::instance()->config();
#endif
}


void DocTreeViewTool::getAllLibraries(QStringList *itemNames, QStringList *fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    QString idx_path = config->readEntry("KDEDocDir", KDELIBS_DOCDIR) + "/kdoc-reference";

    QDir globaldir(idx_path);
    QStringList globalentries =
        globaldir.exists()? globaldir.entryList("*", QDir::Files) : QStringList();
    for (QStringList::Iterator it = globalentries.begin(); it != globalentries.end(); ++it) {
        *itemNames += QFileInfo(*it).baseName();
        *fileNames += globaldir.filePath(*it);
        //        kdDebug(9002) << "Global: " << globaldir.filePath(*it) << endl;
    }
    QDir privatedir(QDir::homeDirPath() + "/.kdoc");
    QStringList privateentries =
        privatedir.exists()? privatedir.entryList("*", QDir::Files) : QStringList();
    for (QStringList::Iterator it = privateentries.begin(); it != privateentries.end(); ++it) {
        *itemNames += i18n("%1 (private)").arg(QFileInfo(*it).baseName());
        *fileNames += privatedir.filePath(*it);
        //        kdDebug(9002) << "Local: " << privatedir.filePath(*it) << endl;
    }
}


void DocTreeViewTool::getHiddenLibraries(QStringList *hiddenNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    *hiddenNames = config->readListEntry("LibrariesHidden");
};


void DocTreeViewTool::setHiddenLibraries(const QStringList &hiddenNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    config->writeEntry("LibrariesHidden", hiddenNames);
};


void DocTreeViewTool::getBookmarks(QStringList *itemNames, QStringList *fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    *itemNames = config->readListEntry("BookmarksTitle");
    *fileNames = config->readListEntry("BookmarksURL");
};


void DocTreeViewTool::setBookmarks(const QStringList &itemNames, const QStringList &fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    config->writeEntry("BookmarksTitle", itemNames);
    config->writeEntry("BookmarksURL", fileNames);
};


void DocTreeViewTool::setIndexOptions(bool shownlibs, bool hiddenlibs, bool others)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    config->writeEntry("IndexShownLibraries", shownlibs);
    config->writeEntry("IndexHiddenLibraries", hiddenlibs);
    config->writeEntry("IndexOthers", others);
}


void DocTreeViewTool::getIndexOptions(bool *shownlibs, bool *hiddenlibs, bool *others)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    *shownlibs = config->readBoolEntry("IndexShownLibraries", true);
    *hiddenlibs = config->readBoolEntry("IndexHiddenLibraries", true);
    *others = config->readBoolEntry("IndexOthers", true);
}
