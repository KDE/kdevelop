/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PARTDOCUMENT_H
#define KDEVPLATFORM_PARTDOCUMENT_H

#include <interfaces/idocument.h>
#include <sublime/urldocument.h>

#include "shellexport.h"

namespace KParts {
class Part;
}

namespace KDevelop {
class PartDocumentPrivate;

/**
The generic document which represents KParts.

This document is used by shell when more specific document classes
are incapable of loading the url.

This document loads one KPart (read-only or read-write) per view
and sets part widget to be a view widget.
*/
class KDEVPLATFORMSHELL_EXPORT PartDocument: public Sublime::UrlDocument, public KDevelop::IDocument {
    Q_OBJECT
    Q_INTERFACES(KDevelop::IDocument)

public:
    PartDocument(const QUrl &url, ICore* core, const QString& preferredPart = QString() );
    ~PartDocument() override;

    QUrl url() const override;
    void setUrl(const QUrl& newUrl);

    QWidget *createViewWidget(QWidget *parent = nullptr) override;
    KParts::Part *partForView(QWidget *view) const override;

    QMimeType mimeType() const override;
    QIcon icon() const override;
    KTextEditor::Document* textDocument() const override;
    bool save(DocumentSaveMode mode = Default) override;
    void reload() override;
    ///Closes and deletes the document. Asks the user before if needed.
    bool close(DocumentSaveMode mode = Default) override;
    bool isActive() const override;
    DocumentState state() const override;

    void setPrettyName(const QString& name) override;
    
    void activate(Sublime::View *activeView, KParts::MainWindow *mainWindow) override;

    KTextEditor::Cursor cursorPosition() const override;
    void setCursorPosition(const KTextEditor::Cursor &cursor) override;
    void setTextSelection(const KTextEditor::Range &range) override;

    //Overridden from Sublime::Document
    bool closeDocument(bool silent) override;
    bool askForCloseFeedback() override;
protected:
    /** Gives us access to the KParts */
    QMap<QWidget*, KParts::Part*> partForView() const;
    
    /** Lets us override the createViewWidget method safely */
    void addPartForView(QWidget* widget, KParts::Part* part);
    
private:
    const QScopedPointer<class PartDocumentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(PartDocument)
};

}

#endif

