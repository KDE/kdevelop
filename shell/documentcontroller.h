/* This document is part of the KDE project
Copyright 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright 2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright 2003 Roberto Raggi <roberto@kdevelop.org>
Copyright 2003 Hamish Rodda <rodda@kde.org>
Copyright 2003 Harald Fernengel <harry@kdevelop.org>
Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
Copyright 2005 Adam Treat <treat@kde.org>
Copyright 2004-2007 Alexander Dymo <adymo@kdevelop.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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

#include <QtCore/QList>

#include <idocumentcontroller.h>

#include <shellexport.h>

namespace KDevelop {

/**
 * \short Interface to control open documents.
 * The document controller manages open documents in the IDE.
 * Open documents are usually editors, GUI designers, html documentation etc.
 *
 * Please note that this interface gives access to documents and not to their views.
 * It is possible that more than 1 view is shown in KDevelop for a document.
*/
class KDEVPLATFORMSHELL_EXPORT DocumentController: public IDocumentController {
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kdevelop.DocumentController" )
public:
    /**Constructor.
    @param parent The parent object.*/
    DocumentController( QObject *parent = 0 );
    virtual ~DocumentController();

    /**Call this before a call to @ref editDocument to set the encoding of the
    document to be opened.
    @param encoding The encoding to open as.*/
    virtual void setEncoding( const QString &encoding );
    virtual QString encoding() const;

    /**Finds the first document object corresponding to a given url.
    @param url The Url of the document.
    @return The corresponding document, or null if not found.*/
    virtual IDocument* documentForUrl( const KUrl & url ) const;

    /**@return The list of open documents*/
    virtual QList<IDocument*> openDocuments() const;

    /**Refers to the document currently active or focused.
    @return The active document.*/
    virtual IDocument* activeDocument() const;

    virtual void activateDocument( IDocument * document );

    virtual void registerDocumentForMimetype( const QString&, KDevelop::IDocumentFactory* );

public Q_SLOTS:
    /**Opens a new or existing document.
    @param url The full Url of the document to open.
    @param range The location information, if applicable.
    @param activate Indicates whether to fully activate the document.*/
    virtual Q_SCRIPTABLE IDocument* openDocument( const KUrl &url,
            const KTextEditor::Cursor& range = KTextEditor::Cursor::invalid(),
            DocumentActivation activate = IDocumentController::ActivateOnOpen );

    virtual void closeDocument( const KUrl &url );

private:
    struct DocumentControllerPrivate *d;
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
