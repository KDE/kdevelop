/*
    SPDX-FileCopyrightText: 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2003 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2004-2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_DOCUMENTCONTROLLER_H
#define KDEVPLATFORM_DOCUMENTCONTROLLER_H

#include <QList>

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
class MainWindow;
class DocumentControllerPrivate;

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
    explicit DocumentController( QObject *parent = nullptr );
    ~DocumentController() override;

    bool isUntitledDocumentUrl(const QUrl& url) const override;

    /**Finds the first document object corresponding to a given url.
    @param url The Url of the document.
    @return The corresponding document, or null if not found.*/
    IDocument* documentForUrl( const QUrl & url ) const override;

    /**@return The list of open documents*/
    QList<IDocument*> openDocuments() const override;

    /**Refers to the document currently active or focused.
    @return The active document.*/
    IDocument* activeDocument() const override;

    KTextEditor::View* activeTextDocumentView() const override;
    
    /// Activate the given \a document. This convenience function does not add the document
    /// to the File/Recent Open menu. Use DocumentController::openDocument if that is desired.
    void activateDocument( IDocument * document, const KTextEditor::Range& range = KTextEditor::Range::invalid() ) override;

    void registerDocumentForMimetype( const QString&, KDevelop::IDocumentFactory* ) override;

    /// Request the document controller to save all documents.
    /// If the \a mode is not IDocument::Silent, ask the user which documents to save.
    /// Returns false if the user cancels the save dialog.
    bool saveAllDocuments(IDocument::DocumentSaveMode mode) override;
    bool saveAllDocumentsForWindow(KParts::MainWindow* mw, IDocument::DocumentSaveMode mode, bool currentAreaOnly = false) override;

    void initialize();

    void cleanup();

    virtual QStringList documentTypes() const;

    QString documentType(Sublime::Document* document) const;

    using IDocumentController::openDocument;

    /**
     * @return whether @p url is a URL of an empty document.
     *
     * @note This function is equivalent to isUntitledDocumentUrl(), but static and therefore more efficient.
     */
    static bool isEmptyDocumentUrl(const QUrl &url);
    static QUrl nextEmptyDocumentUrl();
    
    IDocumentFactory* factory(const QString& mime) const override;

    
    bool openDocument(IDocument* doc,
                              const KTextEditor::Range& range = KTextEditor::Range::invalid(),
                              DocumentActivationParams activationParams = {},
                              IDocument* buddy = nullptr) override;
    
public Q_SLOTS:
    /**Opens a new or existing document.
    @param url The full Url of the document to open. If it is empty, a dialog to choose the document will be opened.
    @param range The location information, if applicable.
    @param activationParams Indicates whether to fully activate the document.
    @param buddy The buddy document
    @return The opened document
    */
    IDocument* openDocument( const QUrl &url,
            const KTextEditor::Range& range = KTextEditor::Range::invalid(),
            DocumentActivationParams activationParams = {},
            const QString& encoding = {},
            IDocument* buddy = nullptr ) override;

    IDocument* openDocumentFromText( const QString& data ) override;

    KDevelop::IDocument* openDocument( const QUrl &url, const QString& prefName ) override;

    void fileClose();
    void slotSaveAllDocuments();
    bool closeAllDocuments() override;
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
    Q_SCRIPTABLE QString activeDocumentPath(const QString& target = {}) const;

    // Returns all open documents in the current area
    Q_SCRIPTABLE QStringList activeDocumentPaths() const;
    void vcsAnnotateCurrentDocument();

private Q_SLOTS:
    virtual void slotOpenDocument(const QUrl &url);
    void notifyDocumentClosed(Sublime::Document* doc);

private:
    bool openDocumentsWithSplitSeparators( Sublime::AreaIndex* index, QStringList urlsWithSeparators, bool& isFirstView );
    QList<IDocument*> visibleDocumentsInWindow(MainWindow* mw) const;
    QList<IDocument*> documentsExclusivelyInWindow(MainWindow* mw, bool currentAreaOnly = false) const;
    QList<IDocument*> modifiedDocuments(const QList<IDocument*>& list) const;

    bool saveSomeDocuments(const QList<IDocument*>& list, IDocument::DocumentSaveMode mode) override;

    void setupActions();

private:
    const QScopedPointer<class DocumentControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DocumentController)

    friend class DocumentControllerPrivate;
};

}

#endif

