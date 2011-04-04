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
#include "qmakeincludefile.h"
#include "variablereferenceparser.h"

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

QStringList QMakeFile::getValueList( const QList<QMake::ValueAST*>& list ) const
{
    QStringList result;
    foreach( QMake::ValueAST* v, list)
    {
        result += resolveVariables( v->value );
    }
    return result;
}

QMakeFile::QMakeFile( const QString& file )
    : m_ast(0), m_projectFile(file)
{
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
        visitNode(m_ast);
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

void QMakeFile::visitFunctionCall( QMake::FunctionCallAST* node )
{
    if( node->identifier->value == "include" || node->identifier->value == "!include" )
    {
        if( node->args.isEmpty() )
            return;
        QStringList arguments = getValueList( node->args );
        Q_ASSERT(!arguments.isEmpty());

        ifDebug(kDebug(9024) << "found include" << node->identifier->value << arguments;)
        QString argument = arguments.join("").trimmed();
        if( QFileInfo( argument ).isRelative() )
        {
            argument = QFileInfo( absoluteDir() + '/' + argument ).canonicalFilePath();
        }
        if (argument.isEmpty()) {
            kWarning() << "empty include file detected" << absoluteFile() << "line" << node->startLine;
            if( node->identifier->value.startsWith('!') ) {
                visitNode( node->body );
            }
            return;
        }
        ifDebug(kDebug(9024) << "Reading Include file:" << argument;)
        QMakeIncludeFile includefile( argument, this );
        bool read = includefile.read();
        ifDebug(kDebug(9024) << "successfully read:" << read;)
        if( read )
        {
            foreach( const QString& var, includefile.variables() )
            {
                if( m_variableValues.value( var ) != includefile.variableValues( var ) )
                {
                    m_variableValues[ var ] = includefile.variableValues( var );
                }
            }
            if( !node->identifier->value.startsWith('!') )
            {
                visitNode( node->body );
            }
        }else if( node->identifier->value.startsWith('!') )
        {
            visitNode( node->body );
        }
    }else
    {
        visitNode( node->body );
    }
}

void QMakeFile::visitAssignment( QMake::AssignmentAST* node )
{
    QString op = node->op->value;
    QStringList values = getValueList(node->values);
    if( op == "=" )
    {
        m_variableValues[node->identifier->value] = values;
    }else if( op == "+=" )
    {
        m_variableValues[node->identifier->value] += values;
    }else if( op == "-=" )
    {
        foreach( const QString& value, values )
        {
            m_variableValues[node->identifier->value].removeAll(value);
        }
    }else if( op == "*=" )
    {
        foreach( const QString& value, values )
        {
            if( !m_variableValues.value(node->identifier->value).contains(value) )
            {
                m_variableValues[node->identifier->value].append(value);
            }
        }
    }else if( op == "~=" )
    {
        if( values.isEmpty() )
            return;
        QString value = values.first().trimmed();
        QString regex = value.mid(2,value.indexOf("/", 2));
        QString replacement = value.mid(value.indexOf("/", 2)+1,value.lastIndexOf("/"));
        m_variableValues[node->identifier->value].replaceInStrings( QRegExp(regex), replacement );
    }
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

QStringList QMakeFile::resolveShellGlobbing( const QString& absolutefile )
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

QStringList QMakeFile::resolveVariables( const QString& var ) const
{
    VariableReferenceParser parser;
    parser.setContent( var );
    if( !parser.parse() ) {
        kWarning(9024) << "Couldn't parse" << var << "to replace variables in it";
        return QStringList() << var;
    }
    if (parser.variableReferences().isEmpty()) {
        return QStringList() << var;
    }

    ///TODO: multiple vars in one place will make the offsets go bonkers
    ///TODO: function arguments
    ///TODO: $$1...
    QString value = var;
    foreach( const QString& variable, parser.variableReferences() ) {
        VariableInfo vi = parser.variableInfo( variable );
        QString varValue;

        switch (vi.type) {
            case VariableInfo::QMakeVariable:
                if (!m_variableValues.contains(variable)) {
                    kWarning(9024) << "unknown variable:" << variable << "skipping";
                    continue;
                }
                varValue = m_variableValues.value( variable, QStringList() ).join(" ");
                break;
            case VariableInfo::ShellVariableResolveQMake:
            case VariableInfo::ShellVariableResolveMake:
                ///TODO: make vs qmake time
                varValue = QProcessEnvironment::systemEnvironment().value(variable);
                break;
            case VariableInfo::QtConfigVariable:
                //should be handled in QMakeProjectfile
                kWarning(9024) << "QtConfigVariable slipped through:" << variable;
                continue;
            case VariableInfo::FunctionCall:
                if (variable == "qtLibraryTarget") {
                    foreach(const VariableInfo::Position& pos, vi.positions ) {
                        int start = pos.start + 3 + variable.length();
                        QString args = value.mid(start , pos.end - start);
                        varValue = resolveVariables( args ).join(" ");
                        value.replace(pos.start, pos.end - pos.start + 1, varValue);
                    }
                } else {
                    ///TODO:
                    kWarning(9024) << "unimplemented function call in variable:" << variable;
                }
                continue;
            case VariableInfo::Invalid:
                kWarning(9024) << "invalid qmake variable:" << variable;
                continue;
        }

        foreach(const VariableInfo::Position& pos, vi.positions ) {
            value.replace(pos.start, pos.end - pos.start + 1, varValue);
        }
    }

    QStringList ret = value.split(" ", QString::SkipEmptyParts);
    ifDebug(kDebug(9024) << "resolved variable" << var << "to" << ret;)
    return ret;
}

//kate: hl c++;

