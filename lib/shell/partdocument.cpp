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
    QMap<QWidget*, KParts::Part*> partForView;
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
    KParts::Part *part = d->partController->createPart(url());
    d->partController->addPart(part);
    QWidget *w = part->widget();
    d->partForView[w] = part;
    return w;
}

KParts::Part *PartDocument::partForView(QWidget *view) const
{
    return d->partForView[view];
}

PartController *PartDocument::partController()
{
    return d->partController;
}



//KDevelop::IDocument implementation

KUrl PartDocument::url() const
{
    return Sublime::UrlDocument::url();
}

KMimeType::Ptr PartDocument::mimeType() const
{
    return KMimeType::findByUrl(url());
}

KTextEditor::Document *PartDocument::textDocument() const
{
    return 0;
}

bool PartDocument::isActive() const
{
    return false;
}

void PartDocument::save()
{
}

void PartDocument::close()
{
}

void PartDocument::reload()
{
}

IDocument::DocumentState PartDocument::state() const
{
    return Clean;
}

}

#include "partdocument.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
