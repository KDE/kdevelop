/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMEMAINWINDOW_P_H
#define KDEVPLATFORM_SUBLIMEMAINWINDOW_P_H

#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSet>

#include "area.h"
#include "sublimedefs.h"

#include "mainwindow.h"
#include <QPushButton>

class QAction;
class QSplitter;
class IdealToolBar;

namespace Sublime {

class View;
class Container;
class Controller;
class AreaIndex;
class IdealMainWidget;
class IdealController;
class MessageWidget;
class Message;

class MainWindowPrivate: public QObject {
    Q_OBJECT
public:
    MainWindowPrivate(MainWindow *w, Controller* controller);
    ~MainWindowPrivate() override;

    /**Use this to create tool views for an area.*/
    class IdealToolViewCreator {
    public:
        explicit IdealToolViewCreator(MainWindowPrivate *_d): d(_d) {}
        Area::WalkerMode operator() (View *view, Sublime::Position position);
    private:
        MainWindowPrivate* const d;
    };

    /**Use this to create views for an area.*/
    class ViewCreator {
    public:
        explicit ViewCreator(MainWindowPrivate* _d, const QList<View*>& _topViews = QList<View*>())
            : d(_d)
            , topViews(_topViews.begin(), _topViews.end())
        {}
        Area::WalkerMode operator() (AreaIndex *index);
    private:
        MainWindowPrivate* const d;
        const QSet<View*> topViews;
    };

    /**Reconstructs the mainwindow according to the current area.*/
    void reconstruct();
    /**Reconstructs the views according to the current area index.*/
    void reconstructViews(const QList<View*>& topViews = QList<View*>());
    /**Clears the area leaving mainwindow empty.*/
    void clearArea();
    
    /** Sets a @p w widget that will be shown when there are no documents on the area */
    void setBackgroundCentralWidget(QWidget* w);

    void activateFirstVisibleView();

    Controller* const controller;
    Area *area;
    QList<View*> docks;
    QMap<View*, Container*> viewContainers;
    QMap<QWidget*, View*> widgetToView;

    View *activeView;
    View *activeToolView;

    QWidget *centralWidget;
    QWidget* bgCentralWidget;
    MessageWidget* messageWidget;
    ViewBarContainer* viewBarContainer;
    QSplitter* splitterCentralWidget;

    IdealController *idealController;
    bool ignoreDockShown;
    bool autoAreaSettingsSave;

    bool eventFilter(QObject* obj, QEvent* event) override;
    void disableConcentrationMode();

    void postMessage(Message* message);

public Q_SLOTS:
    void toggleDocksShown();

    void viewAdded(Sublime::AreaIndex *index, Sublime::View *view);
    void viewRemovedInternal(Sublime::AreaIndex *index, Sublime::View *view);
    void raiseToolView(Sublime::View* view);
    void aboutToRemoveView(Sublime::AreaIndex *index, Sublime::View *view);
    void toolViewAdded(Sublime::View *toolView, Sublime::Position position);
    void aboutToRemoveToolView(Sublime::View *toolView, Sublime::Position position);
    void toolViewMoved(Sublime::View *toolView, Sublime::Position position);

    void setTabBarLeftCornerWidget(QWidget* widget);

private Q_SLOTS:
    void updateAreaSwitcher(Sublime::Area *area);
    void slotDockShown(Sublime::View*, Sublime::Position, bool);
    void widgetCloseRequest(QWidget* widget);

    void showLeftDock(bool b);
    void showRightDock(bool b);
    void showBottomDock(bool b);
    void focusEditor();
    void selectNextDock();
    void selectPreviousDock();

    void messageDestroyed(Message* message);

private:
    void restoreConcentrationMode();

    void setBackgroundVisible(bool v);
    Qt::DockWidgetArea positionToDockArea(Position position);
    void cleanCentralWidget();

    MainWindow* const m_mainWindow;
    // uses QPointer to make already-deleted splitters detectable
    QMap<AreaIndex*, QPointer<QSplitter> > m_indexSplitters;

    QMap<Area*, QAction*> m_areaActions;
    QPointer<QWidget> m_leftTabbarCornerWidget;
    QPointer<QToolBar> m_concentrateToolBar;
    IdealToolBar* m_bottomToolBar;
    IdealToolBar* m_rightToolBar;
    IdealToolBar* m_leftToolBar;

    QAction* m_concentrationModeAction;

    QHash<Message*, QVector<QSharedPointer<QAction>>> m_messageHash;
};

}

#endif

