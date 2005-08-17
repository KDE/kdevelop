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

#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <qvariant.h>
#include <qicon.h>
#include <qstring.h>
#include <q3intdict.h>
#include <qtabwidget.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qpainter.h>
#include <qevent.h>
#include <qobject.h>
#include <qlabel.h>
#include <q3wizard.h>
#include <q3ptrdict.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <q3widgetstack.h>
#include <qpointer.h>
#include <qtoolbox.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <Q3Frame>
#include <Q3CString>
#include <Q3PtrList>

#include "metadatabase.h"
#include "qwidgetfactory.h"

class QWidget;
class QLayout;
class FormWindow;

class CustomWidgetFactory : public QWidgetFactory
{
public:
    CustomWidgetFactory();
    QWidget *createWidget( const QString &className, QWidget *parent, const char *name ) const;

};

class WidgetFactory : public Qt
{
    friend class CustomWidgetFactory;

public:
    enum LayoutType {
	HBox,
	VBox,
	Grid,
	NoLayout
    };

    static QWidget *create( int id, QWidget *parent, const char *name = 0, bool init = TRUE,
			    const QRect *rect = 0, Qt::Orientation orient = Qt::Horizontal );
    static QLayout *createLayout( QWidget *widget, QLayout* layout, LayoutType type );
    static void deleteLayout( QWidget *widget );

    static LayoutType layoutType( QWidget *w );
    static LayoutType layoutType( QWidget *w, QLayout *&layout );
    static LayoutType layoutType( QLayout *layout );
    static QWidget *layoutParent( QLayout *layout );

    static QWidget* containerOfWidget( QWidget *w );
    static QWidget* widgetOfContainer( QWidget *w );

    static bool isPassiveInteractor( QObject* o );
    static const char* classNameOf( QObject* o );

    static void initChangedProperties( QObject *o );

    static bool hasSpecialEditor( int id, QObject *editorWidget );
    static bool hasItems( int id, QObject *editorWidget );
    static void editWidget( int id, QWidget *parent, QWidget *editWidget, FormWindow *fw );

    static bool canResetProperty( QObject *w, const QString &propName );
    static bool resetProperty( QObject *w, const QString &propName );
    static QVariant defaultValue( QObject *w, const QString &propName );
    static QString defaultCurrentItem( QObject *w, const QString &propName );

    static QVariant property( QObject *w, const char *name );
    static void saveDefaultProperties( QObject *w, int id );
    static void saveChangedProperties( QObject *w, int id );

    static QString defaultSignal( QObject *w );

private:
    static QWidget *createWidget( const QString &className, QWidget *parent, const char *name, bool init,
				  const QRect *r = 0, Qt::Orientation orient = Qt::Horizontal );
    static QWidget *createCustomWidget( QWidget *parent, const char *name, MetaDataBase::CustomWidget *w );

    static QPointer<QObject> *lastPassiveInteractor;
    static bool lastWasAPassiveInteractor;
};


class QDesignerTabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY( int currentPage READ currentPage WRITE setCurrentPage STORED false DESIGNABLE true )
    Q_PROPERTY( QString pageTitle READ pageTitle WRITE setPageTitle STORED false DESIGNABLE true )
    Q_PROPERTY( Q3CString pageName READ pageName WRITE setPageName STORED false DESIGNABLE true )
public:
    QDesignerTabWidget( QWidget *parent, const char *name );

    int currentPage() const;
    void setCurrentPage( int i );
    QString pageTitle() const;
    void setPageTitle( const QString& title );
    Q3CString pageName() const;
    void setPageName( const Q3CString& name );

    int count() const;
    QTabBar *tabBar() const { return QTabWidget::tabBar(); }

    bool eventFilter( QObject*, QEvent* );

private:
    QPoint pressPoint;
    QWidget *dropIndicator;
    QWidget *dragPage;
    QString dragLabel;
     bool mousePressed;
};

class QDesignerWidgetStack : public Q3WidgetStack
{
    Q_OBJECT
    Q_PROPERTY( int currentPage READ currentPage WRITE setCurrentPage STORED false DESIGNABLE true )
    Q_PROPERTY( Q3CString pageName READ pageName WRITE setPageName STORED false DESIGNABLE true )
public:
    QDesignerWidgetStack( QWidget *parent, const char *name );

    int currentPage() const;
    void setCurrentPage( int i );
    Q3CString pageName() const;
    void setPageName( const Q3CString& name );

    int count() const;
    QWidget* page( int i ) const;

    int insertPage( QWidget *p, int i = -1 );
    int removePage( QWidget *p );

public slots:
    void updateButtons();

protected:
    void resizeEvent( QResizeEvent *e ) {
	Q3WidgetStack::resizeEvent( e );
	updateButtons();
    }

    void showEvent( QShowEvent *e ) {
	Q3WidgetStack::showEvent( e );
	updateButtons();
    }

private slots:
    void prevPage();
    void nextPage();

private:
    Q3PtrList<QWidget> pages;
    QToolButton *prev, *next;

};

class QDesignerWizard : public Q3Wizard
{
    Q_OBJECT
    Q_PROPERTY( int currentPage READ currentPageNum WRITE setCurrentPage STORED false DESIGNABLE true )
    Q_PROPERTY( QString pageTitle READ pageTitle WRITE setPageTitle STORED false DESIGNABLE true )
    Q_PROPERTY( Q3CString pageName READ pageName WRITE setPageName STORED false DESIGNABLE true )
    Q_OVERRIDE( bool modal READ isModal WRITE setModal )

public:
    QDesignerWizard( QWidget *parent, const char *name ) 
        : Q3Wizard( parent, name ), modal(FALSE) {}

    int currentPageNum() const;
    void setCurrentPage( int i );
    QString pageTitle() const;
    void setPageTitle( const QString& title );
    Q3CString pageName() const;
    void setPageName( const Q3CString& name );
    int pageNum( QWidget *page );
    void addPage( QWidget *p, const QString & );
    void removePage( QWidget *p );
    void insertPage( QWidget *p, const QString &t, int index );
    bool isPageRemoved( QWidget *p ) { return (removedPages.find( p ) != 0); }

    bool isModal() const { return modal; }
    void setModal(bool b) { modal = b; }

    void reject() {}

private:
    struct Page
    {
	Page( QWidget *a, const QString &b ) : p( a ), t( b ) {}
	Page() : p( 0 ), t( QString::null ) {}
	QWidget *p;
	QString t;
    };
    Q3PtrDict<QWidget> removedPages;
    bool modal;

};

class QLayoutWidget : public QWidget
{
    Q_OBJECT

public:
    QLayoutWidget( QWidget *parent, const char *name ) : QWidget( parent, name ), sp( QWidget::sizePolicy() ) {}

    QSizePolicy sizePolicy() const;
    void updateSizePolicy();

protected:
    void paintEvent( QPaintEvent * );
    bool event( QEvent * );
    QSizePolicy sp;

};


class CustomWidget : public QWidget
{
    Q_OBJECT

public:
    CustomWidget( QWidget *parent, const char *name, MetaDataBase::CustomWidget *cw )
	: QWidget( parent, name ), cusw( cw ) {
	    alwaysExpand = parentWidget() && parentWidget()->inherits( "FormWindow" );
	    setSizePolicy( cw->sizePolicy );
	    if ( !alwaysExpand )
		setBackgroundMode( PaletteDark );
    }

    QSize sizeHint() const {
	QSize sh = cusw->sizeHint;
	if ( sh.isValid() )
	    return sh;
	return QWidget::sizeHint();
    }

    QString realClassName() { return cusw->className; }
    MetaDataBase::CustomWidget *customWidget() const { return cusw; }

protected:
    void paintEvent( QPaintEvent *e );

    MetaDataBase::CustomWidget *cusw;
    bool alwaysExpand;

};


class Line : public Q3Frame
{
    Q_OBJECT

    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
    Q_OVERRIDE( int frameWidth DESIGNABLE false )
    Q_OVERRIDE( Shape frameShape DESIGNABLE false )
    Q_OVERRIDE( QRect frameRect DESIGNABLE false )
    Q_OVERRIDE( QRect contentsRect DESIGNABLE false )
public:
    Line( QWidget *parent, const char *name )
	: Q3Frame( parent, name, Qt::WMouseNoMask ) {
	    setFrameStyle( HLine | Sunken );
    }

    void setOrientation( Qt::Orientation orient ) {
	if ( orient == Qt::Horizontal )
	    setFrameShape( HLine );
	else
	    setFrameShape( VLine );
    }
    Qt::Orientation orientation() const {
	return frameShape() == HLine ? Qt::Horizontal : Qt::Vertical;
    }
};

class QDesignerLabel : public QLabel
{
    Q_OBJECT

    Q_PROPERTY( Q3CString buddy READ buddyWidget WRITE setBuddyWidget )

public:
    QDesignerLabel( QWidget *parent = 0, const char *name = 0 )
	: QLabel( parent, name ) { myBuddy = 0; }

    void setBuddyWidget( const Q3CString &b ) {
	myBuddy = b;
	updateBuddy();
    }
    Q3CString buddyWidget() const {
	return myBuddy;
    };

protected:
    void showEvent( QShowEvent *e ) {
	QLabel::showEvent( e );
	updateBuddy();
    }


private:
    void updateBuddy();

    Q3CString myBuddy;

};

class QDesignerWidget : public QWidget
{
    Q_OBJECT

public:
    QDesignerWidget( FormWindow *fw, QWidget *parent, const char *name )
	: QWidget( parent, name, Qt::WResizeNoErase ), formwindow( fw ) {
	    need_frame = parent && parent->inherits("QDesignerWidgetStack" );
    }

protected:
    void resizeEvent( QResizeEvent* e);
    void paintEvent( QPaintEvent *e );

private:
    FormWindow *formwindow;
    uint need_frame : 1;

};

class QDesignerDialog : public QDialog
{
    Q_OBJECT
    Q_OVERRIDE( bool modal READ isModal WRITE setModal )

public:
    QDesignerDialog( FormWindow *fw, QWidget *parent, const char *name )
	: QDialog( parent, name, FALSE, Qt::WResizeNoErase ), formwindow( fw ), modal(FALSE) {}

    bool isModal() const { return modal; }
    void setModal(bool b) { modal = b; }

protected:
    void paintEvent( QPaintEvent *e );

private:
    FormWindow *formwindow;
    bool modal;

};

class QDesignerToolButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( int buttonGroupId READ buttonGroupId WRITE setButtonGroupId )

public:
    QDesignerToolButton( QWidget *parent, const char *name )
	: QToolButton( parent, name ) {}

    bool isInButtonGroup() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" );
    }
    int buttonGroupId() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ? ( (Q3ButtonGroup*)parentWidget() )->id( (Q3Button*)this ) : -1;
    }
    void setButtonGroupId( int id ) {
	if ( parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ) {
	    ( (Q3ButtonGroup*)parentWidget() )->remove( this );
	    ( (Q3ButtonGroup*)parentWidget() )->insert( this, id );
	}
    }
};

class QDesignerRadioButton : public QRadioButton
{
    Q_OBJECT
    Q_PROPERTY( int buttonGroupId READ buttonGroupId WRITE setButtonGroupId )

public:
    QDesignerRadioButton( QWidget *parent, const char *name )
	: QRadioButton( parent, name ) {}

    bool isInButtonGroup() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" );
    }
    int buttonGroupId() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ? ( (Q3ButtonGroup*)parentWidget() )->id( (Q3Button*)this ) : -1;
    }
    void setButtonGroupId( int id ) {
	if ( parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ) {
	    ( (Q3ButtonGroup*)parentWidget() )->remove( this );
	    ( (Q3ButtonGroup*)parentWidget() )->insert( this, id );
	}
    }

};

class QDesignerPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( int buttonGroupId READ buttonGroupId WRITE setButtonGroupId )

public:
    QDesignerPushButton( QWidget *parent, const char *name )
	: QPushButton( parent, name ) {}

    bool isInButtonGroup() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" );
    }
    int buttonGroupId() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ? ( (Q3ButtonGroup*)parentWidget() )->id( (Q3Button*)this ) : -1;
    }
    void setButtonGroupId( int id ) {
	if ( parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ) {
	    ( (Q3ButtonGroup*)parentWidget() )->remove( this );
	    ( (Q3ButtonGroup*)parentWidget() )->insert( this, id );
	}
    }

};

class QDesignerCheckBox : public QCheckBox
{
    Q_OBJECT
    Q_PROPERTY( int buttonGroupId READ buttonGroupId WRITE setButtonGroupId )

public:
    QDesignerCheckBox( QWidget *parent, const char *name )
	: QCheckBox( parent, name ) {}

    bool isInButtonGroup() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" );
    }
    int buttonGroupId() const {
	return parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ? ( (Q3ButtonGroup*)parentWidget() )->id( (Q3Button*)this ) : -1;
    }
    void setButtonGroupId( int id ) {
	if ( parentWidget() && parentWidget()->inherits( "QButtonGroup" ) ) {
	    ( (Q3ButtonGroup*)parentWidget() )->remove( this );
	    ( (Q3ButtonGroup*)parentWidget() )->insert( this, id );
	}
    }

};

class QDesignerToolBox : public QToolBox
{
    Q_OBJECT
    Q_PROPERTY( QString itemLabel READ itemLabel WRITE setItemLabel STORED false DESIGNABLE true )
    Q_PROPERTY( Q3CString itemName READ itemName WRITE setItemName STORED false DESIGNABLE true )
    Q_PROPERTY( Qt::BackgroundMode itemBackgroundMode READ itemBackgroundMode WRITE setItemBackgroundMode STORED false DESIGNABLE true )

public:
    QDesignerToolBox( QWidget *parent, const char *name );

    QString itemLabel() const;
    void setItemLabel( const QString &l );
    Q3CString itemName() const;
    void setItemName( const Q3CString &n );
    Qt::BackgroundMode itemBackgroundMode() const;
    void setItemBackgroundMode( Qt::BackgroundMode );

protected:
    void itemInserted( int index );
};

#endif
