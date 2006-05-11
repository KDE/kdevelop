/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <q3header.h>
#include <qtextstream.h>
#include <QDir>
#include <qstringlist.h>
#include <QRegExp>
#include <qpalette.h>

#include "replaceitem.h"
#include "replaceview.h"



ReplaceItem * ReplaceView::firstChild() const
{
    return static_cast<ReplaceItem*>( Q3ListView::firstChild() );
}


ReplaceView::ReplaceView( QWidget * parent ) : K3ListView( parent ), _latestfile( 0 )
{
    setSorting( -1 );
    addColumn( "" );
    header()->hide();
    setFullWidth(true);

    QPalette pal = palette();
    QColorGroup cg = pal.active();
    cg.setColor( QColorGroup::Highlight, Qt::lightGray );
    pal.setActive( cg );
    setPalette( pal );

    connect( this, SIGNAL( clicked( Q3ListViewItem * ) ), SLOT( slotClicked( Q3ListViewItem * ) ) );
    connect( this, SIGNAL( mouseButtonPressed( int, Q3ListViewItem *, const QPoint &, int) ),
             SLOT( slotMousePressed(int, Q3ListViewItem *, const QPoint &, int) ) );
}

void ReplaceView::makeReplacementsForFile( QTextStream & istream, QTextStream & ostream, ReplaceItem const * fileitem )
{
    int line = 0;

    ReplaceItem const * lineitem = fileitem->firstChild();
    while ( lineitem )
    {
        if ( lineitem->isOn() )
        {
            while ( line < lineitem->line() )
            {
                ostream << istream.readLine() << "\n";
                line++;
            }
            // this is the hit
            ostream << istream.readLine().replace( _regexp, _replacement ) << "\n";
            line++;
        }

        lineitem = lineitem->nextSibling();
    }

    while ( !istream.atEnd() )
    {
        ostream << istream.readLine() << "\n";
    }
}

void ReplaceView::showReplacementsForFile( QTextStream & stream, QString const & file )
{
    ReplaceItem * latestitem = 0;

    int line = 0;
    bool firstline = true;

    while ( !stream.atEnd() )
    {
        QString s = stream.readLine();

        if ( s.contains( _regexp ) > 0 )
        {
            s.replace( _regexp, _replacement );

            if ( firstline )
            {
                _latestfile = new ReplaceItem( this, _latestfile, file );
                firstline = false;
            }
            latestitem = new ReplaceItem( _latestfile, latestitem, file, s.trimmed(), line );
            _latestfile->insertItem( latestitem );
        }
        line++;
    }
}

void ReplaceView::setReplacementData( QRegExp const & re, QString const & replacement )
{
    _regexp = re;
    _replacement = replacement;
}

void ReplaceView::slotMousePressed(int btn, Q3ListViewItem* i, const QPoint& pos, int col)
{
    kDebug(0) << "ReplaceView::slotMousePressed()" << endl;

    if ( ReplaceItem * item = dynamic_cast<ReplaceItem*>( i ) )
    {
        if ( btn == Qt::RightButton )
        {
            //        popup menu?
        }
        else if ( btn == Qt::LeftButton )
        {
            // map pos to item/column and call ReplacetItem::activate(pos)
            item->activate( col, viewport()->mapFromGlobal( pos ) - QPoint( 0, itemRect(item).top() ) );
        }
    }
}

void ReplaceView::slotClicked( Q3ListViewItem * item )
{
    kDebug(0) << "ReplaceView::slotClicked()" << endl;

    if ( ReplaceItem * ri = dynamic_cast<ReplaceItem*>( item ) )
    {
        if ( ri->lineClicked() )
        {
            kDebug(0) << "emitting editDocument" << endl;
            emit editDocument( ri->file(), ri->line() );
        }
    }
}

#include "replaceview.moc"
