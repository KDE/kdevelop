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
#include "document.h"

#include "view.h"
#include "controller.h"

namespace Sublime {

// struct DocumentPrivate

struct DocumentPrivate {
    Controller *controller;
    QList<View*> views;
};



//class Document

Document::Document(Controller *controller)
    :QObject(controller)
{
    d = new DocumentPrivate();
    d->controller = controller;
    d->controller->addDocument(this);
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
    View *view = new View(this);
    connect(view, SIGNAL(destroyed()), this, SLOT(removeView()));
    d->views.append(view);
    return view;
}

const QList<View*> &Document::views() const
{
    return d->views;
}

QString Document::title() const
{
    return "Document";
}

void Document::removeView()
{
    View *view = qobject_cast<Sublime::View*>(sender());
    d->views.removeAll(view);
}

}

#include "document.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
