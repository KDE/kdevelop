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

#include <kdebug.h>

#include "qmakeast.h"
#include "qmakedriver.h"
#include "qmakeincludefile.h"

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
            foreach( QString entry, QDir( dir ).entryList()  )
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
        foreach( QString entry, entries )
        {
            QStringList subentries = resolveShellGlobbingInternal( remainder, dir+'/'+entry );
            if( !subentries.isEmpty() )
            {
                foreach( QString subentry, subentries )
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

QStringList QMakeFile::getValueList( const QList<QMake::ValueAST*>& list ) const
{
    QStringList result;
    foreach( QMake::ValueAST* v, list)
    {
        result += resolveVariables( v->value() );
    }
    return result;
}

QMakeFile::QMakeFile( const QString& file )
    : m_ast(0), m_projectFile(file)
{
}

bool QMakeFile::read()
{
    QFileInfo fi( m_projectFile );
    kDebug(9024) << "Is" << m_projectFile << "a dir?" << fi.isDir() ;
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
        kDebug( 9024 ) << "Couldn't parse project:" << m_projectFile;
        delete m_ast;
        m_ast = 0;
        m_projectFile = QString();
        return false;
    }else
    {
        kDebug(9024) << "found ast:" << m_ast->statements().count() ;
        visitNode(m_ast);
        kDebug(9024) << "Variables found:" << m_variableValues ;
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

void QMakeFile::visitFunctionCall( QMake::FunctionCallAST* node )
{
    if( node->functionName()->value() == "include" || node->functionName()->value() == "!include" )
    {
        if( node->arguments().isEmpty() )
            return;
        QStringList arguments = getValueList( node->arguments() );
        kDebug(9024) << "found include" << node->functionName()->value() << arguments;
        QString argument = arguments.join("").trimmed();
        if( QFileInfo( argument ).isRelative() )
        {
            argument = QFileInfo( absoluteDir() + '/' + argument ).canonicalFilePath();
        }
        kDebug(9024) << "Reading Include file:" << argument;
        QMakeIncludeFile includefile( argument, m_variableValues );
        includefile.setParent( this );
        bool read = includefile.read();
        if( read )
        {
            foreach( QString var, includefile.variables() )
            {
                if( m_variableValues[ var ] != includefile.variableValues( var ) )
                {
                    m_variableValues[ var ] = includefile.variableValues( var );
                }
            }
            if( !node->functionName()->value().startsWith('!') )
            {
                visitNode( node->scopeBody() );
            }
        }else if( node->functionName()->value().startsWith('!') )
        {
            visitNode( node->scopeBody() );
        }
    }else
    {
        visitNode( node->scopeBody() );
    }
}

void QMakeFile::visitAssignment( QMake::AssignmentAST* node )
{
    QString op = node->op()->value();
    QStringList values = getValueList(node->values());
    if( op == "=" )
    {
        m_variableValues[node->variable()->value()] = values;
    }else if( op == "+=" )
    {
        m_variableValues[node->variable()->value()] += values;
    }else if( op == "-=" )
    {
        foreach( QString value, values )
        {
            m_variableValues[node->variable()->value()].removeAll(value);
        }
    }else if( op == "*=" )
    {
        foreach( QString value, values )
        {
            if( !m_variableValues[node->variable()->value()].contains(value) )
            {
                m_variableValues[node->variable()->value()].append(value);
            }
        }
    }else if( op == "~=" )
    {
        if( values.isEmpty() )
            return;
        QString value = values.first().trimmed();
        QString regex = value.mid(2,value.indexOf("/", 2));
        QString replacement = value.mid(value.indexOf("/", 2)+1,value.lastIndexOf("/"));
        QStringList list = m_variableValues[node->variable()->value()];
        list.replaceInStrings( QRegExp(regex), replacement );
        m_variableValues[node->variable()->value()] = list;
    }
}

QStringList QMakeFile::variableValues( const QString& variable ) const
{
    if( m_variableValues.contains( variable ) )
    {
        return m_variableValues[ variable ];
    }
    return QStringList();
}

bool QMakeFile::containsVariable( const QString& variable ) const
{
    return m_variableValues.contains( variable );
}

QStringList QMakeFile::resolveShellGlobbing( const QString& absolutefile )
{
    QStringList result;
    foreach( QString s, resolveShellGlobbingInternal( absolutefile.mid( 1 ), "/" ) )
    {
        result << '/'+s;
    }
    return result;
}

QString QMakeFile::resolveToSingleFileName( const QString& file ) const
{
    return resolveFileName( file ).first();
}

QStringList QMakeFile::resolveFileName( const QString& file ) const
{
    QString absolutepath = file;
    if( QFileInfo( absolutepath ).isRelative() )
    {
        absolutepath = absoluteDir() + '/' + file;
    }
    QStringList result;
    foreach( QString s, resolveShellGlobbing( absolutepath ) )
    {
        result << QFileInfo( s ).canonicalFilePath();
    }
    return result;
}

QStringList QMakeFile::variables() const
{
    return m_variableValues.keys();
}

QStringList QMakeFile::resolveVariables( const QString& value ) const
{
    return QStringList() << value;
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
