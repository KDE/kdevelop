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
#ifndef SUBLIMEAREAINDEX_H
#define SUBLIMEAREAINDEX_H

#include <Qt>
#include <QList>

#include <kdevexport.h>

namespace Sublime {

class View;

/**
@short Index denotes the position of the view in the splitted area.

B-Tree alike structure is used to represent an area with splitted
views. Area has a root index which can either contain one view or contain
two child nodes (@p first and @p second). In the later case area
is considered to be splitted into two parts. Each of those parts can
in turn contain a view or be splitted (with first/second children).

When a view at given index is splitted, then its index becomes an index of the splitter
and the original view goes into the @p first part of the splitter. The new view goes as
@p second part.

For example, consider an area which was splitted once horizontally
and then the second part of it was splitted vertically:
@code
 1. initial state: one view in the area
 |----------------|
 |                |
 |       1        |
 |                |
 |----------------|

 Indices:
    root_index (view 1)

 2. the view is splitted horizontally
 |----------------|
 |       |        |
 |   1   |   2    |
 |       |        |
 |----------------|

 Indices:
        root_index (no view)
             |
     ----------------
     |              |
   view 1         view 2

 3. the second view is splitted vertically
 |----------------|
 |       |   2    |
 |   1   |--------|
 |       |   3    |
 |----------------|

 Indices:
        root_index (horizontal splitter)
             |
     ----------------
     |              |
   view 1   vertical_splitter
                    |
            -----------------
            |                |
          view 2           view 3
@endcode

It is possible that several "stacked" views will have the same area index.
Those views can be considered as the view stack from which only one view
is visible at the time.
@code
 |----------------|
 |                |
 |    1,2,3,4     |
 |                |
 |----------------|

 Indices:
        root_index (view1, view2, view3, view4)
@endcode
*/
class SUBLIME_EXPORT AreaIndex {
public:
    ~AreaIndex();
    AreaIndex(const AreaIndex &index);

    /**@return the parent index, returns 0 for root index.*/
    AreaIndex *parent() const;

    /**@return the first child index if there're any.*/
    AreaIndex *first() const;
    /**@return the second child index if there're any.*/
    AreaIndex *second() const;
    /**@return the orientation of the splitter for this index.*/
    Qt::Orientation orientation() const;

    /**Adds view to the list of views in this position.
    Does nothing if the view is already splitted.
    @param after if not 0, new view will be placed after this one*/
    void add(View *view, View *after = 0);
    /**Removes view and unsplits the parent index when no views
    are left at the current index.*/
    void remove(View *view);
    /**Splits the view in this position by given @p orientation
    and adds the @p newView into the splitter.
    Does nothing if the view is already splitted.*/
    void split(View *newView, Qt::Orientation orientation);

    /**@return the stacked view in @p position,
    returns 0 for splitter's indices and when there's no view at the @p position.*/
    View *viewAt(int position) const;
    /**@return the number of stacked views.*/
    int viewCount() const;
    /**@return true if there's a stacked @p view at this index.*/
    bool hasView(View *view) const;
    /**@return the list of views at this index.*/
    QList<View*> &views() const;


protected:
    /**Constructor for Root index.*/
    AreaIndex();

private:
    /**Constructor for indices other than root.*/
    AreaIndex(AreaIndex *parent);
    void init();

    /**Sets the parent for this index.*/
    void setParent(AreaIndex *parent);

    /**Copies the data from this index to @p target.*/
    void copyTo(AreaIndex *target);
    /**Unsplits the index removing the given @p child and moving the contents
    of another child to this index.*/
    void unsplit(AreaIndex *childToRemove);

    struct AreaIndexPrivate *d;

};

/**
@short Root Area Index

This is the special index class returned by @ref Area::rootIndex().
Doesn't provide any additional functionality beyond AreaIndex.
*/
class SUBLIME_EXPORT RootAreaIndex: public AreaIndex {
public:
    RootAreaIndex();
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
