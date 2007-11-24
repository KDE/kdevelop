/***************************************************************************
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                    *
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
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
#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include <kurl.h>
#include <kmimetype.h>
#include "interfacesexport.h"
#include <ktexteditor/cursor.h>

namespace KParts { class Part; }
namespace KTextEditor { class Document; }
namespace Sublime{ class View; }

namespace KDevelop {
class ICore;

/**
 * A single document being edited by the IDE.
 *
 * The base class for tracking a document.  It contains the URL,
 * the part, and any associated metadata for the document.
 *
 * The advantages are:
 * - an easier key for use in maps and the like
 * - a value which does not change when the filename changes
 * - clearer distinction in the code between an open document and a url
 *   (which may or may not be open)
 */
class KDEVPLATFORMINTERFACES_EXPORT IDocument {
public:
    virtual ~IDocument();

    /**Document state.*/
    enum DocumentState
    {
        Clean,             /**< Document is not touched.*/
        Modified,          /**< Document is modified inside the IDE.*/
        Dirty,             /**< Document is modified by an external process.*/
        DirtyAndModified   /**< Document is modified inside the IDE and at the same time by an external process.*/
    };

    enum DocumentSaveMode
    {
        Default = 0,
        Silent = 1
    };

    /**
     * Returns the URL of this document.
     */
    virtual KUrl url() const = 0;

    /**
     * Returns the mimetype of the document.
     */
    virtual KMimeType::Ptr mimeType() const = 0;

    /**
     * Returns the part for given @p view if this document is a KPart document or 0 otherwise.
     */
    virtual KParts::Part* partForView(QWidget *view) const = 0;

    /**
     * Returns the text editor, if this is a text document or 0 otherwise.
     */
    virtual KTextEditor::Document* textDocument() const = 0;

    /**
     * Saves the document.
     * @return true if the document was saved, false otherwise
     */
    virtual bool save(DocumentSaveMode mode = Default) = 0;

    /**
     * Reloads the document.
     */
    virtual void reload() = 0;

    /**
     * Requests that the document be closed.
     */
    virtual void close() = 0;

    /**
     * Enquires whether this document is currently active in the currently active mainwindow.
     */
    virtual bool isActive() const = 0;

    /**
    * Checks the state of this document.
    * @return The document state.
    */
    virtual DocumentState state() const = 0;

    virtual void setCursorPosition(const KTextEditor::Cursor &cursor) = 0;

    /**
     * Performs document activation actions if any.
     * This needs to call notifyActivated()
     */
    virtual void activate(Sublime::View *activeView) = 0;

protected:
    ICore* core();
    IDocument( ICore* );
    void notifySaved();
    void notifyStateChanged();
    void notifyActivated();
    void notifyContentChanged();

private:
    class IDocumentPrivate* const d;
};

}

#endif

