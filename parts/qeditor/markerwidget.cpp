/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file COPYING included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "markerwidget.h"
#include "qeditor.h"
#include "paragdata.h"

#include <qpopupmenu.h>
#include <private/qrichtext_p.h>
#include <kdebug.h>
#include <kiconloader.h>

using namespace std;

MarkerWidget::MarkerWidget( QEditor* editor, QWidget* parent, const char* name )
    : QWidget( parent, name, WRepaintNoErase | WStaticContents | WResizeNoErase ),
      m_editor( editor )
      ,m_clickChangesBPs(true)
{
    bookmarkPixmap = SmallIcon( "attach" );
    breakpointPixmap = SmallIcon( "stop" );
    execPixmap = SmallIcon( "exec" );
    problemPixmap = SmallIcon( "stop" );
    funStartPixmap = SmallIcon( "start" );

    setFixedWidth( 20 );

    connect( m_editor->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
             this, SLOT( doRepaint() ) );
    connect( m_editor, SIGNAL( textChanged() ),
             this, SLOT( doRepaint() ) );

    doRepaint();
}

MarkerWidget::~MarkerWidget()
{
}

void MarkerWidget::paintEvent( QPaintEvent* /*e*/ )
{
    buffer.fill();

    QTextParagraph *p = m_editor->document()->firstParagraph();
    QPainter painter( &buffer );
    int yOffset = m_editor->contentsY();
    while ( p ) {
        if ( !p->isVisible() ) {
            p = p->next();
            continue;
        }
        if ( p->rect().y() + p->rect().height() - yOffset < 0 ) {
            p = p->next();
            continue;
        }
        if ( p->rect().y() - yOffset > height() )
            break;


        ParagData* paragData = (ParagData*) p->extraData();
        if( paragData ){
            switch( paragData->mark() ){
            case 0x01:
                painter.drawPixmap( 3, p->rect().y() +
                                    ( p->rect().height() - bookmarkPixmap.height() ) / 2 -
                                    yOffset, bookmarkPixmap );
                break;
		
            case 0x02:
                painter.drawPixmap( 3, p->rect().y() +
                                    ( p->rect().height() - breakpointPixmap.height() ) / 2 -
                                    yOffset, breakpointPixmap );
                break;
		
            case 0x05:
                painter.drawPixmap( 3, p->rect().y() +
                                    ( p->rect().height() - execPixmap.height() ) / 2 -
                                    yOffset, execPixmap );
                break;
		
            case 0x200:
                painter.drawPixmap( 3, p->rect().y() +
                                    ( p->rect().height() - problemPixmap.height() ) / 2 -
                                    yOffset, problemPixmap );
                break;
		
	    case 0x400:
                painter.drawPixmap( 3, p->rect().y() +
                                    ( p->rect().height() - funStartPixmap.height() ) / 2 -
                                    yOffset, funStartPixmap );
                break;
		
            default:
                break;
            }
        }

        p = p->next();
    }

    painter.end();
    bitBlt( this, 0, 0, &buffer );
}

void MarkerWidget::resizeEvent( QResizeEvent *e )
{
    buffer.resize( e->size() );
    QWidget::resizeEvent( e );
}

void MarkerWidget::contextMenuEvent( QContextMenuEvent* e )
{
    QPopupMenu m( 0, "editor_breakpointsmenu" );

    int toggleBreakPoint = 0;
    int toggleBookmark = 0;
    int lmbClickChangesBPs = 0;
    int lmbClickChangesBookmarks = 0;
    
    QTextParagraph *p = m_editor->document()->firstParagraph();
    int yOffset = m_editor->contentsY();
    while ( p ) {
        if ( e->y() >= p->rect().y() - yOffset && e->y() <= p->rect().y() + p->rect().height() - yOffset ) {
            ParagData* data = (ParagData*) p->extraData();
            if ( data->mark() & 0x02 )
                toggleBreakPoint = m.insertItem( tr( "Clear Breakpoint" ) );
            else
                toggleBreakPoint = m.insertItem( tr( "Set Breakpoint" ) );

            m.insertSeparator();

            if ( data->mark() & 0x01 )
                toggleBookmark = m.insertItem( tr( "Clear Bookmark" ) );
            else
                toggleBookmark = m.insertItem( tr( "Set Bookmark" ) );

            m.insertSeparator();
            lmbClickChangesBPs = m.insertItem( tr( "Left mouse button click sets breakpoints" ) );
            lmbClickChangesBookmarks = m.insertItem( tr( "Left mouse button click sets bookmarks" ) );
            m.setItemChecked(lmbClickChangesBPs, m_clickChangesBPs);
            m.setItemChecked(lmbClickChangesBookmarks, !m_clickChangesBPs);
                            
            //m.insertSeparator();
            break;
        }
        p = p->next();
    }

    int res = m.exec( e->globalPos() );
    if ( res == -1)
        return;

    ParagData* data = (ParagData*) p->extraData();

    KTextEditor::Mark mark;
    mark.line = p->paragId();
    
    if ( res == toggleBookmark ) {
        mark.type = 0x01;
        if ( data->mark() & 0x01 ) {
            data->setMark( 0 );
            emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkRemoved);
        }
        else {
            data->setMark( 0x01 );
            emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkAdded);
        }
    } else if ( res == toggleBreakPoint ) {
        mark.type = 0x02;
        if ( data->mark() & 0x02 ) {
            data->setMark( 0 );
            emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkRemoved);
        }
        else {
            data->setMark( 0x02 );
            emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkAdded);
        }
    } else if ( res == lmbClickChangesBPs ) {
        m_clickChangesBPs = !m.isItemChecked(lmbClickChangesBPs);
    } else if ( res == lmbClickChangesBookmarks ) {
        m_clickChangesBPs = m.isItemChecked(lmbClickChangesBookmarks);
    }

    doRepaint();
    // emit markersChanged();
}

void MarkerWidget::mousePressEvent( QMouseEvent * e )
{
  QTextParagraph *p = m_editor->document()->firstParagraph();
  int yOffset = m_editor->contentsY();
  ParagData* data = 0L;
  while ( p ) {
    if ( e->y() >= p->rect().y() - yOffset && e->y() <= p->rect().y() + p->rect().height() - yOffset ) {
      data = (ParagData*) p->extraData();
      break;
    }
    p = p->next();
  }

  if (e->button() == Qt::LeftButton) {
    if (!data) return;

    KTextEditor::Mark mark;
    mark.line = p->paragId();
    if (m_clickChangesBPs) {
      mark.type = 0x02;
      if (data->mark() & 0x02) {
        data->setMark(0);
        emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkRemoved);
      }
      else {
        data->setMark(0x02);
        emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkAdded);
      }
    }
    else {
      mark.type = 0x01;
      if (data->mark() & 0x01) {
        data->setMark(0);
        emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkRemoved);
      }
      else {
        data->setMark(0x01);
        emit markChanged(mark, KTextEditor::MarkInterfaceExtension::MarkAdded);
      }
    }
  }

  doRepaint();
}

#include "markerwidget.moc"
