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

#ifndef HIRARCHYVIEW_H
#define HIRARCHYVIEW_H

#include <qvariant.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qguardedptr.h>
#include <private/qcom_p.h>
#include "../interfaces/classbrowserinterface.h"

class FormWindow;
class QCloseEvent;
class QPopupMenu;
class QKeyEvent;
class QMouseEvent;
class QWizard;
class SourceEditor;

class HierarchyItem : public QListViewItem
{
public:
    enum Type {
	Widget,
	SlotParent,
	SlotPublic,
	SlotProtected,
	SlotPrivate,
	Slot,
	DefinitionParent,
	Definition,
	Event,
	EventFunction,
	FunctParent,
	FunctPublic,
	FunctProtected,
	FunctPrivate,
	Function,
	VarParent,
	VarPublic,
	VarProtected,
	VarPrivate,
	Variable
    };

    HierarchyItem( Type type, QListViewItem *parent, QListViewItem *after,
		   const QString &txt1, const QString &txt2, const QString &txt3 );
    HierarchyItem( Type type, QListView *parent, QListViewItem *after,
		   const QString &txt1, const QString &txt2, const QString &txt3 );

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    void updateBackColor();

    void setObject( QObject *o );
    QObject *object() const;

    void setText( int col, const QString &txt ) { if ( !txt.isEmpty() ) QListViewItem::setText( col, txt ); }

    int rtti() const { return (int)typ; }

private:
    void okRename( int col );
    void cancelRename( int col );

private:
    QColor backgroundColor();
    QColor backColor;
    QObject *obj;
    Type typ;

};

class HierarchyList : public QListView
{
    Q_OBJECT

public:
    HierarchyList( QWidget *parent, FormWindow *fw, bool doConnects = TRUE );

    virtual void setup();
    virtual void setCurrent( QObject *o );
    void setOpen( QListViewItem *i, bool b );
    void changeNameOf( QObject *o, const QString &name );
    void changeDatabaseOf( QObject *o, const QString &info );
    void setFormWindow( FormWindow *fw ) { formWindow = fw; }
    void drawContentsOffset( QPainter *p, int ox, int oy,
			     int cx, int cy, int cw, int ch ) {
	setUpdatesEnabled( FALSE );
	triggerUpdate();
	setUpdatesEnabled( TRUE );
	QListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    }

    void insertEntry( QListViewItem *i, const QPixmap &pix = QPixmap(), const QString &s = QString::null );

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseReleaseEvent( QMouseEvent *e );

public slots:
    void addTabPage();
    void removeTabPage();

private:
    void insertObject( QObject *o, QListViewItem *parent );
    QObject *findObject( QListViewItem *i );
    QListViewItem *findItem( QObject *o );
    QObject *current() const;
    QObject *handleObjectClick( QListViewItem *i );

private slots:
    virtual void objectClicked( QListViewItem *i );
    virtual void objectDoubleClicked( QListViewItem *i );
    virtual void showRMBMenu( QListViewItem *, const QPoint & );

protected:
    FormWindow *formWindow;
    QPopupMenu *normalMenu, *tabWidgetMenu;
    bool deselect;

};

class FormDefinitionView : public HierarchyList
{
    Q_OBJECT

public:
    FormDefinitionView( QWidget *parent, FormWindow *fw );

    void setup();
    void setupVariables();
    void refresh();
    void setCurrent( QWidget *w );

protected:
    void contentsMouseDoubleClickEvent( QMouseEvent *e );

private:
    void save( QListViewItem *p, QListViewItem *i );
    void execFunctionDialog( const QString &access, const QString &type, bool addFunc );
    void addVariable( const QString &varName, const QString &access );

private slots:
    void objectClicked( QListViewItem *i );
    void showRMBMenu( QListViewItem *, const QPoint & );
    void renamed( QListViewItem *i );

private:
    bool popupOpen;
    HierarchyItem *itemSlots, *itemPrivate, *itemProtected, *itemPublic;
    HierarchyItem *itemFunct, *itemFunctPriv, *itemFunctProt, *itemFunctPubl;
    HierarchyItem *itemVar, *itemVarPriv, *itemVarProt, *itemVarPubl;
};


class HierarchyView : public QTabWidget
{
    Q_OBJECT

public:
    HierarchyView( QWidget *parent );
    ~HierarchyView();

    void setFormWindow( FormWindow *fw, QObject *o );
    FormWindow *formWindow() const;
    SourceEditor *sourceEditor() const { return editor; }
    void clear();

    void showClasses( SourceEditor *se );
    void updateClassBrowsers();

    void widgetInserted( QWidget *w );
    void widgetRemoved( QWidget *w );
    void widgetsInserted( const QWidgetList &l );
    void widgetsRemoved( const QWidgetList &l );
    void namePropertyChanged( QWidget *w, const QVariant &old );
    void databasePropertyChanged( QWidget *w, const QStringList& info );
    void tabsChanged( QTabWidget *w );
    void pagesChanged( QWizard *w );
    void rebuild();
    void closed( FormWindow *fw );
    void updateFormDefinitionView();

    FormDefinitionView *formDefinitionView() const { return fView; }
    HierarchyList *hierarchyList() const { return listview; }

protected slots:
    void jumpTo( const QString &func, const QString &clss,int type );
    void showClassesTimeout();

protected:
    void closeEvent( QCloseEvent *e );

signals:
    void hidden();

private:
    struct ClassBrowser
    {
	ClassBrowser( QListView * = 0, ClassBrowserInterface * = 0 );
	~ClassBrowser();
	QListView *lv;
	QInterfacePtr<ClassBrowserInterface> iface;

	Q_DUMMY_COMPARISON_OPERATOR( ClassBrowser )
    };
    FormWindow *formwindow;
    HierarchyList *listview;
    FormDefinitionView *fView;
    SourceEditor *editor;
    QMap<QString, ClassBrowser> *classBrowsers;
    QGuardedPtr<SourceEditor> lastSourceEditor;

};


#endif
