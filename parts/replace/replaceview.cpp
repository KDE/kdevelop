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

#include <qheader.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>

#include "replaceitem.h"
#include "replaceview.h"



ReplaceItem * ReplaceView::firstChild() const
{
    return static_cast<ReplaceItem*>( QListView::firstChild() );
}


ReplaceView::ReplaceView( QWidget * parent ) : KListView( parent ), _latestfile( 0 )
{
    setSorting( -1 );
    addColumn( "" );
    header()->hide();
    setFullWidth();
}

void ReplaceView::makeReplacementsForFile( QTextStream & istream, QTextStream & ostream, ReplaceItem const * fileitem )
{
    int line = 0;

    ReplaceItem const * lineitem = fileitem->firstChild();
    while ( lineitem )
    {
        if ( lineitem->isOn() )
        {
            //kdDebug(0) << " #### " << lineitem->text() << endl;

            while ( line < lineitem->line() )
            {
                ostream << istream.readLine() << "\n";
                line++;
            }
            // here is the hit
            Q_ASSERT( line == lineitem->line() );
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
            latestitem = new ReplaceItem( _latestfile, latestitem, file, s.stripWhiteSpace(), line );
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
