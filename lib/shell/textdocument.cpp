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
#include "textdocument.h"

#include <QPointer>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include "partcontroller.h"

namespace KDevelop {

struct TextDocumentPrivate {
    TextDocumentPrivate()
    {
        document = 0;
    }
    QPointer<KTextEditor::Document> document;
};


TextDocument::TextDocument(PartController *partController, Sublime::Controller *controller, const KUrl &url)
    :PartDocument(partController, controller, url)
{
    d = new TextDocumentPrivate();
}

TextDocument::~TextDocument()
{
    delete d;
}

QWidget *TextDocument::createViewWidget(QWidget *parent)
{
    if (!d->document)
    {
        d->document = partController()->createTextPart(url(), "", !url().isEmpty());
        partController()->addPart(d->document);
        return d->document->widget();
    }
    return d->document->createView(parent);
}

KParts::Part *TextDocument::partForView(QWidget *view) const
{
    if (d->document && d->document->views().contains((KTextEditor::View*)view))
        return d->document;
    return 0;
}



// KDevelop::IDocument implementation

void TextDocument::close()
{
}

void TextDocument::reload()
{
}

void TextDocument::save()
{
}

bool TextDocument::isActive() const
{
    return false;
}

IDocument::DocumentState TextDocument::state() const
{
    return Clean;
}

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
