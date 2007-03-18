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
#ifndef KDEV_PARTDOCUMENT_H
#define KDEV_PARTDOCUMENT_H

#include "idocument.h"
#include <sublime/urldocument.h>

namespace Sublime {
class Controller;
}

namespace KParts {
class Part;
}

namespace KDevelop {

class PartController;

//FIXME adymo: refactor
class PartDocument: public Sublime::UrlDocument, public IDocument {
    Q_OBJECT
public:
    PartDocument(PartController *partController, Sublime::Controller *controller, const KUrl &url);
    virtual ~PartDocument();

    virtual QWidget *createViewWidget(QWidget *parent = 0);
    KParts::Part *partForWidget(QWidget *w);

    virtual KUrl url() const;
    virtual KMimeType::Ptr mimeType() const;
    virtual KParts::Part* part() const;
    virtual KTextEditor::Document* textDocument() const;
    virtual void save();
    virtual void reload();
    virtual void close();
    virtual bool isActive() const;
    virtual DocumentState state() const;

private:
    class PartDocumentPrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
