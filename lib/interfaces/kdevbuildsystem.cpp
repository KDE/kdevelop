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

#include <qvbox.h>
#include <qtable.h>
#include <qlayout.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kdialog.h>

#include "kdevbuildsystem.h"

#include "propertyeditor.h"

BuildBaseItem::BuildBaseItem( int type, BuildBaseItem * parent )
    : m_type( type ), m_parent( parent )
{
}

BuildBaseItem::~ BuildBaseItem( )
{
}

QString BuildBaseItem::path( )
{
    BuildBaseItem *it = this;
    QString p;
    while (it)
    {
        if (!p.isEmpty())
            p.prepend("/");
        p.prepend(it->name());
        it = it->parent();
    }
    return p;
}


// ------------------------------------------------------------------------
BuildGroupItem::BuildGroupItem( const QString & name, BuildGroupItem * parent )
    : BuildBaseItem( Group, parent ), m_parentGroup( parent )
{
    m_name = name;
    if( m_parentGroup )
	m_parentGroup->insertGroup( this );
}

BuildGroupItem::~ BuildGroupItem( )
{
    while( m_targets.size() ){
        BuildTargetItem* target = m_targets.front();
        m_targets.front();
	delete target;
    }

    while( m_subGroups.size() ){
        BuildGroupItem* group = m_subGroups.front();
	m_subGroups.pop_front();
	delete group;
    }

    if( m_parentGroup )
	m_parentGroup->takeGroup( this );
}

void BuildGroupItem::insertGroup( BuildGroupItem * group )
{
    m_subGroups.append( group );
}

void BuildGroupItem::removeGroup( BuildGroupItem * group )
{
    m_subGroups.remove( group );
    delete( group );
}

BuildGroupItem * BuildGroupItem::takeGroup( BuildGroupItem * group )
{
    m_subGroups.remove( group );
    return group;
}

void BuildGroupItem::insertTarget( BuildTargetItem * target )
{
    m_targets.append( target );
}

void BuildGroupItem::removeTarget( BuildTargetItem * target )
{
    m_targets.remove( target );
    delete( target );
}

BuildTargetItem * BuildGroupItem::takeTarget( BuildTargetItem * target )
{
    m_targets.remove( target );
    return target;
}

// ------------------------------------------------------------------------
BuildTargetItem::BuildTargetItem( const QString & name, BuildGroupItem * parentGroup )
    : BuildBaseItem( Target, parentGroup ), m_parentGroup( parentGroup )
{
    m_name = name;
    if( m_parentGroup )
	m_parentGroup->insertTarget( this );
}

BuildTargetItem::~ BuildTargetItem( )
{
    while( m_files.size() ){
        BuildFileItem* file = m_files.front();
	m_files.pop_front();
	delete file;
    }

    if( m_parentGroup )
	m_parentGroup->takeTarget( this );
}

void BuildTargetItem::insertFile( BuildFileItem * file )
{
    m_files.append( file );
}

void BuildTargetItem::removeFile( BuildFileItem * file )
{
    m_files.remove( file );
    delete( file );
}

BuildFileItem * BuildTargetItem::takeFile( BuildFileItem * file )
{
    m_files.remove( file );
    return file;
}

// ------------------------------------------------------
BuildFileItem::BuildFileItem( const KURL & url, BuildTargetItem * parentTarget )
    : BuildBaseItem( File, parentTarget ), m_url( url ), m_parentTarget( parentTarget )
{
    if( m_parentTarget )
	m_parentTarget->insertFile( this );
}

BuildFileItem::~ BuildFileItem( )
{
    if( m_parentTarget )
        m_parentTarget->takeFile( this );
}


// ------------------------------------------------------


void KDevBuildSystem::addDefaultBuildWidget(KDialogBase *dlg, QWidget *parent, BuildBaseItem *it )
{
    BuildItemConfigWidget *w = new BuildItemConfigWidget(it, parent);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


KDevBuildSystem::~ KDevBuildSystem( )
{
}

KDevBuildSystem::KDevBuildSystem( QObject *parent, const char * name )
    :QObject(parent, name)
{
}

void KDevBuildSystem::configureBuildItem( KDialogBase * dlg, BuildBaseItem * it)
{
    qWarning("KDevBuildSystem::configureBuildItem");
    if (!dlg)
        return;
    qWarning("KDevBuildSystem::configureBuildItem: dlg exists");
    QVBox *vbox;
    vbox = dlg->addVBoxPage(i18n("Properties"));
    addDefaultBuildWidget(dlg, vbox, it);
}

void KDevBuildSystem::initProject( KDevProject * project )
{
    m_project = project;
}

KDevProject * KDevBuildSystem::project( )
{
    return m_project;
}

void KDevBuildSystem::updateDefaultBuildWidget( )
{
}




// ------------------------------------------------------
BuildItemConfigWidget::BuildItemConfigWidget( BuildBaseItem *it, QWidget * parent, const char * name )
    :QWidget(parent, name)
{
    QVBoxLayout *l = new QVBoxLayout(this, 2, 0);
    PropertyLib::PropertyEditor *ed = new PropertyLib::PropertyEditor(this, "item_propeditor");
    ed->populateProperties(&(it->attributes()));
    l->addWidget(ed);
}

void BuildItemConfigWidget::accept( )
{
}




// ------------------------------------------------------
ProjectConfigTab::ProjectConfigTab( QWidget * parent, const char * name )
    :QWidget(parent, name)
{
}

BuildFileItem * BuildTargetItem::fileByName( const QString & fileName )
{
    for( QValueList<BuildFileItem*>::Iterator it=m_files.begin(); it!=m_files.end(); ++it )
    {
	BuildFileItem* file = *it;
	if( file->name() == fileName )
	    return file;
    }

    return 0;
}

BuildTargetItem * BuildGroupItem::targetByName( const QString & targetName )
{
    for( QValueList<BuildTargetItem*>::Iterator it=m_targets.begin(); it!=m_targets.end(); ++it )
    {
	BuildTargetItem* target = *it;
	if( target->name() == targetName )
	    return target;
    }

    return 0;
}

BuildGroupItem * BuildGroupItem::groupByname( const QString & groupName )
{
    for( QValueList<BuildGroupItem*>::Iterator it=m_subGroups.begin(); it!=m_subGroups.end(); ++it )
    {
	BuildGroupItem* group = *it;
	if( group->name() == groupName )
	    return group;
    }

    return 0;
}

#include "kdevbuildsystem.moc"
