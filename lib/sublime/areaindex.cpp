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
#include "areaindex.h"

#include <QList>

#include <kdebug.h>

#include "view.h"
#include "document.h"

namespace Sublime {

// struct AreaIndexPrivate

struct AreaIndexPrivate {
    AreaIndexPrivate()
        :parent(0), first(0), second(0)
    {
    }
    ~AreaIndexPrivate()
    {
        delete first;
        delete second;
    }
    AreaIndexPrivate(const AreaIndexPrivate &p)
    {
        parent = 0;
        orientation = p.orientation;
        first = p.first ? new AreaIndex(*(p.first)) : 0;
        second = p.second ? new AreaIndex(*(p.second)) : 0;
    }

    bool isSplitted()
    {
        return first || second;
    }

    QList<View*> views;

    AreaIndex *parent;
    AreaIndex *first;
    AreaIndex *second;
    Qt::Orientation orientation;
};



// class AreaIndex

AreaIndex::AreaIndex()
{
    init();
}

AreaIndex::AreaIndex(AreaIndex *parent)
{
    init();
    d->parent = parent;
}

AreaIndex::AreaIndex(const AreaIndex &index)
{
    kDebug() << "copying area index" << endl;
    d = new AreaIndexPrivate(*(index.d));
    if (d->first)
        d->first->setParent(this);
    if (d->second)
        d->second->setParent(this);
    //clone views in this index
    d->views.clear();
    foreach (View *view, index.views())
        add(view->document()->createView());
}

void AreaIndex::init()
{
    d = new AreaIndexPrivate();
}

AreaIndex::~AreaIndex()
{
    delete d;
}

void AreaIndex::add(View *view, View *after)
{
    //we can not add views to the areas that have already been splitted
    if (d->isSplitted())
        return;

    if (after)
        d->views.insert(d->views.indexOf(after), view);
    else
        d->views.append(view);
}

void AreaIndex::remove(View *view)
{
    if (d->isSplitted())
        return;

    d->views.removeAll(view);
    if (d->parent && (d->views.count() == 0))
        d->parent->unsplit(this);
}

void AreaIndex::split(View *newView, Qt::Orientation orientation)
{
    //we can not split areas that have already been splitted
    if (d->isSplitted())
        return;

    d->first = new AreaIndex(this);
    d->second = new AreaIndex(this);
    d->orientation = orientation;

    //assign current views to the first part of splitter
    copyTo(d->first);

    //make new view as second widget in splitter
    d->second->add(newView);
}

void AreaIndex::unsplit(AreaIndex *childToRemove)
{
    if (!d->isSplitted())
        return;

    AreaIndex *left = d->first == childToRemove ? d->second : d->first;
    left->copyTo(this);

    delete d->first;
    d->first = 0;
    delete d->second;
    d->second = 0;
}

void AreaIndex::copyTo(AreaIndex *target)
{
    target->setViews(d->views);
    d->views.clear();
}

QList<View*> &AreaIndex::views() const
{
    return d->views;
}

View *AreaIndex::viewAt(int position) const
{
    return d->views.value(position, 0);
}

int AreaIndex::viewCount() const
{
    return d->views.count();
}

bool AreaIndex::hasView(View *view) const
{
    return d->views.contains(view);
}

AreaIndex *AreaIndex::parent() const
{
    return d->parent;
}

void AreaIndex::setParent(AreaIndex *parent)
{
    d->parent = parent;
}

AreaIndex *AreaIndex::first() const
{
    return d->first;
}

AreaIndex *AreaIndex::second() const
{
    return d->second;
}

Qt::Orientation AreaIndex::orientation() const
{
    return d->orientation;
}

void AreaIndex::setViews(const QList<View*> &views)
{
    d->views = views;
}



// class RootAreaIndex

RootAreaIndex::RootAreaIndex()
    :AreaIndex()
{
}

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
