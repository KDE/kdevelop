/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "view.h"

#include "document.h"
#include "tooldocument.h"

#include <QPointer>
#include <QWidget>

namespace Sublime {

class ViewPrivate
{
public:
    ViewPrivate(Document* doc, View::WidgetOwnership ws);

    QPointer<QWidget> widget;
    Document* const doc;
    const View::WidgetOwnership ws;
};

ViewPrivate::ViewPrivate(Document* doc, View::WidgetOwnership ws)
    : doc(doc)
    , ws(ws)
{
}

View::View(Document *doc, WidgetOwnership ws )
    : QObject(doc)
    , d_ptr(new ViewPrivate(doc, ws))
{
}

View::~View()
{
    Q_D(View);

    // widget is a raw pointer cache that prevents repeated weak pointer access
    auto* const widget = d->widget.get();
    if (widget && d->ws == View::TakeOwnership) {
        widget->hide();
        widget->setParent(nullptr);
        delete widget;
    }
}

Document *View::document() const
{
    Q_D(const View);

    return d->doc;
}

QWidget* View::widget() const
{
    Q_D(const View);

    return d->widget;
}

QWidget* View::initializeWidget(QWidget* parent)
{
    Q_D(View);

    Q_ASSERT(!d->widget);
    // widget is a raw pointer cache that prevents repeated weak pointer access
    auto* const widget = createWidget(parent);
    d->widget = widget;
    return widget;
}

QWidget *View::createWidget(QWidget *parent)
{
    Q_D(View);

    return d->doc->createViewWidget(parent);
}

void View::requestRaise()
{
    emit raise(this);
}

void View::readSessionConfig(KConfigGroup& config)
{
    Q_UNUSED(config);
}

void View::writeSessionConfig(KConfigGroup& config)
{
    Q_UNUSED(config);
}

QList<QAction*> View::toolBarActions() const
{
    Q_D(const View);

    auto* tooldoc = qobject_cast<ToolDocument*>(document());
    if( tooldoc )
    {
        return tooldoc->factory()->toolBarActions( d->widget );
    }
    return QList<QAction*>();
}

QList< QAction* > View::contextMenuActions() const
{
    Q_D(const View);

    auto* tooldoc = qobject_cast<ToolDocument*>(document());
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
