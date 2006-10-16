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

#ifndef _QMAKESCOPEITEM_H_
#define _QMAKESCOPEITEM_H_

#include <qlistview.h>
#include <qstring.h>

class Scope;
class QMakeScopeItem;
class FileItem;
class TrollProjectWidget;

/**
 * Base class for all items appearing in ProjectOverview and ProjectDetails.
 */
class qProjectItem : public QListViewItem
{
public:
    enum Type { Subproject, Group, File };

    qProjectItem( Type type, QListView *parent, const QString &text );
    qProjectItem( Type type, qProjectItem *parent, const QString &text );

    QString scopeString;
    Type type()
    { return typ; }

private:
    Type typ;
    void init();

};


class GroupItem : public qProjectItem
{
public:
    enum GroupType {NoType, Sources, Headers, Forms, Distfiles, Images, Resources, Lexsources, Yaccsources, Translations, IDLs, InstallRoot, InstallObject, MaxTypeEnum };

    static GroupType groupTypeForExtension( const QString &ext );
    static void groupTypeMeanings( GroupItem::GroupType type, QString& title, QString& ext );

    GroupItem( QListView *lv, GroupType type, const QString &text, QMakeScopeItem* spitem );

    void removeFileFromScope( const QString& filename);
    void addFileToScope( const QString& filename);
    void addInstallObject( const QString& objectname);

    // qmake INSTALLS support
    QPtrList<GroupItem> installs;
    QPtrList<FileItem> files;

//     QStringList str_files;
//     QStringList str_files_exclude;
    // end qmake INSTALLS support
    GroupType groupType;
    QMakeScopeItem* owner;

protected:
    void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );
};


// Not sure if this complexity is really necessary...
class FileItem : public qProjectItem
{
public:
    FileItem( QListView *lv, const QString &text );

    QString uiFileLink;
};

/**
 * Stores one Scope
 */
class QMakeScopeItem : public qProjectItem
{
public:
    QMakeScopeItem( QListView *parent, const QString &text, Scope *s, TrollProjectWidget* widget );
    QMakeScopeItem( QMakeScopeItem *parent, const QString &text, Scope* );
    void updateValues( const QString& var, const QStringList& values );
    void addValue( const QString& var, const QString& value );
    void removeValue( const QString& var, const QString& value );
    void addValues( const QString& var, const QStringList& values );
    void removeValues( const QString& var, const QStringList& values );
    ~QMakeScopeItem();

    QMap<GroupItem::GroupType, GroupItem*> groups;

    Scope* scope;
    QString relativePath();
    QString getLibAddPath( QString downDirs );
    QString getLibAddObject( QString downDirs );
    QString getSharedLibAddObject( QString downDirs );
    QString getApplicationObject( QString downDirs );
    QString getIncAddPath( QString downDirs );
    FileItem* createFileItem(const QString& file);
    GroupItem* createGroupItem(GroupItem::GroupType type, const QString& name, QMakeScopeItem* scopeitem);

    QMakeScopeItem* projectFileItem();

    TrollProjectWidget* m_widget;
private:
    void init();
    void buildSubTree();
    void buildGroups();
};


#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
