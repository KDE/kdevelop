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

#include <qfile.h>
#include <qtextstream.h>

#include "cvsdir.h"

///////////////////////////////////////////////////////////////////////////////
// class CVSDir
///////////////////////////////////////////////////////////////////////////////

CVSDir::CVSDir() : QDir()
{
}

///////////////////////////////////////////////////////////////////////////////

CVSDir::CVSDir( const QDir &dir )
    : QDir( dir )
{
    // We deal with absolute paths only
    convertToAbs();

    m_cvsDir = absPath() + QDir::separator() + "CVS";

    if (isValid())
        refreshEntriesCache();
}

///////////////////////////////////////////////////////////////////////////////

CVSDir::CVSDir( const CVSDir &aCvsDir )
    : QDir( aCvsDir )
{
    *this = aCvsDir;
}

///////////////////////////////////////////////////////////////////////////////

CVSDir &CVSDir::operator=( const CVSDir &aCvsDir )
{
    m_cvsDir = aCvsDir.m_cvsDir;
    m_cachedEntries = aCvsDir.m_cachedEntries;
    QDir::operator=( aCvsDir );

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

CVSDir::~CVSDir()
{
}

///////////////////////////////////////////////////////////////////////////////

bool CVSDir::isValid() const
{
    return exists() &&
        QFile::exists( entriesFileName() ) &&
        QFile::exists( rootFileName() ) &&
        QFile::exists( repoFileName() );
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::entriesFileName() const
{
    return m_cvsDir + QDir::separator()  + "Entries";
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::rootFileName() const
{
    return m_cvsDir + QDir::separator()  + "Root";
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::repoFileName() const
{
    return m_cvsDir + QDir::separator()  + "Repository";
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::cvsIgnoreFileName() const
{
    return  absPath() + QDir::separator()  + ".cvsignore";
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::repository() const
{
    // The content of CVS/Repository is a single line with the path into the
    // repository of the modules checked out in this directory (just like
    // "kdevelop/parts/cvsservice"): so we can read a single line of the file
    // and we are done!
    QString content;

    if (!isValid())
        return QString::null;

    QByteArray bytes = cacheFile( repoFileName() );
    QTextStream t( bytes, QIODevice::ReadOnly );
    content += t.readLine();

    return content;
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::root() const
{
    // Same as CVSDir::repository() but CVS/Root contains the path of the
    // CVS server as used in "cvs -d <server-path>" (in example:
    // ":pserver:marios@cvs.kde.org:/home/kde")
    QString content;

    if (!isValid())
        return QString::null;

    QByteArray bytes = cacheFile( repoFileName() );
    QTextStream t( bytes, QIODevice::ReadOnly );
    content += t.readLine();

    return content;
}

///////////////////////////////////////////////////////////////////////////////

QByteArray CVSDir::cacheFile( const QString &fileName )
{
    QFile f( fileName );
    if (!f.open( QIODevice::ReadOnly ))
        return QByteArray();
    return f.readAll();
}

///////////////////////////////////////////////////////////////////////////////

QStringList CVSDir::registeredEntryList() const
{
    QStringList l;
    if (!isValid())
        return l;

    QByteArray bytes = cacheFile( entriesFileName() );
    QTextStream t( bytes, QIODevice::ReadOnly );
    CVSEntry entry;
    while (!t.eof())
    {
        QString line = t.readLine();
        entry.parse( line, *this );
        if (entry.isValid())
            l.append( entry.fileName() );
    }
    return l;
}

///////////////////////////////////////////////////////////////////////////////

bool CVSDir::isRegistered( const QString fileName ) const
{
    CVSEntry entry = fileStatus( fileName );
    return entry.isValid() && entry.fileName() == fileName;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDir::refreshEntriesCache() const
{
    m_cachedEntries.clear();

    QByteArray bytes = cacheFile( entriesFileName() );
    QTextStream t( bytes, QIODevice::ReadOnly );
    CVSEntry entry;
    while (!t.eof())
    {
        QString line = t.readLine();
        entry.parse( line, *this );
        if (entry.isValid())
            m_cachedEntries[ entry.fileName() ] = entry;
    }
}

///////////////////////////////////////////////////////////////////////////////

CVSEntry CVSDir::fileStatus( const QString &fileName, bool refreshCache ) const
{
    if (refreshCache)
        refreshEntriesCache();

    if (m_cachedEntries.contains( fileName ))
    {
        return m_cachedEntries[ fileName ];
    }
    else
        return CVSEntry( fileName, *this ); // Just the file name
}

///////////////////////////////////////////////////////////////////////////////

void CVSDir::ignoreFile( const QString &fileName )
{
    if (!isValid())
        return;

    QFile f( cvsIgnoreFileName() );
    if (!f.open( QIODevice::ReadOnly))
        return;

    QByteArray cachedFile = f.readAll();
    QTextStream t( cachedFile, QIODevice::ReadOnly | QIODevice::WriteOnly );

    QString readFileName;
    bool found = false;

    while (!t.eof() && !found)
    {
        readFileName = t.readLine();
        found = (fileName == readFileName);
    }

    f.close();
    if (!found)
    {
        f.open( QIODevice::WriteOnly );

        t << fileName << "\n";

        f.writeBlock( cachedFile );
        f.close();
    }
}

///////////////////////////////////////////////////////////////////////////////

void CVSDir::doNotIgnoreFile( const QString &fileName )
{
    if (!isValid())
        return;

    // 1. Read all .ignore file in memory
    QFile f( cvsIgnoreFileName() );
    if (!f.open( QIODevice::ReadOnly ))
        return; // No .cvsignore file? Nothing to do then!

    QByteArray cachedFile = f.readAll();
    QTextIStream is( cachedFile );

    QByteArray cachedOutputFile;
    QTextOStream os( cachedOutputFile );

    bool removed = false;
    while (!is.eof())
    {
        QString readLine = is.readLine();
        if (readLine != fileName)
            os << readLine << "\n"; // QTextStream::readLine() eats the "\n" ...
        else
            removed = true;
    }

    f.close();
    if (removed)
    {
        f.open( QIODevice::WriteOnly );
        f.writeBlock( cachedOutputFile );
        f.close();
    }
}

///////////////////////////////////////////////////////////////////////////////

VCSFileInfoMap CVSDir::dirStatus() const
{
    VCSFileInfoMap vcsInfo;
    /// Convert to VCSFileInfoMap: \FIXME : any speed improvement here?
    QStringList entries = registeredEntryList();
    QStringList::const_iterator it = entries.begin(), end = entries.end();
    for ( ; it != end; ++it)
    {
        const QString &fileName = (*it);
        const CVSEntry entry = fileStatus( fileName );

        vcsInfo.insert( fileName, entry.toVCSFileInfo() );
    }

    return vcsInfo;
}

///////////////////////////////////////////////////////////////////////////////

VCSFileInfoMap *CVSDir::cacheableDirStatus() const
{
    VCSFileInfoMap *vcsInfo = new VCSFileInfoMap;
    /// Convert to VCSFileInfoMap: \FIXME : any speed improvement here?
    QStringList entries = registeredEntryList();
    QStringList::const_iterator it = entries.begin(), end = entries.end();
    for ( ; it != end; ++it)
    {
        const QString &fileName = (*it);
        const CVSEntry entry = fileStatus( fileName );

        vcsInfo->insert( fileName, entry.toVCSFileInfo() );
    }

    return vcsInfo;
}
