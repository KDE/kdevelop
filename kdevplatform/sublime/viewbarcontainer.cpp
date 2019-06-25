/*
    Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
    Copyright 2015 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "viewbarcontainer.h"

// Qt
#include <QStackedLayout>
#include <QDebug>

namespace {

class ViewBarStackedLayout : public QStackedLayout
{
    Q_OBJECT

public:
    QSize sizeHint() const override
    {
        if (currentWidget()) {
            return currentWidget()->sizeHint();
        }
        return QStackedLayout::sizeHint();
    }

    QSize minimumSize() const override
    {
        if (currentWidget()) {
            return currentWidget()->minimumSize();
        }
        return QStackedLayout::minimumSize();
    }
};

}

namespace Sublime {

class ViewBarContainerPrivate
{
public:
    ViewBarContainerPrivate(ViewBarContainer* q)
        : layout(new ViewBarStackedLayout)
    {
        q->setLayout(layout);
    }

public:
    ViewBarStackedLayout* layout;
};

ViewBarContainer::ViewBarContainer(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ViewBarContainerPrivate(this))
{
}

ViewBarContainer::~ViewBarContainer()
{
    Q_D(ViewBarContainer);

    // unparent any viewbars which may still exist
    // other code is still tracking those
    for (int i = d->layout->count(); i > 0; --i) {
        auto widget = d->layout->itemAt(i-1)->widget();
        if (widget) {
            d->layout->removeWidget(widget);
            widget->setParent(nullptr);
            widget->hide();
        }
    }
}

void ViewBarContainer::addViewBar(QWidget* viewBar)
{
    Q_D(ViewBarContainer);

    Q_ASSERT(viewBar);
    d->layout->addWidget(viewBar);
}

void ViewBarContainer::removeViewBar(QWidget* viewBar)
{
    Q_D(ViewBarContainer);

    Q_ASSERT(viewBar);
    d->layout->removeWidget(viewBar);
    viewBar->hide();

    if (viewBar == d->layout->currentWidget()) {
        hide();
    }
}

void ViewBarContainer::setCurrentViewBar(QWidget* viewBar)
{
    Q_D(ViewBarContainer);

    Q_ASSERT(viewBar);
    d->layout->setCurrentWidget(viewBar);
}

void ViewBarContainer::showViewBar(QWidget* viewBar)
{
    Q_D(ViewBarContainer);

    Q_ASSERT(viewBar);
    d->layout->setCurrentWidget(viewBar);
    viewBar->show();

    show();
}

void ViewBarContainer::hideViewBar(QWidget* viewBar)
{
    Q_D(ViewBarContainer);

    Q_ASSERT(viewBar);
    d->layout->setCurrentWidget(viewBar);
    viewBar->hide();

    hide();
}

}

#include "viewbarcontainer.moc"
