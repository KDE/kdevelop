/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SUBLIMEAREA_H
#define KDEVPLATFORM_SUBLIMEAREA_H

#include <QObject>

#include "sublimeexport.h"

#include "areaindex.h"
#include "sublimedefs.h"

#include <KConfigGroup>

class QAction;
namespace Sublime {

class AreaIndex;
class RootAreaIndex;
class Controller;
class View;
class AreaPrivate;

/**
@short Area - the universal view container

Area contains views and tool views, knows about their positions
and provides functionality to add new (tool)views and remove existing.

Area takes care of all placement/configuration details so that
in order for @ref MainWindow to show the area it just needs to
reconstruct itself according to the area's rules.

Usual way of creating an area is:
@code
Controller *controller = new Controller();
... //document creation code here
Area *area = new Area(controller, "My Area");
area->addView(document->createView());
MainWindow *mw = new MainWindow(controller);
controller->show(area, mw);
@endcode
*/
class KDEVPLATFORMSUBLIME_EXPORT Area: public QObject {
    Q_OBJECT
public:
    /**Creates and area with given @p name and adds it to the @p controller.
    @param controller is the controller in which this area will be available.
    @param name should identify this area and be unique for all areas in the controller.
        @ref QObject::objectName shall be used to get this name after area creation.
    @param title is the user-visible (translatable) title for the area.
        Use @ref title and @ref setTitle to operate on the title.
        This parameter can be omitted and then name will be used as title.*/
    Area(Controller *controller, const QString &name, const QString &title = {});
    Area(const Area &area);
    ~Area() override;

    QString title() const;
    void setTitle(const QString &title);

    QString iconName() const;
    void setIconName(const QString &iconName);


    /**Adds the @p view to the list of views at the given area index,
    after the given view @p after.

    If @p after == 0  &&  controller()->openAfterCurrent():
        @p view is inserted after current view
    If @p after == 0  &&  !controller()->openAfterCurrent():
        @p view is inserted at the last position.
    */
    void addView(View *view, AreaIndex *index, View *after = nullptr);

    /**Adds the @p view to the list of views at the current area index.*/
    void addView(View *view, View *after = nullptr);
    /**Adds the @p view to the area splitting the @p viewToSplit using
    given @p orientation.
    @p view Will be in the second child index of the area-index containing the view.
    */
    void addView(View *view, View *viewToSplit, Qt::Orientation orientation);
    /**Adds the @p view to the area splitting the area index @p indexToSplit using
    given @p orientation.
    @p view Will be in the <b>second</b> child index of the area-index containing the view.
    */
    void addView(View *view, AreaIndex *indexToSplit, Qt::Orientation orientation);
    /**Removes the @p view from the area. Does not delete it. */
    View* removeView(View *view);
    /**@return the list of all views in this area in no particular order. To
    process the views in ordered manner (tree walk) use @ref walkViews method.
    This method is added only for convenience.*/
    QList<View*> views();

    /** Removes all views from this area and deletes them.
      * If an open document has changes, and it is the last view of that document,
      * the user may push 'Cancel', and the view will stay active.
      * @param silent If this is true, the user is never asked for feedback. */
    void clearViews(bool silent = false);

    /**
     * Returns the view that was last stored through setActiveView(view), or zero
     * if the view was deleted or it was never set.
     */
    View* activeView() const;
    
    /**
     * Allows marking a view as active that can later be retrieved through activeView()
     */
    void setActiveView(View* view);
    
    /** Closes and deletes the view, asking the user for feedback if needed.
      * Closes the document if it is the last view.
      * Does allow breaking the closing process.
      * If it is the last view of the document that has changes, and the user pushed 'Cancel',
      * false will be returned, and the view will not be closed. 
      * @param silent If this is false, the user will be asked for feedback. Otherwise he won't.
      */
    bool closeView(View* view, bool silent = false);
    
    /**@return the index of view or 0 if it can not be found.*/
    AreaIndex *indexOf(View *view);
    /**@return the root index of the area. Root index always exists so this
    method will never return 0.*/
    RootAreaIndex *rootIndex() const;

    /**Adds the tool view to the area. Area will use its configuration and restore
    the proper position for the tool view when necessary. If it has no configuration
    for this view, it will use @p defaultPosition.*/
    void addToolView(View *toolView, Position defaultPosition);

    /**Removes the tool view from the area.*/
    View* removeToolView(View *toolView);

    /**
     * Set the position of @p toolView to @p newPosition.
     *
     * @note Unlike addToolView() and removeToolView(), this function does not emit any signals,
     *       but only updates the tool view's position in this area's data structures. This function
     *       is called when the UI of the tool view is moved to the new position by other means.
     */
    void setToolViewPosition(View* toolView, Position newPosition);

    /**Raise tool view.*/
    void raiseToolView(View *toolView);
    /**@return the list of tool views in the area. No particular sort order is guaranteed.*/
    const QList<View*> &toolViews() const;
    /**@return the current position of @p toolView in the area.*/
    Position toolViewPosition(View *toolView) const;

    /* Returns true if this area actively desires to show a tool view
       with id of 'id'.  The area, of course, will show any tool view
       added with 'addToolView', however, this method can be used
       to guess a set of tool views that make most sense to be added.  */
    bool wantToolView(const QString& id);
    void setDesiredToolViews(
        const QMap<QString, Sublime::Position>& desiredToolViews);

    void save(KConfigGroup& group) const;
    void load(const KConfigGroup& group);

    /**@return the controller for this area.*/
    Controller *controller() const;

    ///Returns the currently set working-set for this area.
    QString workingSet() const;
    ///Returns if the currently set working-set is persistent.
    ///@note This is just a hint and not synchronized with the actual working set.
    bool workingSetPersistent() const;
    ///Sets the working-set for this area. The working-set is just a marker, and does nothing
    ///within Area.
    ///The actual view management has to be implemented in the entity that knows more
    ///about possible views, documents, etc. (kdevplatform/shell)
    ///@warning (KDevelop): Before calling this, make sure that all views are saved! (see IDocumentController::saveAllDocumentsForWindow)
    void setWorkingSet(const QString &name, bool persistent = true, Area *oldArea = nullptr);
    
    /**Walker mode to determine the behavior of area walkers.*/
    enum WalkerMode {
        StopWalker,       /**< Stop after processing this area index or tool view */
        ContinueWalker    /**< Continue walking */
    };

    /**Walks the tree of area indices and executes the operator. It will always walk the
    tree of views from top to bottom from left to right.

    Operator should be the class with <i>WalkerResult operator()(AreaIndex *index)</i>
    method. That method should return Area::StopWalker if the walker has to stop at current index
    or Area::ContinueWalker to continue.

    Example (operator to print the indices, assumes hypothetical operator <<()):
    @code
    struct MyOperator {
        WalkerMode operator()(AreaIndex *index) {
            std::cerr << index << std::endl;
            return Area::ContinueWalker;
        }
    };
    ...
    MyOperator op;
    walkViews(op, rootIndex())
    @endcode*/
    template <typename Operator>
    void walkViews(Operator &op, AreaIndex *index);

    /**Walks the list of tool views. The order in which tool views are walked is not specified.

    Operator should be the class with <i>bool operator()(View *view, Sublime::Position position)</i>
    method. That method should return Area::StopWalker if the walker has to stop at current index
    or Area::ContinueWalker to continue.

    Example (operator to print the list of views):
    @code
    struct MyOperator {
        WalkerMode operator()(View *view, Sublime::Position position) {
            std::cerr << view << " at position " << position << std::endl;
            return Area::ContinueWalker;
        }
    };
    ...
    MyOperator op;
    walkToolViews(op, Sublime::AllPositions)
    @endcode
    */
    template <typename Operator>
    void walkToolViews(Operator &op, Positions positions);

    /** Adds an action to the area. They will be made available from different places, like the Area Display*/
    void addAction(QAction* action);

    /** @returns the actions related to the area  */
    QList<QAction*> actions() const;

    /**
     * Closes all the views and requests the working set to be cleared.
     * Works even though the area isn't opened yet
     */
    void clearDocuments();

Q_SIGNALS:
    /**Emitted when a new view is added to the area.*/
    void viewAdded(Sublime::AreaIndex*, Sublime::View*);
    /**Emitted when a view is going to be removed from the area.*/
    void aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*);
    /**Emitted when a view was removed from the area.*/
    void viewRemoved(Sublime::AreaIndex*, Sublime::View*);
    /**Emitted when a new tool view is added to the area.*/
    void toolViewAdded(Sublime::View*, Sublime::Position);
    /**Emitted when a tool view is requesting to be raised.*/
    void requestToolViewRaise(Sublime::View*);
    /**Emitted when a tool view is going to be removed from the area.*/
    void aboutToRemoveToolView(Sublime::View*, Sublime::Position);
    /**Emitted before the working-set is changed.*/
    void changingWorkingSet(Sublime::Area* area, Sublime::Area* oldArea, const QString& from, const QString& to);
    /**Emitted after the working-set was changed.*/
    void changedWorkingSet(Sublime::Area* area, Sublime::Area* oldArea, const QString& from, const QString& to);
    /** notifies the working set that it should clear */
    void clearWorkingSet(Sublime::Area* area);

private Q_SLOTS:
    void positionChanged(Sublime::View*, int);
    void actionDestroyed(QObject* action);

private:
    template <typename Operator>
    WalkerMode walkViewsInternal(Operator &op, AreaIndex *index);

    void initialize();

private:
    const QScopedPointer<class AreaPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Area)
};

}

#include "areawalkers.h"

#endif

