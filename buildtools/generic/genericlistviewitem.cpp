/***************************************************************************
 *   Copyright (C) 2003 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "genericlistviewitem.h"

#include "kdevbuildsystem.h"

#include <kiconloader.h>

#include <qsimplerichtext.h>
#include <qpixmap.h>
#include <qpainter.h>

GenericListViewItem::GenericListViewItem( bool bold, QListView * parent, const QString & text )
    : KListViewItem( parent, text ), m_bold( bold ), m_buildItem(0)
{
}

GenericListViewItem::GenericListViewItem( bool bold, QListViewItem * parent, const QString & text )
    : KListViewItem( parent, text ), m_bold( bold ), m_buildItem(0)
{
}

GenericListViewItem::~ GenericListViewItem( )
{
}

BuildBaseItem* GenericListViewItem::buildItem( ) const
{
    return m_buildItem;
}

void GenericListViewItem::setBold( bool b )
{
    m_bold = b;
    repaint();
}

void GenericListViewItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int alignment )
{
    if( isBold() ){
        QFont font( p->font() );
        font.setBold( true );
        p->setFont( font );
    }

    KListViewItem::paintCell( p, cg, column, width, alignment );
}

GenericFileListViewItem::GenericFileListViewItem( QListView * parent, BuildFileItem * fileItem )
    : GenericListViewItem( false, parent, fileItem->url().fileName() )
{
    m_buildItem = fileItem;
}

GenericFileListViewItem::GenericFileListViewItem( QListViewItem * parent, BuildFileItem * fileItem )
    : GenericListViewItem( false, parent, fileItem->url().fileName() )
{
    m_buildItem = fileItem;
}

void GenericFileListViewItem::setup( )
{
    setPixmap( 0, SmallIcon("document") );
    GenericListViewItem::setup();
}

GenericTargetListViewItem::GenericTargetListViewItem( QListView * parent, BuildTargetItem * targetItem )
    : GenericListViewItem( false, parent, targetItem->name() )
{
    m_buildItem = targetItem;
}

GenericTargetListViewItem::GenericTargetListViewItem( QListViewItem * parent, BuildTargetItem * targetItem )
    : GenericListViewItem( false, parent, targetItem->name() )
{
    m_buildItem = targetItem;
}

void GenericTargetListViewItem::setup( )
{
    setPixmap( 0, SmallIcon("tar") );
    setExpandable( true );
    setOpen(true);
    GenericListViewItem::setup();
}

GenericGroupListViewItem::GenericGroupListViewItem( QListView * parent, BuildGroupItem * groupItem )
    : GenericListViewItem( false, parent, groupItem->name() )
{
    m_buildItem = groupItem;
    init();
}

GenericGroupListViewItem::GenericGroupListViewItem( QListViewItem * parent, BuildGroupItem * groupItem )
    : GenericListViewItem( false, parent, groupItem->name() )
{
    m_buildItem = groupItem;
    init();
}

void GenericGroupListViewItem::setup( )
{
    setPixmap( 0, SmallIcon("folder") );
    GenericListViewItem::setup();
}

void GenericGroupListViewItem::init()
{
    setExpandable( static_cast<BuildGroupItem*>(m_buildItem)->groups().size() > 0 );
    setOpen(true);
}

BuildGroupItem * GenericGroupListViewItem::groupItem( ) const
{
    return static_cast<BuildGroupItem*>(m_buildItem);
}

BuildFileItem * GenericFileListViewItem::fileItem( ) const
{
    return static_cast<BuildFileItem*>(m_buildItem);
}

BuildTargetItem * GenericTargetListViewItem::targetItem( ) const
{
    return static_cast<BuildTargetItem*>(m_buildItem);
}
