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
#include "areaindex.h"

#include <QList>

#include "view.h"
#include "document.h"
#include <debug.h>

namespace Sublime {

// class AreaIndexPrivate

class AreaIndexPrivate
{
public:
    AreaIndexPrivate()
    {
    }
    ~AreaIndexPrivate()
    {
        delete first;
        delete second;
        foreach( View* v, views )
        {
            // Do the same as AreaIndex::remove(), seems like deletion of the view is happening elsewhere
            views.removeAll( v );
        }
    }
    AreaIndexPrivate(const AreaIndexPrivate &p)
    {
        parent = nullptr;
        orientation = p.orientation;
        first = p.first ? new AreaIndex(*(p.first)) : nullptr;
        second = p.second ? new AreaIndex(*(p.second)) : nullptr;
    }

    bool isSplit() const
    {
        return first || second;
    }

    QList<View*> views;

    AreaIndex *parent = nullptr;
    AreaIndex *first = nullptr;
    AreaIndex *second = nullptr;
    Qt::Orientation orientation = Qt::Horizontal;
};



// class AreaIndex

AreaIndex::AreaIndex() : d(new AreaIndexPrivate)
{
}

AreaIndex::AreaIndex(AreaIndex *parent) : d(new AreaIndexPrivate)
{
    d->parent = parent;
}

AreaIndex::AreaIndex(const AreaIndex &index)  : d(new AreaIndexPrivate( *(index.d) ) )
{
    qCDebug(SUBLIME) << "copying area index";
    if (d->first)
        d->first->setParent(this);
    if (d->second)
        d->second->setParent(this);
    //clone views in this index
    d->views.clear();
    foreach (View *view, index.views())
        add(view->document()->createView());
}

AreaIndex::~AreaIndex() = default;

void AreaIndex::add(View *view, View *after)
{
    //we can not add views to the areas that have already been split
    if (d->isSplit())
        return;

    if (after)
        d->views.insert(d->views.indexOf(after)+1, view);
    else
        d->views.append(view);
}

void AreaIndex::remove(View *view)
{
    if (d->isSplit())
        return;

    d->views.removeAll(view);
    if (d->parent && (d->views.count() == 0))
        d->parent->unsplit(this);
}

void AreaIndex::split(Qt::Orientation orientation, bool moveViewsToSecond)
{
    //we can not split areas that have already been split
    if (d->isSplit())
        return;

    d->first = new AreaIndex(this);
    d->second = new AreaIndex(this);
    d->orientation = orientation;

    if(moveViewsToSecond)
        moveViewsTo(d->second);
    else
        moveViewsTo(d->first);
}

void AreaIndex::split(View *newView, Qt::Orientation orientation)
{
    split(orientation);

    //make new view as second widget in splitter
    d->second->add(newView);
}

void AreaIndex::unsplit(AreaIndex *childToRemove)
{
    if (!d->isSplit())
        return;

    AreaIndex *other = d->first == childToRemove ? d->second : d->first;
    other->moveViewsTo(this);
    d->orientation = other->orientation();
    d->first = nullptr;
    d->second = nullptr;
    other->copyChildrenTo(this);

    delete other;
    delete childToRemove;
}

void AreaIndex::copyChildrenTo(AreaIndex *target)
{
    if (!d->first || !d->second)
        return;
    target->d->first = d->first;
    target->d->second = d->second;
    target->d->first->setParent(target);
    target->d->second->setParent(target);

    d->first = nullptr;
    d->second = nullptr;
}

void AreaIndex::moveViewsTo(AreaIndex *target)
{
    target->d->views = d->views;
    d->views.clear();
}

QList<View*> &AreaIndex::views() const
{
    return d->views;
}

View *AreaIndex::viewAt(int position) const
{
    return d->views.value(position, nullptr);
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

bool Sublime::AreaIndex::isSplit() const
{
    return d->isSplit();
}

void Sublime::AreaIndex::setOrientation(Qt::Orientation orientation) const
{
    d->orientation = orientation;
}

// class RootAreaIndex

RootAreaIndex::RootAreaIndex()
    :AreaIndex()
{
}

QString AreaIndex::print() const
{
    if(isSplit())
        return QLatin1String(" [ ") + first()->print() + QLatin1String(orientation() == Qt::Horizontal ? " / " : " - ") + second()->print() + QLatin1String(" ] ");
    QStringList ret;
    foreach(Sublime::View* view, views())
        ret << view->document()->title();
    return ret.join(QLatin1Char(' '));
}

}
