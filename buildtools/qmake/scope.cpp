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
#include <qregexp.h>

#include <kdirwatch.h>

#include "pathutil.h"
#include "trollprojectpart.h"
#include "qmakedefaultopts.h"
#include "pathutil.h"

const QStringList Scope::KnownVariables = QStringList() << "QT" << "CONFIG" << "TEMPLATE" << "SUBDIRS" << "VERSION" << "LIBS" << "target.path" << "INSTALLS" << "MAKEFILE" << "TARGETDEPS" << "INCLUDEPATH" << "TARGET" << "DESTDIR" << "DEFINES" << "QMAKE_CXXFLAGS_DEBUG" << "QMAKE_CXXFLAGS_RELEASE" << "OBJECTS_DIR" << "UI_DIR" << "MOC_DIR" << "IDL_COMPILER" << "IDL_OPTIONS" << "RCC_DIR" << "IDLS" << "RESOURCES" << "IMAGES" << "LEXSOURCES" << "DISTFILES" << "YACCSOURCES" << "TRANSLATIONS" << "HEADERS" << "SOURCES" << "INTERFACES" << "FORMS" ;

const QStringList Scope::KnownConfigValues = QStringList() << "debug" << "release" << "debug_and_release" << "warn_on" << "warn_off" << "staticlib" << "dll" << "plugin" << "designer" << "create_pkgconf" << "create_libtool" << "qt" << "console" << "windows" << "x11" << "thread" << "exceptions" << "stl" << "rtti" << "opengl" << "thread" << "ordered" << "precompile_header" << "qtestlib" << "uitools" << "dbus" << "assistant" << "build_all";

Scope::Scope( const QString &filename, TrollProjectPart* part )
    : m_root( 0 ), m_incast( 0 ), m_parent( 0 ), m_num(0), m_isEnabled( true ), m_part(part), m_defaultopts(0), m_initFinished(false)
{
    m_defaultopts = new QMakeDefaultOpts();
    connect( m_defaultopts, SIGNAL( variablesRead() ), this, SLOT( init() ) );
    m_defaultopts->readVariables( DomUtil::readEntry( *m_part->projectDom(), "/kdevcppsupport/qt/root", "" ),
                                 QFileInfo( filename ).dirPath( true ) );

    if ( !loadFromFile( filename ) )
    {
        if( !QFileInfo( filename ).exists() )
        {
            m_root = new QMake::ProjectAST();
            m_root->setFileName( filename );
        }else
        {
            delete m_root;
            m_root = 0;
        }
    }
    if( m_root )
        m_part->dirWatch()->addFile(filename);
}

Scope::~Scope()
{
    QMap<unsigned int, Scope*>::iterator it;
    for ( it = m_scopes.begin() ; it != m_scopes.end() ; ++it )
    {
        Scope* s = it.data();
        delete s;
    }
    m_scopes.clear();

    m_customVariables.clear();

    if( !m_parent && m_root->isProject() )
        delete m_defaultopts;

    if ( m_root && m_root->isProject() )
        delete m_root;
    m_root = 0;
}

Scope::Scope( unsigned int num, Scope* parent, QMake::ProjectAST* scope,
              QMakeDefaultOpts* defaultopts, TrollProjectPart* part )
    : m_root( scope ), m_incast( 0 ), m_parent( parent ), m_num(num), m_isEnabled( true ),
        m_part(part), m_defaultopts(defaultopts), m_initFinished(false)
{
    init();
}

Scope::Scope( unsigned int num, Scope* parent, const QString& filename,
              TrollProjectPart* part, QMakeDefaultOpts* defaultopts, bool isEnabled )
    : m_root( 0 ), m_incast( 0 ), m_parent( parent ), m_num(num), m_isEnabled( isEnabled ),
    m_part(part), m_defaultopts(defaultopts), m_initFinished(false)
{
    if ( !loadFromFile( filename ) )
    {
        if( !QFileInfo( filename ).exists() && QFileInfo( QFileInfo( filename ).dirPath( true ) ).exists() )
        {
            m_root = new QMake::ProjectAST();
            m_root->setFileName( filename );
        }else
        {
            delete m_root;
            m_root = 0;
            m_isEnabled = false;
        }
    }
    if( m_root )
        m_part->dirWatch()->addFile(filename);
    init();
}

Scope::Scope( unsigned int num, Scope* parent, QMake::IncludeAST* incast, const QString& path,
              const QString& incfile, QMakeDefaultOpts* defaultopts, TrollProjectPart* part )
    : m_root( 0 ), m_incast( incast ), m_parent( parent ), m_num(num), m_isEnabled( true ),
    m_part(part), m_defaultopts(defaultopts), m_initFinished(false)
{
    QString absfilename;
    QString tmp;
    if( incfile.contains(")" ) )
        tmp = incfile.mid(0, incfile.find(")") );
    else
        tmp = incfile;
    if( QFileInfo(tmp).isRelative() )
    {
        absfilename = path + QString( QChar( QDir::separator() ) ) + tmp;
    }else
        absfilename = tmp;
    if ( !loadFromFile( absfilename ) )
    {
        if( !QFileInfo( absfilename ).exists() && QFileInfo( QFileInfo( absfilename ).dirPath( true ) ).exists() )
        {
            m_root = new QMake::ProjectAST();
            m_root->setFileName( absfilename );
        }else
        {
            delete m_root;
            m_root = 0;
            m_isEnabled = false;
        }
    }
    if( m_root )
        m_part->dirWatch()->addFile( m_root->fileName() );
    init();
}

bool Scope::loadFromFile( const QString& filename )
{
    if ( !QFileInfo(filename).exists() || QMake::Driver::parseFile( filename, &m_root ) != 0 )
    {
        kdDebug( 9024 ) << "Couldn't parse project: " << filename << endl;
        m_root = 0;
        return false;
    }
//     init();
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
    m_part->dirWatch()->stopScan();
    QFile file( filename );
    if ( file.open( IO_WriteOnly ) )
    {

        QTextStream out( &file );
        QString astbuffer;
        m_root->writeBack( astbuffer );
        out << astbuffer;
        file.close();
    }
#ifdef DEBUG
    Scope::PrintAST pa;
    pa.processProject(m_root);
#endif
    m_part->dirWatch()->startScan();
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
    if( !m_root || Scope::listsEqual(values, variableValuesForOp(variable, "+=") ) )
        return;

    updateVariable( variable, "+=", variableValuesForOp( variable, "+=" ), true );
    updateVariable( variable, "+=", values, false );
}

void Scope::setEqualOp( const QString& variable, const QStringList& values )
{
    if( !m_root || Scope::listsEqual(values, variableValuesForOp(variable, "=") ) )
        return;

    updateVariable( variable, "=", variableValuesForOp( variable, "=" ), true );
    updateVariable( variable, "=", values, false );
}

void Scope::setMinusOp( const QString& variable, const QStringList& values )
{
    if( !m_root || Scope::listsEqual(values, variableValuesForOp(variable, "-=") ) )
        return;

    updateVariable( variable, "-=", variableValuesForOp( variable, "-=" ), true );
    updateVariable( variable, "-=", values, false );
}

QStringList Scope::variableValuesForOp( const QString& variable , const QString& op ) const
{
    QStringList result;

    if( !m_root )
        return result;

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

QStringList Scope::variableValues( const QString& variable, bool checkIncParent )
{
    QStringList result;

    if ( !m_root )
        return result;

    if( m_varCache.contains( variable ) && ( checkIncParent || scopeType() != Scope::IncludeScope ) )
    {
        return m_varCache[variable];
    }

    calcValuesFromStatements( variable, result, checkIncParent );
    result.remove( "\\\n" );
    result.remove( "\n" );
    result = Scope::removeWhiteSpace(result);
    if( scopeType() != Scope::IncludeScope || checkIncParent )
    {
        m_varCache[ variable ] = result;
    }
    return result;
}

void Scope::calcValuesFromStatements( const QString& variable, QStringList& result, bool checkIncParent, QMake::AST* stopHere ) const
{
    if( !m_root )
        return;

    /* For variables that we don't know and which are not QT/CONFIG find the default value */
    if( m_defaultopts
        && m_defaultopts->variables().contains(variable) > 0
        && ( variable == "TEMPLATE" || variable == "QT" || !KnownVariables.contains(variable) || variable == "CONFIG" ) )
    {
        result = m_defaultopts->variableValues(variable);
    }

    if ( scopeType() == FunctionScope || scopeType() == SimpleScope )
    {
        m_parent->calcValuesFromStatements( variable, result , this->m_root );
    }
    else if ( scopeType() == IncludeScope && checkIncParent )
    {
        m_parent->calcValuesFromStatements( variable, result , this->m_incast );
    }

    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( stopHere && *it == stopHere )
            return ;
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
    return ;
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
        return "";
    if ( m_incast )
        return "include<" + m_incast->projectName + ">";
    else if ( m_root->isFunctionScope() )
        return funcScopeKey( m_root );
    else if ( m_root->isScope() )
        return m_root->scopedID;
    else if ( m_root->isProject() )
    {
        if( m_parent && QDir::cleanDirPath( m_parent->projectDir() ) != QDir::cleanDirPath( projectDir() ) )
        {
            return getRelativePath( m_parent->projectDir(), projectDir() );
        }else if ( m_parent && QDir::cleanDirPath( m_parent->projectDir() ) == QDir::cleanDirPath( projectDir() ) )
        {
            return fileName();
        }else
            return QFileInfo( projectDir() ).fileName() ;
    }
    return QString();
}

QString Scope::fileName() const
{
    if( !m_root )
        return "";
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
    Scope* funcScope = new Scope( getNextScopeNum(), this, ast, m_defaultopts, m_part );
    if( funcScope->scopeType() != Scope::InvalidScope )
    {
        m_scopes.insert( getNextScopeNum(), funcScope );
        return funcScope;
    }else
        delete funcScope;
    return 0;
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
    if ( m_part->isQt4Project() )
        addToPlusOp( "CONFIG", QStringList( scopename ) );
    Scope* simpleScope = new Scope( getNextScopeNum(), this, ast, m_defaultopts, m_part );

    if( simpleScope->scopeType() != Scope::InvalidScope )
    {
        m_scopes.insert( getNextScopeNum(), simpleScope );
        return simpleScope;
    }else
        delete simpleScope;
    return 0;

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
    if( funcScope == 0 )
        return 0;

    QMake::IncludeAST* ast = new QMake::IncludeAST();
    ast->setDepth( m_root->depth() );
    ast->projectName = includeFile;
    Scope* incScope = new Scope( funcScope->getNextScopeNum(), funcScope, ast, projectDir(), resolveVariables( ast->projectName ), m_defaultopts, m_part );
    if ( incScope->scopeType() != InvalidScope )
    {
        funcScope->m_root->addChildAST( ast );
        funcScope->m_scopes.insert( funcScope->getNextScopeNum(), incScope );
        return funcScope;
    }
    else
    {
        deleteFunctionScope( m_scopes.keys().last() );
        delete incScope;
    }
    return 0;

}

Scope* Scope::createSubProject( const QString& projname )
{
    if( !m_root )
        return 0;

    if( variableValuesForOp( "SUBDIRS", "-=").contains( projname ) )
        removeFromMinusOp( "SUBDIRS", projname );

    QString realprojname = resolveVariables(projname);

    if( variableValuesForOp( "SUBDIRS", "-=").contains( realprojname ) )
        removeFromMinusOp( "SUBDIRS", realprojname );

    QDir curdir( projectDir() );

    if ( variableValues("TEMPLATE").contains( "subdirs" ) )
    {
        QString filename;
        if( !realprojname.endsWith(".pro") )
        {
            if ( !curdir.exists( realprojname ) )
                if ( !curdir.mkdir( realprojname ) )
                    return 0;
            curdir.cd( realprojname );
            QStringList entries = curdir.entryList("*.pro", QDir::Files);

            if ( !entries.isEmpty() && !entries.contains( curdir.dirName()+".pro" ) )
                filename = curdir.absPath() + QString(QChar(QDir::separator()))+entries.first();
            else
                filename = curdir.absPath() + QString(QChar(QDir::separator()))+curdir.dirName()+".pro";
        }else
            filename = curdir.absPath() + QString(QChar(QDir::separator())) + realprojname;

        kdDebug( 9024 ) << "Creating subproject with filename:" << filename << endl;

        Scope* s = new Scope( getNextScopeNum(), this, filename, m_part, m_defaultopts );
        if ( s->scopeType() != InvalidScope )
        {
            if( s->variableValues("TEMPLATE").isEmpty() )
                s->setEqualOp("TEMPLATE", QStringList("app"));
            s->saveToFile();
            addToPlusOp( "SUBDIRS", QStringList( realprojname ) );
            m_scopes.insert( getNextScopeNum(), s );
            return s;
        } else
        {
            delete s;
        }
    }

    return 0;
}

bool Scope::deleteFunctionScope( unsigned int num )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    Scope* funcScope = m_scopes[ num ];
    if ( funcScope )
    {
        QMake::AST* ast = m_root->m_children[ m_root->m_children.findIndex( funcScope->m_root ) ];
        if( !ast )
            return false;
        m_scopes.remove( num );
        m_root->removeChildAST( funcScope->m_root );
        delete funcScope;
        delete ast;
        return true;
    }
    return false;
}

bool Scope::deleteSimpleScope( unsigned int num )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    Scope* simpleScope = m_scopes[ num ];
    if ( simpleScope )
    {
        QMake::AST* ast = m_root->m_children[ m_root->m_children.findIndex( simpleScope->m_root ) ];
        if( !ast )
            return false;
        m_scopes.remove( num );
        removeFromPlusOp( "CONFIG", simpleScope->m_root->scopedID );
        m_root->removeChildAST( simpleScope->m_root );
        delete simpleScope;
        delete ast;
        return true;
    }
    return false;
}

bool Scope::deleteIncludeScope( unsigned int num )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    Scope * incScope = m_scopes[ num ];
    if( !incScope )
        return false;
    QMake::AST* ast = incScope->m_incast;
    if( !ast )
        return false;
    m_scopes.remove( num );
    m_root->removeChildAST( incScope->m_incast);
    delete incScope;
    delete ast;

    return m_parent->deleteFunctionScope( getNum() );
}

bool Scope::deleteSubProject( unsigned int num, bool deleteSubdir )
{
    if ( !m_root || !m_scopes.contains( num ) )
        return false;

    QValueList<QMake::AST*>::iterator it = findExistingVariable( "TEMPLATE" );
    if ( it != m_root->m_children.end() )
    {
        QMake::AssignmentAST * tempast = static_cast<QMake::AssignmentAST*>( *it );
        if ( tempast->values.contains( "subdirs" ) )
        {
            Scope* project = m_scopes[ num ];
            if( !project )
                return false;
            QString projdir = project->scopeName();
            if ( deleteSubdir )
            {
                QDir projdir = QDir( projectDir() );
                QString dir = project->scopeName();
                if( !dir.endsWith(".pro") )
                {
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
            }
            QValueList<QMake::AST*>::iterator foundit = findExistingVariable( "SUBDIRS" );
            if ( foundit != m_root->m_children.end() )
            {
                QMake::AssignmentAST * ast = static_cast<QMake::AssignmentAST*>( *foundit );
                updateValues( ast->values, QStringList( projdir ), true, ast->indent );
                if( m_varCache.contains( "SUBDIRS" ) )
                    m_varCache.erase( "SUBDIRS" );
            }else
                return false;
            m_scopes.remove( num );
            delete project;
            return true;
        }
    }
    return false;
}

void Scope::updateValues( QStringList& origValues, const QStringList& newValues, bool remove, QString indent )
{
    if( !m_root )
        return;

    for ( QStringList::const_iterator it = newValues.begin(); it != newValues.end() ; ++it )
    {
        if ( !origValues.contains( *it ) && !remove )
        {
            while ( !origValues.isEmpty() && origValues.last() == "\n" )
                origValues.pop_back();
            if ( origValues.count() > 0 && origValues.last() != "\\\n" )
            {
                origValues.append( " " );
                origValues.append( "\\\n" );
                if( indent != "" )
                    origValues.append( indent );
            }else if ( !origValues.isEmpty() && origValues.last() == "\\\n" )
            {
                if( indent != "" )
                    origValues.append( indent );
            }else if ( origValues.isEmpty() )
                origValues.append(" ");
            if( (*it).contains(" ") || (*it).contains("\t") || (*it).contains("\n") )
                origValues.append( "\""+*it+"\"" );
            else
                origValues.append( *it );
            origValues.append( "\n" );
        } else if ( origValues.contains( *it ) && remove )
        {
            QStringList::iterator posit = origValues.find( *it );
            posit = origValues.remove( posit );
            while( posit != origValues.end() && ( *posit == "\\\n" || (*posit).stripWhiteSpace() == "" ) )
            {
                posit = origValues.remove( posit );
            }
        }
    }
    while( !origValues.isEmpty() && (origValues.last() == "\\\n"
            || origValues.last() == "\n"
            || origValues.last().stripWhiteSpace() == "" ) && !origValues.isEmpty() )
        origValues.pop_back();
    origValues.append("\n");
}

void Scope::updateVariable( const QString& variable, const QString& op, const QStringList& values, bool removeFromOp )
{
    if ( !m_root || listIsEmpty( values ) )
        return ;

    if( m_varCache.contains( variable ) )
        m_varCache.erase( variable );

    for ( int i = m_root->m_children.count() - 1; i >= 0; --i )
    {
        if ( m_root->m_children[ i ] ->nodeType() == QMake::AST::AssignmentAST )
        {
            QMake::AssignmentAST * assignment = static_cast<QMake::AssignmentAST*>( m_root->m_children[ i ] );
            if ( assignment->scopedID == variable && Scope::isCompatible( assignment->op, op ) )
            {
                updateValues( assignment->values, values, removeFromOp, assignment->indent );
                if ( removeFromOp && listIsEmpty( assignment->values ) )
                {
                    m_root->removeChildAST( assignment );
                    delete assignment;
                }
                return ;
            }
            else if ( assignment->scopedID == variable && !Scope::isCompatible( assignment->op, op ) )
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
        updateValues( ast->values, values );
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

    kdDebug(9024) << "Initializing Scope: " << scopeName() << endl;
    m_maxCustomVarNum = 1;

    QValueList<QMake::AST*>::const_iterator it;
    for ( it = m_root->m_children.begin(); it != m_root->m_children.end(); ++it )
    {
        if ( ( *it ) ->nodeType() == QMake::AST::ProjectAST )
        {
            QMake::ProjectAST * p = static_cast<QMake::ProjectAST*>( *it );
            m_scopes.insert( getNextScopeNum(), new Scope( getNextScopeNum(), this, p, m_defaultopts, m_part ) );
        }
        else if ( ( *it ) ->nodeType() == QMake::AST::IncludeAST )
        {
            QMake::IncludeAST * i = static_cast<QMake::IncludeAST*>( *it );
            QString filename = i->projectName;
            if( i->projectName.startsWith("$") )
            {
                filename = resolveVariables(i->projectName, *it);
            }
            m_scopes.insert( getNextScopeNum(), new Scope( getNextScopeNum(), this, i, projectDir(), filename, m_defaultopts, m_part ) );
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
                    if ( str == "\\\n" || str == "\n" || str == "." || str == "./" || (str).stripWhiteSpace() == "" )
                        continue;
                    QDir subproject;
                    QString projectfile;
                    if( str.startsWith("$") )
                        str = resolveVariables(str, *it);
                    if( str.endsWith(".pro") )
                    {
                        subproject = QDir( projectDir(), "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                        projectfile = str;
                    }else
                    {
                        subproject = QDir( projectDir() + QString( QChar( QDir::separator() ) ) + str,
                                           "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                        if( !subproject.exists() )
                        {
                            kdDebug(9024) << "Project Dir doesn't exist, trying to find name.subdir variable:" << str <<  endl;
                            if( !variableValues(str+".subdir").isEmpty() )
                            {
                                kdDebug(9024) << "Found name.subdir variable for " << str << endl;
                                subproject = QDir( projectDir() + QString( QChar( QDir::separator() ) )
                                    + variableValues(str+".subdir").first(),
                                    "*.pro", QDir::Name | QDir::IgnoreCase, QDir::Files );
                            }else
                                continue;
                        }
                        if ( subproject.entryList().isEmpty() || subproject.entryList().contains( str + ".pro" ) )
                            projectfile = (str) + ".pro";
                        else
                            projectfile = subproject.entryList().first();

                    }
                    kdDebug( 9024 ) << "Parsing subproject: " << projectfile << endl;
                    m_scopes.insert( getNextScopeNum(),
                                     new Scope( getNextScopeNum(), this,
                                                subproject.absFilePath( projectfile ),
                                                m_part, m_defaultopts, ( m->op != "-=" )) );
                }
            }
            else
            {
                if ( !(
                         KnownVariables.contains( m->scopedID )
                         && ( m->op == "=" || m->op == "+=" || m->op == "-=")
                       )
                      && !(
                            ( m->scopedID.contains( ".files" ) || m->scopedID.contains( ".path" ) )
                            && variableValues("INSTALLS").contains(m->scopedID.left( m->scopedID.findRev(".") ) )
                          )
                      && !(
                            ( m->scopedID.contains( ".subdir" ) )
                            && variableValues("SUBDIRS").contains(m->scopedID.left( m->scopedID.findRev(".") ) )
                          )
                    )
                {
                    m_customVariables[ m_maxCustomVarNum++ ] = m;
                }
            }
        }
    }
    m_initFinished = true;
    emit initializationFinished();
}

QString Scope::projectName() const
{
    if( !m_root )
        return "";

    return QFileInfo( projectDir() ).fileName();
}

QString Scope::projectDir() const
{
    if( !m_root )
        return "";
    if ( m_root->isProject() )
    {
        return QFileInfo( m_root->fileName() ).dirPath( true );
    }
    else
    {
        return m_parent->projectDir();
    }
}

const QMap<unsigned int, QMap<QString, QString> > Scope::customVariables() const
{
    QMap<unsigned int, QMap<QString, QString> > result;
    if( !m_root )
        return result;

    QMap<unsigned int, QMake::AssignmentAST*>::const_iterator it = m_customVariables.begin();
    for ( ; it != m_customVariables.end(); ++it )
    {
        QMap<QString,QString> temp;
        temp[ "var" ] = it.data()->scopedID;
        temp[ "op" ] = it.data()->op;
        temp[ "values" ] = it.data()->values.join("");
        result[ it.key() ] = temp;
    }
    return result;
}

void Scope::updateCustomVariable( unsigned int id, const QString& name, const QString& newop, const QString& newvalues )
{
    if( !m_root )
        return;
    if ( id > 0 && m_customVariables.contains( id ) )
    {
        m_customVariables[ id ] ->values.clear();
        updateValues( m_customVariables[ id ] ->values, newvalues );
        if( m_varCache.contains( m_customVariables[ id ]->scopedID ) )
            m_varCache.erase( m_customVariables[ id ]->scopedID );
        m_customVariables[ id ] ->op = newop;
        m_customVariables[ id ] ->scopedID = name;
    }
}

unsigned int Scope::addCustomVariable( const QString& var, const QString& op, const QString& values )
{
    QMake::AssignmentAST* newast = new QMake::AssignmentAST();
    newast->scopedID = var;
    newast->op = op;
    newast->values.append(values);
    if( scopeType() == ProjectScope )
        newast->setDepth( m_root->depth() );
    else
        newast->setDepth( m_root->depth()+1 );
    m_root->addChildAST( newast );
    m_customVariables[ m_maxCustomVarNum++ ] = newast;
    return (m_maxCustomVarNum-1);
}

void Scope::removeCustomVariable( unsigned int id )
{
    if( m_customVariables.contains(id) )
    {
        QMake::AssignmentAST* m = m_customVariables[id];
        m_customVariables.remove(id);
        m_root->m_children.remove( m );
    }
}

bool Scope::isVariableReset( const QString& var )
{
    bool result = false;
    if( !m_root )
        return result;
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

bool Scope::isCompatible( const QString& op1, const QString& op2)
{
    if( op1 == "+=" )
        return ( op2 == "+=" || op2 == "=" );
    else if ( op1 == "-=" )
        return ( op2 == "-=" );
    else if ( op1 == "=" )
        return ( op2 == "=" || op2 == "+=" );
    return false;
}

bool Scope::listsEqual(const QStringList& l1, const QStringList& l2)
{
    QStringList left = l1;
    QStringList right = l2;
    left.sort();
    right.sort();
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

bool Scope::isQt4Project() const
{
    return m_part->isQt4Project();
}

void Scope::reloadProject()
{
    if ( !m_root || !m_root->isProject() )
        return;

    QString filename = m_root->fileName();
    QMap<unsigned int, Scope*>::iterator it;
    for ( it = m_scopes.begin() ; it != m_scopes.end() ; ++it )
    {
        Scope* s = it.data();
        delete s;
    }
    m_scopes.clear();

    m_customVariables.clear();

    if ( m_root->isProject() )
        delete m_root;
    if ( !loadFromFile( filename ) && !QFileInfo( filename ).exists() )
    {
        m_root = new QMake::ProjectAST();
        m_root->setFileName( filename );
    }
}

Scope* Scope::disableSubproject( const QString& dir)
{
    if( !m_root || ( m_root->isProject() && !m_incast ) )
        return 0;

    if( scopeType() != Scope::IncludeScope && variableValuesForOp( "SUBDIRS", "+=").contains( dir ) )
        removeFromPlusOp( "SUBDIRS", dir );
    else if( scopeType() != Scope::IncludeScope )
        removeFromPlusOp( "SUBDIRS", dir );

    QDir curdir( projectDir() );

    if ( variableValues("TEMPLATE").contains( "subdirs" ) )
    {
        curdir.cd(dir);
        QString filename;
        QStringList entries = curdir.entryList("*.pro", QDir::Files);

        if ( !entries.isEmpty() && !entries.contains( curdir.dirName()+".pro" ) )
            filename = curdir.absPath() + QString(QChar(QDir::separator()))+entries.first();
        else
            filename = curdir.absPath() + QString(QChar(QDir::separator()))+curdir.dirName()+".pro";

        kdDebug( 9024 ) << "Disabling subproject with filename:" << filename << endl;

        Scope* s = new Scope( getNextScopeNum(), this, filename, m_part, m_defaultopts, false );
        addToMinusOp( "SUBDIRS", QStringList( dir ) );
        m_scopes.insert( getNextScopeNum(), s );
        return s;
    }

    return 0;
}

QString Scope::resolveVariables( const QString& value, QMake::AST* stopHere ) const
{
    return resolveVariables(QStringList(value),  stopHere).front();
}

QStringList Scope::variableValues( const QString& variable, QMake::AST* stopHere ) const
{
    QStringList result;

    if ( !m_root )
        return result;

    calcValuesFromStatements( variable, result, true, stopHere );
    result.remove( "\\\n" );
    result.remove( "\n" );
    result = Scope::removeWhiteSpace(result);
    return result;
}

QStringList Scope::resolveVariables( const QStringList& values, QMake::AST* stopHere ) const
{
    QStringList result = values;
    QMap<QString, QStringList> variables;
    for( QStringList::iterator it = result.begin(); it != result.end(); ++it )
    {
        QRegExp re("\\$\\$([^\\) /]*)( |\\)|/)");
        int pos = 0;
        while( pos >= 0 )
        {
            pos = re.search( (*it), pos );
            if( pos > -1 )
            {
                if( !variables.contains( re.cap(1) ) )
                    variables[re.cap(1)] = resolveVariables( variableValues( re.cap(1), stopHere ) );
                pos += re.matchedLength();
            }
        }
        re = QRegExp("\\$\\$\\{([^\\)\\}]*)\\}");
        pos = 0;
        while( pos >= 0 )
        {
            pos = re.search( (*it), pos );
            if( pos > -1 )
            {
                if( !variables.contains( re.cap(1) ) )
                    variables[re.cap(1)] = resolveVariables( variableValues( re.cap(1), stopHere ) );
                pos += re.matchedLength();
            }
        }
        for( QMap<QString, QStringList>::const_iterator it2 = variables.begin(); it2 != variables.end(); ++it2 )
        {
            for( QStringList::const_iterator it3 = it2.data().begin(); it3 != it2.data().end(); ++it3 )
            {
                (*it).replace(QRegExp( "\\$\\$"+it2.key() ), *it3 );
                (*it).replace(QRegExp( "\\$\\{"+it2.key()+"\\}" ), *it3 );
            }
        }
    }
    return result;
}

void Scope::allFiles( const QString& projectDirectory, QStringList& res )
{
    QString myRelPath = getRelativePath( projectDirectory, projectDir() );
    if( !variableValues("TEMPLATE").contains("subdirs") )
    {
        QStringList values = variableValues( "INSTALLS" );
        QStringList::const_iterator it;
        for ( it = values.begin(); it != values.end(); ++it )
        {
            if ( ( *it ) == "target" )
                continue;

            QStringList files = variableValues( *it + ".files" );
            QStringList::iterator filesit = files.begin();
            for ( ;filesit != files.end(); ++filesit )
            {
                QString file = myRelPath + QString(QChar(QDir::separator())) + *filesit;
                if( !res.contains( file ) )
                    res.append( file );
            }
        }

        values = variableValues( "LEXSOURCES" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }

        values = variableValues( "YACCSOURCES" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }

        values = variableValues( "DISTFILES" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }

        if ( isQt4Project() )
        {
            values = variableValues( "RESOURCES" );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
                if( !res.contains( file ) )
                    res.append( file );
            }
        }
        else
        {
            values = variableValues( "IMAGES" );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
                if( !res.contains( file ) )
                    res.append( file );
            }
        }

        values = variableValues( "TRANSLATIONS" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }

        values = variableValues( "IDLS" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }

        if ( m_part->isTMakeProject() )
        {
            values = variableValues( "INTERFACES" );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
                if( !res.contains( file ) )
                    res.append( file );
            }
        }
        else
        {
            values = variableValues( "FORMS" );
            for ( it = values.begin(); it != values.end(); ++it )
            {
                QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
                if( !res.contains( file ) )
                    res.append( file );
            }
        }

        values = variableValues( "SOURCES" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }

        values = variableValues( "HEADERS" );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            QString file = myRelPath + QString(QChar(QDir::separator())) + *it;
            if( !res.contains( file ) )
                res.append( file );
        }
    }
    QMap<unsigned int, Scope*>::const_iterator it = m_scopes.begin();
    for( ; it != m_scopes.end(); ++it )
    {
        it.data()->allFiles( projectDirectory, res );
    }
}

QStringList Scope::allFiles( const QString& projectDir )
{
    QStringList result;
    if( !m_initFinished )
        return result;
    allFiles( projectDir, result );
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

#include "scope.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
