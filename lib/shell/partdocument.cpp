/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#include "partdocument.h"

#include <ktexteditor/document.h>

#include "partcontroller.h"

namespace KDevelop {

struct PartDocumentPrivate {
    PartController *partController;
    QMap<QWidget*, KParts::Part*> partForWidget;
};

PartDocument::PartDocument(PartController *partController, Sublime::Controller *controller, const KUrl &url)
    :Sublime::UrlDocument(controller, url)
{
    d = new PartDocumentPrivate();
    d->partController = partController;
}

PartDocument::~PartDocument()
{
    delete d;
}

QWidget *PartDocument::createViewWidget(QWidget *parent)
{
    Q_UNUSED( parent );
    KParts::Part *part;
    if (url().isEmpty())
        part = d->partController->createTextPart(url(), "", false);
    else
        part = d->partController->createTextPart(url(), "", true);
    d->partController->addPart(part);
    QWidget *w = part->widget();
    d->partForWidget[w] = part;
    return w;
}

KParts::Part *PartDocument::partForWidget(QWidget *w)
{
    return d->partForWidget[w];
}

KUrl PartDocument::url() const
{
    return Sublime::UrlDocument::url();
}

KMimeType::Ptr PartDocument::mimeType() const
{
    return KMimeType::mimeType("text/plain");
}

void PartDocument::save()
{
}

KTextEditor::Document *PartDocument::textDocument() const
{
    return 0;
}

KParts::Part *PartDocument::part() const
{
    return 0;
}

bool PartDocument::isActive() const
{
    return false;
}

void PartDocument::close()
{
}

void PartDocument::reload()
{
}

IDocument::DocumentState KDevelop::PartDocument::state() const
{
    return Clean;
}

}

#include "partdocument.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
