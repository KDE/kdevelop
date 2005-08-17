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

#ifndef FORMWINDOW_H
#define FORMWINDOW_H

#include "command.h"
#include "metadatabase.h"
#include "sizehandle.h"
#include "actiondnd.h"

#include <qwidget.h>
#include <q3ptrdict.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qmap.h>
//Added by qt3to4:
#include <QFocusEvent>
#include <QPaintEvent>
#include <QCloseEvent>
#include <Q3PtrList>
#include <QKeyEvent>
#include <QEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMouseEvent>

class QPaintEvent;
class QMouseEvent;
class QKeyEvent;
class QPainter;
class QLabel;
class MainWindow;
class QTimer;
class QFocusEvent;
class QCloseEvent;
class Resource;
class QResizeEvent;
class BreakLayoutCommand;
class QPixmap;
class QSizeGrip;
class Project;
struct DesignerFormWindow;
class FormFile;

#if defined(Q_CC_MSVC) || defined(Q_FULL_TEMPLATE_INSTANTIATION)
#include "orderindicator.h"
#else
class OrderIndicator;
#endif

class FormWindow : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString fileName READ fileName WRITE setFileName )

public:
    FormWindow( FormFile *f, MainWindow *mw, QWidget *parent, const char *name = 0 );
    FormWindow( FormFile *f, QWidget *parent, const char *name = 0 );
    ~FormWindow();

    void init();
    virtual void setMainWindow( MainWindow *w );

    virtual QString fileName() const;
    virtual void setFileName( const QString &fn );

    virtual QPoint grid() const;
    virtual QPoint gridPoint( const QPoint &p );

    virtual CommandHistory *commandHistory();

    virtual void undo();
    virtual void redo();
    virtual QString copy();
    virtual void paste( const QString &cb, QWidget *parent );
    virtual void lowerWidgets();
    virtual void raiseWidgets();
    virtual void checkAccels();

    virtual void layoutHorizontal();
    virtual void layoutVertical();
    virtual void layoutHorizontalSplit();
    virtual void layoutVerticalSplit();
    virtual void layoutGrid();

    virtual void layoutHorizontalContainer( QWidget *w );
    virtual void layoutVerticalContainer( QWidget *w );
    virtual void layoutGridContainer( QWidget *w );

    virtual void breakLayout( QWidget *w );

    virtual void selectWidget( QObject *w, bool select = TRUE );
    virtual void selectAll();
    virtual void updateSelection( QWidget *w );
    virtual void raiseSelection( QWidget *w );
    virtual void repaintSelection( QWidget *w );
    virtual void clearSelection( bool changePropertyDisplay = TRUE );
    virtual void selectWidgets();
    bool isWidgetSelected( QObject *w );
    virtual void updateChildSelections( QWidget *w );
    virtual void raiseChildSelections( QWidget *w );

    virtual void emitUpdateProperties( QObject *w );
    virtual void emitShowProperties( QObject *w = 0 );
    virtual void emitSelectionChanged();

    virtual void setPropertyShowingBlocked( bool b );
    bool isPropertyShowingBlocked() const;

    virtual QLabel *sizePreview() const;
    virtual void checkPreviewGeometry( QRect &r );

    virtual Q3PtrDict<QWidget> *widgets();
    virtual QWidgetList selectedWidgets() const;

    virtual QWidget *designerWidget( QObject *o ) const;

    virtual void handleContextMenu( QContextMenuEvent *e, QWidget *w );
    virtual void handleMousePress( QMouseEvent *e, QWidget *w );
    virtual void handleMouseRelease( QMouseEvent *e, QWidget *w );
    virtual void handleMouseDblClick( QMouseEvent *e, QWidget *w );
    virtual void handleMouseMove( QMouseEvent *e, QWidget *w );
    virtual void handleKeyPress( QKeyEvent *e, QWidget *w );
    virtual void handleKeyRelease( QKeyEvent *e, QWidget *w );

    virtual void updateUndoInfo();

    virtual MainWindow *mainWindow() const { return mainwindow; }

    bool checkCustomWidgets();
    virtual void insertWidget( QWidget *w, bool checkName = FALSE );
    virtual void removeWidget( QWidget *w );
    virtual void deleteWidgets();
    virtual void editAdjustSize();
    virtual void editConnections();

    virtual int numSelectedWidgets() const;
    virtual int numVisibleWidgets() const;

    virtual bool hasInsertedChildren( QWidget *w ) const;

    virtual QWidget *currentWidget() const { return propertyWidget && propertyWidget->isWidgetType() ? (QWidget*)propertyWidget : 0; } // #####
    virtual bool unify( QObject *w, QString &s, bool changeIt );

    virtual bool isCustomWidgetUsed( MetaDataBase::CustomWidget *w );
    virtual bool isDatabaseWidgetUsed() const;
    virtual bool isDatabaseAware() const;

    virtual QPoint mapToForm( const QWidget* w, const QPoint&  ) const;

    bool isMainContainer( QObject *w ) const;
    bool isCentralWidget( QObject *w ) const;
    QWidget *mainContainer() const { return mContainer; }
    void setMainContainer( QWidget *w );

    void paintGrid( QWidget *w, QPaintEvent *e );

    bool savePixmapInline() const;
    QString pixmapLoaderFunction() const;
    void setSavePixmapInline( bool b );
    void setPixmapLoaderFunction( const QString &func );

    bool savePixmapInProject() const;
    void setSavePixmapInProject( bool b );

    void setToolFixed() { toolFixed = TRUE; }

    void setActiveObject( QObject *o );

    Q3PtrList<QAction> &actionList() { return actions; }
    QAction *findAction( const QString &name );

    void setProject( Project *pro );
    Project *project() const;

    void killAccels( QObject *top );

    DesignerFormWindow *iFace();

    int layoutDefaultSpacing() const;
    int layoutDefaultMargin() const;
    void setLayoutDefaultSpacing( int s );
    void setLayoutDefaultMargin( int s );
    QString spacingFunction() const;
    QString marginFunction() const;
    void setSpacingFunction( const QString &func );
    void setMarginFunction( const QString &func );
    bool hasLayoutFunctions() const;
    void hasLayoutFunctions( bool b );

    void initSlots();
    FormFile *formFile() const;
    void setFormFile( FormFile *f );

    bool isFake() const { return fake; }
    bool canBeBuddy( const QWidget* ) const;

public slots:
    virtual void widgetChanged( QObject *w );
    virtual void currentToolChanged();
    virtual void visibilityChanged();
    virtual void modificationChanged( bool m );

signals:
    void showProperties( QObject *w );
    void updateProperties( QObject *w );
    void undoRedoChanged( bool undoAvailable, bool redoAvailable,
			  const QString &undoCmd, const QString &redoCmd );
    void selectionChanged();
    void modificationChanged( bool m, FormWindow *fw );
    void modificationChanged( bool m, const QString &s );
    void fileNameChanged( const QString &s, FormWindow *fw );

protected:
    virtual void closeEvent( QCloseEvent *e );
    virtual void focusInEvent( QFocusEvent *e );
    virtual void focusOutEvent( QFocusEvent *e );
    virtual void resizeEvent( QResizeEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e ) { handleMouseDblClick( e, mainContainer() ); }
    virtual bool event( QEvent *e );

private:
    enum RectType { Insert, Rubber };

    void beginUnclippedPainter( bool doNot );
    void endUnclippedPainter();
    void drawConnectionLine();
    void drawSizePreview( const QPoint &pos, const QString& text );

    void insertWidget();
    void moveSelectedWidgets( int dx, int dy );

    void startRectDraw( const QPoint &p, const QPoint &global, QWidget *w, RectType t );
    void continueRectDraw( const QPoint &p, const QPoint &global, QWidget *w, RectType t );
    void endRectDraw();

    void checkSelectionsForMove( QWidget *w );
    BreakLayoutCommand *breakLayoutCommand( QWidget *w );

    bool allowMove( QWidget *w );

    void saveBackground();
    void restoreConnectionLine();
    void restoreRect( const QRect &rect ) ;

    void showOrderIndicators();
    void updateOrderIndicators();
    void repositionOrderIndicators();
    void hideOrderIndicators();

    QWidget *containerAt( const QPoint &pos, QWidget *notParentOf );

private slots:
    void invalidCheckedSelections();
    void updatePropertiesTimerDone();
    void showPropertiesTimerDone();
    void selectionChangedTimerDone();
    void windowsRepaintWorkaroundTimerTimeout();

private:
    int currTool;
    bool oldRectValid, widgetPressed, drawRubber, checkedSelectionsForMove;
    bool validForBuddy;
    QRect currRect;
    QPoint rectAnchor;
    QPainter *unclippedPainter;
    QPoint sizePreviewPos;
    QPixmap sizePreviewPixmap;
    MainWindow *mainwindow;
    Q3PtrList<WidgetSelection> selections;
    Q3PtrDict<WidgetSelection> usedSelections;
    QRect widgetGeom, rubber;
    QPoint oldPressPos, origPressPos;
    CommandHistory commands;
    QMap<QWidget*, QPoint> moving;
    QWidget *insertParent;
    QObject *propertyWidget;
    QLabel *sizePreviewLabel;
    QTimer *checkSelectionsTimer;
    Q3PtrDict<QWidget> insertedWidgets;
    bool propShowBlocked;
    QTimer* updatePropertiesTimer, *showPropertiesTimer, *selectionChangedTimer,
    *windowsRepaintWorkaroundTimer;
    QPoint startPos, currentPos;
    QWidget *startWidget, *endWidget;
    QPixmap *buffer;
    Q3PtrList<OrderIndicator> orderIndicators;
    QWidgetList orderedWidgets;
    QWidgetList stackedWidgets;
    QWidget *mContainer;
    bool pixInline, pixProject;
    QString pixLoader;
    bool toolFixed;
    Q3PtrList<QAction> actions;
    Project *proj;
    DesignerFormWindow *iface;
    QWidget* targetContainer;
    QPalette restorePalette;
    bool hadOwnPalette;
    int defSpacing, defMargin;
    QString spacFunction, margFunction;
    bool hasLayoutFunc;
    FormFile *ff;
    bool fake;

};

#endif
