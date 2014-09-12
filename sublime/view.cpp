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
#include "view.h"

#include <QWidget>

#include "document.h"
#include "tooldocument.h"

namespace Sublime {

class View;
class Document;

struct ViewPrivate
{
    ViewPrivate();
    Document *doc;
    QWidget *widget;
    void unsetWidget();
    View::WidgetOwnership ws;
};

ViewPrivate::ViewPrivate()
    :doc(0), widget(0)
{
}

void ViewPrivate::unsetWidget()
{
    widget = 0;
}

View::View(Document *doc, WidgetOwnership ws )
    :QObject(doc), d(new ViewPrivate)
{
    d->doc = doc;
    d->ws = ws;
}

View::~View()
{
    if (d->widget && d->ws == View::TakeOwnership ) {
        d->widget->hide();
        d->widget->setParent(0);
        d->widget->deleteLater();
    }
    delete d;
}

Document *View::document() const
{
    return d->doc;
}

QWidget *View::widget(QWidget *parent)
{
    if (!d->widget)
    {
        d->widget = createWidget(parent);
        connect(d->widget, SIGNAL(destroyed()), this, SLOT(unsetWidget()));
    }
    return d->widget;
}

QWidget *View::createWidget(QWidget *parent)
{
    return d->doc->createViewWidget(parent);
}

bool View::hasWidget() const
{
    return d->widget != 0;
}

void View::requestRaise()
{
    emit raise(this);
}

QString View::viewState() const
{
    return QString();
}

void View::setState(const QString & state)
{
    Q_UNUSED(state);
}

QList<QAction*> View::toolBarActions() const
{
    ToolDocument* tooldoc = dynamic_cast<ToolDocument*>( document() );
    if( tooldoc )
    {
        return tooldoc->factory()->toolBarActions( d->widget );
    }
    return QList<QAction*>();
}

QList< QAction* > View::contextMenuActions() const
{
    ToolDocument* tooldoc = dynamic_cast<ToolDocument*>( document() );
    if( tooldoc )
    {
        return tooldoc->factory()->contextMenuActions( d->widget );
    }
    return QList<QAction*>();
}

QString View::viewStatus() const
{
    return QString();
}

void View::notifyPositionChanged(int newPositionInArea)
{
    emit positionChanged(this, newPositionInArea);
}

}

#include "moc_view.cpp"
