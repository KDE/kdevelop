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

    fitem->uiFileLink = owner->m_widget->getUiFileLink( owner->relativePath() + QString( QChar( QDir::separator() ) ), filename );
    files.append( fitem );
    switch ( groupType )
    {
        case GroupItem::Sources:
            owner->scope->addToPlusOp( "SOURCES", filename );
            break;
        case GroupItem::Headers:
            owner->scope->addToPlusOp( "HEADERS", filename );
            break;
        case GroupItem::Forms:
            owner->scope->addToPlusOp( "FORMS", filename );
            break;
        case GroupItem::IDLs:
            owner->scope->addToPlusOp( "IDLS", filename );
            break;
        case GroupItem::Lexsources:
            owner->scope->addToPlusOp( "LEXSOURCES", filename );
            break;
        case GroupItem::Yaccsources:
            owner->scope->addToPlusOp( "YACCSOURCES", filename );
            break;
        case GroupItem::Images:
            owner->scope->addToPlusOp( "IMAGES", filename );
            break;
        case GroupItem::Resources:
            owner->scope->addToPlusOp( "RESOURCES", filename );
            break;
        case GroupItem::Distfiles:
            owner->scope->addToPlusOp( "DISTFILES", filename );
            break;
        case GroupItem::Translations:
            owner->scope->addToPlusOp( "TRANSLATIONS", filename );
            break;
        case GroupItem::InstallObject:
            owner->scope->addToPlusOp( text( 0 ) + ".files", filename );
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
        owner->scope->addToMinusOp( "SOURCES", filename );
    }
    else if ( groupType == GroupItem::Headers )
    {
        owner->scope->addToMinusOp( "HEADERS", filename );
    }
    else if ( groupType == GroupItem::Forms )
    {
        owner->scope->addToMinusOp( "FORMS", filename );
    }
    else if ( groupType == GroupItem::Distfiles )
    {
        owner->scope->addToMinusOp( "DISTFILES", filename );
    }
    else if ( groupType == GroupItem::Images )
    {
        owner->scope->addToMinusOp( "IMAGES", filename );
    }
    else if ( groupType == GroupItem::Resources )
    {
        owner->scope->addToMinusOp( "RESOURCES", filename );
    }
    else if ( groupType == GroupItem::Lexsources )
    {
        owner->scope->addToMinusOp( "LEXSOURCES", filename );
    }
    else if ( groupType == GroupItem::Yaccsources )
    {
        owner->scope->addToMinusOp( "YACCSOURCES", filename );
    }
    else if ( groupType == GroupItem::Translations )
    {
        owner->scope->addToMinusOp( "TRANSLATIONS", filename );
    }
    else if ( groupType == GroupItem::IDLs )
    {
        owner->scope->addToMinusOp( "IDL", filename );
    }
    else if ( groupType == GroupItem::InstallObject )
    {
        owner->scope->addToMinusOp( text( 0 ) + ".files", filename );
    }
    owner->scope->saveToFile();
}

void GroupItem::addInstallObject( const QString& objectname )
{
    GroupItem * objitem = owner->createGroupItem( GroupItem::InstallObject, objectname, owner );
    owner->scope->addToPlusOp( "INSTALL", QStringList( objectname ) );
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
{}

QString QMakeScopeItem::relativePath()
{
    if ( !scope->parent() || !scope->parent() ->parent() || scope->scopeType() != Scope::ProjectScope )
        return ( QString( QChar( QDir::separator() ) ) + scope->scopeName() );
    else if ( scope->scopeType() == Scope::ProjectScope )
        return ( ( ( QMakeScopeItem* ) parent() ) ->relativePath() + QString( QChar( QDir::separator() ) ) + scope->scopeName() );
    else
        return ( ( ( QMakeScopeItem* ) parent() ) ->relativePath() );
}

QString QMakeScopeItem::getDownDirs()
{
    QMakeScopeItem * pItem = this;
    while ( pItem->parent() )
        pItem = ( QMakeScopeItem* ) pItem->parent();
    return getRelativePath( QDir::cleanDirPath( this->scope->projectDir() ), QDir::cleanDirPath( pItem->scope->projectDir() ) );
}

QString QMakeScopeItem::getSharedLibAddObject( QString downDirs )
{
    if ( scope->variableValues( "CONFIG" ).contains( "dll" ) )
    {
        QString tmpPath;
        if ( scope->variableValues( "DESTDIR" ).front() != "" )
        {
            if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
                tmpPath = downDirs + relativePath() + QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
            else
                tmpPath = scope->variableValues( "DESTDIR" ).front();
        }
        else
        {
            tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) );
        }

        tmpPath = QDir::cleanDirPath( tmpPath );

        QString libString;
        if ( scope->variableValues( "TARGET" ).front() != "" )
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

QString QMakeScopeItem::getApplicationObject( QString downDirs )
{
    QString tmpPath;
    if ( scope->variableValues( "DESTDIR" ).front() != "" )
    {
        if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
            tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
        else
            tmpPath = scope->variableValues( "DESTDIR" ).front();
    }
    else
    {
        tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) );
    }

    tmpPath = QDir::cleanDirPath( tmpPath );

    if ( scope->variableValues( "TARGET" ).front().isEmpty() )
        return tmpPath + QString( QChar( QDir::separator() ) ) + scope->projectName();
    else
        return tmpPath + QString( QChar( QDir::separator() ) ) + scope->variableValues( "TARGET" ).front();
}
QString QMakeScopeItem::getLibAddObject( QString downDirs )
{
    if ( scope->variableValues( "CONFIG" ).contains( "dll" ) )
    {
        if ( scope->variableValues( "TARGET" ).front() != "" )
        {
            return ( "-l" + scope->variableValues( "TARGET" ).front() );
        }
        else
        {
            return ( "-l" + scope->projectName() );
        }
    }
    else if ( scope->variableValues( "CONFIG" ).contains( "staticlib" ) )
    {
        QString tmpPath;
        if ( scope->variableValues( "DESTDIR" ).front() != "" )
        {
            if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
                tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
            else
                tmpPath = scope->variableValues( "DESTDIR" ).front();
        }
        else
        {
            tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) );
        }

        tmpPath = QDir::cleanDirPath( tmpPath );

        QString libString;
        if ( scope->variableValues( "TARGET" ).front() != "" )
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
QString QMakeScopeItem::getLibAddPath( QString downDirs )
{

    //PATH only add if shared lib
    if ( !( scope->variableValues( "CONFIG" ).contains( "dll" ) ) ) return ( "" );

    QString tmpPath;
    if ( scope->variableValues( "DESTDIR" ).front() != "" )
    {
        if ( QDir::isRelativePath( scope->variableValues( "DESTDIR" ).front() ) )
            tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) ) + scope->variableValues( "DESTDIR" ).front();
        else
            tmpPath = scope->variableValues( "DESTDIR" ).front();
    }
    else
    {
        tmpPath = downDirs + this->relativePath() + QString( QChar( QDir::separator() ) );
    }

    tmpPath = QDir::cleanDirPath( tmpPath );

    return ( tmpPath );

}

QString QMakeScopeItem::getIncAddPath( QString downDirs )
{
    QString tmpPath = downDirs + this->relativePath();
    tmpPath = QDir::cleanDirPath( tmpPath );

    return ( tmpPath );
}

void QMakeScopeItem::buildSubTree()
{
    QValueList<Scope*>::const_iterator it;

    QValueList<Scope*> scopes = scope->scopesInOrder();

    QMakeScopeItem* lastItem = 0;
    for ( it = scopes.begin(); it != scopes.end(); ++it )
    {
        Scope* s = ( *it );
        QMakeScopeItem* newitem = new QMakeScopeItem( this, s->scopeName(), s );
        if ( lastItem )
            newitem->moveItem( lastItem );
        lastItem = newitem;
    }
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
            if ( tmp.contains( "app" ) )
                setPixmap( 0, SmallIcon( "qmake_app" ) );
            else if ( tmp.contains( "lib" ) )
                setPixmap( 0, SmallIcon( "qmake_lib" ) );
            else
                setPixmap( 0, SmallIcon( "qmake_sub" ) );
        }else
        {
            if ( tmp.contains( "app" ) )
                setPixmap( 0, SmallIcon( "qmake_app_disabled" ) );
            else if ( tmp.contains( "lib" ) )
                setPixmap( 0, SmallIcon( "qmake_lib_disabled" ) );
            else
                setPixmap( 0, SmallIcon( "qmake_sub_disabled" ) );
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
        fitem->uiFileLink = m_widget->getUiFileLink( relativePath() + QString( QChar( QDir::separator() ) ), *it );
        item->files.append( fitem );
    }

    values = scope->variableValues( "HEADERS" );
    item = createGroupItem( GroupItem::Headers, "HEADERS", this );
    groups.insert( item->groupType, item );
    for ( it = values.begin(); it != values.end(); ++it )
    {
        FileItem* fitem = createFileItem( *it );
        fitem->uiFileLink = m_widget->getUiFileLink( relativePath() + QString( QChar( QDir::separator() ) ), *it );
        item->files.append( fitem );
    }

}

void QMakeScopeItem::updateVariableValues( const QString& var, QStringList values )
{
    QStringList curVal = scope->variableValues( var );
    for ( QStringList::iterator it = curVal.begin() ; it != curVal.end() ; ++it )
    {
        if ( !values.contains( *it ) )
        {
            scope->addToMinusOp( var, QStringList( *it ) );
            values.remove( *it );
        }
    }
    scope->addToPlusOp( var, curVal + values );
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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
