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
};

#endif
