//
// C++ Interface: cvsdir
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CVSDIR_H
#define CVSDIR_H

#include <qdir.h>
#include <qstringlist.h>
#include <qmap.h>

#include "cvsentry.h"

/**
Helper classes for handling CVS dirs

@author KDevelop Authors
*/
class CVSDir : public QDir
{
public:
    CVSDir( const QDir &dir );
    virtual ~CVSDir();

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

    QString entriesFileName() const;
    QString rootFileName() const;
    QString repoFileName() const;
    QString cvsIgnoreFileName() const;

private:
    void refreshEntriesCache() const;
    inline QByteArray cacheFile( const QString &fileName ) const;

    QString m_cvsDir;

    mutable QMap<QString,CVSEntry> m_cachedEntries;
};

#endif
