/***************************************************************************
*   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
*   bernd@kdevelop.org                                                    *
*                                                                         *
*   Copyright (C) 2002 by Victor Röder                                    *
*   victor_roeder@gmx.de                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <qpainter.h>

#include <kiconloader.h>

#include "autolistviewitems.h"

/**
* Class ProjectItem
*/

ProjectItem::ProjectItem( Type type, QListView *parent, const QString &text )
		: QListViewItem( parent, text ), typ( type )
{
	bld = false;
}


ProjectItem::ProjectItem( Type type, ProjectItem *parent, const QString &text )
		: QListViewItem( parent, text ), typ( type )
{
	bld = false;
}


void ProjectItem::paintCell( QPainter *p, const QColorGroup &cg,
                             int column, int width, int alignment )
{
	if ( isBold() )
	{
		QFont font( p->font() );
		font.setBold( true );
		p->setFont( font );
	}
	QListViewItem::paintCell( p, cg, column, width, alignment );
}


/**
* Class SubprojectItem
*/

SubprojectItem::SubprojectItem( QListView *parent, const QString &text )
		: ProjectItem( Subproject, parent, text )
{
	init();
}


SubprojectItem::SubprojectItem( SubprojectItem *parent, const QString &text )
		: ProjectItem( Subproject, parent, text )
{
	init();
}


void SubprojectItem::init()
{
	targets.setAutoDelete( true );
	setPixmap( 0, SmallIcon( "folder" ) );
}


QString SubprojectItem::relativePath()
{
    QString relpath = subdir;

    SubprojectItem *it = this;
    qWarning("prepend: %s", subdir.latin1());
    while ( (it= dynamic_cast<SubprojectItem*>(it->parent())) )
    {
        qWarning("prepend: %s", it->subdir.latin1());
        relpath.prepend(it->subdir + "/");
    }
    relpath.remove(0, 2);

    qWarning("_________CREATED relpath is %s", relpath.latin1());
    return relpath;
//    return path.remove(0,projectDirectory().length());
//    return subdir;
}


/**
* Class TargetItem
*/

TargetItem::TargetItem( QListView *lv, bool group, const QString &text )
		: ProjectItem( Target, lv, text )
{
	sources.setAutoDelete( true );
	setPixmap( 0, group ? SmallIcon( "tar" ) : SmallIcon( "binary" ) );
}


/**
* Class FileItem
*/

FileItem::FileItem( QListView *lv, const QString &text )
		: ProjectItem( File, lv, text )
{
	setPixmap( 0, SmallIcon( "document" ) );
}

#include "autolistviewitems.moc"
