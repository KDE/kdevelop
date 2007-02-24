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
    :Sublime::PartDocument(controller, url)
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
        part = d->partController->createPart(url());
    d->partController->addPart(part);
    QWidget *w = part->widget();
    d->partForWidget[w] = part;
    return w;
}

KParts::Part *PartDocument::partForWidget(QWidget *w)
{
    return d->partForWidget[w];
}

}

#include "partdocument.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
