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

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <kdebug.h>

#include "qmakeast.h"
#include "qmakedriver.h"

QMakeFile::QMakeFile( const KUrl& file )
    : m_ast(0)
{
    m_projectFileUrl = file;
    QFileInfo fi( file.toLocalFile() );
    kDebug(9024) << k_funcinfo << "Is " << file << " a dir?" << fi.isDir() << endl;
    if( fi.isDir() )
    {
        QDir dir( m_projectFileUrl.toLocalFile() );
        QStringList l = dir.entryList( QStringList() << "*.pro" );

        QString projectfile;

        if( !l.count() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
        {
            projectfile = fi.baseName() + ".pro";
        }else
        {
            projectfile = l.first();
        }
        m_projectFileUrl.adjustPath( KUrl::AddTrailingSlash );
        m_projectFileUrl.setFileName( projectfile );
    }
    QMake::Driver d;
    d.readFile( m_projectFileUrl.toLocalFile() );
    if( !d.parse( &m_ast ) )
    {
        kDebug( 9024 ) << "Couldn't parse project: " << m_projectFileUrl.toLocalFile() << endl;
        delete m_ast;
        m_ast = 0;
        m_projectFileUrl = KUrl();
    }else
    {
        kDebug(9024) << "found ast:" << m_ast->statements().count() << endl;
        visitNode(m_ast);
        kDebug(9024) << "Variables found:" << m_variableValues << endl;
    }
}

QMakeFile::~QMakeFile()
{
    delete m_ast;
    m_ast = 0;
}

KUrl QMakeFile::absoluteDirUrl() const
{
    return m_projectFileUrl.directory();
}

KUrl QMakeFile::absoluteFileUrl() const
{
    return m_projectFileUrl;
}

QMake::ProjectAST* QMakeFile::ast() const
{
    return m_ast;
}

void QMakeFile::visitAssignment( QMake::AssignmentAST* node )
{
    QString op = node->op()->value();
    if( op == "=" )
    {
        m_variableValues[node->variable()->value()] = QMakeFile::getValueList(node->values());
    }else if( op == "+=" )
    {
        m_variableValues[node->variable()->value()] += QMakeFile::getValueList(node->values());
    }else if( op == "-=" )
    {
        foreach( QString value, QMakeFile::getValueList(node->values()) )
        {
            m_variableValues[node->variable()->value()].removeAll(value);
        }
    }else if( op == "*=" )
    {
        foreach( QString value, QMakeFile::getValueList(node->values()) )
        {
            if( !m_variableValues[node->variable()->value()].contains(value) )
            {
                m_variableValues[node->variable()->value()].append(value);
            }
        }
    }else if( op == "~=" )
    {
        if( node->values().isEmpty() )
            return;
        QString value = node->values().first()->value().trimmed();
        QString regex = value.mid(2,value.indexOf("/", 2));
        QString replacement = value.mid(value.indexOf("/", 2)+1,value.lastIndexOf("/"));
        kDebug(9024) << "Replacing variable, using regex " << regex << " value " << value << endl;
        QStringList list = m_variableValues[node->variable()->value()];
        list.replaceInStrings( QRegExp(regex), replacement );
        m_variableValues[node->variable()->value()] = list;
    }
}

QStringList QMakeFile::getValueList( const QList<QMake::ValueAST*>& list )
{
    QStringList result;
    foreach( QMake::ValueAST* v, list)
    {
        result << v->value();
    }
    return result;
}

QStringList QMakeFile::variableValues( const QString& variable ) const
{
    if( m_variableValues.contains( variable ) )
    {
        return m_variableValues[ variable ];
    }
    return QStringList();
}


//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
