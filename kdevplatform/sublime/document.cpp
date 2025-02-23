/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "document.h"

#include "view.h"
#include "area.h"
#include "controller.h"
// Qt
#include <QIcon>

namespace Sublime {

// class DocumentPrivate

class DocumentPrivate
{
public:
    DocumentPrivate(Controller* controller, Document* doc)
        : controller(controller)
        , document(doc)
    {}

    void removeView(Sublime::View* view)
    {
        views.removeAll(view);
        //no need to keep empty document - we need to remove it
        if (views.count() == 0)
        {
            emit document->aboutToDelete(document);
            document->deleteLater();
        }
    }

    QList<View*> views;
    QIcon statusIcon;
    QString documentToolTip;

    Controller* const controller;
    Document* const document;
};



//class Document

Document::Document(const QString &title, Controller *controller)
    : QObject(controller)
    , d_ptr(new DocumentPrivate(controller, this))
{
    setObjectName(title);
}

Document::~Document() = default;

View *Document::createView()
{
    Q_D(Document);

    View *view = newView(this);
    connect(view, &View::destroyed, this, [this, view](QObject* obj) {
        Q_D(Document);
        Q_ASSERT(obj == view);
        d->removeView(view);
    });
    d->views.append(view);
    return view;
}

const QList<View*> &Document::views() const
{
    Q_D(const Document);

    return d->views;
}

QString Document::title(TitleType /*type*/) const
{
    return objectName();
}

QString Document::toolTip() const
{
    Q_D(const Document);

    return d->documentToolTip;
}

void Document::setTitle(const QString& newTitle)
{
    setObjectName(newTitle);
    emit titleChanged(this);
}

void Document::setToolTip(const QString& newToolTip)
{
    Q_D(Document);

    d->documentToolTip=newToolTip;
}

View *Document::newView(Document *doc)
{
    //first create View, second emit the signal
    View *newView = new View(doc);
    return newView;
}


void Document::setStatusIcon(const QIcon& icon)
{
    Q_D(Document);

    d->statusIcon = icon;
    emit statusIconChanged(this);
}

QIcon Document::statusIcon() const
{
    Q_D(const Document);

    return d->statusIcon;
}

void Document::closeViews()
{
    Q_D(const Document);

    for (auto* const area : d->controller->allAreas()) {
        const QList<Sublime::View*> areaViews = area->views();
        for (Sublime::View* view : areaViews) {
            if (views().contains(view)) {
                area->removeView(view);
                delete view;
            }
        }
    }
    Q_ASSERT(views().isEmpty());
}

bool Document::askForCloseFeedback()
{
   return true;
}

bool Document::closeDocument(bool silent)
{
    if( !silent && !askForCloseFeedback() )
        return false;
    closeViews();
    deleteLater();
    return true;
}

QIcon Document::icon() const
{
    QIcon ret = statusIcon();
    if (!ret.isNull()) {
        return ret;
    }
    return defaultIcon();
}

QIcon Document::defaultIcon() const
{
    return QIcon();
}

}

#include "moc_document.cpp"
