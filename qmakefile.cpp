/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "qmakefile.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcessEnvironment>

#include <kdebug.h>

#include "parser/ast.h"
#include "qmakedriver.h"

#define ifDebug(x)

//@TODO: Make the globbing stuff work with drives on win32

QStringList resolveShellGlobbingInternal( const QString& relativefile,
        const QString& dir )
{
    QStringList result;
    if( !relativefile.isEmpty() )
    {
        int index = relativefile.indexOf( QRegExp( "[^\\\\]/" ) );
        QString firstpath = relativefile.mid( 0,  index+1 );
        QString remainder = relativefile.mid( index+2 );
        if( index == -1 )
        {
            firstpath = relativefile;
            remainder = "";
        }
        QStringList entries;
        if( firstpath.contains( QRegExp( "[^\\\\]?(\\*|\\?|\\[)" ) ) )
        {
            QRegExp wildcard( firstpath, Qt::CaseSensitive, QRegExp::Wildcard );
            foreach(const  QString& entry, QDir( dir ).entryList()  )
            {
                if( wildcard.exactMatch( entry ) )
                {
                    entries << entry;
                }
            }
        }else
        {
            entries << firstpath;
        }
        foreach( const QString& entry, entries )
        {
            const QStringList subentries = resolveShellGlobbingInternal( remainder, dir+'/'+entry );
            if( !subentries.isEmpty() )
            {
                foreach( const QString& subentry, subentries )
                {
                    result << entry+'/'+subentry;
                }
            }else if( QFileInfo( dir+'/'+entry ).exists() && remainder.isEmpty() )
            {
                result << entry;
            }
        }
    }
    return result;
}

QMakeFile::QMakeFile( const QString& file )
    : m_ast(0), m_projectFile(file)
{
    Q_ASSERT(!m_projectFile.isEmpty());
}

bool QMakeFile::read()
{
    Q_ASSERT(!m_projectFile.isEmpty());
    QFileInfo fi( m_projectFile );
    ifDebug(kDebug(9024) << "Is" << m_projectFile << "a dir?" << fi.isDir() ;)
    if( fi.isDir() )
    {
        QDir dir( m_projectFile );
        QStringList l = dir.entryList( QStringList() << "*.pro" );

        QString projectfile;

        if( !l.count() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
        {
            projectfile = fi.baseName() + ".pro";
        }else
        {
            projectfile = l.first();
        }
        m_projectFile += '/' + projectfile;
    }
    QMake::Driver d;
    d.readFile( m_projectFile );

    if( !d.parse( &m_ast ) )
    {
        kWarning( 9024 ) << "Couldn't parse project:" << m_projectFile;
        delete m_ast;
        m_ast = 0;
        m_projectFile = QString();
        return false;
    }else
    {
        ifDebug(kDebug(9024) << "found ast:" << m_ast->statements.count() ;)
        QMakeFileVisitor visitor(this, this);
        m_variableValues = visitor.visitFile(m_ast);
        ifDebug(kDebug(9024) << "Variables found:" << m_variableValues ;)
    }
    return true;
}

QMakeFile::~QMakeFile()
{
    delete m_ast;
    m_ast = 0;
}

QString QMakeFile::absoluteDir() const
{
    return QFileInfo( m_projectFile ).absoluteDir().canonicalPath();
}

QString QMakeFile::absoluteFile() const
{
    return m_projectFile;
}

QMake::ProjectAST* QMakeFile::ast() const
{
    return m_ast;
}

QStringList QMakeFile::variables() const
{
    return m_variableValues.keys();
}

QStringList QMakeFile::variableValues( const QString& variable ) const
{
    return m_variableValues.value( variable, QStringList() );
}

bool QMakeFile::containsVariable( const QString& variable ) const
{
    return m_variableValues.contains( variable );
}

QMakeFile::VariableMap QMakeFile::variableMap() const
{
    return m_variableValues;
}

QStringList QMakeFile::resolveVariable(const QString& variable, VariableInfo::VariableType type) const
{
    if (type == VariableInfo::QMakeVariable && m_variableValues.contains(variable)) {
        return m_variableValues.value(variable);
    } else {
        kWarning(9024) << "unresolved variable:" << variable << "type:" << type;
        return QStringList();
    }
}

QStringList QMakeFile::resolveShellGlobbing( const QString& absolutefile ) const
{
    QStringList result;
    foreach( const QString& s, resolveShellGlobbingInternal( absolutefile.mid( 1 ), "/" ) )
    {
        result << '/'+s;
    }
    return result;
}

QString QMakeFile::resolveToSingleFileName( const QString& file ) const
{
    QStringList l = resolveFileName( file );
    if(l.isEmpty())
        return QString();
    else
        return l.first();
}

QStringList QMakeFile::resolveFileName( const QString& file ) const
{
    QString absolutepath = file;
    if( QFileInfo( absolutepath ).isRelative() )
    {
        absolutepath = absoluteDir() + '/' + file;
    }
    QStringList result;
    foreach( const QString& s, resolveShellGlobbing( absolutepath ) )
    {
        result << QFileInfo( s ).canonicalFilePath();
    }
    return result;
}

//kate: hl c++;
