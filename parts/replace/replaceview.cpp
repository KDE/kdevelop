/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qheader.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qstringlist.h>

#include "replaceitem.h"
#include "replaceview.h"



ReplaceItem * ReplaceView::firstChild() const
{
    return static_cast<ReplaceItem*>( QListView::firstChild() );
}


ReplaceView::ReplaceView( QWidget * parent ) : KListView( parent )
{
    setSorting( -1 );
    addColumn( "" );
    header()->hide();
    setFullWidth();
}


void ReplaceView::makeReplacements(QString const & pattern, QString const & replacement )
{
    ReplaceItem const * fileitem = firstChild();

    while ( fileitem )
    {
        if ( fileitem->isOn() )
        {
            kdDebug(0) << " ## " << fileitem->file() << endl;

            QString outfilename = fileitem->file() + "_modified";

            QFile infile( fileitem->file() );
            QFile outfile( outfilename );
            if ( ! ( infile.open( IO_ReadOnly ) && outfile.open( IO_WriteOnly ) ) )
            {
                kdDebug(0) << " **** ERROR opening file! **** " << endl;
                return;
            }
            QTextStream instream( &infile);
            QTextStream outstream( &outfile );

            int line = 0;

            ReplaceItem const * lineitem = fileitem->firstChild();
            while ( lineitem )
            {
                if ( lineitem->isOn() )
                {
                    kdDebug(0) << " #### " << lineitem->text() << endl;

                    while ( line < lineitem->line() )
                    {
                        outstream << instream.readLine() << "\n";
                        line++;
                    }
                    // here is the hit
                    Q_ASSERT( line == lineitem->line() );
                    outstream << instream.readLine().replace( pattern, replacement ) << "\n";
                    line++;
                }

                lineitem = lineitem->nextSibling();
            }

            while ( !instream.atEnd() )
            {
                outstream << instream.readLine() << "\n";
            }

            infile.close();
            outfile.close();

            QDir().rename( outfilename, fileitem->file(), true );
        }
        fileitem = fileitem->nextSibling();
    }
}

void ReplaceView::showReplacements( QStringList const & files, QString const & pattern, QString const &replacement )
{
    ReplaceItem::s_listview_done = false;

    ReplaceItem * latestfile = 0;

    QStringList::ConstIterator it = files.begin();
    while ( it != files.end() )
    {
        ReplaceItem * latestitem = 0;

        QFile file( *it );
        if ( file.open ( IO_ReadOnly ) )
        {
            int line = 0;
            bool firstline = true;
            QTextStream stream ( &file );

            while ( !stream.atEnd() )
            {
                QString s = stream.readLine();

                if ( s.contains( pattern ) > 0 )
                {
                    s.replace( pattern, replacement );

                    if ( firstline )
                    {
                        latestfile = new ReplaceItem( this, latestfile, *it );
                        firstline = false;
                    }
                    latestitem = new ReplaceItem( latestfile, latestitem, *it, s.stripWhiteSpace(), line );
                    latestfile->insertItem( latestitem );
                }
                line++;
            }
        }
        ++it;
    }

    ReplaceItem::s_listview_done = true;
}
