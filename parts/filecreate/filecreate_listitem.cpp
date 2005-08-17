/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *   thanks: Roberto Raggi for QSimpleRichText stuff                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/


#include "filecreate_listitem.h"

#include <kglobal.h>
#include <kiconloader.h>

#include <q3simplerichtext.h>
#include <qpixmap.h>

namespace FileCreate {

  ListItem::ListItem(Q3ListView * listview, const FileType * filetype) :
    KListViewItem(listview), m_filetype(filetype),
    m_filetypeRenderer(NULL)
  {
    init();
  }


  ListItem::ListItem(ListItem * listitem, const FileType * filetype) :
    KListViewItem(listitem), m_filetype(filetype),
    m_filetypeRenderer(NULL)
  {
    init();
  }

  ListItem::~ListItem()
  {
    if (m_filetypeRenderer) delete m_filetypeRenderer;
  }

  void ListItem::setup()
  {
    if (m_filetypeRenderer) delete m_filetypeRenderer;
    m_filetypeRenderer = new Q3SimpleRichText( text(1), listView()->font() );
    m_filetypeRenderer->setWidth(listView()->columnWidth(1));
    setHeight(m_filetypeRenderer->height());
    KListViewItem::setup();
  }

  void ListItem::setHeight( int height )
  {
    KListViewItem::setHeight( QMAX(QMAX(height,m_iconHeight), m_filetypeRenderer->height() ) );
  }

  void ListItem::prepareResize() {
    if (m_filetypeRenderer) {
      m_filetypeRenderer->setWidth(listView()->columnWidth(1));
      setHeight(m_filetypeRenderer->height());
    }
  }

  void ListItem::paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align )
  {

    QBrush brush( isSelected() ? cg.highlight() : backgroundColor() );

    if( column == 1 ){
//       m_filetypeRenderer->setWidth(width);
//       setHeight(m_filetypeRenderer->height());
      m_filetypeRenderer->draw( p, 0, 0, QRect(0, 0, width, height()), cg, &brush );
      return;
    }

    KListViewItem::paintCell( p, cg, column, width, align );
  }

  void ListItem::init()
  {
    m_iconHeight = 0;
    setText(0, m_filetype->ext()!="" ? QString("." + m_filetype->ext()) : QString("") );
    setText(1, "<qt><b>"+m_filetype->name()+"</b>. " + m_filetype->descr() );

    KIconLoader * loader = KGlobal::iconLoader();
    QPixmap iconPix = loader->loadIcon(m_filetype->icon(), KIcon::Desktop,
                                       KIcon::SizeMedium,
                                       KIcon::DefaultState, NULL,
                                       true);
    if (!iconPix.isNull()) {
      setPixmap(0, iconPix);
      m_iconHeight = iconPix.height();
    }

  }



}


