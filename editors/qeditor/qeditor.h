/*
 *  This file is part of Klint
 *  Copyright (C) 2002 Roberto Raggi (roberto@kdevelop.org)
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include <qptrlist.h>

class ParenMatcher;
class QPopupMenu;
class QSourceColorizer;
class QEditorIndenter;
class QEditorKey;

class QEditor: public KTextEdit
{
    Q_OBJECT
public:
    QEditor( QWidget* parent=0, const char* name=0 );
    virtual ~QEditor();

    bool isRecording() const { return m_recording; }
    void setIsRecording( bool b ) { m_recording = b; }

    QEditorIndenter* indenter() const;

    QTextCursor* textCursor() const;

    QTextDocument* document() const;
    void setDocument( QTextDocument*);

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
    
    virtual void indent();

    QSourceColorizer* colorizer() const;
    
signals:
    void parsed();
    void ensureTextIsVisible( QTextParagraph* );

public slots:
    void doGotoLine( int line );
    virtual void configChanged();
    virtual void refresh();
    virtual void startMacro();
    virtual void stopMacro();
    virtual void executeMacro();

private slots:
    void doMatch( QTextCursor* );
    void slotCursorPositionChanged( int, int );

protected:
    virtual bool event( QEvent* );
    virtual void keyPressEvent( QKeyEvent* );
    virtual void backspaceIndent( QKeyEvent* );
    int backspace_indentation( const QString &s );
    int backspace_indentForLine( int line );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    bool isDelimiter(const QChar& c);

private:
    ParenMatcher* parenMatcher;
    bool m_tabIndent;
    bool m_completion;
    bool m_backspaceIndent;
    QString m_language;
    int m_currentLine;
    int m_tabStop;
    QString m_electricKeys;
    QPopupMenu* m_applicationMenu;
    bool m_recording;
    QPtrList<QEditorKey> m_keys;
};

#endif
