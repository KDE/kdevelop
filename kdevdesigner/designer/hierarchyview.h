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
#include <q3listview.h>
#include <qtabwidget.h>
#include <qpointer.h>
//Added by qt3to4:
#include <QPixmap>
#include <QCloseEvent>
#include <QKeyEvent>
#include <Q3PopupMenu>
#include <QMouseEvent>
#include <private/qcom_p.h>
#include "../interfaces/classbrowserinterface.h"

class FormWindow;
class QCloseEvent;
class Q3PopupMenu;
class QKeyEvent;
class QMouseEvent;
class Q3Wizard;
class SourceEditor;

class HierarchyItem : public Q3ListViewItem
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

    HierarchyItem( Type type, Q3ListViewItem *parent, Q3ListViewItem *after,
		   const QString &txt1, const QString &txt2, const QString &txt3 );
    HierarchyItem( Type type, Q3ListView *parent, Q3ListViewItem *after,
		   const QString &txt1, const QString &txt2, const QString &txt3 );

    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
    void updateBackColor();

    void setObject( QObject *o );
    QObject *object() const;

    void setText( int col, const QString &txt ) { if ( !txt.isEmpty() ) Q3ListViewItem::setText( col, txt ); }

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

class HierarchyList : public Q3ListView
{
    Q_OBJECT

public:
    HierarchyList( QWidget *parent, FormWindow *fw, bool doConnects = TRUE );

    virtual void setup();
    virtual void setCurrent( QObject *o );
    void setOpen( Q3ListViewItem *i, bool b );
    void changeNameOf( QObject *o, const QString &name );
    void changeDatabaseOf( QObject *o, const QString &info );
    void setFormWindow( FormWindow *fw ) { formWindow = fw; }
    void drawContentsOffset( QPainter *p, int ox, int oy,
			     int cx, int cy, int cw, int ch ) {
	setUpdatesEnabled( FALSE );
	triggerUpdate();
	setUpdatesEnabled( TRUE );
	Q3ListView::drawContentsOffset( p, ox, oy, cx, cy, cw, ch );
    }

    void insertEntry( Q3ListViewItem *i, const QPixmap &pix = QPixmap(), const QString &s = QString::null );

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseReleaseEvent( QMouseEvent *e );

public slots:
    void addTabPage();
    void removeTabPage();

private:
    void insertObject( QObject *o, Q3ListViewItem *parent );
    QObject *findObject( Q3ListViewItem *i );
    Q3ListViewItem *findItem( QObject *o );
    QObject *current() const;
    QObject *handleObjectClick( Q3ListViewItem *i );

private slots:
    virtual void objectClicked( Q3ListViewItem *i );
    virtual void objectDoubleClicked( Q3ListViewItem *i );
    virtual void showRMBMenu( Q3ListViewItem *, const QPoint & );

protected:
    FormWindow *formWindow;
    Q3PopupMenu *normalMenu, *tabWidgetMenu;
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
    void save( Q3ListViewItem *p, Q3ListViewItem *i );
    void execFunctionDialog( const QString &access, const QString &type, bool addFunc );
    void addVariable( const QString &varName, const QString &access );

private slots:
    void objectClicked( Q3ListViewItem *i );
    void showRMBMenu( Q3ListViewItem *, const QPoint & );
    void renamed( Q3ListViewItem *i );

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
    void pagesChanged( Q3Wizard *w );
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
	ClassBrowser( Q3ListView * = 0, ClassBrowserInterface * = 0 );
	~ClassBrowser();
	Q3ListView *lv;
	QInterfacePtr<ClassBrowserInterface> iface;

	Q_DUMMY_COMPARISON_OPERATOR( ClassBrowser )
    };
    FormWindow *formwindow;
    HierarchyList *listview;
    FormDefinitionView *fView;
    SourceEditor *editor;
    QMap<QString, ClassBrowser> *classBrowsers;
    QPointer<SourceEditor> lastSourceEditor;

};


#endif
