/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "scope.h"

#include <kdebug.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qpair.h>
#include <qmakedriver.h>

#include "pathutil.h"

const QStringList Scope::KnownVariables = QStringList() << "QT" << "CONFIG" << "TEMPLATE" << "SUBDIRS" << "VERSION" << "LIBS" << "target.path" << "INSTALLS" << "MAKEFILE" << "TARGETDEPS" << "INCLUDEPATH" << "TARGET" << "DESTDIR" << "DEFINES" << "QMAKE_CXXFLAGS_DEBUG" << "QMAKE_CXXFLAGS_RELEASE" << "OBJECTS_DIR" << "UI_DIR" << "MOC_DIR" << "IDL_COMPILER" << "IDL_OPTIONS" << "RCC_DIR" << "IDLS" << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES" << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES" << "INTERFACES" << "FORMS" ;

const QStringList Scope::KnownConfigValues = QStringList() << "debug" << "release" << "debug_and_release" << "warn_on" << "warn_off" << "staticlib" << "dll" << "plugin" << "designer" << "create_pkgconf" << "create_libtool" << "qt" << "console" << "windows" << "x11" << "thread" << "exceptions" << "stl" << "rtti" << "opengl" << "thread" << "ordered" << "precompile_header" << "qtestlib" << "uitools" << "dbus" << "assistant" << "build_all";

Scope::Scope( const QString &filename, bool isQt4Project )
        : m_root( 0 ), m_incast( 0 ), m_parent( 0 ), m_isQt4Project( isQt4Project ), m_isEnabled( true )
{
    if ( !loadFromFile( filename ) && !QFileInfo( filename ).exists() )
    {
        m_root = new QMake::ProjectAST();
        m_root->setFileName( filename );
    }
}

Scope::~Scope()
{
    QMap<QString, Scope*>::iterator it;
    for ( it = m_funcScopes.begin() ; it != m_funcScopes.end() ; ++it )
    {
        Scope* s = it.data();
        m_funcScopes.remove( it );
        delete s;
    }
    m_funcScopes.clear();

    for ( it = m_simpleScopes.begin() ; it != m_simpleScopes.end() ; ++it )
    {
        Scope* s = it.data();
        m_simpleScopes.remove( it );
        delete s;
    }
    m_simpleScopes.clear();

    for ( it = m_incScopes.begin() ; it != m_incScopes.end() ; ++it )
    {
        Scope* s = it.data();
        m_incScopes.remove( it );
        delete s;
    }
    m_incScopes.clear();

    for ( it = m_subProjects.begin() ; it != m_subProjects.end() ; ++it )
    {
        Scope* s = it.data();
        m_subProjects.remove( it );
        delete s;
    }
    m_subProjects.clear();

    if ( m_root->isProject() )
        delete m_root;
}

Scope::Scope( bool isQt4Project, QMake::ProjectAST* scope, Scope* parent )
        : m_root( scope ), m_incast( 0 ), m_parent( parent ), m_isQt4Project( isQt4Project ), m_isEnabled( true )
{
    init();
}

Scope::Scope( bool isQt4Project, Scope* parent, const QString& filename )
        : m_root( 0 ), m_incast( 0 ), m_parent( parent ), m_isQt4Project( isQt4Project ), m_isEnabled( true )
{
    if ( !loadFromFile( filename ) && !QFileInfo( filename ).exists() )
    {
        m_root = new QMake::ProjectAST();
        m_root->setFileName( filename );
    }
}

Scope::Scope( bool isQt4Project, Scope* parent, const QString& filename, bool isEnabled )
: m_root( 0 ), m_incast( 0 ), m_parent( parent ), m_isQt4Project( isQt4Project ), m_isEnabled( isEnabled )
{
    if ( !loadFromFile( filename ) && !QFileInfo( filename ).exists() )
    {
        m_root = new QMake::ProjectAST();
        m_root->setFileName( filename );
    }
}

Scope::Scope( bool isQt4Project, Scope* parent, QMake::IncludeAST* incast, const QString& path, const QString& incfile )
        : m_root( 0 ), m_incast( incast ), m_parent( parent ), m_isQt4Project( isQt4Project ), m_isEnabled( true )
{
    if ( !loadFromFile( path + QString( QChar( QDir::separator() ) ) + incfile ) && !QFileInfo( path + QString( QChar( QDir::separator() ) ) + incfile ).exists() )
    {
        m_root = new QMake::ProjectAST();
        m_root->setFileName( QString( path + QString( QChar( QDir::separator() ) ) + incfile ) );
    }
}

bool Scope::loadFromFile( const QString& filename )
{
    if ( QMake::Driver::parseFile( filename, &m_root ) != 0 )
    {
        kdDebug( 9024 ) << "Couldn't parse project: " << filename << endl;
        m_root = 0;
        return false;
    }
    init();
    return true;
}

void Scope::saveToFile() const
{
    if ( !m_root )
        return ;

    if ( scopeType() != ProjectScope && scopeType() != IncludeScope )
    {
        m_parent->saveToFile();
        return;
    }

    QString filename;
    if ( scopeType() == ProjectScope )
        filename = m_root->fileName() ;
    else if ( scopeType() == IncludeScope )
        filename = m_parent->projectDir() + QString( QChar( QDir::separator() ) ) + m_incast->projectName;
    if ( filename.isEmpty() )
        return ;

    QFile file( filename );
    if ( file.open( IO_WriteOnly ) )
    {

        QTextStream out( &file );
        QString astbuffer;
        m_root->writeBack( astbuffer );
        out << astbuffer;
    }
#ifdef DEBUG
    Scope::PrintAST pa;
    pa.processProject(m_root);
#endif
}

void Scope::addToPlusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "+=", values, false );
}

void Scope::removeFromPlusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "+=", values, true );
}


void Scope::addToMinusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "-=", values, false );
}

void Scope::removeFromMinusOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "-=", values, true );
}

void Scope::addToEqualOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "=", values, false );
}

void Scope::removeFromEqualOp( const QString& variable, const QStringList& values )
{
    if ( !m_root )
        return ;

    updateVariable( variable, "=", values, true );
}

void Scope::setPlusOp( const QString& variable, const QStringList& values )
{
    if( listsEqual(values, variableValuesForOp(variable, "+=") ) )
        return;

    updateVariable( variable, "+=", variableValuesForOp( variable, "+=" ), true );
    updateVariable( variable, "+=", values, false );
}

void Scope::setEqualOp( const QString& variable, const QStringList& values )
{
    if( listsEqual(values, variableValuesForOp(variable, "=") ) )
        return;

    updateVariable( variable, "=", variableValuesForOp( variable, "=" ), true );
    updateVariable( variable, "=", values, false );
}

void Scope::setMinusOp( const QString& variable, const QStringList& values )
{
    if( listsEqual(values, variableValuesForOp(variable, "-=") ) )
        return;

    updateVariable( variable, "-=", variableValuesForOp( variable, "-=" ), true );
    updateVariable( variable, "-=", values, false );
}

QStringList Scope::variableValuesForOp( const QString& variable , const QString& op ) const
{
    QStringList result;
    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        QMake::AST* ast = *it;
        if ( ast->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assign = static_cast<QMake::AssignmentAST*>( ast );
            if ( assign->scopedID == variable && assign->op == op )
            {
                result += assign->values;
            }
        }
    }
    result.remove( "\\\n" );
    result.remove( "\n" );
    result = Scope::removeWhiteSpace(result);
    return result;
}

QStringList Scope::variableValues( const QString& variable ) const
{
    QStringList result;

    if ( !m_root )
        return result;

    if ( variable == "QT" )
    {
        result << "gui" << "core";
    }
    else if ( variable == "CONFIG" && m_isQt4Project )
    {
        result << "warn_on" << "qt" << "stl";
    }
    else if ( variable == "CONFIG" && !m_isQt4Project )
    {
        result << "qt" << "warn_on" << "thread";
    }

    result = calcValuesFromStatements( variable, result );
    result.remove( "\\\n" );
    result.remove( "\n" );
    result = Scope::removeWhiteSpace(result);
    return result;
}

QStringList Scope::calcValuesFromStatements( const QString& variable, QStringList result, QMake::AST* stopHere ) const
{
    if ( scopeType() == FunctionScope || scopeType() == SimpleScope )
    {
        result = m_parent->calcValuesFromStatements( variable, result , this->m_root );
    }
    else if ( scopeType() == IncludeScope )
    {
        result = m_parent->calcValuesFromStatements( variable, result , this->m_incast );
    }

    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( stopHere && *it == stopHere )
            return result;
        QMake::AST* ast = *it;
        if ( ast->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assign = static_cast<QMake::AssignmentAST*>( ast );
            if ( assign->scopedID == variable )
            {
                if ( assign->op == "=" )
                {
                    result = assign->values;
                }
                else if ( assign->op == "+=" )
                {
                    for ( QStringList::const_iterator sit = assign->values.begin(); sit != assign->values.end() ; ++sit )
                    {
                        if ( !result.contains( *sit ) )
                            result.append( *sit );
                    }
                }
                else if ( assign->op == "-=" )
                {
                    for ( QStringList::const_iterator sit = assign->values.begin(); sit != assign->values.end() ; ++sit )
                    {
                        if ( result.contains( *sit ) )
                            result.remove( *sit );
                    }
                }
            }
        }
    }

    result.remove( "\\\n" );
    result.remove( "\n" );
    return result;
}

Scope::ScopeType Scope::scopeType() const
{
    if ( !m_root )
        return InvalidScope;
    else if ( m_incast )
        return IncludeScope;
    else if ( m_root->isProject() )
        return ProjectScope;
    else if ( m_root->isScope() )
        return SimpleScope;
    else if ( m_root->isFunctionScope() )
        return FunctionScope;
    return InvalidScope;
}

QString Scope::scopeName() const
{
    if ( !m_root )
        return QString();
    if ( m_incast )
        return "include<" + m_incast->projectName + ">";
    else if ( m_root->isFunctionScope() )
        return funcScopeKey( m_root );
    else if ( m_root->isScope() )
        return m_root->scopedID;
    else if ( m_root->isProject() )
    {
        if( m_parent )
        {
            return getRelativePath( m_parent->projectDir(), projectDir() );
        }else
            return QFileInfo( projectDir() ).fileName() ;
    }
    return QString();
}

QString Scope::fileName() const
{
    if ( m_incast )
        return m_incast->projectName;
    else if ( m_root->isProject() )
        return QFileInfo( m_root->fileName() ).fileName();
    else
        return m_parent->fileName();
}

Scope* Scope::createFunctionScope( const QString& funcName, const QString& args )
{
    if ( !m_root )
        return 0;

    QMake::ProjectAST* ast = new QMake::ProjectAST( QMake::ProjectAST::FunctionScope );
    ast->scopedID = funcName;
    ast->args = args;
    ast->setDepth( m_root->depth() );
    ast->addChildAST( new QMake::NewLineAST() );
    m_root->addChildAST( ast );
    m_root->addChildAST( new QMake::NewLineAST() );
    Scope* funcScope = new Scope( m_isQt4Project, ast, this );
    m_funcScopes.insert( funcScopeKey( ast ), funcScope );
    return funcScope;
}

Scope* Scope::createSimpleScope( const QString& scopename )
{
    if ( !m_root )
        return 0;

    QMake::ProjectAST* ast = new QMake::ProjectAST( QMake::ProjectAST::Scope );
    ast->scopedID = scopename;
    ast->addChildAST( new QMake::NewLineAST() );
    ast->setDepth( m_root->depth() );
    m_root->addChildAST( ast );
    m_root->addChildAST( new QMake::NewLineAST() );
    if ( isQt4Project() )
        addToPlusOp( "CONFIG", QStringList( scopename ) );
    Scope* simpleScope = new Scope( m_isQt4Project, ast, this );
    m_simpleScopes.insert( scopename, simpleScope );
    return simpleScope;
}

Scope* Scope::createIncludeScope( const QString& includeFile, bool negate )
{
    if ( !m_root )
        return 0;

    Scope* funcScope;
    if ( negate )
    {
        funcScope = createFunctionScope( "!include", includeFile );
    }
    else
    {
        funcScope = createFunctionScope( "include", includeFile );
    }
    QMake::IncludeAST* ast = new QMake::IncludeAST();
    ast->setDepth( m_root->depth() );
    ast->projectName = includeFile;
    Scope* incScope = new Scope( m_isQt4Project, funcScope, ast, projectDir(), ast->projectName );
    if ( incScope->scopeType() != InvalidScope )
    {
        funcScope->m_incScopes.insert( includeFile, incScope );
        funcScope->m_root->addChildAST( ast );
        return funcScope;
    }
    else
    {
        deleteFunctionScope( funcScope->m_root->scopedID );
        delete incScope;
    }
    return 0;

}

Scope* Scope::createSubProject( const QString& dir )
{
    QDir curdir( projectDir() );
    QValueList<QMake::AST*>::iterator it = findExistingVariable( "TEMPLATE" );
    if ( it != m_root->m_children.end() )
    {
        QMake::AssignmentAST * tempast = static_cast<QMake::AssignmentAST*>( *it );
        if ( tempast->values.contains( "subdirs" ) )
        {

            if ( !curdir.exists( dir ) )
                if ( !curdir.mkdir( dir ) )
                    return 0;
            curdir.cd(dir);
            QString filename;
            QStringList entries = curdir.entryList("*.pro", QDir::Files);

            if ( !entries.isEmpty() && !entries.contains( curdir.dirName()+".pro" ) )
                filename = curdir.absPath() + QString(QChar(QDir::separator()))+entries.first();
            else
                filename = curdir.absPath() + QString(QChar(QDir::separator()))+curdir.dirName()+".pro";

            kdDebug( 9024 ) << "Creating subproject with filename:" << filename << endl;

            Scope* s = new Scope( m_isQt4Project, this, filename );
            if ( s->scopeType() != InvalidScope )
            {
                if( s->variableValues("TEMPLATE").isEmpty() )
                    s->setEqualOp("TEMPLATE", QStringList("app"));
                s->saveToFile();
                addToPlusOp( "SUBDIRS", QStringList( dir ) );
                m_subProjects.insert( dir, s );
                return s;
            } else
            {
                delete s;
            }
        }
    }
    return 0;
}

bool Scope::deleteFunctionScope( const QString& functionCall )
{
    if ( !m_root )
        return false;

    Scope* funcScope = m_funcScopes[ functionCall ];
    if ( funcScope )
    {
        QMake::AST* ast = m_root->m_children[ m_root->m_children.findIndex( funcScope->m_root ) ];
        if( !ast )
            return false;
        m_funcScopes.remove( functionCall );
        m_root->removeChildAST( funcScope->m_root );
        delete funcScope;
        delete ast;
        return true;
    }
    return false;
}

bool Scope::deleteSimpleScope( const QString& scopeId )
{
    if ( !m_root )
        return false;

    Scope* simpleScope = m_simpleScopes[ scopeId ];
    if ( simpleScope )
    {
        QMake::AST* ast = m_root->m_children[ m_root->m_children.findIndex( simpleScope->m_root ) ];
        if( !ast )
            return false;
        m_simpleScopes.remove( scopeId );
        removeFromPlusOp( "CONFIG", scopeId );
        m_root->removeChildAST( simpleScope->m_root );
        delete simpleScope;
        delete ast;
        return true;
    }
    return false;
}

bool Scope::deleteIncludeScope( const QString& includeFile, bool negate )
{
    if ( !m_root )
        return false;

    QString funcScopeId;
    if ( negate )
    {
        funcScopeId = "!include(" + includeFile + ")";
    }
    else
    {
        funcScopeId = "include(" + includeFile + ")";
    }


    Scope * incScope = m_incScopes[ includeFile ];
    if( !incScope )
        return false;
    QMake::AST* ast = incScope->m_incast;
    if( !ast )
        return false;
    m_incScopes.remove( includeFile );
    m_root->removeChildAST( incScope->m_incast);
    delete incScope;
    delete ast;

    return m_parent->deleteFunctionScope( funcScopeId );
}

bool Scope::deleteSubProject( const QString& dir, bool deleteSubdir )
{
    if ( !m_root )
        return false;

    QValueList<QMake::AST*>::iterator it = findExistingVariable( "TEMPLATE" );
    if ( it != m_root->m_children.end() )
    {
        QMake::AssignmentAST * tempast = static_cast<QMake::AssignmentAST*>( *it );
        if ( m_subProjects.contains( dir ) && tempast->values.contains( "subdirs" ) )
        {
            if ( deleteSubdir )
            {
                QDir projdir = QDir( projectDir() );
                QDir subdir = QDir( projectDir() + QString( QChar( QDir::separator() ) ) + dir );
                if ( subdir.exists() )
                {
                    QStringList entries = subdir.entryList();
                    for ( QStringList::iterator eit = entries.begin() ; eit != entries.end() ; ++eit )
                    {
                        if( *eit == "." || *eit == ".." )
                            continue;
                        if( !subdir.remove( *eit ) )
                            kdDebug( 9024 ) << "Couldn't delete " << *eit << " from " << subdir.absPath() << endl;
                    }
                    if( !projdir.rmdir( dir ) )
                        kdDebug( 9024 ) << "Couldn't delete " << dir << " from " << projdir.absPath() << endl;
                }
            }
            Scope* project = m_subProjects[ dir ];
            if( !project )
                return false;
            QValueList<QMake::AST*>::iterator foundit = findExistingVariable( "SUBDIRS" );
            if ( foundit != m_root->m_children.end() )
            {
                QMake::AssignmentAST * ast = static_cast<QMake::AssignmentAST*>( *foundit );
                updateValues( ast->values, QStringList( dir ), true, ast->indent );
            }else
                return false;
            m_subProjects.remove( dir );
            delete project;
            return true;
        }
    }
    return false;
}

void Scope::updateValues( QStringList& origValues, const QStringList& newValues, bool remove, QString indent )
{
    for ( QStringList::const_iterator it = newValues.begin(); it != newValues.end() ; ++it )
    {
        if ( !origValues.contains( *it ) && !remove )
        {
            while ( origValues.last() == "\n" )
                origValues.pop_back();
            if ( origValues.count() > 0 && origValues.last() != "\\\n" )
            {
                origValues.append( "\\\n" );
                kdDebug(9024) << " indent == |" << indent << "|" <<endl;
                if( indent == "" )
                    origValues.append( "  " );
                else
                    origValues.append( indent );
            }
            if( (*it).contains(" ") || (*it).contains("\t") || (*it).contains("\n") )
                origValues.append( "\""+*it+"\"" );
            else
                origValues.append( *it );
            origValues.append( "\n" );
        } else if ( origValues.contains( *it ) && remove )
        {
            QStringList::iterator posit = origValues.find( *it );
            posit++;
            if ( *posit == "\\\n" || (*posit).stripWhiteSpace() == "" )
                origValues.remove( posit );
            origValues.remove( *it );

        }
    }
    while( (origValues.last() == "\\\n"
            || origValues.last() == "\n"
            || origValues.last().stripWhiteSpace() == "" ) )
        origValues.pop_back();
    origValues.append("\n");
}

void Scope::updateVariable( const QString& variable, const QString& op, const QStringList& values, bool removeFromOp )
{
    if ( listIsEmpty( values ) )
        return ;

    kdDebug(9024) << "Updating variable:" << variable << endl;

    for ( int i = m_root->m_children.count() - 1; i >= 0; --i )
    {
        if ( m_root->m_children[ i ] ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assignment = static_cast<QMake::AssignmentAST*>( m_root->m_children[ i ] );
            if ( assignment->scopedID == variable && isCompatible( assignment->op, op ) )
            {
                updateValues( assignment->values, values, removeFromOp, assignment->indent );
                if ( removeFromOp && listIsEmpty( assignment->values ) )
                {
                    m_root->removeChildAST( assignment );
                    delete assignment;
                }
                return ;
            }
            else if ( assignment->scopedID == variable && !isCompatible( assignment->op, op ) )
            {
                for ( QStringList::const_iterator it = values.begin() ; it != values.end() ; ++it )
                {
                    if ( op == "+=" && !removeFromOp && assignment->values.contains( *it ) )
                    {
                        if ( assignment->op == "=" )
                        {
                            updateValues( assignment->values, values, false, assignment->indent );
                            return ;
                        }
                        else if ( assignment->op == "-=" )
                        {
                            updateValues( assignment->values, QStringList( *it ), true, assignment->indent );
                            if ( listIsEmpty( assignment->values ) )
                            {
                                m_root->removeChildAST( assignment );
                                delete assignment;
                                break;
                            }
                        }
                    }
                    else if ( op == "-=" && !removeFromOp && assignment->values.contains( *it ) )
                    {
                        updateValues( assignment->values, QStringList( *it ), true, assignment->indent );
                        if ( listIsEmpty( assignment->values ) )
                        {
                            m_root->removeChildAST( assignment );
                            delete assignment;
                            break;
                        }
                    }
                    else if ( op == "=" )
                    {
                        if ( !removeFromOp )
                        {
                            m_root->removeChildAST( assignment );
                            delete assignment;
                        }
                        else if ( assignment->op == "+=" && assignment->values.contains( *it ) )
                        {
                            updateValues( assignment->values, QStringList( *it ), true, assignment->indent );
                            if ( listIsEmpty( assignment->values ) )
                            {
                                m_root->removeChildAST( assignment );
                                delete assignment;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !removeFromOp )
    {
        QMake::AssignmentAST * ast = new QMake::AssignmentAST();
        ast->scopedID = variable;
        ast->op = op;
        ast->values = values;
        if( scopeType() == ProjectScope )
            ast->setDepth( m_root->depth() );
        else
            ast->setDepth( m_root->depth()+1 );
        m_root->addChildAST( ast );
        if ( !values.contains( "\n" ) )
        {
            ast->values.append("\n");
        }
    }
}

QValueList<QMake::AST*>::iterator Scope::findExistingVariable( const QString& variable )
{
    QValueList<QMake::AST*>::iterator it;
    QStringList ops;
    ops << "=" << "+=";

    for ( it = m_root->m_children.begin(); it != m_root->m_children.end() ; ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assignment = static_cast<QMake::AssignmentAST*>( *it );
            if ( assignment->scopedID == variable && ops.contains( assignment->op ) )
            {
                return it;
            }
        }
    }
    return m_root->m_children.end();
}

void Scope::init()
{
    if( !m_root )
        return;
    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::ProjectAST )
        {
            QMake::ProjectAST * p = static_cast<QMake::ProjectAST*>( *it );
            if ( p->isFunctionScope() )
            {
                m_funcScopes.insert( funcScopeKey( p ), new Scope( m_isQt4Project, p, this ) );
            }
            else if ( p->isScope() )
            {
                m_simpleScopes.insert( p->scopedID, new Scope( m_isQt4Project, p, this ) );
            }
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::IncludeAST )
        {
            QMake::IncludeAST * i = static_cast<QMake::IncludeAST*>( *it );
            m_incScopes.insert( i->projectName, new Scope( m_isQt4Project, this, i, projectDir(), i->projectName ) );
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * m = static_cast<QMake::AssignmentAST*>( *it );
            // Check wether TEMPLATE==subdirs here too!
            if ( m->scopedID == "SUBDIRS" && variableValues("TEMPLATE").contains("subdirs") )
            {
                for ( QStringList::const_iterator sit = m->values.begin() ; sit != m->values.end(); ++sit )
                {
                    QString str = *sit;
                    if ( *sit == "\\\n" || *sit == "\n" || *sit == "." || *sit == "./" )
                        continue;
                    QDir subproject = QDir( projectDir() + QString( QChar( QDir::separator() ) ) + *sit, "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                    QString projectfile;
                    if ( subproject.entryList().contains( *sit + ".pro" ) )
                        projectfile = *subproject.entryList().find( *sit + ".pro" );
                    else
                        projectfile = subproject.entryList().first();
                    kdDebug( 9024 ) << "Parsing subproject: " << projectfile << endl;
                    m_subProjects.insert( *sit, new Scope( m_isQt4Project, this, subproject.absFilePath( projectfile ), ( m->op != "-=" ) ) );
                }
            }
            else if ( !KnownVariables.contains( m->scopedID ) && !m->scopedID.contains( ".files" ) && !m->scopedID.contains( ".path" ) && !variableValues("INSTALL").contains(m->scopedID) )
            {
                m_customVariables[ qMakePair( m->scopedID, m->op ) ] = m;
            }
        }
    }
}

QString Scope::projectName() const
{
    return QFileInfo( projectDir() ).fileName();
}

QString Scope::projectDir() const
{
    if ( m_root->isProject() )
    {
        return QFileInfo( m_root->fileName() ).dirPath( true );
    }
    else
    {
        return m_parent->projectDir();
    }
}

const QValueList<Scope*> Scope::scopesInOrder() const
{
    QValueList<Scope*> result;
    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::ProjectAST )
        {
            QMake::ProjectAST * p = static_cast<QMake::ProjectAST*>( *it );
            if ( p->isScope() && m_simpleScopes.contains( p->scopedID ) )
            {
                result.append( m_simpleScopes[ p->scopedID ] );
            }
            else if ( p->isFunctionScope() && m_funcScopes.contains( funcScopeKey( p ) ) )
            {
                result.append( m_funcScopes[ funcScopeKey( p ) ] );
            }
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::IncludeAST )
        {
            QMake::IncludeAST * i = static_cast<QMake::IncludeAST*>( *it );
            if ( m_incScopes.contains( i->projectName ) )
            {
                result.append( m_incScopes[ i->projectName ] );
            }
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * s = static_cast<QMake::AssignmentAST*>( *it );
            if ( s->scopedID == "SUBDIRS" )
            {
                for ( QStringList::const_iterator sit = s->values.begin(); sit != s->values.end() ; ++sit )
                {
                    if ( (*sit).stripWhiteSpace() != "" && *sit != "\\\n" && m_subProjects.contains( *sit ) )
                    {
                        if ( m_subProjects.contains( *sit ) )
                        {
                            result.append( m_subProjects[ *sit ] );
                        }
                    }
                }
            }
        }
    }
    return result;
}


const QMap<QPair<QString, QString>, QStringList> Scope::customVariables() const
{
    QMap<QPair<QString, QString>, QStringList> result;
    kdDebug( 9024 ) << "# of custom vars:" << m_customVariables.size() << endl;
    QMap<QPair<QString, QString>, QMake::AssignmentAST*>::const_iterator it = m_customVariables.begin();
    for ( ; it != m_customVariables.end(); ++it )
    {
        kdDebug( 9024 ) << "Pair is: " << it.key().first << "|" << it.key().second << endl;
        it.data()->values;
        kdDebug( 9024 ) << "access test passed, now assign" << endl;
        result[ it.key() ] = it.data()->values;
    }
    return result;
}

void Scope::updateCustomVariable( const QString& var, const QString& op, const QStringList& values )
{
    if ( m_customVariables.keys().contains( qMakePair( var, op ) ) )
    {
        m_customVariables[ qMakePair( var, op ) ] ->values = values;
    }
    else
    {
        QMake::AssignmentAST* newast = new QMake::AssignmentAST();
        newast->scopedID = var;
        newast->op = op;
        newast->values = values;
        if( scopeType() == ProjectScope )
            newast->setDepth( m_root->depth() );
        else
            newast->setDepth( m_root->depth()+1 );
        m_root->addChildAST( newast );
        if ( values.contains( "\n" ) )
        {
            newast->values.append("\n");
        }
    }
}

bool Scope::isVariableReset( const QString& var )
{
    bool result = false;
    QValueList<QMake::AST*>::const_iterator it = m_root->m_children.begin();
    for ( ; it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * ast = static_cast<QMake::AssignmentAST*>( *it );
            if ( ast->scopedID == var && ast->op == "=" )
            {
                result = true;
                break;
            }
        }
    }
    return result;
}

void Scope::removeVariable( const QString& var, const QString& op )
{
    if ( !m_root )
        return ;

    QMake::AssignmentAST* ast = 0;

    QValueList<QMake::AST*>::iterator it = m_root->m_children.begin();
    for ( ; it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::AssignmentAST )
        {
            ast = static_cast<QMake::AssignmentAST*>( *it );
            if ( ast->scopedID == var && ast->op == op )
            {
                m_root->m_children.remove( ast );
                it = m_root->m_children.begin();
            }
        }
    }
}

bool Scope::listIsEmpty( const QStringList& values )
{
    if ( values.size() < 1 )
        return true;
    for ( QStringList::const_iterator it = values.begin(); it != values.end(); ++it )
    {
        if ( ( *it ).stripWhiteSpace() != "" && ( *it ).stripWhiteSpace() != "\\" )
            return false;
    }
    return true;
}

bool Scope::isCompatible( const QString& op1, const QString& op2) const
{
    if( op1 == "+=" )
        return ( op2 == "+=" || op2 == "=" );
    else if ( op1 == "-=" )
        return ( op2 == "-=" );
    else if ( op1 == "=" )
        return ( op2 == "=" || op2 == "+=" );
    return false;
}

bool Scope::listsEqual(const QStringList& l1, const QStringList& l2) const
{
    QStringList left = l1;
    QStringList right = l2;
    left.sort();
    right.sort();
    kdDebug(9024) << "comparing lists:" << left.join("|").replace("\n","%n") << " == " << right.join("|").replace("\n","%n") << endl;
    kdDebug(9024) << "Result:" << (left == right) << endl;
    return (left == right);
}

QStringList Scope::removeWhiteSpace(const QStringList& list)
{
    QStringList result;
    for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
    {
        QString s = *it;
        if( s.stripWhiteSpace() != "" )
            result.append(s);
    }
    return result;
}

#ifdef DEBUG
void Scope::printTree()
{
    PrintAST p;
    p.processProject(m_root);
}

Scope::PrintAST::PrintAST() : QMake::ASTVisitor()
{
    indent = 0;
}

void Scope::PrintAST::processProject( QMake::ProjectAST* p )
{
    QMake::ASTVisitor::processProject(p);
}

void Scope::PrintAST::enterRealProject( QMake::ProjectAST* p )
{
    kdDebug(9024) << getIndent() << "--------- Entering Project: " << replaceWs(p->fileName()) << " --------------" << endl;
    indent += 4;
    QMake::ASTVisitor::enterRealProject(p);
}

void Scope::PrintAST::leaveRealProject( QMake::ProjectAST* p )
{
    indent -= 4;
    kdDebug(9024) << getIndent() << "--------- Leaving Project: " << replaceWs(p->fileName()) << " --------------" << endl;
    QMake::ASTVisitor::leaveRealProject(p);
}

void Scope::PrintAST::enterScope( QMake::ProjectAST* p )
{
    kdDebug(9024) << getIndent() << "--------- Entering Scope: " << replaceWs(p->scopedID) << " --------------" << endl;
    indent += 4;
    QMake::ASTVisitor::enterScope(p);
}

void Scope::PrintAST::leaveScope( QMake::ProjectAST* p )
{
    indent -= 4;
    kdDebug(9024) << getIndent() << "--------- Leaving Scope: " << replaceWs(p->scopedID) << " --------------" << endl;
    QMake::ASTVisitor::leaveScope(p);
}

void Scope::PrintAST::enterFunctionScope( QMake::ProjectAST* p )
{
    kdDebug(9024) << getIndent() << "--------- Entering FunctionScope: " << replaceWs(p->scopedID) << "(" << replaceWs(p->args) << ")"<< " --------------" << endl;
    indent += 4;
    QMake::ASTVisitor::enterFunctionScope(p);
}

void Scope::PrintAST::leaveFunctionScope( QMake::ProjectAST* p )
{
    indent -= 4;
    kdDebug(9024) << getIndent() << "--------- Leaving FunctionScope: " << replaceWs(p->scopedID) << "(" << replaceWs(p->args) << ")"<< " --------------" << endl;
    QMake::ASTVisitor::leaveFunctionScope(p);
}

QString Scope::PrintAST::replaceWs(QString s)
{
    return s.replace("\n", "%nl").replace("\t", "%tab").replace(" ", "%spc");
}

void Scope::PrintAST::processAssignment( QMake::AssignmentAST* a)
{
    kdDebug(9024) << getIndent() << "Assignment: " << replaceWs(a->scopedID) << " " << replaceWs(a->op) << " "
        << replaceWs(a->values.join("|"))<< endl;
    QMake::ASTVisitor::processAssignment(a);
}

void Scope::PrintAST::processNewLine( QMake::NewLineAST* n)
{
    kdDebug(9024) << getIndent() << "Newline " << endl;
    QMake::ASTVisitor::processNewLine(n);
}

void Scope::PrintAST::processComment( QMake::CommentAST* a)
{
    kdDebug(9024) << getIndent() << "Comment: " << replaceWs(a->comment) << endl;
    QMake::ASTVisitor::processComment(a);
}

void Scope::PrintAST::processInclude( QMake::IncludeAST* a)
{
    kdDebug(9024) << getIndent() << "Include: " << replaceWs(a->projectName) << endl;
    QMake::ASTVisitor::processInclude(a);
}

QString Scope::PrintAST::getIndent()
{
    QString ind;
    for( int i = 0 ; i < indent ; i++)
        ind += " ";
    return ind;
}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
