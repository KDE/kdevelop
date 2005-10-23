/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2005 by Sascha Cunz                                     *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVAPPTEMPLATE_H_
#define _KDEVAPPTEMPLATE_H_

#include <QString>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QIcon>

#include <kapplication.h>

#include "kdevitemmodel.h"

class KArchiveDirectory;
class KConfig;

class AppWizardPart;

class KDevAppTemplate;
class KDevAppGroup;

class KDevAppItem : public KDevItemCollection
{
public:
    KDevAppItem( const QString &name, KDevItemGroup *parent = 0 );
    virtual ~KDevAppItem();

    virtual KDevAppGroup *groupItem() const { return 0; }
    virtual KDevAppTemplate *templateItem() const { return 0; }
};

class KDevAppGroup : public KDevAppItem
{
public:
    KDevAppGroup(const QString &name, const QString path, KDevItemGroup *parent = 0);

    QIcon icon() const{ return m_icon; }
    QString path() const { return m_path; }
    virtual KDevAppGroup* groupItem() const { return const_cast<KDevAppGroup*>(this); }

private:
    QString m_path;
    QIcon m_icon;
};

class KDevAppTemplate : public KDevAppItem
{
public:
    struct Archive;
    struct File;
    struct Dir;

public:
    KDevAppTemplate( KConfig* config, const QString& rootDir, KDevAppGroup* parent );
    virtual ~KDevAppTemplate();

    QString name() const{ return m_name; }
    QIcon icon() const{ return m_icon; }
    QString whatsThis() const{ return m_comment; }

    virtual KDevAppTemplate* templateItem()const{ return const_cast<KDevAppTemplate*>(this); }

    QString basePath() const{ return m_basePath; }
    QStringList openFilesAfterGeneration(){ return m_openFilesAfterGeneration; }
    QString fileTemplates(){ return m_fileTemplates; }
    QString iconName(){ return m_iconName; }

    void delayedLoadDetails();

    void addDir( Dir& dir );
    void addFile( File file );
    void addToSubMap( const QString& key, const QString& value ){ m_subMap.insert( key, value ); }
    void removeFromSubMap( const QString& key ){ m_subMap.remove( key ); }
    void expandLists();
    void setSubMapXML();

    bool execFinishCommand( AppWizardPart* part );
    bool unpackTemplateArchive();
    bool installProject( QWidget* parentWidget );

    const QHash<QString,QString>& subMap(){ return m_subMap; }
    const QHash<QString,QString>& subMapXML(){ return m_subMapXML; }
    QList<File> files(){ return m_fileList; }

private:
    KConfig* m_config;
    bool m_haveLoadedDetails;
    QString m_basePath;
    QString m_name;
    QString m_iconName;
    QString m_comment;
    QString m_fileTemplates;
    QString m_sourceArchive;
    QString m_customUI;
    QString m_message;
    QString m_finishCmd;
    QString m_finishCmdDir;
    QStringList m_openFilesAfterGeneration;
    QIcon m_icon;
    QHash<QString,QString> m_subMap;
    QHash<QString,QString> m_subMapXML;
public:
    QList<File> m_fileList;
    QList<Archive> m_archList;
    QList<Dir> m_dirList;
};

class KDevAppTemplateModel : public KDevItemModel
{
public:
    KDevAppTemplateModel(QObject *parent = 0);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    KDevAppGroup* getCategory( const QString& path );

private:
    QIcon folderIcon;
};

struct KDevAppTemplate::File
{
    QString source;
    QString dest;
    QString option;
    bool process;
    bool isXML;

    void setPermissions() const;
    bool copy(KDevAppTemplate*);
    void expand( QHash<QString, QString> hash );
};

struct KDevAppTemplate::Archive
{
    QString source;
    QString dest;
    QString option;
    bool process;

    void expand( QHash<QString, QString> hash );
    void unpack( const KArchiveDirectory *dir );
};

struct KDevAppTemplate::Dir
{
    Dir(){}
    Dir( QString aDir ) : dir( aDir ), perms( 0644 ){}
    void expand( QHash<QString, QString> hash );

    QString dir;
    QString option;
    int perms;
};

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
