/****************************************************************************
** $Id$
**
** Definition of the QTextEdit class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTEXTEDIT_H
#define QTEXTEDIT_H

#ifndef QT_H
#include "qlist.h"
#include "qpixmap.h"
#include "qcolor.h"
#include "qtextview.h"
#endif // QT_H

#ifndef QT_NO_TEXTEDIT

// This class is out of the Qt3 namespace on purpose : it will make code
// more easy to port to Qt 3. The other classes are inside Qt3 because otherwise
// they would conflict with Qt 2.
class Q_EXPORT QTextEdit : public Qt3::QTextView
{
    Q_OBJECT
    Q_PROPERTY( int undoDepth READ undoDepth WRITE setUndoDepth )
    Q_PROPERTY( bool overWriteMode READ isOverwriteMode WRITE setOverwriteMode )

public:
    QTextEdit(QWidget *parent = 0, const char *name = 0 );
    ~QTextEdit();

    void getCursorPosition( int &parag, int &index ) const;
    bool isModified() const;
    bool italic() const;
    bool bold() const;
    bool underline() const;
    QString family() const;
    int pointSize() const;
    QColor color() const;
    QFont font() const;
    int alignment() const;
    bool isOverwriteMode() const;
    int undoDepth() const;
    virtual void insert( const QString &text, bool indent = FALSE, bool checkNewLine = TRUE, bool removeSelected = TRUE );
    virtual bool getFormat( int parag, int index, QFont &font, QColor &color );

public slots:
    virtual void setOverwriteMode( bool b );
    virtual void undo();
    virtual void redo();
    virtual void cut();
    virtual void paste();
    virtual void pasteSubType( const QCString &subtype );
    virtual void clear();
    virtual void del();
    virtual void indent();
    virtual void setItalic( bool b );
    virtual void setBold( bool b );
    virtual void setUnderline( bool b );
    virtual void setFamily( const QString &f );
    virtual void setPointSize( int s );
    virtual void setColor( const QColor &c );
    virtual void setFont( const QFont &f );
    virtual void setAlignment( int a );
    virtual void setParagType( Qt3::QStyleSheetItem::DisplayMode dm, Qt3::QStyleSheetItem::ListStyle listStyle );
    virtual void setCursorPosition( int parag, int index );
    virtual void setSelection( int parag_from, int index_from, int parag_to, int index_to, int selNum = 0 );
    virtual void setSelectionAttributes( int selNum, const QColor &back, bool invertText );
    virtual void setModified( bool m );
    virtual void resetFormat();
    virtual void setUndoDepth( int d );
    virtual void removeSelectedText();

signals:
    void undoAvailable( bool yes );
    void redoAvailable( bool yes );
    void currentFontChanged( const QFont &f );
    void currentColorChanged( const QColor &c );
    void currentAlignmentChanged( int a );
    void cursorPositionChanged( Qt3::QTextCursor *c );
    void returnPressed();
    void modificationChanged( bool m );

protected:
    enum KeyboardAction { // keep in sync with QTextView
	ActionBackspace,
	ActionDelete,
	ActionReturn,
	ActionKill
    };

    enum MoveDirection { // keep in sync with QTextView
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	MoveHome,
	MoveEnd,
	MovePgUp,
	MovePgDown
    };

    void setFormat( Qt3::QTextFormat *f, int flags );
    void ensureCursorVisible();
    void placeCursor( const QPoint &pos, Qt3::QTextCursor *c = 0 );
    void moveCursor( MoveDirection direction, bool shift, bool control );
    void moveCursor( MoveDirection direction, bool control );
    void doKeyboardAction( KeyboardAction action );
    Qt3::QTextCursor *textCursor() const;

private:
    bool isReadOnly() const { return FALSE; }
    void emitUndoAvailable( bool b ) { emit undoAvailable( b ); }
    void emitRedoAvailable( bool b ) { emit redoAvailable( b ); }
    void emitCurrentFontChanged( const QFont &f ) { emit currentFontChanged( f ); }
    void emitCurrentColorChanged( const QColor &c ) { emit currentColorChanged( c ); }
    void emitCurrentAlignmentChanged( int a ) { emit currentAlignmentChanged( a ); }
    void emitCursorPositionChanged( Qt3::QTextCursor *c ) { emit cursorPositionChanged( c ); }
    void emitReturnPressed() { emit returnPressed(); }
    void emitModificationChanged( bool m ) { emit modificationChanged( m ); }

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextEdit( const QTextEdit & );
    QTextEdit& operator=( const QTextEdit & );
#endif
};

inline void QTextEdit::getCursorPosition( int &parag, int &index ) const
{
    Qt3::QTextView::getCursorPosition( parag, index );
}

inline bool QTextEdit::isModified() const
{
    return Qt3::QTextView::isModified();
}

inline bool QTextEdit::italic() const
{
    return Qt3::QTextView::italic();
}

inline bool QTextEdit::bold() const
{
    return Qt3::QTextView::bold();
}

inline bool QTextEdit::underline() const
{
    return Qt3::QTextView::underline();
}

inline QString QTextEdit::family() const
{
    return Qt3::QTextView::family();
}

inline int QTextEdit::pointSize() const
{
    return Qt3::QTextView::pointSize();
}

inline QColor QTextEdit::color() const
{
    return Qt3::QTextView::color();
}

inline QFont QTextEdit::font() const
{
    return Qt3::QTextView::font();
}

inline int QTextEdit::alignment() const
{
    return Qt3::QTextView::alignment();
}

inline bool QTextEdit::isOverwriteMode() const
{
    return Qt3::QTextView::isOverwriteMode();
}

inline int QTextEdit::undoDepth() const
{
    return Qt3::QTextView::undoDepth();
}

inline void QTextEdit::insert( const QString &text, bool indent, bool checkNewLine, bool removeSelected )
{
    Qt3::QTextView::insert( text, indent, checkNewLine, removeSelected );
}

inline void QTextEdit::setOverwriteMode( bool b )
{
    Qt3::QTextView::setOverwriteMode( b );
}

inline void QTextEdit::undo()
{
    Qt3::QTextView::undo();
}

inline void QTextEdit::redo()
{
    Qt3::QTextView::redo();
}

inline void QTextEdit::cut()
{
    Qt3::QTextView::cut();
}

inline void QTextEdit::paste()
{
    Qt3::QTextView::paste();
}

inline void QTextEdit::pasteSubType( const QCString &subtype )
{
    Qt3::QTextView::pasteSubType( subtype );
}

inline void QTextEdit::clear()
{
    Qt3::QTextView::clear();
}

inline void QTextEdit::del()
{
    Qt3::QTextView::del();
}

inline void QTextEdit::indent()
{
    Qt3::QTextView::indent();
}

inline void QTextEdit::setItalic( bool b )
{
    Qt3::QTextView::setItalic( b );
}

inline void QTextEdit::setBold( bool b )
{
    Qt3::QTextView::setBold( b );
}

inline void QTextEdit::setUnderline( bool b )
{
    Qt3::QTextView::setUnderline( b );
}

inline void QTextEdit::setFamily( const QString &f )
{
    Qt3::QTextView::setFamily( f );
}

inline void QTextEdit::setPointSize( int s )
{
    Qt3::QTextView::setPointSize( s );
}

inline void QTextEdit::setColor( const QColor &c )
{
    Qt3::QTextView::setColor( c );
}

inline void QTextEdit::setFont( const QFont &f )
{
    Qt3::QTextView::setFontInternal( f );
}

inline void QTextEdit::setAlignment( int a )
{
    Qt3::QTextView::setAlignment( a );
}

inline void QTextEdit::setParagType( Qt3::QStyleSheetItem::DisplayMode dm, Qt3::QStyleSheetItem::ListStyle listStyle )
{
    Qt3::QTextView::setParagType( dm, listStyle );
}

inline void QTextEdit::setCursorPosition( int parag, int index )
{
    Qt3::QTextView::setCursorPosition( parag, index );
}

inline void QTextEdit::setSelection( int parag_from, int index_from, int parag_to, int index_to, int selNum )
{
    Qt3::QTextView::setSelection( parag_from, index_from, parag_to, index_to, selNum );
}

inline void QTextEdit::setSelectionAttributes( int selNum, const QColor &back, bool invertText )
{
    Qt3::QTextView::setSelectionAttributes( selNum, back, invertText );
}

inline void QTextEdit::setModified( bool m )
{
    Qt3::QTextView::setModified( m );
}

inline void QTextEdit::resetFormat()
{
    Qt3::QTextView::resetFormat();
}

inline void QTextEdit::setUndoDepth( int d )
{
    Qt3::QTextView::setUndoDepth( d );
}

inline void QTextEdit::setFormat( Qt3::QTextFormat *f, int flags )
{
    Qt3::QTextView::setFormat( f, flags );
}

inline void QTextEdit::ensureCursorVisible()
{
    Qt3::QTextView::ensureCursorVisible();
}

inline void QTextEdit::placeCursor( const QPoint &pos, Qt3::QTextCursor *c )
{
    Qt3::QTextView::placeCursor( pos, c );
}

inline void QTextEdit::moveCursor( MoveDirection direction, bool shift, bool control )
{
    Qt3::QTextView::moveCursor( (MoveDirectionPrivate)direction, shift, control );
}

inline void QTextEdit::moveCursor( MoveDirection direction, bool control )
{
    Qt3::QTextView::moveCursor( (MoveDirectionPrivate)direction, control );
}

inline void QTextEdit::removeSelectedText()
{
    Qt3::QTextView::removeSelectedText();
}

inline void QTextEdit::doKeyboardAction( KeyboardAction action )
{
    Qt3::QTextView::doKeyboardAction( (KeyboardActionPrivate)action );
}

inline Qt3::QTextCursor *QTextEdit::textCursor() const
{
    return cursor;
}

inline bool QTextEdit::getFormat( int parag, int index, QFont &font, QColor &color )
{
    return Qt3::QTextView::getFormat( parag, index, font, color );
}

#endif // QT_NO_TEXTEDIT

#endif
