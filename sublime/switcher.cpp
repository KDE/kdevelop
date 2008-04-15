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
#include "switcher.h"

using namespace Sublime;

#include "view.h"
#include "document.h"
#include "areaindex.h"

class Sublime::SwitcherPrivate
{
public:
    SwitcherPrivate(AreaIndex* ai)
    : areaIndex(ai)
    {
    }

    AreaIndex* areaIndex;
};

Switcher::Switcher(AreaIndex* areaIndex, QWidget *parent)
    :QTabBar(parent), d(new SwitcherPrivate(areaIndex))
{
    connect(areaIndex, SIGNAL(viewAdded(Sublime::AreaIndex*, Sublime::View*)), this, SLOT(viewAdded(Sublime::AreaIndex*, Sublime::View*)));
    connect(areaIndex, SIGNAL(aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*)), this, SLOT(aboutToRemoveView(Sublime::AreaIndex*, Sublime::View*)));

    foreach (View* view, areaIndex->views())
        addTab(view->document()->title());
}

Switcher::~Switcher()
{
    delete d;
}

AreaIndex * Sublime::Switcher::areaIndex() const
{
    return d->areaIndex;
}

void Sublime::Switcher::viewAdded(Sublime::AreaIndex* index, Sublime::View* view)
{
    int idx = index->views().indexOf(view);
    Q_ASSERT(idx != -1);

    insertTab(idx, view->document()->title());
}

void Sublime::Switcher::aboutToRemoveView(Sublime::AreaIndex *index, Sublime::View *view)
{
    int idx = index->views().indexOf(view);
    Q_ASSERT(idx != -1);

    removeTab(idx);
}

#include "switcher.moc"

