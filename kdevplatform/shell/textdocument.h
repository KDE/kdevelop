/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEXTDOCUMENT_H
#define KDEVPLATFORM_TEXTDOCUMENT_H

#include <QWidget>

#include <sublime/view.h>

#include "partdocument.h"

#include "shellexport.h"

class QMenu;

namespace KTextEditor {
class View;
}

namespace KDevelop {

class TextViewPrivate;
class TextDocumentPrivate;

/**
Text document which represents KTextEditor documents.

Usually Kate documents are represented by this class but TextDocument is not
limited to Kate. Each conforming text editor will work.
*/
class KDEVPLATFORMSHELL_EXPORT TextDocument: public PartDocument {
    Q_OBJECT
public:
    TextDocument(const QUrl &url, ICore*, const QString& encoding );
    ~TextDocument() override;

    QWidget *createViewWidget(QWidget *parent = nullptr) override;
    KParts::Part *partForView(QWidget *view) const override;

    bool save(DocumentSaveMode mode = Default) override;
    DocumentState state() const override;

    KTextEditor::Cursor cursorPosition() const override;
    void setCursorPosition(const KTextEditor::Cursor &cursor) override;

    KTextEditor::Range textSelection() const override;
    void setTextSelection(const KTextEditor::Range &range) override;

    QString text(const KTextEditor::Range &range) const override;

    QString textLine() const override;
    QString textWord() const override;

    KTextEditor::Document* textDocument() const override;

    QString documentType() const override;

    KTextEditor::View* activeTextView() const override;

public Q_SLOTS:
    void reload() override;

protected:
    Sublime::View *newView(Sublime::Document *doc) override;

private:
    void newDocumentStatus(KTextEditor::Document*);
    void populateContextMenu(KTextEditor::View*, QMenu*);
    void unpopulateContextMenu();
    void textChanged(KTextEditor::Document*);
    void documentUrlChanged(KTextEditor::Document*);
    void slotDocumentLoaded();
    void documentSaved(KTextEditor::Document*,bool);
    void repositoryCheckFinished(bool);

private:
    const QScopedPointer<class TextDocumentPrivate> d_ptr;
    Q_DECLARE_PRIVATE(TextDocument)
    friend class TextDocumentPrivate;
};

class KDEVPLATFORMSHELL_EXPORT TextView : public Sublime::View
{
    Q_OBJECT
public:
    explicit TextView(TextDocument* doc);
    ~TextView() override;

    QWidget *createWidget(QWidget *parent = nullptr) override;

    KTextEditor::View *textView() const;

    QString viewStatus() const override;

    void readSessionConfig(KConfigGroup & config) override;
    void writeSessionConfig(KConfigGroup & config) override;

    void setInitialRange(const KTextEditor::Range& range);
    KTextEditor::Range initialRange() const;

private:
    void sendStatusChanged();

private:
    const QScopedPointer<class TextViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(TextView)
};

}


#endif

