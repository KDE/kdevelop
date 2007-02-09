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
#include "container.h"

#include <QLayout>
#include <QStackedLayout>
#include <QTabWidget>

#include "switcher.h"

namespace Sublime {

// struct ContainerPrivate

struct ContainerPrivate {

    void widgetActivated(int idx)
    {
        stack->setCurrentIndex(idx);
    }

    Switcher *switcher;
    QStackedLayout *stack;

    QMap<QWidget*, int> indices;
};



// class Container

Container::Container(QWidget *parent)
    :QWidget(parent)
{
    d = new ContainerPrivate();

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    d->switcher = new Switcher(this);
    connect(d->switcher, SIGNAL(activated(int)), this, SLOT(widgetActivated(int)));
    l->addWidget(d->switcher);

    d->stack = new QStackedLayout(l);
}

Container::~Container()
{
    delete d;
}

void Container::addWidget(QWidget *w)
{
    int idx = d->stack->addWidget(w);
    d->indices[w] = idx;
    d->switcher->insertItem(idx, "View");
}

void Sublime::Container::removeWidget(QWidget *w)
{
    if (!d->indices.contains(w))
        return;

    d->switcher->removeItem(d->indices[w]);
    d->indices.remove(w);
}

int Container::count() const
{
    return d->stack->count();
}

QWidget *Container::widget(int index) const
{
    return d->stack->widget(index);
}

bool Sublime::Container::hasWidget(QWidget *w)
{
    return d->stack->indexOf(w) != -1;
}

}

#include "container.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
