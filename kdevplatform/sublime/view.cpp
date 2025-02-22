/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "view.h"

#include "document.h"
#include "tooldocument.h"

#include <debug.h>

#include <QWidget>

namespace Sublime {

class ViewPrivate
{
public:
    ViewPrivate(Document* doc, View::WidgetOwnership ws);

    void unsetWidget();

    QWidget* widget = nullptr;
    Document* const doc;
    const View::WidgetOwnership ws;
};

ViewPrivate::ViewPrivate(Document* doc, View::WidgetOwnership ws)
    : doc(doc)
    , ws(ws)
{
}

void ViewPrivate::unsetWidget()
{
    widget = nullptr;
}

View::View(Document *doc, WidgetOwnership ws )
    : QObject(doc)
    , d_ptr(new ViewPrivate(doc, ws))
{
}

View::~View()
{
    Q_D(View);

    if (d->widget && d->ws == View::TakeOwnership ) {
        d->widget->hide();
        d->widget->setParent(nullptr);
        delete d->widget;
    }
}

Document *View::document() const
{
    Q_D(const View);

    return d->doc;
}

QWidget *View::widget(QWidget *parent)
{
    Q_D(View);

    if (!d->widget)
    {
        if (!parent) {
            qCWarning(SUBLIME) << "creating a widget for view" << d->doc->documentSpecifier()
                               << "without a parent => possibly unintentionally";
        }

        d->widget = createWidget(parent);
        // if we own this widget, we will also delete it and ideally would disconnect
        // the following connect before doing that. For that though we would need to store
        // a reference to the connection.
        // As the d object still exists in the destructor when we delete the widget
        // this lambda method though can be still safely executed, so we spare ourselves such disconnect.
        connect(d->widget, &QWidget::destroyed,
                this, [this] { Q_D(View); d->unsetWidget(); });
    }
    return d->widget;
}

QWidget *View::createWidget(QWidget *parent)
{
    Q_D(View);

    return d->doc->createViewWidget(parent);
}

bool View::hasWidget() const
{
    Q_D(const View);

    return d->widget != nullptr;
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
