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
#ifndef KDEV_TEXTDOCUMENT_H
#define KDEV_TEXTDOCUMENT_H

#include <partdocument.h>

namespace KDevelop {

/**
Text document which represents KTextEditor documents.

Usually Kate documents are represented by this class but TextDocument is not
limited to Kate. Each conforming text editor will work.
*/
class TextDocument: public PartDocument {
    Q_OBJECT
public:
    TextDocument(const KUrl &url, ICore* );
    virtual ~TextDocument();

    virtual QWidget *createViewWidget(QWidget *parent = 0);
    virtual KParts::Part *partForView(QWidget *view) const;
    virtual void close();

    virtual bool save(DocumentSaveMode mode = Default);
    virtual void reload();
    virtual DocumentState state() const;

    virtual void setCursorPosition(const KTextEditor::Cursor &cursor);

    virtual KTextEditor::Document* textDocument() const;

private:
    Q_PRIVATE_SLOT(d, void newDocumentStatus(KTextEditor::Document*))
    Q_PRIVATE_SLOT(d, void textChanged(KTextEditor::Document*))
    Q_PRIVATE_SLOT(d, void modifiedOnDisk(KTextEditor::Document *, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason))

    struct TextDocumentPrivate * const d;
    friend class TextDocumentPrivate;
};

}

#endif

