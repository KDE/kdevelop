/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
        // TODO: does this still make sense?
        const auto oldViews = views;
        for (View* v : oldViews) {
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

AreaIndex::AreaIndex()
    : d_ptr(new AreaIndexPrivate)
{
}

AreaIndex::AreaIndex(AreaIndex *parent)
    : d_ptr(new AreaIndexPrivate)
{
    Q_D(AreaIndex);

    d->parent = parent;
}

AreaIndex::AreaIndex(const AreaIndex &index)
    : d_ptr(new AreaIndexPrivate(*(index.d_ptr)))
{
    Q_D(AreaIndex);

    qCDebug(SUBLIME) << "copying area index";
    if (d->first)
        d->first->setParent(this);
    if (d->second)
        d->second->setParent(this);
    //clone views in this index
    d->views.clear();
    for (View* view : std::as_const(index.views())) {
        add(view->document()->createView());
    }
}

AreaIndex::~AreaIndex() = default;

void AreaIndex::add(View *view, View *after)
{
    Q_D(AreaIndex);

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
    Q_D(AreaIndex);

    if (d->isSplit())
        return;

    d->views.removeAll(view);
    if (d->parent && (d->views.count() == 0))
        d->parent->unsplit(this);
}

void AreaIndex::split(Qt::Orientation orientation, bool moveViewsToSecond)
{
    Q_D(AreaIndex);

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
    Q_D(AreaIndex);

    split(orientation);

    //make new view as second widget in splitter
    d->second->add(newView);
}

void AreaIndex::unsplit(AreaIndex *childToRemove)
{
    Q_D(AreaIndex);

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
    Q_D(AreaIndex);

    if (!d->first || !d->second)
        return;
    target->d_ptr->first = d->first;
    target->d_ptr->second = d->second;
    target->d_ptr->first->setParent(target);
    target->d_ptr->second->setParent(target);

    d->first = nullptr;
    d->second = nullptr;
}

void AreaIndex::moveViewsTo(AreaIndex *target)
{
    Q_D(AreaIndex);

    target->d_ptr->views = d->views;
    d->views.clear();
}

void AreaIndex::moveViewPosition(View* view, int newPos)
{
    Q_D(AreaIndex);

    const auto oldPos = d->views.indexOf(view);

    d->views.move(oldPos, newPos);
}

const QList<View*>& AreaIndex::views() const
{
    Q_D(const AreaIndex);

    return d->views;
}

View *AreaIndex::viewAt(int position) const
{
    Q_D(const AreaIndex);

    return d->views.value(position, nullptr);
}

int AreaIndex::viewCount() const
{
    Q_D(const AreaIndex);

    return d->views.count();
}

bool AreaIndex::hasView(View *view) const
{
    Q_D(const AreaIndex);

    return d->views.contains(view);
}

AreaIndex *AreaIndex::parent() const
{
    Q_D(const AreaIndex);

    return d->parent;
}

void AreaIndex::setParent(AreaIndex *parent)
{
    Q_D(AreaIndex);

    d->parent = parent;
}

AreaIndex *AreaIndex::first() const
{
    Q_D(const AreaIndex);

    return d->first;
}

AreaIndex *AreaIndex::second() const
{
    Q_D(const AreaIndex);

    return d->second;
}

Qt::Orientation AreaIndex::orientation() const
{
    Q_D(const AreaIndex);

    return d->orientation;
}

bool Sublime::AreaIndex::isSplit() const
{
    Q_D(const AreaIndex);

    return d->isSplit();
}

void Sublime::AreaIndex::setOrientation(Qt::Orientation orientation)
{
    Q_D(AreaIndex);

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
    const auto views = this->views();
    ret.reserve(views.size());
    for (const auto* view : views) {
        ret << view->document()->title();
    }
    return ret.join(QLatin1Char(' '));
}

}
