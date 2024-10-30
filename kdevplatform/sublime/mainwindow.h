/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMEMAINWINDOW_H
#define KDEVPLATFORM_SUBLIMEMAINWINDOW_H

#include <QList>
#include <QMenu>
#include <QSet>

#include <KParts/MainWindow>

#include "sublimeexport.h"

namespace Sublime {

class Container;
class Area;
class View;
class Controller;
class MainWindowOperator;
class ViewBarContainer;
class Message;
class MainWindowPrivate;

/**
@short Sublime Main Window

The area-enabled mainwindow to show Sublime views and tool views.

To use, a controller and constructed areas are necessary:
@code
MainWindow w(controller);
controller->showArea(area, &w);
@endcode
*/
class KDEVPLATFORMSUBLIME_EXPORT MainWindow: public KParts::MainWindow {
Q_OBJECT
public:
    /**Creates a mainwindow and adds it to the controller.*/
    explicit MainWindow(Controller *controller, Qt::WindowFlags flags = {});
    ~MainWindow() override;

    /**@return the list of dockwidgets that contain area's tool views.*/
    QList<View*> toolDocks() const;
    /**@return area which mainwindow currently shows or 0 if no area has been set.*/
    Area *area() const;
    /**@return controller for this mainwindow.*/
    Controller *controller() const;

    /**@return active view inside this mainwindow.*/
    View *activeView() const;
    /**@return active tool view inside this mainwindow.*/
    View *activeToolView() const;

    /**Enable saving of per-area UI settings (like toolbar properties
       and position) whenever area is changed.  This should be
       called after all areas are restored, and main window area is
       set, to prevent saving a half-broken state.  */
    void enableAreaSettingsSave();

    /** Allows setting an additional widget that will be inserted left to the document tab-bar.
      *  The ownership goes to the target.  */
    void setTabBarLeftCornerWidget(QWidget* widget);

    /**Sets the area of main window and fills it with views.
      *The contents is reconstructed, even if the area equals the currently set area. */
    void setArea(Area *area);

    /**
     * Reconstruct the view structure. This is required after significant untracked changes to the
     * area-index structure.
     * Views listed in topViews will be on top of their view stacks.
     * */
    void reconstructViews(const QList<View*>& topViews = QList<View*>());

    /**Returns a list of all views which are on top of their corresponding view stacks*/
    QList<View*> topViews() const;

    QSet<Container*> containers() const;

    /**Returns the view that is closest to the given global position, or zero.*/
    View* viewForPosition(const QPoint& globalPos) const;

    /**Returns true if this main-window contains this view*/
    bool containsView(View* view) const;

    /**Returns all areas that belong to this main-window*/
    QList<Area*> areas() const;

    /** Sets a @p w widget that will be shown when there are no opened documents.
     * This method takes the ownership of @p w.
     * Pass @c nullptr for @p w for a blank background (default).
     */
    void setBackgroundCentralWidget(QWidget* w);

    /**Returns a widget that can hold a centralized view bar*/
    ViewBarContainer *viewBarContainer() const;

public Q_SLOTS:
    /**Shows the @p view and makes it active, focusing it by default).*/
    void activateView(Sublime::View *view, bool focus = true);
    /** Shows the @p message in the message area */
    void postMessage(Sublime::Message* message);
    /**Loads size/toolbar/menu/statusbar settings to the global configuration file.
    Reimplement in subclasses to load more and don't forget to call inherited method.*/
    virtual void loadSettings();

Q_SIGNALS:
    /**Emitted before the area is cleared from this mainwindow.*/
    void areaCleared(Sublime::Area*);
    /**Emitted after the new area has been shown in this mainwindow.*/
    void areaChanged(Sublime::Area*);
    /**Emitted when the active view is changed.*/
    void activeViewChanged(Sublime::View*);
    /**Emitted when the active tool view is changed.*/
    void activeToolViewChanged(Sublime::View*);
    /**Emitted when the user interface settings have changed.*/
    void settingsLoaded();

    /**Emitted when a new view is added to the mainwindow.*/
    void viewAdded(Sublime::View*);
    /**Emitted when a view is going to be removed from the mainwindow.*/
    void aboutToRemoveView(Sublime::View*);

protected:
    QWidget *statusBarLocation() const;
    virtual void initializeStatusBar();
protected Q_SLOTS:
    virtual void tabDoubleClicked(Sublime::View* view);
    virtual void tabContextMenuRequested(Sublime::View*, QMenu*);
    virtual void tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab);
    virtual void newTabRequested();
    /**Called whenever the user requests a context menu on a dockwidget bar.
       You can then e.g. add actions to add dockwidgets.
       Default implementation does nothing.**/
    virtual void dockBarContextMenuRequested(Qt::DockWidgetArea, const QPoint&);

public: // FIXME?
    /**Saves size/toolbar/menu/statusbar settings to the global configuration file.
    Reimplement in subclasses to save more and don't forget to call inherited method.*/
    virtual void saveSettings();

    /** Allow connecting to activateView without the need for a lambda for the default parameter */
    void activateViewAndFocus(Sublime::View *view) { activateView(view, true); }

private:
    //Inherit MainWindowOperator to access four methods below
    /**Unsets the area clearing main window.*/
    void clearArea();
    /**Sets the active view.*/
    void setActiveView(Sublime::View* view, bool focus = true);
    /**Sets the active tool view and focuses it.*/
    void setActiveToolView(View *view);

private:
    const QScopedPointer<class MainWindowPrivate> d_ptr;
    Q_DECLARE_PRIVATE(MainWindow)

    friend class MainWindowOperator;
    friend class MainWindowPrivate;
};

}


#endif

