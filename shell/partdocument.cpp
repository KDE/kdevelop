/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/mainwindow.h>

#include "core.h"
#include "uicontroller.h"
#include "partcontroller.h"
#include "documentcontroller.h"

namespace KDevelop {

struct PartDocumentPrivate {
    QMap<QWidget*, KParts::Part*> partForView;
};

PartDocument::PartDocument(const KUrl &url, ICore* core)
    : Sublime::UrlDocument(core->uiController()->controller(), url), KDevelop::IDocument(core), d(new PartDocumentPrivate)
{
}

PartDocument::~PartDocument()
{
    delete d;
}

QWidget *PartDocument::createViewWidget(QWidget */*parent*/)
{
    KParts::Part *part = Core::self()->partManagerInternal()->createPart(url());
    if( part )
    {
        Core::self()->partManager()->addPart(part);
        QWidget *w = part->widget();
        d->partForView[w] = part;
        return w;
    }
    return 0;
}

KParts::Part *PartDocument::partForView(QWidget *view) const
{
    return d->partForView[view];
}



//KDevelop::IDocument implementation


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
    return Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView()->document() == this;
}

bool PartDocument::save(DocumentSaveMode /*mode*/)
{
    //part document is read-only so do nothing here
    return true;
}

void PartDocument::close()
{
    //close all views and then delete ourself
    ///@todo test this
    foreach (Sublime::Area *area, Core::self()->uiControllerInternal()->areas())
    {
        QList<Sublime::View*> areaViews = area->views();
        foreach (Sublime::View *view, areaViews) {
            if (views().contains(view)) {
                area->removeView(view);
                view->deleteLater();
            }
        }
    }

    foreach (KParts::Part* part, d->partForView.values())
        part->deleteLater();

    Core::self()->documentControllerInternal()->notifyDocumentClosed(this);

    // Here we go...
    deleteLater();
}

void PartDocument::reload()
{
    //part document is read-only so do nothing here
}

IDocument::DocumentState PartDocument::state() const
{
    return Clean;
}

void PartDocument::activate(Sublime::View *activeView)
{
    KParts::Part *part = partForView(activeView->widget());
    if (Core::self()->partManager()->activePart() != part)
        Core::self()->partManager()->setActivePart(part);
    notifyActivated();
}

void PartDocument::setCursorPosition(const KTextEditor::Cursor &cursor)
{
    //do nothing here
    Q_UNUSED(cursor);
}

KUrl PartDocument::url() const
{
    return Sublime::UrlDocument::url();
}

}

#include "partdocument.moc"

