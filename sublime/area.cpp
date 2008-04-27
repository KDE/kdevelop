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
#include "area.h"

#include <QMap>
#include <QList>
#include <QStringList>

#include <kdebug.h>

#include "view.h"
#include "document.h"
#include "areaindex.h"
#include "controller.h"

namespace Sublime {

// struct AreaPrivate

struct AreaPrivate {
    AreaPrivate()
    {
        rootIndex = new RootAreaIndex();
        currentIndex = rootIndex;
        controller = 0;
    }
    AreaPrivate(const AreaPrivate &p)
    {
        rootIndex = new RootAreaIndex(*(p.rootIndex));
        currentIndex = rootIndex;
        controller = p.controller;
        toolViewPositions.clear();
        title = p.title;
    }
    ~AreaPrivate()
    {
        delete rootIndex;
    }

    struct ViewFinder {
        ViewFinder(View *_view): view(_view), index(0) {}
        Area::WalkerMode operator() (AreaIndex *idx) {
            if (idx->hasView(view))
            {
                index = idx;
                return Area::StopWalker;
            }
            return Area::ContinueWalker;
        }
        View *view;
        AreaIndex *index;
    };

    struct ViewLister {
        Area::WalkerMode operator()(AreaIndex *idx) {
            views += idx->views();
            return Area::ContinueWalker;
        }
        QList<View*> views;
    };

    QString title;

    RootAreaIndex *rootIndex;
    AreaIndex *currentIndex;
    Controller *controller;

    QList<View*> toolViews;
    QMap<View *, Sublime::Position> toolViewPositions;
    QMap<QString, Sublime::Position> savedToolViewPositions;
    QStringList desiredToolViewsIds;
};



// class Area

Area::Area(Controller *controller, const QString &name, const QString &title)
    :QObject(controller), d( new AreaPrivate() )
{
    setObjectName(name);
    d->title = title;
    d->controller = controller;
    d->controller->addArea(this);
    connect(this, SIGNAL(destroyed(QObject*)), d->controller, SLOT(removeArea(QObject*)));
}

Area::Area(const Area &area)
    : QObject(area.controller()), d( new AreaPrivate( *(area.d) ) )
{
    static QMap<QString, int> nums;
    QString areaBaseName = area.objectName().split("(").first();
    setObjectName(areaBaseName + QString("(copy %1)").arg(nums[areaBaseName]));
    d->controller->addArea(this);

    //clone toolviews
    d->toolViews.clear();
    foreach (View *view, area.toolViews())
        addToolView(view->document()->createView(), area.toolViewPosition(view));

    nums[areaBaseName] += 1;
}

Area::~Area()
{
    delete d;
}

void Sublime::Area::addView(View *view, AreaIndex *index)
{
    index->add(view);
    emit viewAdded(index, view);
    connect(this, SIGNAL(destroyed()), view, SLOT(deleteLater()));
}

void Area::addView(View *view, View *after)
{
    AreaIndex *index = d->currentIndex;
    if (after)
    {
        AreaIndex *i = indexOf(after);
        if (i)
            index = i;
    }
    addView(view, index);
}

void Area::addView(View *view, View *viewToSplit, Qt::Orientation orientation)
{
    AreaIndex *indexToSplit = indexOf(viewToSplit);
    indexToSplit->split(view, orientation);
    emit viewAdded(indexToSplit, view);
    connect(this, SIGNAL(destroyed()), view, SLOT(deleteLater()));
}

View* Area::removeView(View *view)
{
    AreaIndex *index = indexOf(view);
    Q_ASSERT(index);

    emit aboutToRemoveView(index, view);
    index->remove(view);
    return view;
}

AreaIndex *Area::indexOf(View *view)
{
    AreaPrivate::ViewFinder f(view);
    walkViews(f, d->rootIndex);
    return f.index;
}

RootAreaIndex *Area::rootIndex() const
{
    return d->rootIndex;
}

void Area::addToolView(View *view, Position defaultPosition)
{
    ///@fixme adymo: read toolview position from config
    d->toolViews.append(view);
    QString id = view->document()->documentSpecifier();
    Position position = defaultPosition;
    if (d->savedToolViewPositions.contains(id))
        position = d->savedToolViewPositions[id];
    d->toolViewPositions[view] = position;
    emit toolViewAdded(view, position);
}

void Sublime::Area::raiseToolView(View * toolView)
{
    emit requestToolViewRaise(toolView);
}

View* Area::removeToolView(View *view)
{
    if (!d->toolViews.contains(view))
        return 0;

    emit aboutToRemoveToolView(view, d->toolViewPositions[view]);
    d->toolViews.removeAll(view);
    d->toolViewPositions.remove(view);
    return view;
}

void Area::moveToolView(View *toolView, Position newPosition)
{
    if (!d->toolViews.contains(toolView))
        return;

    d->toolViewPositions[toolView] = newPosition;
    emit toolViewMoved(toolView, newPosition);
}

QList<View*> &Area::toolViews() const
{
    return d->toolViews;
}

Position Area::toolViewPosition(View *toolView) const
{
    return d->toolViewPositions[toolView];
}

Controller *Area::controller() const
{
    return d->controller;
}

QList<View*> Sublime::Area::views()
{
    AreaPrivate::ViewLister lister;
    walkViews(lister, d->rootIndex);
    return lister.views;
}

QString Area::title() const
{
    return d->title;
}

void Area::setTitle(const QString &title)
{
    d->title = title;
}

void Area::save(KConfigGroup& group) const
{
    QStringList desired;
    foreach (View *v, d->toolViews)
    {
        desired << v->document()->documentSpecifier() + ":"
            + QString::number(static_cast<int>(d->toolViewPositions[v]));
    }
    group.writeEntry("desired views", desired);
}

void Area::load(const KConfigGroup& group)
{
    d->desiredToolViewsIds.clear();
    d->savedToolViewPositions.clear();
    QStringList desired = group.readEntry("desired views", QStringList());
    foreach (const QString &s, desired)
    {
        int i = s.indexOf(':');
        if (i == -1)
            d->desiredToolViewsIds << s;
        else
        {
            QString id = s.left(i);
            int pos_i = s.mid(i+1).toInt();
            Sublime::Position pos = static_cast<Sublime::Position>(pos_i);
            if (pos != Sublime::Left && pos != Sublime::Right
                && pos != Sublime::Top && pos != Sublime::Bottom)
                pos = Sublime::Bottom;
            d->desiredToolViewsIds << id;
            d->savedToolViewPositions[id] = pos;
        }
    }
}

bool Area::wantToolView(const QString& id)
{
    return (d->desiredToolViewsIds.contains(id));
}

}

#include "area.moc"
