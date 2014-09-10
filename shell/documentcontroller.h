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
#ifndef KDEVPLATFORM_DOCUMENTCONTROLLER_H
#define KDEVPLATFORM_DOCUMENTCONTROLLER_H

#include <QtCore/QList>

#include <interfaces/idocumentcontroller.h>

#include "shellexport.h"

namespace KTextEditor {
class View;
}

namespace Sublime {
    class Document;
    class Area;
    class AreaIndex;
}

namespace KDevelop {
class ProjectFileItem;
class IProject;
class MainWindow;

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
    virtual IDocument* documentForUrl( const QUrl & url ) const;

    /**@return The list of open documents*/
    virtual QList<IDocument*> openDocuments() const;

    /**Refers to the document currently active or focused.
    @return The active document.*/
    virtual IDocument* activeDocument() const;

    virtual KTextEditor::View* activeTextDocumentView() const;
    
    virtual void activateDocument( IDocument * document, const KTextEditor::Range& range = KTextEditor::Range::invalid() );

    virtual void registerDocumentForMimetype( const QString&, KDevelop::IDocumentFactory* );

    /// Request the document controller to save all documents.
    /// If the \a mode is not IDocument::Silent, ask the user which documents to save.
    /// Returns false if the user cancels the save dialog.
    virtual bool saveAllDocuments(IDocument::DocumentSaveMode mode);
    bool saveAllDocumentsForWindow(KParts::MainWindow* mw, IDocument::DocumentSaveMode mode, bool currentAreaOnly = false);

    void initialize();

    void cleanup();

    virtual QStringList documentTypes() const;

    QString documentType(Sublime::Document* document) const;

    using IDocumentController::openDocument;

    /**checks that url is an url of empty document*/
    static bool isEmptyDocumentUrl(const QUrl &url);
    static QUrl nextEmptyDocumentUrl();
    
    virtual IDocumentFactory* factory(const QString& mime) const;

    
    virtual bool openDocument(IDocument* doc,
                              const KTextEditor::Range& range = KTextEditor::Range::invalid(),
                              DocumentActivationParams activationParams = 0,
                              IDocument* buddy = 0);

    virtual KTextEditor::Document* globalTextEditorInstance();
    
public Q_SLOTS:
    /**Opens a new or existing document.
    @param url The full Url of the document to open. If it is empty, a dialog to choose the document will be opened.
    @param range The location information, if applicable.
    @param activationParams Indicates whether to fully activate the document.
    @param buddy The buddy document
    @return The opened document
    */
    virtual Q_SCRIPTABLE IDocument* openDocument( const QUrl &url,
            const KTextEditor::Range& range = KTextEditor::Range::invalid(),
            DocumentActivationParams activationParams = 0,
            const QString& encoding = "",
            IDocument* buddy = 0 );

    virtual Q_SCRIPTABLE IDocument* openDocumentFromText( const QString& data );
    
    virtual KDevelop::IDocument* openDocument( const QUrl &url, const QString& prefname );

    virtual void closeDocument( const QUrl &url );
    void fileClose();
    void slotSaveAllDocuments();
    virtual void closeAllDocuments();
    void closeAllOtherDocuments();
    void reloadAllDocuments();

    // DBUS-compatible versions of openDocument
    virtual Q_SCRIPTABLE bool openDocumentSimple( QString url, int line = -1, int column = 0 );
    // Opens a list of documents, with optional split-view separators, like: "file1 / [ file2 - fil3 ]" (see kdevplatform_shell_environment.sh)
    virtual Q_SCRIPTABLE bool openDocumentsSimple( QStringList urls );
    virtual Q_SCRIPTABLE bool openDocumentFromTextSimple( QString text );
    
    // If 'target' is empty, returns the currently active document, or
    // the currently selected project-item if no document is active.
    // If 'target' is "[selection]", returns the path of the currently active selection.
    // If 'target' is the name of a project, returns the root-path of that project.
    // Whenever the returned path corresponds to a directory, a '/.' suffix is appended.
    Q_SCRIPTABLE QString activeDocumentPath(QString target="") const;

    // Returns all open documents in the current area
    Q_SCRIPTABLE QStringList activeDocumentPaths() const;
    void vcsAnnotateCurrentDocument();

    void reloaded(KTextEditor::Document* doc);
    
private Q_SLOTS:
    virtual void slotOpenDocument(const QUrl &url);
    void notifyDocumentClosed(Sublime::Document* doc);

private:
    bool openDocumentsWithSplitSeparators( Sublime::AreaIndex* index, QStringList urlsWithSeparators, bool& isFirstView );
    QList<IDocument*> visibleDocumentsInWindow(MainWindow* mw) const;
    QList<IDocument*> documentsExclusivelyInWindow(MainWindow* mw, bool currentAreaOnly = false) const;
    QList<IDocument*> modifiedDocuments(const QList<IDocument*>& list) const;

    bool saveSomeDocuments(const QList<IDocument*>& list, IDocument::DocumentSaveMode mode);

    void setupActions();
    Q_PRIVATE_SLOT(d, void removeDocument(Sublime::Document*))
    Q_PRIVATE_SLOT(d, void chooseDocument())
    Q_PRIVATE_SLOT(d, void changeDocumentUrl(KDevelop::IDocument*))

    friend struct DocumentControllerPrivate;
    struct DocumentControllerPrivate *d;
};

}

#endif

