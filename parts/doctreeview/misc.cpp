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
#include <qdom.h>

#include "domutil.h"
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

void DocTreeViewTool::getLibraries(QStringList *libNames, QStringList *docDirs, QStringList *sourceDirs)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    *libNames = config->readListEntry("LibrariesNames");
    *docDirs = config->readListEntry("LibrariesDocDirs");
    *sourceDirs = config->readListEntry("LibrariesSourceDirs");
}

void DocTreeViewTool::setLibraries(QStringList *libNames, QStringList *docDirs, QStringList *sourceDirs)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    config->writeEntry("LibrariesNames", *libNames);
    config->writeEntry("LibrariesDocDirs", *docDirs );
    config->writeEntry("LibrariesSourceDirs", *sourceDirs);
}

void DocTreeViewTool::readLibraryDocs(QString dir, QStringList *itemNames, QStringList *fileNames)
{
    QDir globaldir(dir + "/kdoc-reference");
    QStringList globalentries =
        globaldir.exists()? globaldir.entryList("*", QDir::Files) : QStringList();
    for (QStringList::Iterator it = globalentries.begin(); it != globalentries.end(); ++it) {
        *itemNames += QFileInfo(*it).baseName();
        *fileNames += globaldir.filePath(*it);
                kdDebug(9002) << "Global: " << globaldir.filePath(*it) << endl;
    }
    QDir privatedir(QDir::homeDirPath() + "/.kdoc");
    QStringList privateentries =
        privatedir.exists()? privatedir.entryList("*", QDir::Files) : QStringList();
    for (QStringList::Iterator it = privateentries.begin(); it != privateentries.end(); ++it) {
        *itemNames += i18n("%1 (private)").arg(QFileInfo(*it).baseName());
        *fileNames += privatedir.filePath(*it);
                kdDebug(9002) << "Local: " << privatedir.filePath(*it) << endl;
    } 
}

void DocTreeViewTool::getAllLibraries(QStringList *itemNames, QStringList *fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    QString idx_path = config->readEntry("KDEDocDir", KDELIBS_DOCDIR);// + "/kdoc-reference";

    readLibraryDocs(idx_path, itemNames, fileNames);
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

void DocTreeViewTool::addBookmark(const QString& itemName, const QString & fileName)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    QStringList iNames = config->readListEntry("BookmarksTitle");
    QStringList fNames = config->readListEntry("BookmarksURL");
    
    iNames.append( itemName );
    fNames.append( fileName );
    
    setBookmarks( iNames, fNames );
}
void DocTreeViewTool::removeBookmark(int index)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    QStringList itemNames = config->readListEntry("BookmarksTitle");
    QStringList fileNames = config->readListEntry("BookmarksURL");
    
    fileNames.remove( fileNames.at( fileNames.size() - index ) );
    itemNames.remove( itemNames.at( itemNames.size() - index ) );
    
    setBookmarks( itemNames, fileNames );
}

QString DocTreeViewTool::tocDocDefaultLocation(const QString& fileName)
{
    QFile f(fileName);
    if (!f.open(IO_ReadOnly)) {
        kdDebug(9002) << "Could not read doc toc: " << fileName << endl;
        return QString::null;
    }
    QDomDocument doc;
    if (!doc.setContent(&f) || doc.doctype().name() != "kdeveloptoc") {
        kdDebug(9002) << "Not a valid kdeveloptoc file: " << fileName << endl;
        return QString::null;
    }
    f.close();
    
    QDomElement docEl = doc.documentElement();
    QDomElement childEl = docEl.firstChild().toElement();
    QString base;
    while (!childEl.isNull()) 
    {
        if (childEl.tagName() == "base") 
        {
            base = childEl.attribute("href");
            if (!base.isEmpty())
                base += "/";
            break;
        }
        childEl = childEl.nextSibling().toElement();
    }
    return base;
}

QString DocTreeViewTool::tocLocation(const QString& fileName)
{
    KConfig *config = instanceConfig();
    config->setGroup("TocDirs");
    const QString docName( QFileInfo( fileName ).baseName() );
    return config->readEntry( docName, DocTreeViewTool::tocDocDefaultLocation( fileName ));
}

