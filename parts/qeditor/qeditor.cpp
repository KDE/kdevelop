/* $Id$
 *
 *  This file is part of Klint
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


#include <private/qrichtext_p.h>
#include <qregexp.h>
#include <qmap.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qaccel.h>
#include <qkeysequence.h>

#include "qeditor.h"
#include "qsourcecolorizer.h"
#include "simple_indent.h"
#include "parenmatcher.h"

#include "cpp_colorizer.h"
#include "java_colorizer.h"
#include "python_colorizer.h"
#include "xml_colorizer.h"
#include "qmake_colorizer.h"

// from trolltech's editor -- START
static int indentation( const QString &s )
{
//    if ( s.simplifyWhiteSpace().length() == 0 )
//        return 0;
    int i = 0;
    int ind = 0;
    while ( i < (int)s.length() ) {
        QChar c = s.at( i );
        if ( c == ' ' )
            ind++;
        else if ( c == '\t' )
            ind += 4;
        else
            break;
        ++i;
    }
    return ind;
}

static void indentLine( QTextParag *p, int &oldIndent, int &newIndent )
{
    QString indentString;
    indentString.fill( ' ', newIndent );
    indentString.append( "a" );
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

static int indentForLine( QTextParag* parag )
{
    int line_ind = indentation( parag->string()->toString() );
    line_ind = line_ind > 0 ? line_ind-1 : 0;
    int ind = 0;
    QTextParag* p = parag->prev();
    while( p ){
        QString raw_text = p->string()->toString();
        QString line = raw_text.stripWhiteSpace();
        if( !line.isEmpty() ){
            int new_ind = indentation( raw_text );
            if( new_ind < line_ind ){
                ind = new_ind;
                break;
            }
        }
        p = p->prev();
    }
    return ind;
}



QEditor::QEditor( QWidget* parent, const char* name )
    : QTextEdit( parent, name )
{
    setWordWrap( NoWrap );
    setHScrollBarMode( QScrollView::AlwaysOn );
    setVScrollBarMode( QScrollView::AlwaysOn );
    document()->setUseFormatCollection( FALSE );

    parenMatcher = new ParenMatcher();

    m_tabIndent = TRUE;
    m_backspaceIndent = TRUE;

    document()->addSelection( ParenMatcher::Match );
    document()->addSelection( ParenMatcher::Mismatch );
    document()->setSelectionColor( ParenMatcher::Match, QColor( 204, 232, 195 ) );
    document()->setSelectionColor( ParenMatcher::Mismatch, Qt::magenta );
    document()->setInvertSelectionText( ParenMatcher::Match, FALSE );
    document()->setInvertSelectionText( ParenMatcher::Mismatch, FALSE );

    document()->addSelection( 1000 );
    document()->setSelectionColor( 1000, QColor( 204, 232, 195 ) );

    document()->setIndent( new SimpleIndent() );

    connect( this, SIGNAL(cursorPositionChanged(QTextCursor*) ),
             this, SLOT(doMatch(QTextCursor*)) );

    configChanged();
    QAccel* a = new QAccel( this );
    a->connectItem( a->insertItem( CTRL + Key_L ),
                    this, SLOT(refresh()) );
    a->connectItem( a->insertItem( CTRL + Key_J ),
                    this, SLOT(expandText()) );
}

QEditor::~QEditor()
{
    if( parenMatcher ){
        delete( parenMatcher );
    }
}

void QEditor::keyPressEvent( QKeyEvent* e )
{
    if( e->key() == Key_Tab ){
        if( tabIndentEnabled() ){
            int parag, index;
            getCursorPosition( &parag, &index );
            QString s = textLine( parag ).mid( index ).stripWhiteSpace();
            if( s.isEmpty() ){
                insert( "\t" );
            } else {
                indent();
            }
        } else {
            insert( "\t" );
        }
        e->accept();
    } else if( e->ascii() == '{' || e->ascii() == '}'
               || e->ascii() == ':' || e->ascii() == '#' ){
        insert( e->text(), FALSE );
        e->accept();
    } else if( e->key() == Key_Backspace ){
        if( backspaceIndentEnabled() ){
            backspaceIndent( e );
        } else {
            QTextEdit::keyPressEvent( e );
        }
    } else {
        QTextEdit::keyPressEvent( e );
    }
}

void QEditor::expandText()
{
}

void QEditor::doMatch( QTextCursor* c )
{
    if( parenMatcher->match(c) ){
        repaintChanged();
    }
}

void QEditor::doGotoLine( int line )
{
    setCursorPosition( line, 0 );
    QTextParag *p = document()->paragAt( line );
    if ( !p )
        return;
    QTextCursor c( document() );
    c.setParag( p );
    c.setIndex( 0 );
    document()->removeSelection( 1000 );
    document()->setSelectionStart( 1000, &c );
    c.gotoLineEnd();
    document()->setSelectionEnd( 1000, &c );
    viewport()->repaint( FALSE );
}

QTextCursor* QEditor::textCursor() const
{
    return QTextEdit::textCursor();
}

QTextDocument* QEditor::document() const
{
    return QTextEdit::document();
}

void QEditor::drawCursor( bool visible )
{
    QTextEdit::drawCursor( visible );
}

void QEditor::configChanged()
{
    updateStyles();
}

void QEditor::zoomIn()
{
    QTextEdit::zoomIn();
    updateStyles();
}

void QEditor::zoomOut()
{
    QTextEdit::zoomOut();
    updateStyles();
}

void QEditor::updateStyles()
{
    QSourceColorizer* colorizer = dynamic_cast<QSourceColorizer*>( document()->preProcessor() );
    if( colorizer ){
        setTabStopWidth( colorizer->format(0)->width('x') * 8 );
        document()->setTabStops( colorizer->format(0)->width('x') * 8 );
    }
    QTextEdit::updateStyles();
}

void QEditor::backspaceIndent( QKeyEvent* e )
{
    QTextCursor* c = textCursor();
    QTextParag* p = c->parag();
    QString raw_text = p->string()->toString();
    QString line = raw_text.stripWhiteSpace();

    if( raw_text.left(c->index()).stripWhiteSpace().isEmpty()
        && c->index() > 0 && !hasSelectedText() ){
        drawCursor( FALSE );
        int oi = indentation( raw_text );
        int ni = indentForLine( p );

        indentLine( p, oi, ni );

        int idx = c->index();
        if ( idx >= oi )
            idx += ni - oi;
        else
            idx = ni;
        c->setIndex( idx );
        repaintChanged();
        drawCursor( TRUE );
        e->accept();
    } else {
        // doKeyboardAction( QTextEdit::ActionBackspace );
        QTextEdit::keyPressEvent( e );
    }
}

bool QEditor::replace( const QString &text, const QString &replace,
                      bool cs, bool wo, bool forward, bool startAtCursor,
                      bool replaceAll )
{
    // from trolltech's editor sources -- START
    bool ok = FALSE;
    if ( startAtCursor ) {
        ok = find( text, cs, wo, forward );
    } else {
        int dummy = 0;
        ok =  find( text, cs, wo, forward, &dummy, &dummy );
    }

    if ( ok ) {
        removeSelectedText();
        insert( replace, FALSE, FALSE );
    }

    if ( !replaceAll || !ok ) {
        if ( ok )
            setSelection( textCursor()->parag()->paragId(),
                          textCursor()->index() - replace.length(),
                          textCursor()->parag()->paragId(),
                          textCursor()->index() );
        return ok;
    }

    bool ok2 = TRUE;
    while ( ok2 ) {
        ok2 = find( text, cs, wo, forward );
        if ( ok2 ) {
            removeSelectedText();
            insert( replace, FALSE, FALSE );
        }
    }

    return TRUE;
    // from trolltech's editor sources -- END
}

void QEditor::setDocument( QTextDocument* doc )
{
    QTextEdit::setDocument( doc );
}

void QEditor::refresh()
{
    QTextParag* p = document()->firstParag();
    while( p ){
        if( p->endState() == -1 ){
            break;
        }
        p->setEndState( -1 );
        p = p->next();
    }
    document()->invalidate();
    repaintChanged();
}

void QEditor::repaintChanged()
{
    QTextEdit::repaintChanged();
}

bool QEditor::openFile( const QString& fn )
{
    QFile f( fn );
    if ( !f.open(IO_ReadOnly) ){
        return FALSE;
    }

    clear();

    QTextStream ts( &f );
    QString contents = ts.read();
    tabify( contents );
    setText( contents );

    f.close();

    QFileInfo info( fn );
    QString ext = info.extension();
    if( ext == "cpp" || ext == "cc" || ext == "c" ||
        ext == "h" || ext == "hh" ){
        setLanguage( "c++" );
    } else if( ext == "py" || ext == "pyw" ){
        setLanguage( "python" );
    } else if( ext == "java" ){
        setLanguage( "java" );
    } else if( ext == "xml" || ext == "xslt" || ext == "rc" || ext == "ui" ){
        setLanguage( "xml" );
    } else {
        document()->setPreProcessor( 0 );
    }

    return TRUE;
}

bool QEditor::saveFile( const QString& fn )
{
    QFile f( fn );
    if( !f.open(IO_WriteOnly) ){
        return FALSE;
    }

    QTextStream ts( &f );
    QString contents = text();
    tabify( contents );
    ts << contents;
    f.close();

    QFileInfo info( fn );
    QString ext = info.extension();
    if( ext == "cpp" || ext == "cc" || ext == "c" ||
        ext == "h" || ext == "hh" ){
        setLanguage( "c++" );
    } else if( ext == "py" || ext == "pyw" ){
        setLanguage( "python" );
    } else if( ext == "java" ){
        setLanguage( "java" );
    } else if( ext == "xml" || ext == "xslt" || ext == "rc" || ext == "ui" ){
        setLanguage( "xml" );
    } else if( ext == "pro" ){
        setLanguage( "qmake" );
    } else {
        document()->setPreProcessor( 0 );
    }

    return TRUE;
}

QString QEditor::textLine( uint line ) const
{
    if ( line < 0 || line >= lines() )
        return QString::null;

    QString str = document()->paragAt( line )->string()->toString();
    str.truncate( str.length() - 1 );
    return str;
}

void QEditor::setLanguage( const QString& l )
{
    m_language = l;
    if( m_language == "c++" ){
        document()->setPreProcessor( new CppColorizer() );
    } else if( m_language == "python" ){
        document()->setPreProcessor( new PythonColorizer() );
    } else if( m_language == "xml" ){
        document()->setPreProcessor( new XMLColorizer() );
    } else if( m_language == "java" ){
        document()->setPreProcessor( new JavaColorizer() );
    } else if( m_language == "qmake" ){
        document()->setPreProcessor( new QMakeColorizer() );
    } else {
        document()->setPreProcessor( 0 );
    }
}

QString QEditor::language() const
{
    return m_language;
}
