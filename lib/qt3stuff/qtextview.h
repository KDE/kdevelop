/****************************************************************************
** $Id$
**
** Definition of the QTextView class
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

#ifndef QTEXTVIEW_H
#define QTEXTVIEW_H

#ifndef QT_H
#include "qscrollview.h"
#include "qstylesheet.h"
#include "qpainter.h"
#include "qvector.h"
#include "qvaluelist.h"
#endif // QT_H

class QPainter;
class QKeyEvent;
class QResizeEvent;
class QMouseEvent;
class QTimer;
class QFont;
class QColor;
class QPopupMenu;
class QTextEdit;

namespace Qt3 {

    class QTextString;
    class QTextCommand;
    class QTextParag;
    class QTextFormat;
    class QTextDocument;
    class QTextCursor;
    class QTextViewPrivate;
    class QTextBrowser;
    struct QUndoRedoInfoPrivate;

class Q_EXPORT QTextView : public QScrollView
{
    friend class ::QTextEdit;
    friend class QTextBrowser;

    Q_OBJECT
    Q_ENUMS( WordWrap WrapPolicy )
    Q_PROPERTY( TextFormat textFormat READ textFormat WRITE setTextFormat )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QBrush paper READ paper WRITE setPaper )
    Q_PROPERTY( bool linkUnderline READ linkUnderline WRITE setLinkUnderline )
    Q_PROPERTY( QString documentTitle READ documentTitle )
    Q_PROPERTY( int length READ length )
    Q_PROPERTY( WordWrap wordWrap READ wordWrap WRITE setWordWrap )
    Q_PROPERTY( int wrapColumnOrWidth READ wrapColumnOrWidth WRITE setWrapColumnOrWidth )
    Q_PROPERTY( WrapPolicy wrapPolicy READ wrapPolicy WRITE setWrapPolicy )

public:
    enum WordWrap {
	NoWrap,
	WidgetWidth,
	FixedPixelWidth,
	FixedColumnWidth
    };

    enum WrapPolicy {
	AtWhiteSpace,
	Anywhere
    };

    QTextView( const QString& text, const QString& context = QString::null,
	       QWidget *parent=0, const char *name=0);
    QTextView( QWidget *parent = 0, const char *name = 0 );
    virtual ~QTextView();
    void setPalette( const QPalette & );
    void setFont( const QFont &f );

    QString text() const;
    QString text( int para ) const;
    TextFormat textFormat() const;
    QString context() const;
    QString documentTitle() const;

    void getSelection( int &paraFrom, int &indexFrom,
		    int &paraTo, int &indexTo, int selNum = 0 ) const;
    virtual bool find( const QString &expr, bool cs, bool wo, bool forward = TRUE,
		       int *para = 0, int *index = 0 );

    void append( const QString& text );

    int paragraphs() const;
    int lines() const;
    int linesOfParagraph( int para ) const;
    int lineOfChar( int para, int chr );
    int length() const;

    Qt3::QStyleSheet* styleSheet() const;
    QMimeSourceFactory* mimeSourceFactory() const;

    QBrush paper() const;
    bool linkUnderline() const;

    int heightForWidth( int w ) const;

    bool hasSelectedText() const;
    QString selectedText() const;

    WordWrap wordWrap() const;
    int wrapColumnOrWidth() const;
    WrapPolicy wrapPolicy() const;

    int tabStopWidth() const;
    void setHScrollBarMode( ScrollBarMode );

    QString anchorAt( const QPoint& pos );

    QSize sizeHint() const;

public slots:
    void setEnabled( bool );
    virtual void setMimeSourceFactory( QMimeSourceFactory* factory );
    virtual void setStyleSheet( Qt3::QStyleSheet* styleSheet );
    virtual void scrollToAnchor( const QString& name );
    virtual void setPaper( const QBrush& pap );
    virtual void setLinkUnderline( bool );

    virtual void setWordWrap( WordWrap mode );
    virtual void setWrapColumnOrWidth( int );
    virtual void setWrapPolicy( WrapPolicy policy );

    virtual void copy();

    void setText( const QString &txt ) { setText( txt, QString::null ); }
    virtual void setTextFormat( TextFormat f );
    virtual void setText( const QString &txt, const QString &context );

    virtual void selectAll( bool select = TRUE );
    virtual void setTabStops( int ts );
    virtual void zoomIn( int range );
    virtual void zoomOut( int range );
    virtual void zoomIn() { zoomIn( 1 ); }
    virtual void zoomOut() { zoomOut( 1 ); }

    virtual void sync();

signals:
    void textChanged();
    void selectionChanged();
    void copyAvailable( bool );

protected:
    void repaintChanged();
    void updateStyles();
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    bool event( QEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void resizeEvent( QResizeEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    void contentsWheelEvent( QWheelEvent *e );
#ifndef QT_NO_DRAGANDDROP
    void contentsDragEnterEvent( QDragEnterEvent *e );
    void contentsDragMoveEvent( QDragMoveEvent *e );
    void contentsDragLeaveEvent( QDragLeaveEvent *e );
    void contentsDropEvent( QDropEvent *e );
#endif
    // void contentsContextMenuEvent( QContextMenuEvent *e ); QT2HACK
    bool eventFilter( QObject *o, QEvent *e );
    bool focusNextPrevChild( bool next );
    QTextDocument *document() const;
    void setDocument( QTextDocument *doc );
    virtual QPopupMenu *createPopupMenu();

protected slots:
    virtual void doChangeInterval();

private slots:
    void formatMore();
    void doResize();
    void doAutoScroll();
    void blinkCursor();
    void setModified();
    void startDrag();
    void setRealWidth( int w );

private:
    struct Q_EXPORT UndoRedoInfo {
	enum Type { Invalid, Insert, Delete, Backspace, Return, RemoveSelected, Format, Alignment, ParagType };

	UndoRedoInfo( QTextDocument *dc );
	~UndoRedoInfo();
	void clear();
	bool valid() const;

    	QUndoRedoInfoPrivate *d;
	int id;
	int index;
	int eid;
	int eindex;
	QTextFormat *format;
	int flags;
	Type type;
	QTextDocument *doc;
	QArray<int> oldAligns;
	int newAlign;
	bool list;
	Qt3::QStyleSheetItem::ListStyle listStyle;
	QValueList< QVector<Qt3::QStyleSheetItem> > oldStyles;
	QValueList<Qt3::QStyleSheetItem::ListStyle> oldListStyles;
    };

    enum KeyboardActionPrivate { // keep in sync with QTextEdit
	ActionBackspace,
	ActionDelete,
	ActionReturn,
	ActionKill
    };

    enum MoveDirectionPrivate { // keep in sync with QTextEdit
	MoveLeft,
	MoveRight,
	MoveUp,
	MoveDown,
	MoveHome,
	MoveEnd,
	MovePgUp,
	MovePgDown
    };

private:
    virtual bool isReadOnly() const { return TRUE; }
    virtual bool linksEnabled() const { return FALSE; }
    void init();
    void drawCursor( bool visible );
    void checkUndoRedoInfo( UndoRedoInfo::Type t );
    void updateCurrentFormat();
    void handleReadOnlyKeyEvent( QKeyEvent *e );
    void makeParagVisible( QTextParag *p );
#ifndef QT_NO_MIME
    QCString pickSpecial(QMimeSource* ms, bool always_ask, const QPoint&);
#endif
#ifndef QT_NO_MIMECLIPBOARD
    void pasteSpecial(const QPoint&);
#endif

private: // these are functions which actually do editing stuff, but
 // are implemented here as it is easier to implement viewing and
 // editing in the same class. In QTextEdit these functions are made
 // public
    virtual void emitUndoAvailable( bool ) {}
    virtual void emitRedoAvailable( bool ) {}
    virtual void emitCurrentFontChanged( const QFont & ) {}
    virtual void emitCurrentColorChanged( const QColor & ) {}
    virtual void emitCurrentAlignmentChanged( int ) {}
    virtual void emitCursorPositionChanged( QTextCursor * ) {}
    virtual void emitReturnPressed() {}
    virtual void emitHighlighted( const QString & ) {}
    virtual void emitLinkClicked( const QString & ) {}
    virtual void emitModificationChanged( bool ) {}

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
    void setOverwriteMode( bool b ) { overWrite = b; }
    bool isOverwriteMode() const { return overWrite; }
    int undoDepth() const;
    void undo();
    void redo();
    void cut();
    void paste();
    void del();
    void clear();
    void pasteSubType( const QCString &subtype );
    void indent();
    void setItalic( bool b );
    void setBold( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setColor( const QColor &c );
    void setFontInternal( const QFont &f );
    void setAlignment( int );
    void setParagType( QStyleSheetItem::DisplayMode, QStyleSheetItem::ListStyle listStyle );
    void setCursorPosition( int parag, int index );
    void setSelection( int parag_from, int index_from,
			       int parag_to, int index_to, int selNum = 0 );
    void setSelectionAttributes( int selNum, const QColor &back, bool invertText );
    void setModified( bool m );
    void resetFormat();
    void setUndoDepth( int d );
    void setFormat( QTextFormat *f, int flags );
    void ensureCursorVisible();
    void placeCursor( const QPoint &pos, QTextCursor *c = 0 );
    void moveCursor( MoveDirectionPrivate direction, bool shift, bool control );
    void moveCursor( MoveDirectionPrivate direction, bool control );
    void removeSelectedText();
    void doKeyboardAction( KeyboardActionPrivate action );
    void insert( const QString &text, bool indent = FALSE, bool checkNewLine = TRUE, bool removeSelected = TRUE );
    void readFormats( QTextCursor &c1, QTextCursor &c2, int oldLen, QTextString &text, bool fillStyles = FALSE );
    void clearUndoRedo();
    bool getFormat( int parag, int index, QFont &font, QColor &color );
    void paintDocument( bool drawAll, QPainter *p, int cx = -1, int cy = -1, int cw = -1, int ch = -1 );

private:
    QTextDocument *doc;
    QTextCursor *cursor;
    bool mousePressed;
    QTimer *formatTimer, *scrollTimer, *changeIntervalTimer, *blinkTimer, *dragStartTimer, *resizeTimer;
    QTextParag *lastFormatted;
    int interval;
    UndoRedoInfo undoRedoInfo;
    QTextFormat *currentFormat;
    int currentAlignment;
    bool inDoubleClick;
    QPoint oldMousePos, mousePos;
    bool cursorVisible, blinkCursorVisible;
    bool readOnly, modified, mightStartDrag;
    QPoint dragStartPos;
    QString onLink;
    bool overWrite;
    WordWrap wrapMode;
    WrapPolicy wPolicy;
    int wrapWidth;
    QScrollView::ScrollBarMode setMode;
    QString pressedLink;
    QTextViewPrivate *d;
    bool inDnD;

};

inline QTextDocument *QTextView::document() const
{
    return doc;
}

}; // namespace

#endif
