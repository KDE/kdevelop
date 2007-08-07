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

QStringList getValueList( const QList<QMake::ValueAST*>& list )
{
    QStringList result;
    foreach( QMake::ValueAST* v, list)
    {
        result << v->value();
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
    kDebug(9024) << k_funcinfo << "Is" << m_projectFile << "a dir?" << fi.isDir() ;
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
        kDebug(9024) << "My dir is:" << absoluteDir();
        if( QFileInfo( argument ).isRelative() )
        {
            argument = QFileInfo( absoluteDir() + '/' + argument ).canonicalFilePath();
        }
        kDebug(9024) << "Reading Include file:" << argument;
        QMakeIncludeFile includefile( argument, m_variableValues );
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
        kDebug(9024) << "Setting var" << node->variable()->value() << values;
        m_variableValues[node->variable()->value()] = values;
    }else if( op == "+=" )
    {
        m_variableValues[node->variable()->value()] += values;
    }else if( op == "-=" )
    {
        kDebug(9024) << "Removing from var" << node->variable()->value() << values;
        foreach( QString value, values )
        {
            m_variableValues[node->variable()->value()].removeAll(value);
        }
    }else if( op == "*=" )
    {
        kDebug(9024) << "adding to var if not existent" << node->variable()->value() << values;
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
        kDebug(9024) << "replacing in var" << node->variable()->value() << values;
        QString value = values.first().trimmed();
        QString regex = value.mid(2,value.indexOf("/", 2));
        QString replacement = value.mid(value.indexOf("/", 2)+1,value.lastIndexOf("/"));
        kDebug(9024) << "Replacing variable, using regex" << regex << "value" << value ;
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
    return resolveShellGlobbingInternal( absolutefile.mid( 1 ), "/" );
}


QString QMakeFile::resolveFileName( const QString& file ) const
{
    QString absolutefile = file;
    if( QFileInfo( file ).isRelative() )
    {
        absolutefile = absoluteDir() + "/" + file;
    }
    return QFileInfo( absolutefile ).canonicalFilePath();
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
