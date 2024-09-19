/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IDOCUMENT_H
#define KDEVPLATFORM_IDOCUMENT_H

#include <QObject>
#include <QUrl>
#include <QScopedPointer>

#include "interfacesexport.h"

namespace KParts { class Part; class MainWindow; }
namespace KTextEditor {
class Cursor;
class Document;
class Range;
class View;
}
namespace Sublime{ class View; }

class QIcon;
class QMimeType;
class QWidget;

namespace KDevelop {
class ICore;
class IDocumentPrivate;

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
        Default = 0x0 /**< standard save mode, gives a warning message if the file was modified outside the editor */,
        Silent = 0x1 /**< silent save mode, doesn't warn the user if the file was modified outside the editor */,
        Discard = 0x2 /**< discard mode, don't save any unchanged data */
    };

    /**
     * Returns the URL of this document.
     */
    virtual QUrl url() const = 0;

    /**
     * Returns the mimetype of the document.
     *
     * @note The returned MIME type is cached. If accuracy is paramount, consider using
     *       KTextEditor::Document::mimeType() for a text document instead.
     */
    virtual QMimeType mimeType() const = 0;

    /**
     * @return The status icon of the document, or, if none is present, an icon
     *         that resembles the document, i.e. based on its MIME type.
     */
    virtual QIcon icon() const = 0;

    /**
     * Returns the part for given @p view if this document is a KPart document or 0 otherwise.
     */
    virtual KParts::Part* partForView(QWidget *view) const = 0;

    /**
     * Set a 'pretty' name for this document. That name will be used when displaying the document in the UI,
     * instead of the filename and/or path.
     * @param name The pretty name to use. Give an empty name to reset.
     * */
    virtual void setPrettyName(const QString& name);
    
    /**
     * returns the pretty name of this document that was set through setPrettyName(...).
     * If no pretty name was set, an empty string is returned.
     * */
    virtual QString prettyName() const;
    
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
     *
     * \returns whether the document was successfully closed.
     */
    virtual bool close(DocumentSaveMode mode = Default) = 0;

    /**
     * Enquires whether this document is currently active in the currently active mainwindow.
     */
    virtual bool isActive() const = 0;

    /**
    * Checks the state of this document.
    * @return The document state.
    */
    virtual DocumentState state() const = 0;

    /**
     * Access the current text cursor position, if possible.
     *
     * \returns the current text cursor position, or an invalid cursor otherwise.
     */
    virtual KTextEditor::Cursor cursorPosition() const = 0;

    /**
     * Set the current text cursor position, if possible.
     *
     * \param cursor new cursor position.
     */
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor) = 0;

    /**
     * Retrieve the current text selection, if one exists.
     *
     * \returns the current text selection
     */
    virtual KTextEditor::Range textSelection() const;

    /**
     * Set the current text selection, if possible.
     *
     * \param range new cursor position.
     */
    virtual void setTextSelection(const KTextEditor::Range &range) = 0;

    /**
     * @returns the text in a given range
     */
    virtual QString text(const KTextEditor::Range &range) const;

    /**
     * Retrieve the current text line, if one exists.
     *
     * @returns the current text line
     */
    virtual QString textLine() const;

    /**
     * Retrieve the current text word, if one exists.
     *
     * @returns the current text word
     */
    virtual QString textWord() const;

    /**
     * Performs document activation actions if any.
     * This needs to call notifyActivated()
     */
    virtual void activate(Sublime::View *activeView, KParts::MainWindow *mainWindow) = 0;

    /**
     * @returns the active text view in case it's a text document and it has one.
     */
    virtual KTextEditor::View* activeTextView() const;

protected:
    ICore* core();
    explicit IDocument( ICore* );
    void notifySaved();
    void notifyStateChanged();
    void notifyActivated();
    void notifyContentChanged();
    void notifyTextDocumentCreated();
    void notifyUrlChanged(const QUrl& previousUrl);
    void notifyLoaded();

private:
    const QScopedPointer<class IDocumentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(IDocument)
    friend class IDocumentPrivate;
};

}

Q_DECLARE_INTERFACE(KDevelop::IDocument, "org.kdevelop.IDocument")

#endif

