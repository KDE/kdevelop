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
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
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
** Foundation and appearing in the file LICENSE.GPL included in the
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

using namespace std;

const char*bookmark_xpm[]={
"12 16 4 1",
"b c #808080",
"a c #000080",
"# c #0000ff",
". c None",
"............",
"............",
"........###.",
".......#...a",
"......#.##.a",
".....#.#..aa",
"....#.#...a.",
"...#.#.a.a..",
"..#.#.a.a...",
".#.#.a.a....",
"#.#.a.a.....",
"#.#a.a...bbb",
"#...a..bbb..",
".aaa.bbb....",
"............",
"............"};

const char* breakpoint_xpm[]={
"11 16 6 1",
"c c #c6c6c6",
". c None",
"# c #000000",
"d c #840000",
"a c #ffffff",
"b c #ff0000",
"...........",
"...........",
"...#####...",
"..#aaaaa#..",
".#abbbbbb#.",
"#abbbbbbbb#",
"#abcacacbd#",
"#abbbbbbbb#",
"#abcacacbd#",
"#abbbbbbbb#",
".#bbbbbbb#.",
"..#bdbdb#..",
"...#####...",
"...........",
"...........",
"..........."};

const char*breakpoint_bl_xpm[]={
"11 16 7 1",
"a c #c0c0ff",
"# c #000000",
"c c #0000c0",
"e c #0000ff",
"b c #dcdcdc",
"d c #ffffff",
". c None",
"...........",
"...........",
"...#####...",
"..#ababa#..",
".#bcccccc#.",
"#acccccccc#",
"#bcadadace#",
"#acccccccc#",
"#bcadadace#",
"#acccccccc#",
".#ccccccc#.",
"..#cecec#..",
"...#####...",
"...........",
"...........",
"..........."};

const char*breakpoint_gr_xpm[]={
"11 16 6 1",
"c c #c6c6c6",
"d c #2c2c2c",
"# c #000000",
". c None",
"a c #ffffff",
"b c #555555",
"...........",
"...........",
"...#####...",
"..#aaaaa#..",
".#abbbbbb#.",
"#abbbbbbbb#",
"#abcacacbd#",
"#abbbbbbbb#",
"#abcacacbd#",
"#abbbbbbbb#",
".#bbbbbbb#.",
"..#bdbdb#..",
"...#####...",
"...........",
"...........",
"..........."};

const char*exec_xpm[]={
"11 16 4 1",
"a c #00ff00",
"b c #000000",
". c None",
"# c #00c000",
"...........",
"...........",
"...........",
"#a.........",
"#aaa.......",
"#aaaaa.....",
"#aaaaaaa...",
"#aaaaaaaaa.",
"#aaaaaaa#b.",
"#aaaaa#b...",
"#aaa#b.....",
"#a#b.......",
"#b.........",
"...........",
"...........",
"..........."};


static QPixmap *bookmarkPixmap = 0;
static QPixmap *breakpointPixmap = 0;
static QPixmap *execPixmap = 0;



MarkerWidget::MarkerWidget( QEditor* editor, QWidget* parent, const char* name )
	: QWidget( parent, name, WRepaintNoErase | WStaticContents | WResizeNoErase ),
	  m_editor( editor )
{
    if ( !bookmarkPixmap ){
		bookmarkPixmap = new QPixmap( bookmark_xpm );
                breakpointPixmap = new QPixmap( breakpoint_xpm );
                execPixmap = new QPixmap( exec_xpm );
	}

	setFixedWidth( 20 );

	connect( m_editor->verticalScrollBar(), SIGNAL( valueChanged( int ) ),
			 this, SLOT( doRepaint() ) );
	connect( m_editor, SIGNAL( textChanged() ),
			 this, SLOT( doRepaint() ) );
}

MarkerWidget::~MarkerWidget()
{
}

void MarkerWidget::paintEvent( QPaintEvent* /*e*/ )
{
	buffer.fill( backgroundColor() );

	QTextParag *p = m_editor->document()->firstParag();
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


                painter.drawLine( 0, p->rect().y() - yOffset,
                                  0, p->rect().y() - yOffset + p->rect().height() );

		ParagData* paragData = (ParagData*) p->extraData();
		if( paragData ){
                    switch( paragData->mark() ){
                    case 0x01:
			painter.drawPixmap( 3, p->rect().y() +
				( p->rect().height() - bookmarkPixmap->height() ) / 2 -
				yOffset, *bookmarkPixmap );
                        break;
                    case 0x02:
			painter.drawPixmap( 3, p->rect().y() +
				( p->rect().height() - breakpointPixmap->height() ) / 2 -
				yOffset, *breakpointPixmap );
                        break;
                    case 0x05:
			painter.drawPixmap( 3, p->rect().y() +
				( p->rect().height() - execPixmap->height() ) / 2 -
				yOffset, *execPixmap );
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

    QTextParag *p = m_editor->document()->firstParag();
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

            //m.insertSeparator();
            break;
        }
        p = p->next();
    }

    int res = m.exec( e->globalPos() );
    if ( res == -1)
        return;

    ParagData* data = (ParagData*) p->extraData();

    if ( res == toggleBookmark ) {
        if ( data->mark() & 0x01 )
            data->setMark( 0 );
        else
            data->setMark( 0x01 );
    } else if ( res == toggleBreakPoint ) {
        if ( data->mark() & 0x02 )
            data->setMark( 0 );
        else
            data->setMark( 0x02 );
    }

    doRepaint();
    // emit markersChanged();
}
