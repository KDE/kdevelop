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
#ifndef KDEV_PARTDOCUMENT_H
#define KDEV_PARTDOCUMENT_H

#include <idocument.h>
#include <sublime/urldocument.h>

namespace KParts {
class Part;
}

namespace KDevelop {

/**
The generic document which represents KParts.

This document is used by shell when more specific document classes
are incapable of loading the url.

This document loads one KPart (read-only or read-write) per view
and sets part widget to be a view widget.
*/
class PartDocument: public Sublime::UrlDocument, public KDevelop::IDocument {
    Q_OBJECT
public:
    PartDocument(const KUrl &url, ICore* core );
    virtual ~PartDocument();

    virtual KUrl url() const;

    virtual QWidget *createViewWidget(QWidget *parent = 0);
    virtual KParts::Part *partForView(QWidget *view) const;

    virtual KMimeType::Ptr mimeType() const;
    virtual KTextEditor::Document* textDocument() const;
    virtual bool save(DocumentSaveMode mode = Default);
    virtual void reload();
    virtual void close();
    virtual bool isActive() const;
    virtual DocumentState state() const;

    virtual void activate(Sublime::View *activeView);
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor);

private:
    class PartDocumentPrivate * const d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
