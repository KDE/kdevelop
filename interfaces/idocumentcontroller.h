/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
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
#ifndef IDOCUMENTCONTROLLER_H
#define IDOCUMENTCONTROLLER_H

#include <QtCore/QObject>

#include <kurl.h>
#include <ktexteditor/cursor.h>

#include "interfacesexport.h"

#include "idocument.h"

namespace KDevelop {

class ICore;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentFactory {
public:
    virtual ~IDocumentFactory() {}
    virtual IDocument* create(const KUrl&, ICore* ) = 0;
};


class KDEVPLATFORMINTERFACES_EXPORT IDocumentController: public QObject {
    Q_OBJECT
public:
    enum DocumentActivation
    {
        ActivateOnOpen,        /**Activate Document on Opening.*/
        DontActivate           /**Don't activate the Document.*/
    };
    IDocumentController(QObject *parent);

    /**Call this before a call to @ref editDocument to set the encoding of the
    document to be opened.
    @param encoding The encoding to open as.*/
    virtual void setEncoding( const QString &encoding ) = 0;
    virtual QString encoding() const = 0;

    /**Finds the first document object corresponding to a given url.
    @param url The Url of the document.
    @return The corresponding document, or null if not found.*/
    virtual IDocument* documentForUrl( const KUrl & url ) const = 0;

    /**@return The list of open documents*/
    virtual QList<IDocument*> openDocuments() const = 0;

    /**Refers to the document currently active or focused.
    @return The active document.*/
    virtual IDocument* activeDocument() const = 0;

    virtual void activateDocument( IDocument * document ) = 0;

    virtual void registerDocumentForMimetype( const QString&, KDevelop::IDocumentFactory* ) = 0;

    virtual void saveAllDocuments(IDocument::DocumentSaveMode mode = IDocument::Default) = 0;

public Q_SLOTS:
    /**Opens a new or existing document.
    @param url The full Url of the document to open.
    @param range The location information, if applicable.
    @param activate Indicates whether to fully activate the document.*/
    virtual Q_SCRIPTABLE IDocument* openDocument( const KUrl &url,
            const KTextEditor::Cursor& range = KTextEditor::Cursor::invalid(),
            DocumentActivation activate = ActivateOnOpen ) = 0;

    virtual void closeAllDocuments() = 0;

Q_SIGNALS:
    /**Emitted when the document has been activated.*/
    void documentActivated( KDevelop::IDocument* document );

    /**Emitted when a document has been saved.*/
    void documentSaved( KDevelop::IDocument* document );

    /**Emitted when a document has been loaded.
    Note, no views exist for the document at the time this signal is emitted.*/
    void documentLoaded( KDevelop::IDocument* document );

    /**Emitted when a document has been closed.*/
    void documentClosed( KDevelop::IDocument* document );

    /**This is emitted when the document state(the relationship
     * between the file in the editor and the file stored on disk) changes.*/
    void documentStateChanged( KDevelop::IDocument* document );

    /**This is emitted when the document content changed.*/
    void documentContentChanged( KDevelop::IDocument* document );

    friend class IDocument;
};

}

#endif

