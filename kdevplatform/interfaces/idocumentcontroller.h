/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDOCUMENTCONTROLLER_H
#define KDEVPLATFORM_IDOCUMENTCONTROLLER_H

#include "idocument.h"
#include "interfacesexport.h"

#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <KTextEditor/Range>

#include <QList>
#include <QObject>
#include <QUrl>

namespace KTextEditor {
    class View;
}

namespace KDevelop {

class ICore;

class KDEVPLATFORMINTERFACES_EXPORT IDocumentFactory {
public:
    virtual ~IDocumentFactory() {}
    virtual IDocument* create(const QUrl&, ICore* ) = 0;
};

/**
 *
 * Allows to access the open documents and also open new ones
 *
 * @class IDocumentController
 */
class KDEVPLATFORMINTERFACES_EXPORT IDocumentController: public QObject {
    Q_OBJECT
public:
    enum DocumentActivation
    {
        DefaultMode = 0,            /**Activate document and create a view if no other flags passed.*/
        DoNotActivate = 1,          /**Don't activate the Document.*/
        DoNotCreateView = 2,        /**Don't create and show the view for the Document.*/
        DoNotFocus = 4,             /**Don't change the keyboard focus.*/
        DoNotAddToRecentOpen = 8,   /**Don't add the document to the File/Open Recent menu.*/
    };
    Q_DECLARE_FLAGS(DocumentActivationParams, DocumentActivation)

    explicit IDocumentController(QObject *parent);

    /**
     * @return whether @p url matches KDevelop's untitled document URL pattern.
     *
     * @note An untitled document URL is a notional unique numbered IDocument::url() that disambiguates
     *       unsaved text documents, all of which have the same empty KTextEditor::Document::url().
     */
    virtual bool isUntitledDocumentUrl(const QUrl& url) const = 0;

    /**
     * Finds the first document object corresponding to a given url.
     *
     * @param url The Url of the document.
     * @return The corresponding document, or null if not found.
     */
    virtual KDevelop::IDocument* documentForUrl( const QUrl & url ) const = 0;

    /// @return The list of all open documents
    virtual QList<KDevelop::IDocument*> openDocuments() const = 0;

    /**
     * Returns the currently active or focused document.
     *
     * @return The active document.
     */
    virtual KDevelop::IDocument* activeDocument() const = 0;

    virtual void activateDocument( KDevelop::IDocument * document, const KTextEditor::Range& range = KTextEditor::Range::invalid() ) = 0;

    virtual void registerDocumentForMimetype( const QString&, KDevelop::IDocumentFactory* ) = 0;

    enum class SaveSelectionMode {
        LetUserSelect,
        DontAskUser
    };
    virtual bool saveAllDocuments(SaveSelectionMode mode = SaveSelectionMode::LetUserSelect) = 0;
    virtual bool saveSomeDocuments(const QList<IDocument*>& list,
                                   SaveSelectionMode mode = SaveSelectionMode::LetUserSelect) = 0;
    virtual bool saveAllDocumentsForWindow(KParts::MainWindow* mw, SaveSelectionMode mode,
                                           bool currentAreaOnly = false) = 0;

    /// Opens a text document containing the @p data text.
    virtual KDevelop::IDocument* openDocumentFromText( const QString& data ) = 0;

    virtual IDocumentFactory* factory(const QString& mime) const = 0;

    /**
     * @returns the KTextEditor::View of the current document, in case it is a text document
     */
    virtual KTextEditor::View* activeTextDocumentView() const = 0;

public Q_SLOTS:
    /**
     * Opens a new or existing document.
     *
     * @param url The full Url of the document to open.
     * @param cursor The location information, if applicable.
     * @param activationParams Indicates whether to fully activate the document.
     * @param encoding the encoding for the document, the name must be accepted by QTextCodec,
     *                 if an empty encoding name is given, the document should fallback to its
     *                 own default encoding, e.g. the system encoding or the global user settings
     */
    KDevelop::IDocument* openDocument( const QUrl &url,
            const KTextEditor::Cursor& cursor,
            DocumentActivationParams activationParams = {},
            const QString& encoding = {});

    /**
     * Opens a new or existing document.
     *
     * @param url The full Url of the document to open.
     * @param range The range of text to select, if applicable.
     * @param activationParams Indicates whether to fully activate the document
     * @param encoding the encoding for the document, the name must be accepted by QTextCodec,
     *                 if an empty encoding name is given, the document should fallback to its
     *                 own default encoding, e.g. the system encoding or the global user settings
     * @param buddy Optional buddy document. If 0, the registered IBuddyDocumentFinder
     *              for the URL's mimetype will be queried to find a fitting buddy.
     *              If a buddy was found (or passed) @p url will be opened next
     *              to its buddy.
     *
     * @return The opened document
     */
    virtual KDevelop::IDocument* openDocument( const QUrl &url,
            const KTextEditor::Range& range = KTextEditor::Range::invalid(),
            DocumentActivationParams activationParams = {},
            const QString& encoding = {},
            IDocument* buddy = nullptr) = 0;

    /**
     * Opens a document from the IDocument instance.
     *
     * @param doc The IDocument to add
     * @param range The location information, if applicable.
     * @param activationParams Indicates whether to fully activate the document.
     * @param buddy Optional buddy document. If 0, the registered IBuddyDocumentFinder
     *              for the Documents mimetype will be queried to find a fitting buddy.
     *              If a buddy was found (or passed) @p url will be opened next
     *              to its buddy.
     */
    virtual bool openDocument(IDocument* doc,
            const KTextEditor::Range& range = KTextEditor::Range::invalid(),
            DocumentActivationParams activationParams = {},
            IDocument* buddy = nullptr) = 0;

    /**
     * Opens a new or existing document.
     *
     * @param url The full Url of the document to open.
     * @param prefName The name of the preferred KPart to open that document
     */
    virtual KDevelop::IDocument* openDocument( const QUrl &url, const QString& prefName ) = 0;

    virtual bool closeAllDocuments() = 0;

Q_SIGNALS:
    /// Emitted when the document has been activated.
    void documentActivated( KDevelop::IDocument* document );

    /**
     * Emitted whenever the active cursor jumps from one document+cursor to another,
     * as e.g. caused by a call to openDocument(..).
     *
     * This is also emitted when a document is only activated. Then previousDocument is zero.
     */
    void documentJumpPerformed( KDevelop::IDocument* newDocument, KTextEditor::Cursor newCursor,
                                KDevelop::IDocument* previousDocument, KTextEditor::Cursor previousCursor);
    
    /// Emitted when a document has been saved.
    void documentSaved( KDevelop::IDocument* document );

    /**
     * Emitted when a document has been opened.
     *
     * NOTE: The document may not be loaded from disk/network at this point.
     * NOTE: No views exist for the document at the time this signal is emitted.
     */
    void documentOpened( KDevelop::IDocument* document );

    /**
     * Emitted when a document has been loaded.
     *
     * NOTE: No views exist for the document at the time this signal is emitted.
     */
    void documentLoaded( KDevelop::IDocument* document );

    /**
     * Emitted when a text document has been loaded, and the text document created.
     *
     * NOTE: no views exist for the document at the time this signal is emitted.
     */
    void textDocumentCreated( KDevelop::IDocument* document );

    /// Emitted when a document has been closed.
    void documentClosed( KDevelop::IDocument* document );

    /**
     * This is emitted when the document state(the relationship
     * between the file in the editor and the file stored on disk) changes.
     *
     * @note The document state affects its icon, so this signal also notifies about icon changes.
     */
    void documentStateChanged( KDevelop::IDocument* document );

    /// This is emitted when the document content changed.
    void documentContentChanged( KDevelop::IDocument* document );

    /**
     * Emitted when a document has been loaded, but before documentLoaded(..) is emitted.
     * 
     * This allows parts of kdevplatform to prepare data-structures that can be used by other parts
     * during documentLoaded(..).
     */
    void documentLoadedPrepare( KDevelop::IDocument* document );

    /// Emitted when a document url has changed.
    void documentUrlChanged(KDevelop::IDocument* document, const QUrl& previousUrl);

    friend class IDocument;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IDocumentController::DocumentActivationParams)
} // namespace KDevelop

#endif
