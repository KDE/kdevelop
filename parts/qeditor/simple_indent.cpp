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


/* $Id$
 *
 *  This file is part of Klint
 *  Copyright (C) 2001 Roberto Raggi (raggi@cli.di.unipi.it)
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
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */


#include "simple_indent.h"
#include "qsourcecolorizer.h"
#include "paragdata.h"
#include "qeditor.h"

// from trolltech's editor -- START
static int indentation( const QString &s, int tabwidth )
{
    if ( s.simplifyWhiteSpace().length() == 0 )
        return 0;
    int i = 0;
    int ind = 0;
    while ( i < (int)s.length() ) {
        QChar c = s.at( i );
        if ( c == ' ' ){
            ind++;
        } else if ( c == '\t' ){
            ind += tabwidth;
        } else {
            break;
        }
        ++i;
    }
    return ind;
}

void tabify( QString &s, int tabwidth )
{
    int i = 0;

    for ( ;; ) {
        for ( int j = i; j < (int)s.length(); ++j ) {
            if ( s[ j ] != ' ' && s[ j ] != '\t' ) {
                if ( j > i ) {
                    QString t  = s.mid( i, j - i );
                    int spaces = 0;
                    for ( int k = 0; k < (int)t.length(); ++k )
                        spaces += ( t[ k ] == ' ' ? 1 : tabwidth );
                    s.remove( i, t.length() );
                    int tabs = spaces / tabwidth;
                    spaces = spaces - ( tabwidth * tabs );
                    QString tmp;
                    tmp.fill( ' ', spaces );
                    if ( spaces > 0 )
                        s.insert( i, tmp );
                    tmp.fill( '\t', tabs );
                    if ( tabs > 0 )
                        s.insert( i, tmp );
                }
                break;
            }
        }
        i = s.find( '\n', i );
        if ( i == -1 )
            break;
        ++i;
    }
}

void indentLine( QTextParag *p, int tabwidth, int &oldIndent, int &newIndent )
{
    QString indentString;
    indentString.fill( ' ', newIndent );
    indentString.append( "a" );
    tabify( indentString, tabwidth );
    indentString.remove( indentString.length() - 1, 1 );
    newIndent = indentString.length();
    oldIndent = 0;
    while ( p->length() > 0 && ( p->at( 0 )->c == ' ' || p->at( 0 )->c == '\t' ) ) {
        ++oldIndent;
        p->remove( 0, 1 );
    }
    if ( p->string()->length() == 0 )
        p->append( " " );
    if ( !indentString.isEmpty() )
        p->insert( 0, indentString );
}
// from trolltech's editor -- END

SimpleIndent::SimpleIndent( QEditor* editor )
    : m_editor( editor )
{

}

SimpleIndent::~SimpleIndent()
{

}

static int indentForLine( QTextParag* parag, int tabwidth )
{
    int ind = 0;
    QTextParag* p = parag->prev();
    while( p ){
        QString raw_text = p->string()->toString();
        QString line = raw_text.stripWhiteSpace();
        if( !line.isEmpty() ){
            ind = indentation( raw_text, tabwidth );
            break;
        }
        p = p->prev();
    }
    return ind;
}

void SimpleIndent::indent( QTextDocument* doc, QTextParag* parag,
                           int* oldIndent, int* newIndent )
{
    int tabwidth = m_editor->tabStop();
    QString s = parag->string()->toString();
    int oi = indentation( s, tabwidth );
    int ind = indentForLine( parag, tabwidth );

    indentLine( parag, tabwidth, oi, ind );
    if( oldIndent ){
        *oldIndent = oi;
    }
    if( newIndent ){
        *newIndent = ind;
    }
}
