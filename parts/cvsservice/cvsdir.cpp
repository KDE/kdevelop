//
// C++ Implementation: cvsdir
//
// Description:
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qfile.h>
#include <qtextstream.h>

#include "cvsdir.h"

///////////////////////////////////////////////////////////////////////////////
// class CVSDir
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
    return m_cvsDir + QDir::separator()  + "Entries";
}

///////////////////////////////////////////////////////////////////////////////

QString CVSDir::repoFileName() const
{
    return m_cvsDir + QDir::separator()  + "Repository";
}

///////////////////////////////////////////////////////////////////////////////

QByteArray CVSDir::cacheFile( const QString &fileName ) const
{
    QFile f( fileName );
    if (!f.open( IO_ReadOnly ))
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
    QTextStream t( bytes, IO_ReadOnly );

    while (!t.eof())
    {
        QString line = t.readLine();

        l.append( line );
    }
    return l;
}

///////////////////////////////////////////////////////////////////////////////

void CVSDir::refreshEntriesCache() const
{
    m_cachedEntries.clear();

    QByteArray bytes = cacheFile( entriesFileName() );
    QTextStream t( bytes, IO_ReadOnly );
    CVSEntry entry;
    while (!t.eof())
    {
        QString line = t.readLine();
        CVSEntry::parse( line, entry );
        if (entry.isValid())
            m_cachedEntries[ entry.fileName() ] = entry;
    }
}

///////////////////////////////////////////////////////////////////////////////

CVSEntry CVSDir::fileState( const QString &fileName, bool refreshCache ) const
{
    if (refreshCache)
        refreshEntriesCache();

    if (m_cachedEntries.contains( fileName ))
    {
        return m_cachedEntries[ fileName ];
    }
    else
        return CVSEntry();
}
