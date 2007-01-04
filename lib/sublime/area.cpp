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

    struct ViewFinder {
        ViewFinder(View *_view): view(_view), index(0) {}
        bool operator() (AreaIndex *idx) {
            if (idx->hasView(view))
            {
                index = idx;
                return true;
            }
            return false;
        }
        View *view;
        AreaIndex *index;
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
}

Area::Area(const Area &area)
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
}

void Area::addView(View *view, View *viewToSplit, Qt::Orientation orientation)
{
    AreaIndex *indexToSplit = indexOf(viewToSplit);
    indexToSplit->split(view, orientation);
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

}

#include "area.moc"
