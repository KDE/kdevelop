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

namespace Sublime {

// struct DocumentPrivate

struct DocumentPrivate {
    DocumentPrivate(Document *doc): m_document(doc) {}

    void removeView(Sublime::View* view)
    {
        views.removeAll(view);
        emit m_document->viewNumberChanged(m_document);
        //no need to keep empty document - we need to remove it
        if (views.count() == 0)
        {
            emit m_document->aboutToDelete(m_document);
            m_document->deleteLater();
        }
    }

    Controller *controller;
    QList<View*> views;
    QIcon statusIcon;
    QString documentToolTip;

    Document *m_document;
};



//class Document

Document::Document(const QString &title, Controller *controller)
    :QObject(controller), d( new DocumentPrivate(this) )
{
    setObjectName(title);
    d->controller = controller;
    d->controller->addDocument(this);
    // Functor will be called after destructor has run -> capture controller pointer by value
    // otherwise we crash because we access the already freed pointer this->d
    connect(this, &Document::destroyed, d->controller,
            [controller] (QObject* obj) { controller->removeDocument(static_cast<Document*>(obj)); });
}

Document::~Document()
{
    delete d;
}

Controller *Document::controller() const
{
    return d->controller;
}

View *Document::createView()
{
    View *view = newView(this);
    connect(view, &View::destroyed, this, [&] (QObject* obj) { d->removeView(static_cast<View*>(obj)); });
    d->views.append(view);
    return view;
}

const QList<View*> &Document::views() const
{
    return d->views;
}

QString Document::title() const
{
    return objectName();
}

QString Document::toolTip() const
{
    return d->documentToolTip;
}

void Document::setTitle(const QString& newTitle)
{
    setObjectName(newTitle);
    emit titleChanged(this);
}

void Document::setToolTip(const QString& newToolTip)
{
    d->documentToolTip=newToolTip;
}

View *Document::newView(Document *doc)
{
    //first create View, second emit the signal
    View *newView = new View(doc);
    emit viewNumberChanged(this);
    return newView;
}


void Document::setStatusIcon(QIcon icon)
{
    d->statusIcon = icon;
    emit statusIconChanged(this);
}

QIcon Document::statusIcon() const
{
    return d->statusIcon;
}

void Document::closeViews()
{
    foreach (Sublime::Area *area, controller()->allAreas())
    {
        QList<Sublime::View*> areaViews = area->views();
        foreach (Sublime::View *view, areaViews) {
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
