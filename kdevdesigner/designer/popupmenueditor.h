/**********************************************************************
** Copyright (C) 2003 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef POPUPMENUEDITOR_H
#define POPUPMENUEDITOR_H

#include <qwidget.h>
#include <q3ptrlist.h>
#include <qaction.h>
//Added by qt3to4:
#include <Q3ActionGroup>
#include <QDragLeaveEvent>
#include <QFocusEvent>
#include <QPaintEvent>
#include <QDragMoveEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

class PopupMenuEditor;
class QMenuItem;

class PopupMenuEditorItem : public QObject
{
    Q_OBJECT

    friend class PopupMenuEditor;

    PopupMenuEditorItem( PopupMenuEditor * menu = 0, QObject * parent = 0, const char * name = 0 );

public:
    enum ItemType {
	Unknown = -1,
	Separator = 0,
	Action = 1
    };

    PopupMenuEditorItem( QAction * action, PopupMenuEditor * menu,
			 QObject * parent = 0, const char * name = 0 );
    PopupMenuEditorItem( PopupMenuEditorItem * item, PopupMenuEditor * menu,
			 QObject * parent = 0, const char * name = 0 );
    ~PopupMenuEditorItem();

    void init();

    ItemType type() const;
    QAction * action() const { return a; }

    void setVisible( bool enable );
    bool isVisible() const;

    void setSeparator( bool enable ) { separator = enable; }
    bool isSeparator() const { return separator; }

    void setRemovable( bool enable ) { removable = enable; }
    bool isRemovable() const { return removable; }
    
    void showMenu( int x, int y );
    void hideMenu();
    void focusOnMenu();
    PopupMenuEditor * subMenu() const { return s; }

    int count() const;

    bool eventFilter( QObject *, QEvent * event );
    
public slots:
    void selfDestruct();

protected:

private:
    QAction * a;
    PopupMenuEditor * s;
    PopupMenuEditor * m;
    uint separator : 1;
    uint removable : 1;
};

class FormWindow;
class MainWindow;
class QLineEdit;

#include <q3popupmenu.h>

class PopupMenuEditor : public QWidget
{
    Q_OBJECT

    friend class PopupMenuEditorItem;
    friend class MenuBarEditor;
    friend class Resource;

public:
    PopupMenuEditor( FormWindow * fw, QWidget * parent = 0, const char * name = 0 );
    PopupMenuEditor( FormWindow * fw, PopupMenuEditor * menu, QWidget * parent, const char * name = 0 );
    ~PopupMenuEditor();

    void init();

    void insert( PopupMenuEditorItem * item, int index = -1 );
    void insert( QAction * action, int index = -1 );
    void insert( Q3ActionGroup * actionGroup, int index = -1 );
    int find( const QAction * action );
    int find( PopupMenuEditor * menu );
    int count();
    PopupMenuEditorItem * at( int index );
    PopupMenuEditorItem * at( QPoint pos ) { return itemAt( pos.y() ); }
    void exchange( int a, int b );

    void cut( int index );
    void copy( int index );
    void paste( int index );

    void insertedActions( Q3PtrList<QAction> & list );

    void show();
    void choosePixmap( int index = -1 );
    void showLineEdit( int index = -1);
    void setAccelerator( int key, Qt::ButtonState state, int index = -1 );

    FormWindow * formWindow() { return formWnd; }
    bool isCreatingAccelerator() { return ( currentField == 2 ); }

    Q3PtrList<PopupMenuEditorItem> * items() { return &itemList; }

    QWidget * parentEditor() { return parentMenu; }

signals:
    void inserted( QAction * );
    void removed(  QAction * );
    
public slots:

    void cut() { cut( currentIndex ); }
    void copy() { copy( currentIndex ); }
    void paste() { paste( currentIndex ); }

    void remove( int index );
    void remove( QAction * a ) { remove( find( a ) ); }

    void resizeToContents();
    void showSubMenu();
    void hideSubMenu();
    void focusOnSubMenu();
    
protected:
    PopupMenuEditorItem * createItem( QAction * a = 0 );
    void removeItem( int index = -1 );
    PopupMenuEditorItem * currentItem();
    PopupMenuEditorItem * itemAt( int y );
    void setFocusAt( const QPoint & pos );

    bool eventFilter( QObject * o, QEvent * e );
    void paintEvent( QPaintEvent * e );
    void mousePressEvent( QMouseEvent * e );
    void mouseDoubleClickEvent( QMouseEvent * e );
    void mouseMoveEvent( QMouseEvent * e );
    void dragEnterEvent( QDragEnterEvent * e );
    void dragLeaveEvent( QDragLeaveEvent * e );
    void dragMoveEvent( QDragMoveEvent * e );
    void dropEvent( QDropEvent * e );
    void keyPressEvent( QKeyEvent * e );
    void focusInEvent( QFocusEvent * e );
    void focusOutEvent( QFocusEvent * e );

    void drawItems( QPainter * p );
    void drawItem( QPainter * p, PopupMenuEditorItem * i, const QRect & r, int f ) const;
    void drawWinFocusRect( QPainter * p, const QRect & r ) const;

    QSize contentsSize();
    int itemHeight( const PopupMenuEditorItem * item ) const;
    int itemPos( const PopupMenuEditorItem * item ) const;
    
    int snapToItem( int y );
    void dropInPlace( PopupMenuEditorItem * i, int y );
    void dropInPlace( Q3ActionGroup * g, int y );

    void safeDec();
    void safeInc();

    void clearCurrentField();
    void navigateUp( bool ctrl );
    void navigateDown( bool ctrl );
    void navigateLeft();
    void navigateRight();
    void enterEditMode( QKeyEvent * e );
    void leaveEditMode( QKeyEvent * e );

    QString constructName( PopupMenuEditorItem *item );

private:
    FormWindow * formWnd;
    QLineEdit * lineEdit;
    QWidget * dropLine;
    Q3PtrList<PopupMenuEditorItem> itemList;
    PopupMenuEditorItem addItem;
    PopupMenuEditorItem addSeparator;
    QWidget * parentMenu;

    int iconWidth;
    int textWidth;
    int accelWidth;
    int arrowWidth;
    int borderSize;
    
    int currentField;
    int currentIndex;
    QPoint mousePressPos;
    static PopupMenuEditorItem * draggedItem;

    enum ClipboardOperation {
	None = 0,
	Cut = 1,
	Copy = 2
    };
    static int clipboardOperation;
    static PopupMenuEditorItem * clipboardItem;
};

#endif //POPUPMENUEDITOR_H
