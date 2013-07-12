/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_SUBLIMEMAINWINDOW_H
#define KDEVPLATFORM_SUBLIMEMAINWINDOW_H

#include <QtCore/QList>
#include <kparts/mainwindow.h>

#include "sublimeexport.h"

class QDockWidget;

namespace Sublime {

class Container;
class Area;
class View;
class Controller;
class MainWindowOperator;

/**
@short Sublime Main Window

The area-enabled mainwindow to show Sublime views and toolviews.

To use, a controller and constructed areas are necessary:
@code
MainWindow w(controller);
controller->showArea(area, &w);
@endcode
*/
class SUBLIME_EXPORT MainWindow: public KParts::MainWindow {
Q_OBJECT
public:
    /**Creates a mainwindow and adds it to the controller.*/
    explicit MainWindow(Controller *controller, Qt::WindowFlags flags = KDE_DEFAULT_WINDOWFLAGS);
    ~MainWindow();

    /**@return the list of dockwidgets that contain area's toolviews.*/
    QList<View*> toolDocks() const;
    /**@return area which mainwindow currently shows or 0 if no area has been set.*/
    Area *area() const;
    /**@return controller for this mainwindow.*/
    Controller *controller() const;

    /**@return active view inside this mainwindow.*/
    View *activeView();
    /**@return active toolview inside this mainwindow.*/
    View *activeToolView();

    /**Enable saving of per-area UI settings (like toolbar properties 
       and position) whenever area is changed.  This should be
       called after all areas are restored, and main window area is
       set, to prevent saving a half-broken state.  */
    void enableAreaSettingsSave();

    /** Allows setting an additional widget that will be inserted next to the area-switcher tabs */
    void setAreaSwitcherCornerWidget(QWidget* widget);
    
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
    void reconstructViews(QList<View*> topViews = QList<View*>());
    
    /**Returns a list of all views which are on top of their corresponding view stacks*/
    QList<View*> getTopViews() const;
    
    /**Returns the view that is closest to the given global position, or zero.*/
    View* viewForPosition(QPoint globalPos) const;
    
    /**Returns true if this main-window contains this view*/
    bool containsView(View* view) const;
    
    /**Returns all areas that belong to this main-window*/
    QList<Area*> areas() const;
    
    /** Sets a @p w widget that will be shown when there are no opened documents.
     * This method takes the ownership of @p w.
     */
    void setBackgroundCentralWidget(QWidget* w);
    
public Q_SLOTS:
    /**Shows the @p view and makes it active, focusing it by default).*/
    void activateView(Sublime::View *view, bool focus = true);
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
    /**Emitted when the active toolview is changed.*/
    void activeToolViewChanged(Sublime::View*);
    /**Emitted when the user interface settings have changed.*/
    void settingsLoaded();
    
    /**Emitted when a new view is added to the mainwindow.*/
    void viewAdded(Sublime::View*);
    /**Emitted when a view is going to be removed from the mainwindow.*/
    void aboutToRemoveView(Sublime::View*);


protected:
    QWidget *statusBarLocation();
    virtual void initializeStatusBar();
protected Q_SLOTS:
    virtual void setupAreaSelector();
    virtual void tabContextMenuRequested(Sublime::View*, KMenu*);
    virtual void tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab);
    /**Called whenever the user requests a context menu on a dockwidget bar.
       You can then e.g. add actions to add dockwidgets.
       Default implementation does nothing.**/
    virtual void dockBarContextMenuRequested(Qt::DockWidgetArea, const QPoint&);

public: // FIXME?
    /**Saves size/toolbar/menu/statusbar settings to the global configuration file.
    Reimplement in subclasses to save more and don't forget to call inherited method.*/
    virtual void saveSettings();

    /**Reimplemented to save settings.*/
    virtual bool queryClose();

    /**Reimplement this to add custom buttons into the area switchers. The default-implementation returns zero, which means that no button is added
      *The returned button is owned by the caller, and deleted at will. */
    virtual QWidget* customButtonForAreaSwitcher ( Area* area );

    QWidget* areaSwitcher() const;

private:
    
    Q_PRIVATE_SLOT(d, void viewAdded(Sublime::AreaIndex*, Sublime::View*))
    Q_PRIVATE_SLOT(d, void viewRemovedInternal(Sublime::AreaIndex*, Sublime::View*))
    Q_PRIVATE_SLOT(d, void aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*))
    Q_PRIVATE_SLOT(d, void toolViewAdded(Sublime::View*, Sublime::Position))
    Q_PRIVATE_SLOT(d, void raiseToolView(Sublime::View*))
    Q_PRIVATE_SLOT(d, void aboutToRemoveToolView(Sublime::View*, Sublime::Position))
    Q_PRIVATE_SLOT(d, void toolViewMoved(Sublime::View*, Sublime::Position))

    //Inherit MainWindowOperator to access four methods below
    /**Unsets the area clearing main window.*/
    void clearArea();
    /**Sets the active view.*/
    void setActiveView(Sublime::View* view, bool focus = true);
    /**Sets the active toolview and focuses it.*/
    void setActiveToolView(View *view);

    void resizeEvent(QResizeEvent* event);

    void saveGeometry(KConfigGroup &config);
    void loadGeometry(const KConfigGroup &config);

    class MainWindowPrivate *const d;
    friend class MainWindowOperator;
    friend class MainWindowPrivate;
};

}


#endif

