
/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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

    bool isTextDocument() const override;
    KTextEditor::Document* textDocument() const override;

    QString documentType() const override;

    QIcon defaultIcon() const override;

    KTextEditor::View* activeTextView() const override;

public Q_SLOTS:
    void reload() override;

protected:
    Sublime::View *newView(Sublime::Document *doc) override;

private:
    Q_PRIVATE_SLOT(d_func(), void saveSessionConfig())
    Q_PRIVATE_SLOT(d_func(), void modifiedOnDisk(KTextEditor::Document *, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason))

    void newDocumentStatus(KTextEditor::Document*);
    void populateContextMenu(KTextEditor::View*, QMenu*);
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

