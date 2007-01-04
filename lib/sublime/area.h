/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef SUBLIMEAREA_H
#define SUBLIMEAREA_H

#include <QObject>

#include "areaindex.h"
#include "sublimedefs.h"

namespace Sublime {

class AreaIndex;
class RootAreaIndex;
class Controller;
class View;

/**
@short Area - the universal view container

Area contains views and toolviews, knows about their positions
and provides functionality to add new (tool)views and remove existing.

Area takes care of all placement/configuration details so that
in order for @ref MainWindow to show the area it just needs to
reconstruct itself according to the area's rules.
*/
class Area: public QObject {
    Q_OBJECT
public:
    Area(Controller *controller, const QString &name);
    Area(const Area &area);
    ~Area();

    /**Adds the @p view to the list of views at the current area index.*/
    void addView(View *view);
    /**Adds the @p view to the area splitting the @p viewToSplit using
    given @p orientation.*/
    void addView(View *view, View *viewToSplit, Qt::Orientation orientation);
    /**Removes the @p view from the area. Also deletes the view.*/
    void removeView(View *view);

    /**@return the index of view or 0 if it can not be found.*/
    AreaIndex *indexOf(View *view);
    /**@return the root index of the area. Root index always exists so this
    method will never return 0.*/
    RootAreaIndex *rootIndex() const;

    /**Adds the toolview to the area. Area will use its configuration and restore
    the proper position for the toolview when necessary. If it has no configuration
    for this view, it will use @p defaultPosition.*/
    void addToolView(View *toolView, Position defaultPosition);
    /**Removes the toolview from the area. Also deletes the toolview*/
    void removeToolView(View *toolView);
    QList<View*> &toolViews() const;
    Position toolViewPosition(View *toolView) const;

    /**Walks the tree of area indices and executes the operator. It will always walk the
    tree of views from top to bottom from left to right.

    Operator should be the class with <i>bool operator()(AreaIndex *index)</i>
    method. That method should return true if the walker has to stop at current index
    or false to continue.

    Example (operator to print the indices):
    @code
    struct MyOperator {
        bool operator()(AreaIndex *index) {
            std::cerr << index << std::endl;
            return false;
        }
    };
    ...
    MyOperator op;
    walkViews(op, rootIndex())
    @endcode*/
    template <typename Operator>
    void walkViews(Operator &op, AreaIndex *index);

    /**Walks the list of toolviews. The order in which toolviews are walked is not specified.

    Operator should be the class with <i>bool operator()(View *view, Sublime::Position position)</i> method.
    That method should return true if the walker has to stop at current toolview
    or falst to continue.

    Example (operator to print the list of views):
    @code
    struct MyOperator {
        bool operator()(View *view, Sublime::Position position) {
            std::cerr << view << " at position " << position << std::endl;
            return false;
        }
    };
    ...
    MyOperator op;
    walkToolViews(op, Sublime::AllPositions)
    */
    template <typename Operator>
    void walkToolViews(Operator &op, Positions positions);

private:
    struct AreaPrivate *d;

};

}

#include "areawalkers.h"

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
