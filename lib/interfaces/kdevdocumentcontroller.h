/* This file is part of the KDE project
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
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/
#ifndef KDEV_DOCUMENTCONTROLLER_H
#define KDEV_DOCUMENTCONTROLLER_H

#include <kurl.h>
#include <ktrader.h>
#include <kparts/partmanager.h>

/**
@file kdevdocumentcontroller.h
KDevelop document controller interface.
*/

namespace KParts
{
class ReadOnlyPart;
}

/**Document state enum.*/
enum DocumentState
{
    Clean,             /**<Document is not touched.*/
    Modified,          /**<Document is modified inside a shell.*/
    Dirty,             /**<Document is modified by an external process.*/
    DirtyAndModified  /**<Document is modified inside a shell and at the same time by an external process.*/
};

/**
Interface to control loaded parts and other documents.
Document controller works with embedded into the shell parts. Such parts are usually editors,
GUI designers, etc.
*/
class KDevDocumentController: public KParts::PartManager
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
    @param url The URL of the document to open.
    @param lineNum The line number to place the cursor at, if applicable.
    @param col The column number to place the cursor at, if applicable.*/
    virtual void editDocument( const KURL &url, int lineNum = -1, int col = -1 ) = 0;

    /**Shows a HTML document in the documentation viewer.
    @param url The URL of the document to view.
    @param newWin If true, the new window will be created instead of using current.*/
    virtual void showDocument( const KURL &url, bool newWin = false ) = 0;

    /**Embeds a part into the main area of the mainwindow.
    @param part The part to embed.
    @param name The name of the part.
    @param shortDescription Currently not used.*/
    virtual void showPart( KParts::Part* part, const QString& name, const QString& shortDescription ) = 0;

    /**Finds the embedded part corresponding to a given URL.
    @param url The URL of the document.
    @return The corresponding part, 0 if not found.*/
    virtual KParts::ReadOnlyPart *partForURL( const KURL & url ) = 0;

    /**Finds the embedded part corresponding to a given main widget
    @param widget The parts main widget.
    @return The corresponding part, 0 if not found.*/
    virtual KParts::Part *partForWidget( const QWidget *widget ) = 0;

    /**@return The list of open documents*/
    virtual KURL::List openURLs() = 0;

    /**Saves all open documents.
     @return false if it was cancelled by the user, true otherwise */
    virtual bool saveAllDocuments() = 0;

    /**Saves a list of documents.
    @param list The list of URLs to save.
    @return false if it was cancelled by the user, true otherwise */
    virtual bool saveDocuments( const KURL::List &list ) = 0;

    /**Reloads all open documents.*/
    virtual void reloadAllDocuments() = 0;

    /**Reloads a list of documents.
    * @param list The list of URLs to reload.*/
    virtual void reloadDocuments( const KURL::List &list ) = 0;

    /**Closes all open documents.*/
    virtual bool closeAllDocuments() = 0;

    /**Closes a list of documents.
    @param list The list of URLs for the documents to close.*/
    virtual bool closeDocuments( const KURL::List &list ) = 0;

    /**Closes this part (closes the window/tab for this part).
    @param part The part to close.
    @return true if the part was sucessfuly closed.*/
    virtual bool closePart( KParts::Part *part ) = 0;

    /**Activate this part.
    @param part The part to activate.*/
    virtual void activatePart( KParts::Part * part ) = 0;

    /**Checks the state of a document.
    @param url The URL to check.
    @return The DocumentState enum corresponding to the document state.*/
    virtual DocumentState documentState( KURL const & url ) = 0;

    /**Refers to the document currently active or focused.
    @return The URL of the active document.*/
    virtual KURL activeDocument() = 0;

signals:

    /**Emitted when the document is given focus or activated.*/
    void documentActivated( const KURL & );

    /**Emitted when a document has been saved.*/
    void documentSaved( const KURL & );

    /**Emitted when a document has been loaded.*/
    void documentLoaded( const KURL & );

    /**Emitted when a document has been closed.*/
    void documentClosed( const KURL & );

    /**Emitted when a document has been modified outside of KDevelop.*/
    void documentExternallyModified( const KURL & );

    /**This is typically emitted when an editorpart does "save as"
    which will change the document's URL from 'old' to 'new'*/
    void documentURLChanged( const KURL &oldURL, const KURL &newURL );

    /**This is emitted when the document changes, either internally
    or on disc.*/
    void documentStateChanged( const KURL &, DocumentState );

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
