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
#include "document.h"

#include <sublime/view.h>
#include <sublime/mainwindow.h>

#include "core.h"
#include "uicontroller.h"
#include "documentcontroller.h"

namespace KDevelop {

struct DocumentPrivate {
};

Document::Document(const KUrl &url)
    :Sublime::UrlDocument(Core::self()->uiControllerInternal(), url)
{
    d = new DocumentPrivate();
}

Document::~Document()
{
    delete d;
}

void Document::notifyStateChanged()
{
    Core::self()->documentControllerInternal()->emitStateChanged(this);
}

void Document::notifySaved()
{
    Core::self()->documentControllerInternal()->emitSaved(this);
}

void Document::notifyActivated()
{
    Core::self()->documentControllerInternal()->emitActivated(this);
}

void Document::activate(Sublime::View */*activeView*/)
{
    notifyActivated();
}



//KDevelop::IDocument implementation

KUrl Document::url() const
{
    return Sublime::UrlDocument::url();
}

}

#include "document.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
