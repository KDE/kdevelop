/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMECONTROLLER_H
#define KDEVPLATFORM_SUBLIMECONTROLLER_H

#include <QObject>

#include "sublimeexport.h"

#include "mainwindowoperator.h"


namespace Sublime {

class Area;
class AreaIndex;
class MainWindow;
class ControllerPrivate;

/**
@short Handles association of areas to main windows.

Whereas the MainWindow class can be told to show arbitrary Area
instance, this class establishes more high-level rules based
on the following assumptions:

1. It's desirable to have a list of "area types" -- basically string,
and be able to switch each main window between those "area types". For
example, to switch main window between "Code" and "Debug"

2. It's also desirable to save the state of area -- like the set of tool views,
position of tool views, etc. This need to be done per-main window, so that
"code" area of one window is allowed to be different from "code" area
of another window.

3. Is it desirable to be able to reset an area of given type in a given
main window to a default state.

The current implementation achieves those goals as follows.

1. Controller keeps a list of default areas. Those areas are not shown by any
main window, and never modified as result of user actions. They are directly 
constructed by kdevelop core. Those areas are returned by the defaultAreas
method. Each Area instance in the list provides area type id, and human name
of the area -- via Area::objectName and Area::title methods respectively.
All methods in this class accept area id, and human name of the area is only
used to present the area type to the user, for selection.

2. Controller also keeps a list of MainWindow instances that it manages. For
each instance, it keeps a list of areas private to the MainWindow instance.
There's one area for each area in defaultAreas. That is, for each area type,
there's one area in defaultAreas, and one area per each main window

3. It's possible to switch a given main window to display an area of the
given type -- which finds the area with the given id in the list of area
private to that main window, and switches the main window to the found area.

When we create a new main window, we create fresh set of private areas by
cloning the default areas. An alternative approach would be to create a clone
only when we try to show a specific area type in a main window. However, 
I think that knowing that each main window has its Area instance for each 
area type simplifies the code. For example, most of the time, during 
restoring areas we'd need per-window area instances anyway. Of course, we 
can introduce a method demand_area_type(MainWindow*, QString) that
clones the default area of the necessary type, but I don't see what that will
buy us.

Controller has to exist before any area, document or mainwindow can be created.
There's no point in having two controllers for one application unless they
need to show completely different sets of areas.
*/
class KDEVPLATFORMSUBLIME_EXPORT Controller: public QObject, public MainWindowOperator {
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller() override;

    /** Add the area to the set of default areas in this controller. */
    void addDefaultArea(Area *area);

    /** Return the list of default areas.  */
    const QList<Area*> &defaultAreas() const;

    /** Return the default area with given @p id.*/
    Area *defaultArea(const QString &id) const;

    /** Add a main window to the set of windows managed by this
        controller.  The ownership of the window is passed to the
        controller.  The window will be associated with a set of
        areas created by cloning the current defaultAreas.  */
    void addMainWindow(MainWindow* mainWindow);

    /** Return the set of MainWindow instances managed by
        *this. */
    const QList<MainWindow*> &mainWindows() const;

    /** Return all areas associated with the main window with the specified
        index. */
   const QList<Area*> &areas(int mainWindow) const;

   /** Return all areas associated with the main window with the specified
        index. */
   const QList<Area*> &areas(MainWindow* mainWindow) const;

    /** Return the area with the given in main window specified
        by its index, @p mainWindow.  */
    Area *area(int mainWindow, const QString& id) const;

    /** Returns the area that contains the given view.
     * */
    Area* areaForView(View* view) const;

    /**Shows an @p area in @p mainWindow. @todo Remove this method */
    void showArea(Area *area, MainWindow *mainWindow);

    /** Show area with the id of @p areaTypeId in @p mainWindow. */
    void showArea(const QString& areaTypeId, MainWindow *mainWindow);

    /** Make the tool configuration of the area currently shown
        in @p mainWindow match those of default area with the same
        area type. */
    void resetCurrentArea(MainWindow *mainWindow);
    
    /** Return the list of all areas, including default area and
        area private to each main window.  */
    const QList<Area*> &allAreas() const;

    bool openAfterCurrent() const;
    bool arrangeBuddies() const;

    void loadSettings();
public Q_SLOTS:
    //@todo adymo: this should not be a part of public API
    /**Area can connect to this slot to release itself from its mainwindow.*/
    void areaReleased();
    /**Releases @p area from its mainwindow.*/
    void areaReleased(Sublime::Area *area);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void showAreaInternal(Area* area, MainWindow *mainWindow);

Q_SIGNALS:
    void mainWindowAdded(Sublime::MainWindow*);
    void areaCreated(Sublime::Area* area);

private:
    void init();

    void removeArea(Sublime::Area*); // called by Sublime::Area

private:
    const QScopedPointer<class ControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Controller)

    friend class Area;
};

}

#endif

