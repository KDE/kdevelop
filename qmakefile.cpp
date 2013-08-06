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

void resolveShellGlobbingInternal( QStringList& entries, const QStringList& segments, const QFileInfo& match, QDir& dir, int offset );

QStringList resolveShellGlobbingInternal( const QStringList& segments, QDir& dir, int offset = 0 )
{
    if (offset >= segments.size()) {
        return QStringList();
    }

    const QString& pathPattern = segments.at(offset);

    QStringList entries;
    if (pathPattern.contains('*') || pathPattern.contains('?') || pathPattern.contains('[')) {
        // pattern contains globbing chars
        foreach(const QFileInfo& match, dir.entryInfoList(QStringList() << pathPattern, QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Unsorted)) {
            resolveShellGlobbingInternal(entries, segments, match, dir, offset);
        }
    } else {
        // pattern is "simple" hence be fast, but make sure the file exists
        QFileInfo info(dir.filePath(pathPattern));
        if (info.exists()) {
            resolveShellGlobbingInternal(entries, segments, info, dir, offset);
        }
    }

    return entries;
}

void resolveShellGlobbingInternal( QStringList& entries, const QStringList& segments, const QFileInfo& match, QDir& dir, int offset )
{
    if (match.isDir() && offset + 1 < segments.size()) {
        dir.cd(match.fileName());
        entries += resolveShellGlobbingInternal(segments, dir, offset + 1);
        dir.cdUp();
    } else {
        entries << match.canonicalFilePath();
    }
}

QStringList resolveShellGlobbingInternal( const QString& pattern, const QString& dir )
{
    if( pattern.isEmpty() )
    {
        return QStringList();
    }

    QDir dir_(pattern.startsWith('/') ? QLatin1String("/") : dir);

    // break up pattern into path segments
    return resolveShellGlobbingInternal(pattern.split(QLatin1Char('/'), QString::SkipEmptyParts), dir_);
}

QMakeFile::QMakeFile( const QString& file )
    : m_ast(0), m_projectFile(file), m_project(0)
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
        ///TODO: cleanup, re-use m_variableValues directly in the visitor
        visitor.setVariables(m_variableValues);
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

QStringList QMakeFile::resolveShellGlobbing( const QString& pattern ) const
{
    return resolveShellGlobbingInternal(pattern, absoluteDir());
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
    return resolveShellGlobbing( file );
}

void QMakeFile::setProject(KDevelop::IProject* project)
{
    m_project = project;
}

KDevelop::IProject* QMakeFile::project() const
{
    return m_project;
}
