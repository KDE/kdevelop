
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
#include <KXMLGUIClient>

#include <sublime/view.h>

#include "partdocument.h"

#include "shellexport.h"

namespace KTextEditor {
class View;
}

namespace KDevelop {

/**
Text document which represents KTextEditor documents.

Usually Kate documents are represented by this class but TextDocument is not
limited to Kate. Each conforming text editor will work.
*/
class KDEVPLATFORMSHELL_EXPORT TextDocument: public PartDocument {
    Q_OBJECT
public:
    TextDocument(const QUrl &url, ICore*, const QString& encoding );
    virtual ~TextDocument();

    virtual QWidget *createViewWidget(QWidget *parent = 0) override;
    virtual KParts::Part *partForView(QWidget *view) const override;
    virtual bool close(DocumentSaveMode mode = Default) override;

    virtual bool save(DocumentSaveMode mode = Default) override;
    virtual DocumentState state() const override;

    virtual KTextEditor::Cursor cursorPosition() const override;
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor) override;

    virtual KTextEditor::Range textSelection() const override;
    virtual void setTextSelection(const KTextEditor::Range &range) override;

    virtual QString textLine() const override;
    virtual QString textWord() const override;

    virtual bool isTextDocument() const override;
    virtual KTextEditor::Document* textDocument() const override;

    virtual QString documentType() const override;

    virtual QIcon defaultIcon() const override;

    virtual KTextEditor::View* activeTextView() const override;

public Q_SLOTS:
    virtual void reload() override;

protected:
    virtual Sublime::View *newView(Sublime::Document *doc) override;

private:
    Q_PRIVATE_SLOT(d, void newDocumentStatus(KTextEditor::Document*))
    Q_PRIVATE_SLOT(d, void populateContextMenu(KTextEditor::View*, QMenu*))
    Q_PRIVATE_SLOT(d, void textChanged(KTextEditor::Document*))
    Q_PRIVATE_SLOT(d, void modifiedOnDisk(KTextEditor::Document *, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason))
    Q_PRIVATE_SLOT(d, void documentUrlChanged(KTextEditor::Document*))
    Q_PRIVATE_SLOT(d, void slotDocumentLoaded())
    Q_PRIVATE_SLOT(d, void documentSaved(KTextEditor::Document*,bool))
    Q_PRIVATE_SLOT(d, void saveSessionConfig());
    Q_PRIVATE_SLOT(d, void repositoryCheckFinished(bool));

    struct TextDocumentPrivate * const d;
    friend struct TextDocumentPrivate;
};

class KDEVPLATFORMSHELL_EXPORT TextView : public Sublime::View
{
    Q_OBJECT
public:
    TextView(TextDocument* doc);
    virtual ~TextView();

    QWidget *createWidget(QWidget *parent = 0) override;

    KTextEditor::View *textView() const;

    virtual QString viewStatus() const override;
    virtual QString viewState() const override;
    virtual void setState(const QString& state) override;

    void setInitialRange(const KTextEditor::Range& range);
    KTextEditor::Range initialRange() const;

private:
    struct TextViewPrivate* const d;

    Q_PRIVATE_SLOT(d, void sendStatusChanged());
};

}


#endif

