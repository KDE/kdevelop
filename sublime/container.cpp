/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>              *
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

#include <QMap>
#include <QLayout>
#include <QTabBar>
#include <QStackedLayout>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kglobal.h>
#include <kdeversion.h>
#include <kacceleratormanager.h>

#include "view.h"
#include "document.h"
#include "containerstyle.h"

namespace Sublime {

// struct ContainerPrivate

struct ContainerPrivate {
    QMap<QWidget*, View*> viewForWidget;
};



// class Container

Container::Container(QWidget *parent)
    :KTabWidget(parent), d(new ContainerPrivate())
{
    KAcceleratorManager::setNoAccel(this);

    // Set the widget style to a forwarding proxy style that removes the tabwidget frame,
    // and draws a tabbar base underneath the tabbar.
    setStyle(new ContainerStyle(this));
    // The base will be drawn on the frame instead of on the tabbar, so it extends across
    // the whole widget.
    tabBar()->setDrawBase(false);

    KConfigGroup group = KGlobal::config()->group("UiSettings");
    setTabBarHidden(group.readEntry("TabBarVisibility", 1) == 0);
#if KDE_VERSION < KDE_MAKE_VERSION(4,2,60)
    setHoverCloseButton(true);
    setCloseButtonEnabled(true);
    setTabReorderingEnabled(true);
#else
    setTabsClosable(true);
    setMovable(true);
#endif
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(widgetActivated(int)));
}

Container::~Container()
{
    delete d;
}

void Container::widgetActivated(int idx)
{
    if (idx < 0)
        return;
    if (QWidget* w = widget(idx)) {
        w->setFocus();
        if(d->viewForWidget.contains(w))
            emit activateView(d->viewForWidget[w]);
    }
}

void Container::addWidget(View *view)
{
    QWidget *w = view->widget(this);
    addTab(w, view->document()->title());
    d->viewForWidget[w] = view;
    connect(view->document(), SIGNAL(titleChanged(Sublime::Document*)), this, SLOT(documentTitleChanged(Sublime::Document*)));
}

void Container::documentTitleChanged(Sublime::Document* doc)
{
    QMapIterator<QWidget*, View*> it = d->viewForWidget;
    while (it.hasNext()) {
        if (it.next().value()->document() == doc) {
            int tabIndex = indexOf(it.key());
            if (tabIndex != -1) {
                setTabText(tabIndex, doc->title());
            }
            break;
        }
    }
}

void Sublime::Container::removeWidget(QWidget *w)
{
    if (w) {
        removeTab(indexOf(w));
        View* view = d->viewForWidget.take(w);
        disconnect(view->document(), SIGNAL(titleChanged(Sublime::Document*)), this, SLOT(documentTitleChanged(Sublime::Document*)));
    }
}

bool Container::hasWidget(QWidget *w)
{
    return indexOf(w) != -1;
}

View *Container::viewForWidget(QWidget *w) const
{
    return d->viewForWidget[w];
}

void Container::paintEvent(QPaintEvent *ev)
{
    //paint ourselves only if tabbar is visible
    if (tabBar()->isVisible())
        KTabWidget::paintEvent(ev);
    //otherwise don't paint anything (especially the border around the widget)
}

void Container::setTabBarHidden(bool hide)
{
    KTabWidget::setTabBarHidden(hide);
}

}

#include "container.moc"

