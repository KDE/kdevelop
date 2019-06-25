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
    Q_D(Document);

    setObjectName(title);
    d->controller->addDocument(this);
    // Functor will be called after destructor has run -> capture controller pointer by value
    // otherwise we crash because we access the already freed pointer this->d
    connect(this, &Document::destroyed, d->controller,
            [controller] (QObject* obj) { controller->removeDocument(static_cast<Document*>(obj)); });
}

Document::~Document() = default;

Controller *Document::controller() const
{
    Q_D(const Document);

    return d->controller;
}

View *Document::createView()
{
    Q_D(Document);

    View *view = newView(this);
    connect(view, &View::destroyed,
            this, [this] (QObject* obj) { Q_D(Document); d->removeView(static_cast<View*>(obj)); });
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
    for (Sublime::Area* area : controller()->allAreas()) {
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
