/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#ifndef ACTIONDND_H
#define ACTIONDND_H

#include <qaction.h>
#include <q3dragobject.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qpixmap.h>
#include <q3popupmenu.h>
#include <q3ptrlist.h>
#include <q3toolbar.h>
#include <qpointer.h>
//Added by qt3to4:
#include <Q3ActionGroup>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include "../shared/widgetdatabase.h"

//class QDesignerPopupMenu;
class QDesignerIndicatorWidget;
class FormWindow;
class Q3PopupMenu;

class QDesignerIndicatorWidget : public QWidget
{
    Q_OBJECT

public:
    QDesignerIndicatorWidget( QWidget *p )
	: QWidget( p, "qt_dockwidget_internal" ) {
	    setBackgroundColor( red );
    }

};

class ActionDrag : public Q3StoredDrag
{
    Q_OBJECT

public:
    ActionDrag(const QString &type, QAction *action, QWidget *source);
    ActionDrag(QAction *action, QWidget *source);
    ActionDrag(Q3ActionGroup *group, QWidget *source);

    ~ActionDrag();

    static bool canDecode(QDropEvent *e);

    static QAction *action()
    {
	return the_action;
    }

private:
    static QAction *the_action;
};

class QDesignerActionGroup : public Q3ActionGroup
{
    Q_OBJECT

public:
    QDesignerActionGroup( QObject *parent )
	: Q3ActionGroup( ::qt_cast<Q3ActionGroup*>(parent) ? parent : 0 ), wid( 0 ), idx( -1 ) { init(); }

    void init();

    QWidget *widget() const { return wid; }
    QWidget *widget( QAction *a ) const { return *widgets.find( a ); }
    int index() const { return idx; }

protected:
#if !defined(Q_NO_USING_KEYWORD)
    using Q3ActionGroup::addedTo;
#endif
    void addedTo( QWidget *w, QWidget * ) {
	wid = w;
    }
    void addedTo( QWidget *w, QWidget *, QAction *a ) {
	widgets.insert( a, w );
    }
    void addedTo( int index, Q3PopupMenu * ) {
	idx = index;
    }

private:
    QWidget *wid;
    QMap<QAction *, QWidget *> widgets;
    int idx;

};

class QDesignerAction : public QAction
{
    Q_OBJECT

public:
    QDesignerAction( QObject *parent )
	: QAction( ::qt_cast<Q3ActionGroup*>(parent) ? parent : 0 ), wid( 0 ), idx( -1 ), widgetToInsert( 0 ) { init(); }
    QDesignerAction( QWidget *w, QObject *parent )
	: QAction( ::qt_cast<Q3ActionGroup*>(parent) ? parent : 0 ), wid( 0 ), idx( -1 ), widgetToInsert( w ) { init(); }

    void init();

    QWidget *widget() const { return wid; }
    int index() const { return idx; }

    bool addTo( QWidget *w );
    bool removeFrom( QWidget *w );

    void remove();
    bool supportsMenu() const { return !widgetToInsert; }

protected:
    void addedTo( QWidget *w, QWidget * ) {
	wid = w;
    }
    void addedTo( int index, Q3PopupMenu * ) {
	idx = index;
    }

private:
    QWidget *wid;
    int idx;
    QWidget *widgetToInsert;

};

class QDesignerToolBarSeparator : public QWidget
{
    Q_OBJECT

public:
    QDesignerToolBarSeparator( Qt::Orientation, Q3ToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    Qt::Orientation orientation() const { return orient; }
public slots:
   void setOrientation( Qt::Orientation );
protected:
    void styleChange( QStyle& );
    void paintEvent( QPaintEvent * );
private:
    Qt::Orientation orient;
};


class QSeparatorAction : public QAction
{
    Q_OBJECT

public:
    QSeparatorAction( QObject *parent );

    bool addTo( QWidget *w );
    bool removeFrom( QWidget *w );
    QWidget *widget() const;
    int index() const;

private:
    QWidget *wid;
    int idx;

};


class QDesignerToolBar : public Q3ToolBar
{
    Q_OBJECT

public:
    QDesignerToolBar( Q3MainWindow *mw );
    QDesignerToolBar( Q3MainWindow *mw, Qt::ToolBarDock dock );
    Q3PtrList<QAction> insertedActions() const { return actionList; }
    void addAction( QAction *a );

    void clear();
    void installEventFilters( QWidget *w );
    void insertAction( QWidget *w, QAction *a ) { actionMap.insert( w, a ); }
    void insertAction( int index, QAction *a ) { actionList.insert( index, a ); }
    void appendAction( QAction *a ) { actionList.append( a ); }
    void removeAction( QAction *a ) { actionList.remove( a ); }
    void reInsert();
    void removeWidget( QWidget *w );

protected:
    bool eventFilter( QObject *, QEvent * );
    void paintEvent( QPaintEvent * );
#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
#endif
    void contextMenuEvent( QContextMenuEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private slots:
    void actionRemoved();

private:
    void drawIndicator( const QPoint &pos );
    QPoint calcIndicatorPos( const QPoint &pos );
    void buttonContextMenuEvent( QContextMenuEvent *e, QObject *o );
    void buttonMousePressEvent( QMouseEvent *e, QObject *o );
    void buttonMouseMoveEvent( QMouseEvent *e, QObject *o );
    void buttonMouseReleaseEvent( QMouseEvent *e, QObject *o );
    void doInsertWidget( const QPoint &p );
    void findFormWindow();

private:
    QPoint lastIndicatorPos;
    QWidget *insertAnchor;
    bool afterAnchor;
    Q3PtrList<QAction> actionList;
    QMap<QWidget*, QAction*> actionMap;
    QPoint dragStartPos;
    QDesignerIndicatorWidget *indicator;
    bool widgetInserting;
    FormWindow *formWindow;

};

#endif
