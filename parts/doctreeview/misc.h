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

#ifndef _MISC_H_
#define _MISC_H_

#include <qstringlist.h>
class QDomDocument;

struct BookInfo{
    QString title;
    QString name;
    QString author;
    QString defaultLocation;
};

class DocTreeViewTool
{
public:
    // List of libraries to document
    static void getLibraries(QStringList *libNames, QStringList *docDirs, QStringList *sourceDirs);
    static void setLibraries(QStringList *libNames, QStringList *docDirs, QStringList *sourceDirs);
    // Looks up kdoc index files and returns lists of library names
    // and the corresponding index file names. Index files in ~/.kdoc
    // have a (private) suffix in their item name
    static void readLibraryDocs(QString dir, QStringList *itemNames, QStringList *fileNames);
    static void getAllLibraries(QStringList *itemNames, QStringList *fileNames); // TODO: not a good name
    // Returns the list of index files to be hidden
    static void getHiddenLibraries(QStringList *fileNames);
    static void setHiddenLibraries(const QStringList &fileNames);
    // Returns the list of items for the Others tree
    static void getBookmarks(QStringList *itemNames, QStringList *fileNames);
    static void setBookmarks(const QStringList &itemNames, const QStringList &fileNames);
    static void addBookmark(const QString& itemName, const QString & fileName);
    static void removeBookmark( int index );
        
    static QString tocDocDefaultLocation(const QString& fileName);
    static QString tocLocation(const QString& fileName);
    static QString tocTitle(const QString& fileName);
    
    // Scans for devhelp installation in path and copies all .devhelp files found into $kde_datadir/kdevdoctreeview/devhelp/
    // If the path is not provided, use setting from instance config in DevHelp/DevHelpDir
    static void scanDevHelpDirs(const QString path = QString::null);
    static BookInfo devhelpInfo(const QString& fileName);
    static QString devhelpLocation(const QString& fileName);
    static QString devhelpLocation(const QString& docName, const QString &defaultLocation);
};

#endif
