/* This document is part of the KDE project
  Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
  Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
  Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
  Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
  Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
  Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>
  Copyright (C) 2005 Adam Treat <treat@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the document COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef KDEV_DOCUMENTCONTROLLER_H
#define KDEV_DOCUMENTCONTROLLER_H

#include <kurl.h>
#include <ktrader.h>
#include <kparts/partmanager.h>
#include <ktexteditor/cursor.h>

#include "kdevdocument.h"

/**
@document kdevdocumentcontroller.h
KDevelop document controller interface.
*/

namespace KParts
{
class Part;
}

namespace KTextEditor
{
class Document;
}

class KDevDocument;

/**
 * \short Interface to control loaded parts and other documents.
 * The document controller enables parts to be embedded into the IDE, and control of them.
 * Such parts are usually editors, GUI designers, etc.
*/
class KDEVINTERFACES_EXPORT KDevDocumentController: public KParts::PartManager
{
    Q_OBJECT

public:
    /**Constructor.
    @param parent The parent object.*/
    KDevDocumentController( QWidget *parent );

    /**Call this before a call to @ref editDocument to set the encoding of the
    document to be opened.
    @param encoding The encoding to open as.*/
    virtual void setEncoding( const QString &encoding ) = 0;

    /**Opens a new or existing document.
    @param url The Url of the document to open.
    @param line The line number to place the cursor at, if applicable.
    @param col The column number to place the cursor at, if applicable.
    @param newWin If true, the new window will be created instead of using current.*/
    virtual KDevDocument* editDocument( const KUrl &url, const KTextEditor::Cursor& range = KTextEditor::Cursor::invalid() ) = 0;

    /**Shows a read-only document in the documentation viewer.
    @param url The Url of the document to view.*/
    virtual KDevDocument* showDocumentation( const KUrl &url, bool newWin ) = 0;

    /**Embeds a part into the main area of the mainwindow.
    @param part The part to embed.
    @param name The name of the part.
    @param shortDescription Currently not used.*/
    virtual KDevDocument* showPart( KParts::Part* part, const QString& name, const QString& shortDescription ) = 0;

    /**Finds the document object corresponding to a given url.
    @param url The Url of the document.
    @return The corresponding document, or null if not found.*/
    virtual KDevDocument* documentForUrl( const KUrl & url ) const;

    /**Finds the document representing an embedded part.
    @param part the embedded part
    @return The corresponding document, or null if not found.*/
    virtual KDevDocument* documentForPart( KParts::Part * part ) const = 0;

    /**Finds the embedded part corresponding to a given url.
    @param url The Url of the document.
    @return The corresponding part, 0 if not found.*/
    KParts::Part* partForUrl( const KUrl & url ) const;

    /**Finds the embedded part corresponding to a given main widget
    @param widget The parts main widget.
    @return The corresponding part, 0 if not found.*/
    virtual KParts::Part* partForWidget( const QWidget *widget ) const = 0;

    /**@return The list of open documents*/
    virtual QList<KDevDocument*> openDocuments() const = 0;

    /**Saves all open documents.
     @return false if it was cancelled by the user, true otherwise */
    virtual bool saveAllDocuments() = 0;

    /**Saves a single document.
    @param doc the document to save
    @param force if true, force save even if the file was not modified.
    @return false if it was cancelled by the user, true otherwise */
    virtual bool saveDocument( KDevDocument* document, bool force = false ) = 0;

    /**Saves a list of documents.
    @param list The list of Urls to save.
    @return false if it was cancelled by the user, true otherwise */
    virtual bool saveDocuments( const QList<KDevDocument*> &list ) = 0;

    /**Reloads all open documents.*/
    virtual void reloadAllDocuments() = 0;

    /**Reloads a document.
    * @param url The document to reload.*/
    virtual void reloadDocument( KDevDocument* document ) = 0;

    /**Reloads a list of documents.
    * @param list The documents to reload.*/
    virtual void reloadDocuments( const QList<KDevDocument*> &list ) = 0;

    /**Closes a document.
    * @param url The document to close.*/
    virtual bool closeDocument( KDevDocument* document ) = 0;

    /**Closes all open documents.*/
    virtual bool closeAllDocuments() = 0;

    /**Closes a list of documents.
    @param list The list of documents to close.*/
    virtual bool closeDocuments( const QList<KDevDocument*> &list ) = 0;

    /**Closes all other open documents.
    @param document The document not to close.*/
    virtual bool closeAllOthers( KDevDocument* document ) = 0;

    /**Activate this part.
    @param part The part to activate.*/
    virtual void activateDocument( KDevDocument* document ) = 0;

    /**Refers to the document currently active or focused.
    @return The Url of the active document.*/
    virtual KDevDocument* activeDocument() const = 0;

    /** Convenience function to proved the url of the currently active document, if one exists.
    @return The Url of the active document.*/
    KUrl activeDocumentUrl() const;

    /**Checks the state of a document.
    @param document The document to check
    @return The DocumentState enum corresponding to the document state.*/
    virtual KDevDocument::DocumentState documentState(KDevDocument* document) const = 0;

Q_SIGNALS:
    /**Emitted when the document is given focus or activated.*/
    void documentActivated( KDevDocument* document );

    /**Emitted when a document has been saved.*/
    void documentSaved( KDevDocument* document );

    /**Emitted when a document has been loaded.*/
    void documentLoaded( KDevDocument* document );

    /**Emitted when a document has been closed.*/
    void documentClosed( KDevDocument* document );

    /**Emitted when a document has been modified outside of KDevelop.*/
    void documentExternallyModified( KDevDocument* document );

    /**This is typically emitted when an editorpart does "save as"
    which will change the document's Url from 'old' to 'new'*/
    void documentUrlChanged( KDevDocument* document, const KUrl &oldUrl, const KUrl &newUrl );

    /**This is emitted when the document changes, either internally
    or on disc.*/
    void documentStateChanged( KDevDocument* document, KDevDocument::DocumentState state );
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
