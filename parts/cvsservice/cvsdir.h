/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CVSDIR_H
#define CVSDIR_H

#include <qdir.h>
#include <qstringlist.h>
#include <qmap.h>

#include "cvsentry.h"

/**
Helper classes for handling CVS dirs

@author Mario Scalas
*/
class CVSDir : public QDir
{
public:
    CVSDir( const QDir &dir );
    virtual ~CVSDir();

    /**
    * A client can use this method to validate the directory state.
    * @return true if the directory is a valid CVS dir, false otherwise
    */
    bool isValid() const;
    /**
    * Returns a list of all the files registered into repository
    */
    QStringList registeredEntryList() const;
    /**
    * @param fileName is the file name (with no path info, just the file name!)
    * @param refreshCache update internal cache re-parsing <dirPath>/CVS/Entries
    * @return an empty CVSEntry if the file is not present
    */
    CVSEntry fileState( const QString &fileName, bool refreshCache = false ) const;
    /**
    * Check if the specified @p fileName is in <CVSDIR>/.cvsignore and, if not,
    * append it.
    */
    void ignoreFile( const QString &fileName );
    /**
    * Check if the specified @p fileName is in <CVSDIR>/.cvsignore and, if yes,
    * remove it.
    */
    void doNotIgnoreFile( const QString &fileName );
    /**
    * @return the content of <CVSDIR>/CVS/Repository
    */
    QString repository() const;
    /**
    * @return the content of <CVSDIR>/CVS/Root
    */
    QString root() const;
    /**
    * @return full path of "<this-dir>/CVS/Entries"
    */
    QString entriesFileName() const;
    /**
    * @return full path of "<this-dir>/CVS/Root"
    */
    QString rootFileName() const;
    /**
    * @return full path of "<this-dir>/CVS/Repository"
    */
    QString repoFileName() const;
    /**
    * @return full path of "<this-dir>/.cvsignore"
    */
    QString cvsIgnoreFileName() const;

private:
    void refreshEntriesCache() const;
    inline QByteArray cacheFile( const QString &fileName ) const;

    QString m_cvsDir;

    mutable QMap<QString,CVSEntry> m_cachedEntries;
};

#endif
