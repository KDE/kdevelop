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
    Switcher *switcher;
    QStackedLayout *stack;
    QTabWidget *tab;
};



// class Container

Container::Container(QWidget *parent)
    :QWidget(parent)
{
    d = new ContainerPrivate();

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    // d->switcher = new Switcher(this);
    // l->addWidget(d->switcher);

    d->stack = new QStackedLayout(l);
    d->tab = new QTabWidget(this);
    d->stack->addWidget(d->tab);
}

Container::~Container()
{
    delete d;
}

void Container::addWidget(QWidget *w)
{
    // d->stack->addWidget(w);
    //@todo adymo: remove tabwidget
    d->tab->addTab(w, "Tab");
}

int Container::count() const
{
    return d->tab->count();
}

QWidget *Container::widget(int index) const
{
    return d->tab->widget(index);
}

}

#include "container.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
