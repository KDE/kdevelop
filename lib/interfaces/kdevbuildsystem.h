/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KDEVBUILDSYSTEM_H_
#define _KDEVBUILDSYSTEM_H_

#include <qobject.h>

#include <kurl.h>

#include <qmap.h>
#include <qvariant.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <qdom.h>

#include "kdevproject.h"

#include "propertylist.h"
#include "multiproperty.h"

class BuildFileItem;
class BuildGroupItem;
class BuildTargetItem;
class KDialogBase;

class BuildBaseItem
{
public:
    enum
    {
	Group,
	Target,
	File,
	
	Custom=1000
    };

public:
    BuildBaseItem( int type, BuildBaseItem* parent=0 );
    virtual ~BuildBaseItem();

    virtual int type() const { return m_type; }
    virtual QString name() const { return m_name; }

    BuildBaseItem* parent() { return m_parent; }
    const BuildBaseItem* parent() const { return m_parent; }

    PropertyLib::PropertyList& attributes() { return m_attributes; }
    PropertyLib::PropertyList *pAttributes() { return &m_attributes; }

//    bool hasAttribute( const QString& name ) const { return m_attributes.contains( name ); }
    QVariant attribute( const QString& name ) { return m_attributes[name]->value(); }
    void setAttribute( const QString& name, const QVariant& value ) { m_attributes[name]->setValue(value); }

    virtual QString path();

protected:
    QString m_name;

private:
    int m_type;
    BuildBaseItem* m_parent;
    PropertyLib::PropertyList m_attributes;

private:
    BuildBaseItem( const BuildBaseItem& source );
    void operator = ( const BuildBaseItem& source );
};

class BuildGroupItem: public BuildBaseItem
{
public:
    BuildGroupItem( const QString& name, BuildGroupItem* parent=0 );
    virtual ~BuildGroupItem();

    BuildGroupItem* parentGroup() { return m_parentGroup; }
    const BuildGroupItem* parentGroup() const { return m_parentGroup; }

    void insertGroup( BuildGroupItem* group );
    void removeGroup( BuildGroupItem* group );
    BuildGroupItem* takeGroup( BuildGroupItem* group );
    BuildGroupItem* groupByname( const QString& groupName );
    QValueList<BuildGroupItem*> groups() { return m_subGroups; }

    void insertTarget( BuildTargetItem* target );
    void removeTarget( BuildTargetItem* target );
    BuildTargetItem* takeTarget( BuildTargetItem* target );
    BuildTargetItem* targetByName( const QString& targetName );
    QValueList<BuildTargetItem*> targets() { return m_targets; }

private:
    BuildGroupItem* m_parentGroup;
    QValueList<BuildGroupItem*> m_subGroups;
    QValueList<BuildTargetItem*> m_targets;

private:
    BuildGroupItem( const BuildGroupItem& source );
    void operator = ( const BuildGroupItem& source );
};

class BuildTargetItem: public BuildBaseItem
{
public:
    BuildTargetItem( const QString& name, BuildGroupItem* parent );
    virtual ~BuildTargetItem();

    BuildGroupItem* parentGroup() { return m_parentGroup; }
    const BuildGroupItem* parentGroup() const { return m_parentGroup; }

    void insertFile( BuildFileItem* file );
    void removeFile( BuildFileItem* file );
    BuildFileItem* takeFile( BuildFileItem* file );
    BuildFileItem* fileByName( const QString& fileName );
    QValueList<BuildFileItem*> files() { return m_files; }

private:
    BuildGroupItem* m_parentGroup;
    QValueList<BuildFileItem*> m_files;

private:
    BuildTargetItem( const BuildTargetItem& source );
    void operator = ( const BuildTargetItem& source );
};

class BuildFileItem: public BuildBaseItem
{
public:
    BuildFileItem( const KURL& url, BuildTargetItem* parentTarget );
    virtual ~BuildFileItem();

    BuildTargetItem* parentTarget() { return m_parentTarget; }
    const BuildTargetItem* parentTarget() const { return m_parentTarget; }

    const KURL& url() const { return m_url; }
    virtual QString name() const { return m_url.fileName(); }

private:
    KURL m_url;
    BuildTargetItem* m_parentTarget;

private:
    BuildFileItem( const BuildFileItem& source );
    void operator = ( const BuildFileItem& source );
};

class BuildItemConfigWidget: public QWidget{
    Q_OBJECT
public:
    BuildItemConfigWidget(BuildBaseItem *it, QWidget *parent = 0, const char *name = 0);
public slots:
    virtual void accept();
};

class ProjectConfigTab: public QWidget{
public:
    ProjectConfigTab(QWidget *parent = 0, const char *name = 0);
    virtual QDomDocument configuration() = 0;
};

/**
    Interface to a build system plugin for KDevelop Generic Project Manager.
    Generic project manager is a project manager that offers an unique interface
    to various build systems like make, ant, cook, shell scripts, etc.
    KDevBuildSystem is an interface for the generic build operations that are
    necessary for a build system plugin to work with generic manager.
*/
class KDevBuildSystem : public QObject
{
    Q_OBJECT

public:
    KDevBuildSystem( QObject *parent=0, const char *name=0 );
    virtual ~KDevBuildSystem();

    virtual void initProject(KDevProject *project);
    virtual KDevProject *project();

    /**the name of the build system for using in project configuration files*/
    virtual QString buildSystemName() = 0;
    /**reimplement this to create your build system like makefiles ant's xml
       files or shell scripts to build the currently selected project item
       (like target or group)*/
    virtual void createBuildSystem(BuildBaseItem*) = 0;

    /**implement this to allow building of files, targets and groups*/
    virtual void build(BuildBaseItem* it = 0) = 0;

    /**implement this to allow installing of targets and groups*/
    virtual void install(BuildBaseItem* it = 0) = 0;

    /**implement this to allow executing of targets and groups*/
    virtual void execute(BuildBaseItem* it = 0) = 0;

    /**implement this to allow cleaning of targets and groups*/
    virtual void clean(BuildBaseItem* it = 0) = 0;

    /**reimplement to show configure build item dialog*/
    virtual void configureBuildItem(KDialogBase *, BuildBaseItem*);
    /**reimplement to include project configuration widgets
       that will be included in Project|Project Options|Configure Options
       all those widgets will go to the tab bar below the configuration
       profiles widget*/
    virtual QValueList<ProjectConfigTab *> confWidgets() = 0;

    /**this provides a simple propery editor for the given build item
       if the build system don't want to provide its own configuration
       widget (there is no need to reimplement this)*/
    //@todo adymo: implement
    virtual void addDefaultBuildWidget(KDialogBase *dlg, QWidget *parent, BuildBaseItem*);

protected:
    virtual void updateDefaultBuildWidget();

private:
    KDevProject *m_project;
};


#endif
