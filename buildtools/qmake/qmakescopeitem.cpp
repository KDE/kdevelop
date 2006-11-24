/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   Part of this file is taken from Qt Designer.                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qmakescopeitem.h"

#include <qdir.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdirwatch.h>

#include "scope.h"
#include "pathutil.h"
#include "trollprojectwidget.h"
/*
 * Class qProjectItem
 */

qProjectItem::qProjectItem( Type type, QListView *parent, const QString &text )
        : QListViewItem( parent, text ), typ( type )
{}


qProjectItem::qProjectItem( Type type, qProjectItem *parent, const QString &text )
        : QListViewItem( parent, text ), typ( type )
{}



/*
 * Class GroupItem
 */

GroupItem::GroupItem( QListView *lv, GroupType type, const QString &text, QMakeScopeItem* spitem )
        : qProjectItem( Group, lv, text )
{
    this->owner = spitem;
    groupType = type;
//     files.setAutoDelete( true );
    setPixmap( 0, SmallIcon( "tar" ) );
}

GroupItem::GroupType GroupItem::groupTypeForExtension( const QString &ext )
{
    if ( ext == "cpp" || ext == "cc" || ext == "c" || ext == "C" || ext == "c++" || ext == "cxx" || ext == "ocl" )
        return Sources;
    else if ( ext == "hpp" || ext == "h" || ext == "hxx" || ext == "hh" || ext == "h++" || ext == "H" )
        return Headers;
    else if ( ext == "ui" )
        return Forms;
    else if ( ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "xpm" || ext == "gif" || ext == "bmp" )
        return Images;
    else if ( ext == "idl" )
        return IDLs;
    else if ( ext == "l" || ext == "ll" || ext == "lxx" || ext == "l++" )
        return Lexsources;
    else if ( ext == "y" || ext == "yy" || ext == "yxx" || ext == "y++" )
        return Yaccsources;
    else if ( ext == "ts" )
        return Translations;
    else if ( ext == "qrc" )
        return Resources;
    else
        return NoType;
}

void GroupItem::groupTypeMeanings( GroupItem::GroupType type, QString& title, QString& ext )
{
    switch ( type )
    {
        case GroupItem::Sources:
            title = i18n( "Sources" );
            ext = "*.cpp *.c";
            break;
        case GroupItem::Headers:
            title = i18n( "Headers" );
            ext = "*.h *.hpp";
            break;
        case GroupItem::Forms:
            title = i18n( "Forms" );
            ext = "*.ui";
            break;
        case GroupItem::IDLs:
            title = i18n( "Corba IDLs" );
            ext = "*.idl *.kidl";
            break;
        case GroupItem::Lexsources:
            title = i18n( "Lexsources" );
            ext = "*.l *.ll *.lxx *.l++";
            break;
        case GroupItem::Yaccsources:
            title = i18n( "Yaccsources" );
            ext = "*.y *.yy *.yxx *.y++";
            break;
        case GroupItem::Images:
            title = i18n( "Images" );
            ext = "*.jpg *.jpeg *.png *.xpm *.gif *.bmp";
            break;
        case GroupItem::Resources:
            title = i18n( "Resources" );
            ext = "*.qrc";
            break;
        case GroupItem::Distfiles:
            title = i18n( "Distfiles" );
            ext = "*";
            break;
        case GroupItem::Translations:
            title = i18n( "Translations" );
            ext = "*.ts";
            break;
        case GroupItem::InstallRoot:
            title = i18n( "Installs" );
            ext = "*";
            break;
        case GroupItem::InstallObject:
            title = i18n( "Install object" );
            ext = "*";
            break;

        default:        // just give back source files, et all
            title = i18n( "Source Files" );
            ext = "*.cpp *.cc *.ocl *.c *.hpp *.h *.ui";
    }
}

void GroupItem::paintCell( QPainter* p, const QColorGroup& c, int column, int width, int align )
{
    QColorGroup cg( c );
    if ( !firstChild() )
    {
        cg.setColor( QColorGroup::Text, cg.mid() );
    }

    qProjectItem::paintCell( p, cg, column, width, align );
}

void GroupItem::addFileToScope( const QString& filename )
{
    QPtrListIterator<FileItem> it( files );
    while ( it.current() != 0 )
    {
        if ( it.current() ->text( 0 ) == filename )      //File already exists in this subproject
            return ;
        ++it;
    }

    FileItem *fitem = owner->createFileItem( filename );

    fitem->uiFileLink = owner->m_widget->getUiFileLink( owner->relativePath() + QString( QChar( QDir::separator() ) ), owner->scope->resolveVariables( filename ) );
    files.append( fitem );
    switch ( groupType )
    {
        case GroupItem::Sources:
            owner->addValue( "SOURCES", filename );
            break;
        case GroupItem::Headers:
            owner->addValue( "HEADERS", filename );
            break;
        case GroupItem::Forms:
            owner->addValue( "FORMS", filename );
            break;
        case GroupItem::IDLs:
            owner->addValue( "IDLS", filename );
            break;
        case GroupItem::Lexsources:
            owner->addValue( "LEXSOURCES", filename );
            break;
        case GroupItem::Yaccsources:
            owner->addValue( "YACCSOURCES", filename );
            break;
        case GroupItem::Images:
            owner->addValue( "IMAGES", filename );
            break;
        case GroupItem::Resources:
            owner->addValue( "RESOURCES", filename );
            break;
        case GroupItem::Distfiles:
            owner->addValue( "DISTFILES", filename );
            break;
        case GroupItem::Translations:
            owner->addValue( "TRANSLATIONS", filename );
            break;
        case GroupItem::InstallObject:
            owner->addValue( text( 0 ) + ".files", filename );
            break;
        default:
            break;
    }
    owner->scope->saveToFile();
}

void GroupItem::removeFileFromScope( const QString& filename )
{
    QPtrListIterator<FileItem> it( files );
    while ( it.current() != 0 )
    {
        if ( it.current() ->text( 0 ) == filename )      //File already exists in this subproject
        {
            FileItem * fitem = it.current();
            files.remove( it );
            delete fitem;
            break;
        }
        ++it;
    }

    if ( groupType == GroupItem::Sources )
    {
        owner->removeValue( "SOURCES", filename );
    }
    else if ( groupType == GroupItem::Headers )
    {
        owner->removeValue( "HEADERS", filename );
    }
    else if ( groupType == GroupItem::Forms )
    {
        owner->removeValue( "FORMS", filename );
    }
    else if ( groupType == GroupItem::Distfiles )
    {
        owner->removeValue( "DISTFILES", filename );
    }
    else if ( groupType == GroupItem::Images )
    {
        owner->removeValue( "IMAGES", filename );
    }
    else if ( groupType == GroupItem::Resources )
    {
        owner->removeValue( "RESOURCES", filename );
    }
    else if ( groupType == GroupItem::Lexsources )
    {
        owner->removeValue( "LEXSOURCES", filename );
    }
    else if ( groupType == GroupItem::Yaccsources )
    {
        owner->removeValue( "YACCSOURCES", filename );
    }
    else if ( groupType == GroupItem::Translations )
    {
        owner->removeValue( "TRANSLATIONS", filename );
    }
    else if ( groupType == GroupItem::IDLs )
    {
        owner->removeValue( "IDL", filename );
    }
    else if ( groupType == GroupItem::InstallObject )
    {
        owner->removeValue( text( 0 ) + ".files", filename );
    }
    owner->scope->saveToFile();
}

void GroupItem::addInstallObject( const QString& objectname )
{
    GroupItem * objitem = owner->createGroupItem( GroupItem::InstallObject, objectname, owner );
    owner->addValue( "INSTALLS", objectname );
    owner->scope->saveToFile();
    installs.append( objitem );
}

/*
 * Class FileItem
 */

FileItem::FileItem( QListView *lv, const QString &text )
        : qProjectItem( File, lv, text ), uiFileLink( "" )
{
    // if excluded is set the file is excluded in the subproject/project.
    // by default excluded is set to false, thus file is included
    //     excluded = exclude;
    setPixmap( 0, SmallIcon( "document" ) );
}


/*
 * Class QMakeScopeItem
 */

QMakeScopeItem::QMakeScopeItem( QListView *parent, const QString &text, Scope* s, TrollProjectWidget* widget )
        : qProjectItem( Subproject, parent, text ), scope( s ), m_widget( widget )
{
    //   configuration.m_template = QTMP_APPLICATION;
    init();
}


QMakeScopeItem::QMakeScopeItem( QMakeScopeItem *parent, const QString &text, Scope* s )
        : qProjectItem( Subproject, parent, text ), scope( s ), m_widget( parent->m_widget )
{
    init();
}

QMakeScopeItem::~QMakeScopeItem()
{
    QMap<GroupItem::GroupType, GroupItem*>::iterator it;
    for ( it = groups.begin() ; it != groups.end() ; ++it )
    {
        GroupItem* s = it.data();
        delete s;
    }
    groups.clear();

}

QString QMakeScopeItem::relativePath()
{
    if( !scope || !scope->parent() )
        return "";
    if( scope->scopeType() == Scope::ProjectScope )
        return getRelativePath( m_widget->projectDirectory(), scope->projectDir() );
    else
        return static_cast<QMakeScopeItem*>( parent() ) ->relativePath();
//     if( !scope->parent() )
//         return "";
//     else if ( !scope->parent()->parent() || scope->scopeType() != Scope::ProjectScope )
//         return scope->scopeName();
//     else if ( scope->scopeType() == Scope::ProjectScope )
//         return ( static_cast<QMakeScopeItem*>( parent() ) ->relativePath()
//                  + QString( QChar( QDir::separator() ) ) + scope->scopeName() );
//     else
//         return (  static_cast<QMakeScopeItem*>( parent() ) ->relativePath() );
}

QString QMakeScopeItem::getSharedLibAddObject( QString basePath )
{
    if ( scope->variableValues( "CONFIG" ).findIndex( "dll" ) != -1 )
    {
        QString tmpPath = getRelativePath(basePath, scope->projectDir() );
        if ( !scope->variableValues( "DESTDIR" ).front().isEmpty() )
        {
            if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
                tmpPath += QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
            else
                tmpPath = scope->variableValues( "DESTDIR" ).front();
        }
        else
        {
            tmpPath += QString( QChar( QDir::separator() ) );
        }

        tmpPath = QDir::cleanDirPath( tmpPath );

        QString libString;
        if ( !scope->variableValues( "TARGET" ).front().isEmpty() )
        {
            libString = tmpPath + QString( QChar( QDir::separator() ) ) + "lib" + scope->variableValues( "TARGET" ).front() + ".so";

        }
        else
        {
            libString = tmpPath + QString( QChar( QDir::separator() ) ) + "lib" + scope->projectName() + ".so";

        }
        return ( libString );
    }
    return "";
}

QString QMakeScopeItem::getApplicationObject( QString basePath )
{
    QString tmpPath = getRelativePath(basePath, scope->projectDir() );
    if ( !scope->variableValues( "DESTDIR" ).front().isEmpty() )
    {
        if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
            tmpPath += QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
        else
            tmpPath = scope->variableValues( "DESTDIR" ).front();
    }
    else
    {
        tmpPath += QString( QChar( QDir::separator() ) );
    }

    tmpPath = QDir::cleanDirPath( tmpPath );

    if ( scope->variableValues( "TARGET" ).front().isEmpty() )
        return tmpPath + QString( QChar( QDir::separator() ) ) + scope->projectName();
    else
        return tmpPath + QString( QChar( QDir::separator() ) ) + scope->variableValues( "TARGET" ).front();
}

QString QMakeScopeItem::getLibAddObject( QString basePath )
{
    if ( scope->variableValues( "CONFIG" ).findIndex( "dll" ) != -1 )
    {
        if ( !scope->variableValues( "TARGET" ).front().isEmpty() )
        {
            return ( "-l" + scope->variableValues( "TARGET" ).front() );
        }
        else
        {
            return ( "-l" + scope->projectName() );
        }
    }
    else if ( scope->variableValues( "CONFIG" ).findIndex( "staticlib" ) != -1
            || scope->variableValues("TEMPLATE").findIndex("lib") != -1 )
    {
        QString tmpPath = getRelativePath(basePath, scope->projectDir() );
        if ( !scope->variableValues( "DESTDIR" ).front().isEmpty() )
        {
            if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
                tmpPath += QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
            else
                tmpPath = scope->variableValues( "DESTDIR" ).front();
        }
        else
        {
            tmpPath += QString( QChar( QDir::separator() ) );
        }

        tmpPath = QDir::cleanDirPath( tmpPath );

        QString libString;
        if ( !scope->variableValues( "TARGET" ).front().isEmpty() )
        {
            libString = tmpPath + QString( QChar( QDir::separator() ) ) + "lib" + scope->variableValues( "TARGET" ).front() + ".a";

        }
        else
        {
            libString = tmpPath + QString( QChar( QDir::separator() ) ) + "lib" + scope->projectName() + ".a";

        }
        return ( libString );
    }

    return ( "" );
}
QString QMakeScopeItem::getLibAddPath( QString basePath )
{

    //PATH only add if shared lib
    if ( scope->variableValues( "CONFIG" ).findIndex( "dll" ) == -1 ) return ( "" );

    QString tmpPath = getRelativePath(basePath, scope->projectDir() );
    if ( !scope->variableValues( "DESTDIR" ).front().isEmpty() )
    {
        if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
            tmpPath += QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
        else
            tmpPath = scope->variableValues( "DESTDIR" ).front();
    }
    else
    {
        tmpPath += QString( QChar( QDir::separator() ) );
    }

    tmpPath = QDir::cleanDirPath( tmpPath );

    return ( tmpPath );

}

QString QMakeScopeItem::getIncAddPath( QString basePath )
{
    QString tmpPath = getRelativePath( basePath, scope->projectDir() );
    tmpPath = QDir::cleanDirPath( tmpPath );

    return ( tmpPath );
}

void QMakeScopeItem::buildSubTree()
{
    QValueList<Scope*>::const_iterator it;

    QValueList<Scope*> scopes = scope->scopesInOrder();

    for ( it = scopes.begin(); it != scopes.end(); ++it )
    {
        if( (*it)->scopeType() != Scope::InvalidScope )
            new QMakeScopeItem( this, ( *it )->scopeName(), ( *it ) );
        else
            kdDebug( 9024 ) << "No QMakeScopeItem created" << endl;
    }
    sortChildItems( 0, true );
}


void QMakeScopeItem::init()
{
    if ( scope->scopeType() == Scope::SimpleScope )
    {
        setPixmap( 0, SmallIcon( "qmake_scope" ) );
    }
    else if ( scope->scopeType() == Scope::FunctionScope )
    {
        setPixmap( 0, SmallIcon( "qmake_func_scope" ) );
    }
    else if ( scope->scopeType() == Scope::IncludeScope )
    {
        setPixmap( 0, SmallIcon( "qmake_inc_scope" ) );
    }
    else
    {
        QStringList tmp = scope->variableValues( "TEMPLATE" );
        if( scope->isEnabled() )
        {
            if ( tmp.findIndex( "subdirs" ) != -1 )
                setPixmap( 0, SmallIcon( "qmake_sub" ) );
            else if ( tmp.findIndex( "lib" ) != -1 )
                setPixmap( 0, SmallIcon( "qmake_lib" ) );
            else
                setPixmap( 0, SmallIcon( "qmake_app" ) );
        }else
        {
            if ( tmp.findIndex( "subdirs" ) != -1 )
                setPixmap( 0, SmallIcon( "qmake_sub_disabled" ) );
            else if ( tmp.findIndex( "lib" ) != -1 )
                setPixmap( 0, SmallIcon( "qmake_lib_disabled" ) );
            else
                setPixmap( 0, SmallIcon( "qmake_app_disabled" ) );
        }
    }

    setEnabled( scope->isEnabled() );
    if( scope->isEnabled() )
    {
        buildGroups();
        buildSubTree();
    }
}

GroupItem* QMakeScopeItem::createGroupItem( GroupItem::GroupType type, const QString& label, QMakeScopeItem* scopeitem )
{
    GroupItem * item = new GroupItem( scopeitem->listView(), type, label, scopeitem );
    scopeitem->listView() ->takeItem( item );
    return item;
}

FileItem* QMakeScopeItem::createFileItem( const QString& name )
{
    FileItem * fitem = new FileItem( listView(), name );
    listView() ->takeItem( fitem );
    return fitem;
}

void QMakeScopeItem::buildGroups()
{
    if( scope->variableValues("TEMPLATE").findIndex("subdirs") != -1 )
        return;
    QStringList values;

    GroupItem* item;
    QStringList::iterator it;

    values = scope->variableValues( "INSTALLS" );
    item = createGroupItem( GroupItem::InstallRoot, "INSTALLS", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        if ( ( *it ) == "target" )
            continue;

        QString path = scope->variableValues( *it + ".path" ).front();
        GroupItem* installitem = createGroupItem( GroupItem::InstallObject, *it, this );
        item->installs.append( installitem );
        QStringList files = scope -> variableValues( *it + ".files" );
        if ( !files.isEmpty() )
        {
            QStringList::iterator filesit = files.begin();
            for ( ;filesit != files.end(); ++filesit )
            {
                installitem->files.append( createFileItem( *filesit ) );
            }
        }
    }

    values = scope->variableValues( "LEXSOURCES" );
    item = createGroupItem( GroupItem::Lexsources, "LEXSOURCES", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        item->files.append( createFileItem( *it ) );
    }

    values = scope->variableValues( "YACCSOURCES" );
    item = createGroupItem( GroupItem::Yaccsources, "YACCSOURCES", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        item->files.append( createFileItem( *it ) );
    }

    values = scope->variableValues( "DISTFILES" );
    item = createGroupItem( GroupItem::Distfiles, "DISTFILES", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        item->files.append( createFileItem( *it ) );
    }

    if ( scope->isQt4Project() )
    {
        values = scope->variableValues( "RESOURCES" );
        item = createGroupItem( GroupItem::Resources, "RESOURCES", this );
        groups.insert( item->groupType, item );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            item->files.append( createFileItem( *it ) );
        }
    }
    else
    {
        values = scope->variableValues( "IMAGES" );
        item = createGroupItem( GroupItem::Images, "IMAGES", this );
        groups.insert( item->groupType, item );
        for ( it = values.begin(); it != values.end(); ++it )
        {
            item->files.append( createFileItem( *it ) );
        }
    }

    values = scope->variableValues( "TRANSLATIONS" );
    item = createGroupItem( GroupItem::Translations, "TRANSLATIONS", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        item->files.append( createFileItem( *it ) );
    }

    values = scope->variableValues( "IDLS" );
    item = createGroupItem( GroupItem::IDLs, "Corba IDL", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        item->files.append( createFileItem( *it ) );
    }

    if ( m_widget->isTMakeProject() )
    {
        values = scope->variableValues( "INTERFACES" );
        item = createGroupItem( GroupItem::Forms, "INTERFACES", this );
    }
    else
    {
        values = scope->variableValues( "FORMS" );
        item = createGroupItem( GroupItem::Forms, "FORMS", this );
    }
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        item->files.append( createFileItem( *it ) );
    }

    values = scope->variableValues( "SOURCES" );
    item = createGroupItem( GroupItem::Sources, "SOURCES", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        FileItem* fitem = createFileItem( *it );
        fitem->uiFileLink = m_widget->getUiFileLink( relativePath() + QString( QChar( QDir::separator() ) ), scope->resolveVariables( *it ) );
        item->files.append( fitem );
    }

    values = scope->variableValues( "HEADERS" );
    item = createGroupItem( GroupItem::Headers, "HEADERS", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        FileItem* fitem = createFileItem( *it );
        fitem->uiFileLink = m_widget->getUiFileLink( relativePath() + QString( QChar( QDir::separator() ) ), scope->resolveVariables( *it ) );
        item->files.append( fitem );
    }

}

void QMakeScopeItem::removeValues( const QString& var, const QStringList& values )
{
    for( QStringList::const_iterator it = values.begin() ; it != values.end(); ++it )
    {
        removeValue( var, *it );
    }
}

void QMakeScopeItem::addValues( const QString& var, const QStringList& values )
{
    for( QStringList::const_iterator it = values.begin() ; it != values.end(); ++it )
    {
        addValue( var, *it );
    }
}

void QMakeScopeItem::removeValue( const QString& var, const QString& value )
{
    if( scope->scopeType() != Scope::IncludeScope && scope->variableValues( var ).findIndex( value ) != -1 )
    {
        if( scope->variableValuesForOp( var, "+=" ).findIndex(value) != -1 )
            scope->removeFromPlusOp( var, QStringList( value ) );
        else
            scope->addToMinusOp( var, QStringList( value ) );
    }else if( scope->scopeType() == Scope::IncludeScope )
    {
        scope->addToMinusOp( var, QStringList( value ) );
    }
}

void QMakeScopeItem::addValue( const QString& var, const QString& value )
{
    if( scope->scopeType() != Scope::IncludeScope && scope->variableValues( var ).findIndex( value ) == -1 )
    {
        if( scope->variableValuesForOp( var, "-=" ).findIndex(value) != -1 )
            scope->removeFromMinusOp( var, QStringList( value ) );
        else
            scope->addToPlusOp( var, QStringList( value ) );
    }else if( scope->scopeType() == Scope::IncludeScope )
    {
        scope->addToPlusOp( var, QStringList( value ) );
    }
}

void QMakeScopeItem::updateValues( const QString& var, const QStringList& values )
{
    QStringList curValues = scope->variableValues( var, (scope->scopeType() != Scope::IncludeScope) );
    QStringList scopeValues = scope->variableValuesForOp( var, "+=" );
    for( QStringList::const_iterator it = curValues.begin(); it != curValues.end(); ++it )
    {
        if ( values.findIndex( *it ) == -1 )
        {
            if( scopeValues.findIndex( *it ) != -1 )
                scope->removeFromPlusOp( var, QStringList( *it ) );
            else
                scope->addToMinusOp( var, QStringList( *it ) );
        }
    }
    for( QStringList::const_iterator it = values.begin(); it != values.end(); ++it )
    {
        if ( curValues.findIndex( *it ) == -1 )
        {
            scope->addToPlusOp( var, QStringList( *it ) );
        }
    }
}

QMakeScopeItem* QMakeScopeItem::projectFileItem()
{
    if( scope->scopeType() != Scope::ProjectScope )
    {
        QMakeScopeItem* parentitem = dynamic_cast<QMakeScopeItem*>(parent());
        if( parentitem )
            return parentitem->projectFileItem();
    }
    return this;
}

void QMakeScopeItem::reloadProject()
{
    kdDebug(9024) << "Reloading Project" << endl;
    QListViewItem* item = firstChild();
    while( item )
    {
        QListViewItem* olditem = item;
        item = olditem->nextSibling();
        delete olditem;
    }
    QMap<GroupItem::GroupType, GroupItem*>::iterator it;
    for ( it = groups.begin() ; it != groups.end() ; ++it )
    {
        GroupItem* s = it.data();
        QListView* l = s->listView();
        if(l)
            l->removeItem(s);
        delete s;
    }
    groups.clear();
    scope->reloadProject();
    init();
}

void QMakeScopeItem::disableSubprojects( const QStringList& dirs )
{
    QStringList::const_iterator it = dirs.begin();
    for( ; it != dirs.end() ; ++it)
    {
        if( scope->variableValues("SUBDIRS").findIndex(*it) != -1 )
        {
            Scope* s = scope->disableSubproject(*it);
            if( !s )
                return;
            else
            {
                QMakeScopeItem* newitem = new QMakeScopeItem( this, s->scopeName(), s );
                QListViewItem* lastitem = firstChild();
                while( lastitem && lastitem->nextSibling() )
                    lastitem = lastitem->nextSibling();
                if( lastitem )
                    newitem->moveItem(lastitem);
            }
        }
    }

}

int QMakeScopeItem::compare( QListViewItem* i, int , bool ) const
{
    QMakeScopeItem* other = dynamic_cast<QMakeScopeItem*>(i);
    if( !i )
        return -1;
    if( other->scope->getNum() < scope->getNum() )
        return 1;
    else if ( other->scope->getNum() > scope->getNum() )
        return -1;
    else
        return 0;
}

QMap<QString, QString> QMakeScopeItem::getLibInfos( QString basePath )
{

    QMap<QString, QString> result;
    if ( scope->variableValues( "TARGET" ).front().isEmpty() )
        result["shared_lib"] = "-l"+scope->projectName();
    else
        result["shared_lib"] = "-l"+scope->variableValues( "TARGET" ).front();

    QString tmpPath = getRelativePath(basePath, scope->projectDir() );
    if ( !scope->variableValues( "DESTDIR" ).front().isEmpty() )
    {
        if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
            tmpPath += QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
        else
            tmpPath = scope->variableValues( "DESTDIR" ).front();
    }
    else
    {
        tmpPath += QString( QChar( QDir::separator() ) );
    }

    tmpPath = QDir::cleanDirPath( tmpPath );

    result["shared_libdir"] = "-L"+tmpPath;

    if ( scope->variableValues( "TARGET" ).front().isEmpty() )
        result["shared_depend"] = tmpPath+QString(QChar(QDir::separator()))+"lib"+scope->projectName()+".so";
    else
        result["shared_depend"] = tmpPath+QString(QChar(QDir::separator()))+"lib"+scope->variableValues( "TARGET" ).front()+".so";


    if ( scope->variableValues( "TARGET" ).front().isEmpty() )
        result["static_lib"] = tmpPath+QString(QChar(QDir::separator()))+"lib"+scope->projectName()+".a";
    else
        result["static_lib"] = tmpPath+QString(QChar(QDir::separator()))+"lib"+scope->variableValues( "TARGET" ).front()+".a";

    result["static_depend"] = result["static_lib"];

    if ( scope->variableValues( "TARGET" ).front().isEmpty() )
        result["app_depend"] = tmpPath + QString( QChar( QDir::separator() ) ) + scope->projectName();
    else
        result["app_depend"] = tmpPath + QString( QChar( QDir::separator() ) ) + scope->variableValues( "TARGET" ).front();

    QString map;
    for( QMap<QString, QString>::const_iterator it = result.begin(); it != result.end(); ++it )
        map += "["+it.key() + "=>" +it.data() + "],";
    kdDebug(9024) << "Running getLibInfo for" << scope->projectName() << "|" << map << endl;
    return result;
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
