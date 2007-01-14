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
#include "area.h"

#include <QMap>
#include <QList>
#include <QStringList>

#include "kdebug.h"

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

    RootAreaIndex *rootIndex;
    AreaIndex *currentIndex;
    Controller *controller;

    QList<View*> toolViews;
    QMap<View *, Sublime::Position> toolViewPositions;
};



// class Area

Area::Area(Controller *controller, const QString &name)
    :QObject(controller)
{
    setObjectName(name);
    d = new AreaPrivate();
    d->controller = controller;
    d->controller->addArea(this);
    connect(this, SIGNAL(destroyed(QObject*)), d->controller, SLOT(removeArea(QObject*)));
}

Area::Area(const Area &area): QObject(area.controller())
{
    static QMap<QString, int> nums;
    d = new AreaPrivate(*(area.d));
    QString areaBaseName = area.objectName().split("(").first();
    setObjectName(areaBaseName + QString("(copy %1)").arg(nums[areaBaseName]));

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

void Area::addView(View *view)
{
    d->currentIndex->add(view);
    connect(this, SIGNAL(destroyed()), view, SLOT(deleteLater()));
}

void Area::addView(View *view, View *viewToSplit, Qt::Orientation orientation)
{
    AreaIndex *indexToSplit = indexOf(viewToSplit);
    indexToSplit->split(view, orientation);
    connect(this, SIGNAL(destroyed()), view, SLOT(deleteLater()));
}

void Area::removeView(View *view)
{
    AreaIndex *index = indexOf(view);
    index->remove(view);
    delete view;
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
    d->toolViewPositions[view] = defaultPosition;
}

void Area::removeToolView(View *view)
{
    d->toolViews.removeAll(view);
    d->toolViewPositions.remove(view);
    delete view;
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

}

#include "area.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
