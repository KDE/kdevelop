/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#include <kurl.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kapplication.h>
#include <qdom.h>
#include <qvaluestack.h>

#include "domutil.h"
#include "urlutil.h"
//#ifndef INDEXER
#include "doctreeviewfactory.h"
//#endif
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
        QDir libdir(dir + "/" + QFileInfo(*it).baseName());
        if (libdir.exists())
        {
            *itemNames += i18n("%1 (private)").arg(QFileInfo(*it).baseName());
            *fileNames += privatedir.filePath(*it);
            kdDebug(9002) << "Local: " << privatedir.filePath(*it) << endl;
        }
    } 
}

void DocTreeViewTool::getAllLibraries(QStringList *itemNames, QStringList *fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    QString idx_path = config->readPathEntry("KDEDocDir", KDELIBS_DOCDIR);// + "/kdoc-reference";

    readLibraryDocs(idx_path, itemNames, fileNames);
}


void DocTreeViewTool::getHiddenLibraries(QStringList *hiddenNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    *hiddenNames = config->readListEntry("LibrariesHidden");
}


void DocTreeViewTool::setHiddenLibraries(const QStringList &hiddenNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    config->writeEntry("LibrariesHidden", hiddenNames);
}


void DocTreeViewTool::getBookmarks(QStringList *itemNames, QStringList *fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    *itemNames = config->readListEntry("BookmarksTitle");
    *fileNames = config->readListEntry("BookmarksURL");
}


void DocTreeViewTool::setBookmarks(const QStringList &itemNames, const QStringList &fileNames)
{
    KConfig *config = instanceConfig();
    config->setGroup("DocTreeView");
    config->writeEntry("BookmarksTitle", itemNames);
    config->writeEntry("BookmarksURL", fileNames);
}

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

QString DocTreeViewTool::tocTitle(const QString& fileName)
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
    QString title;
    while (!childEl.isNull()) 
    {
        if (childEl.tagName() == "title") 
        {
            title = childEl.text();
            break;
        }
        childEl = childEl.nextSibling().toElement();
    }
    return title;
}


QString DocTreeViewTool::tocLocation(const QString& fileName)
{
    KConfig *config = instanceConfig();
    config->setGroup("TocDirs");
    const QString docName( QFileInfo( fileName ).baseName() );
    return config->readPathEntry( docName, DocTreeViewTool::tocDocDefaultLocation( fileName ));
}

QString DocTreeViewTool::devhelpLocation(const QString& docName, const QString &defaultLocation)
{
    KConfig *config = instanceConfig();
    config->setGroup("TocDevHelp");
    return config->readPathEntry( docName, defaultLocation);
}

QString DocTreeViewTool::devhelpLocation(const QString& fileName)
{
    KConfig *config = instanceConfig();
    config->setGroup("TocDevHelp");
    QString docName = QFileInfo(fileName).baseName();
    BookInfo inf = DocTreeViewTool::devhelpInfo(fileName);
    return config->readPathEntry( docName, inf.defaultLocation);
}


void DocTreeViewTool::scanDevHelpDirs( const QString path )
{
//    scanDevHelpOldWay(path);
    scanDevHelpNewWay();
}

BookInfo DocTreeViewTool::devhelpInfo(const QString& fileName)
{
    BookInfo inf;

    QFileInfo fi(fileName);
    if (!fi.exists())
        return inf;
    QFile f(fileName);
    if (!f.open(IO_ReadOnly)) {
        return inf;
    }
    QDomDocument doc;
    if (!doc.setContent(&f)) {
        return inf;
    }
    f.close();
    QDomElement docEl = doc.documentElement();
    inf.name = docEl.attribute("name", QString::null);
    inf.title = docEl.attribute("title", QString::null);
    inf.author = docEl.attribute("author", QString::null);
    inf.defaultLocation = docEl.attribute("base", QString::null);

    return inf;
}

void DocTreeViewTool::scanDevHelpOldWay( const QString path )
{
    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();

    QString devhelpDir;
    if (path.isEmpty())
    {
        KConfig *config = instanceConfig();
        config->setGroup("DevHelp");
        devhelpDir = config->readPathEntry("DevHelpDir", "~/.devhelp");
    }
    else
        devhelpDir = path;


    if (devhelpDir.isEmpty())
        return;


    if (devhelpDir[devhelpDir.length()-1] == QChar('/'))
        devhelpDir.remove(devhelpDir.length()-1, 1);
    QDir d(devhelpDir + QString("/specs/"));
    if (! d.exists())
    {
        return;
    }
    d.setFilter( QDir::Files );
    //scan for *.devhelp files in spec directory
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
        if (fi->extension() == "devhelp")
        {
            //extract document information and store into $docdevhelp$ resource dir

            KURL src;
            src.setPath(fi->absFilePath());
            KURL dest;
            dest.setPath(dirs->saveLocation("docdevhelp") + fi->baseName() + ".devhelp");

            QString contentDirURL = devhelpDir + QString("/books/") + fi->baseName() + "/";
            QDir contentDir(contentDirURL);
            if (contentDir.exists())
            {
                KConfig *config = DocTreeViewFactory::instance()->config();
                config->setGroup("TocDevHelp");
                QString temp = config->readPathEntry( fi->baseName());
                if (temp.isEmpty() ) {
#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif
#if defined(_KDE_3_1_3_)
                    config->writePathEntry( fi->baseName(), contentDirURL);
#else
                    config->writeEntry( fi->baseName(), contentDirURL);
#endif
                }
            }

            KIO::NetAccess::copy(src, dest);
        }
        ++it;
    }
}

void DocTreeViewTool::scanDevHelpNewWay( )
{
    QValueStack<QString> scanStack;
//    scanStack << URLUtil::envExpand("$DEVHELP_SEARCH_PATH");
    scanStack << "/usr/share/devhelp/books/"
             << "/usr/local/share/devhelp/books"
             << "/opt/gnome/share/devhelp/books"
             << "/opt/gnome2/share/devhelp/books"
             << "/usr/share/gtk-doc/html"
             << "/usr/local/share/gtk-doc/html"
             << "/opt/gnome/share/gtk-doc/html"
             << "/opt/gnome2/share/gtk-doc/html";

    KConfig *config = instanceConfig();
    config->setGroup("DevHelp");
    QString confDir = config->readPathEntry("DevHelpDir", URLUtil::envExpand("$HOME/.devhelp"));
    if ((!confDir.isEmpty()) && (!scanStack.contains(confDir)))
        scanStack << confDir;

    QStringList scanList;

    QDir dir;
    do {
        dir.setPath(scanStack.pop());
        if (!dir.exists())
            continue;
        scanList << dir.path();

        const QFileInfoList *dirEntries = dir.entryInfoList();
        QPtrListIterator<QFileInfo> it(*dirEntries);
        for (; it.current(); ++it) {
            QString fileName = it.current()->fileName();
            if (fileName == "." || fileName == "..")
                continue;
            QString path = it.current()->absFilePath();
            if (it.current()->isDir()) {
                scanStack.push(path);
            }
        }
    } while (!scanStack.isEmpty());

    for (QStringList::const_iterator it = scanList.begin(); it != scanList.end(); ++it)
    {
        scanDevHelpNewWay(*it);
    }
}

void DocTreeViewTool::scanDevHelpNewWay( const QString & path )
{
    KStandardDirs *dirs = DocTreeViewFactory::instance()->dirs();

    QDir d(path);
    if (! d.exists())
    {
        return;
    }
    d.setFilter( QDir::Files );
    //scan for *.devhelp files in spec directory
    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;
    while ( (fi = it.current()) != 0 ) {
        if (fi->extension() == "devhelp")
        {
            //extract document information and store into $docdevhelp$ resource dir

            KURL src;
            src.setPath(fi->absFilePath());
            KURL dest;
            dest.setPath(dirs->saveLocation("docdevhelp") + fi->baseName() + ".devhelp");

            QString contentDirURL = QDir::cleanDirPath(path) + "/";
            QDir contentDir(contentDirURL);
            if (contentDir.exists())
            {
                KConfig *config = DocTreeViewFactory::instance()->config();
                config->setGroup("TocDevHelp");
#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif
#if defined(_KDE_3_1_3_)
                config->writePathEntry( fi->baseName(), contentDirURL);
#else
                config->writeEntry( fi->baseName(), contentDirURL);
#endif
            }

            KIO::NetAccess::copy(src, dest);
        }
        ++it;
    }
}
