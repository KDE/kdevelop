/* This file is part of the KDE project
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVDOCUMENT_H
#define KDEVDOCUMENT_H

#include <QObject>
#include <QPointer>

#include <kurl.h>
#include <kmimetype.h>
#include "kdevexport.h"

namespace KParts
{
class Part;
}

namespace KTextEditor { class Document; }

class KDevDocumentController;

/**
 * \short An object which represents a single document being edited by the IDE.
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
class KDEVINTERFACES_EXPORT KDevDocument : public QObject
{
  Q_OBJECT

  public:
    /**Document state enum.*/
    enum DocumentState
    {
        Clean,             /**< Document is not touched.*/
        Modified,          /**< Document is modified inside the IDE.*/
        Dirty,             /**< Document is modified by an external process.*/
        DirtyAndModified   /**< Document is modified inside the IDE and at the same time by an external process.*/
    };

    /**
     * Constructor.
     */
    KDevDocument(KParts::Part* part, KDevDocumentController* parent);

    /**
     * Returns the document controller.
     */
    KDevDocumentController* parent() const;

    /**
     * Returns the URL of this document.
     */
    KUrl url() const;

    /**
     * Returns the part corresponding to this document.
     */
    KParts::Part* part() const;

    /**
     * Returns the mimetype of the document.
     */
    KMimeType::Ptr mimeType() const;

    /**
     * Returns the text editor, if this is a text document.
     */
    KTextEditor::Document* textDocument() const;

    /**
     * Returns the gui builder, if this is a gui document.
     */
//     KDevGuiBuilder* guiDocument() const;

    /**
     * Enquires whether this document is currently the active part in the editor.
     */
    bool isActive() const;

    /**
     * Enquires whether this document is editable..
     */
    bool isReadWrite() const;

    /**
     * Make this the active document.
     */
    void activate();

    /**
     * Requests that the document be saved.
     */
    void save();

    /**
     * Requests that the document be reloaded.
     */
    void reload();

    /**
     * Requests that the document be closed.
     */
    void close();

    /**Checks the state of the document.
    @return The DocumentState corresponding to the document state.*/
    DocumentState state() const;

  private:
    QPointer<KParts::Part> m_part;
};

#endif
