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

#ifndef qeditor_h
#define qeditor_h


#if defined(HAVE_KTEXTEDIT)
#  include <ktextedit.h>
#else
#  include <qtextedit.h>
#  define KTextEdit QTextEdit
#endif

#include <qstringlist.h>

class ParenMatcher;
class QPopupMenu;
class QSourceColorizer;
class BackgroundParser;

class QEditor: public KTextEdit
{
    Q_OBJECT
public:
    QEditor( QWidget* parent=0, const char* name=0 );
    virtual ~QEditor();

    QTextCursor* textCursor() const;

    QTextDocument* document() const;
    void setDocument( QTextDocument*);

    BackgroundParser* parser() const;
    void setBackgroundParser( BackgroundParser* );

    void drawCursor( bool visible );
    void updateStyles();
    void repaintChanged();

    virtual void zoomIn();
    virtual void zoomOut();

    virtual QString electricKeys() const { return m_electricKeys; }
    virtual void setElectricKeys( const QString& keys ) { m_electricKeys = keys; }

    virtual int tabStop() const;
    virtual void setTabStop( int );

    virtual bool tabIndentEnabled() const { return m_tabIndent; }
    virtual void setTabIndentEnabled( bool b ) { m_tabIndent = b; }

    virtual bool backspaceIndentEnabled() const { return m_backspaceIndent; }
    virtual void setBackspaceIndentEnabled( bool b ) { m_backspaceIndent = b; }

    virtual bool replace( const QString &find, const QString &replace,
			  bool cs, bool wo, bool forward, bool startAtCursor,
			  bool replaceAll );
    virtual QString textLine( uint ) const;

    virtual void setLanguage( const QString& );
    virtual QString language() const;

    virtual void setText( const QString& );

    virtual int level( int ) const;
    virtual void setLevel( int, int );

    virtual void setApplicationMenu( QPopupMenu* menu ) { m_applicationMenu = menu; }
    virtual QPopupMenu* createPopupMenu( const QPoint& );

    QSourceColorizer* colorizer() const;

signals:
    void parsed();

public slots:
    void doGotoLine( int line );
    void configChanged();
    void refresh();

private slots:
    void doMatch( QTextCursor* );
    void slotCursorPositionChanged( int, int );

protected:
    virtual void keyPressEvent( QKeyEvent* );
    virtual void backspaceIndent( QKeyEvent* );

private:
    ParenMatcher* parenMatcher;
    BackgroundParser* m_parser;
    bool m_tabIndent;
    bool m_completion;
    bool m_backspaceIndent;
    QString m_language;
    int m_currentLine;
    int m_tabStop;
    QString m_electricKeys;
    QPopupMenu* m_applicationMenu;
};

#endif
