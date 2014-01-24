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
#include <KDE/KXMLGUIClient>

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
    TextDocument(const KUrl &url, ICore*, const QString& encoding );
    virtual ~TextDocument();

    virtual QWidget *createViewWidget(QWidget *parent = 0);
    virtual KParts::Part *partForView(QWidget *view) const;
    virtual bool close(DocumentSaveMode mode = Default);

    virtual bool save(DocumentSaveMode mode = Default);
    virtual DocumentState state() const;

    virtual KTextEditor::Cursor cursorPosition() const;
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor);

    virtual KTextEditor::Range textSelection() const;
    virtual void setTextSelection(const KTextEditor::Range &range);

    virtual QString textLine() const;
    virtual QString textWord() const;

    virtual bool isTextDocument() const;
    virtual KTextEditor::Document* textDocument() const;

    virtual QString documentType() const;

    virtual QIcon defaultIcon() const;

    virtual KTextEditor::View* activeTextView() const override;

public Q_SLOTS:
    virtual void reload();

protected:
    virtual Sublime::View *newView(Sublime::Document *doc);

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

    QWidget *createWidget(QWidget *parent = 0);

    KTextEditor::View *textView() const;

    virtual QString viewStatus() const;

    virtual QString viewState() const;
    virtual void setState(const QString& state);

    void setInitialRange(const KTextEditor::Range& range);
    KTextEditor::Range initialRange() const;

private Q_SLOTS:
    void sendStatusChanged();
    void editorDestroyed(QObject* obj);

private:
    class TextViewPrivate* const d;
};

class KDEVPLATFORMSHELL_EXPORT TextEditorWidget : public QWidget, public KXMLGUIClient
{
    Q_OBJECT
public:
    TextEditorWidget(const KDevelop::TextView* view, QWidget* parent = 0);
    virtual ~TextEditorWidget();

    KTextEditor::View* editorView();

    QString status() const;
    bool isInitialized() const;
    virtual void showEvent(QShowEvent* event);

Q_SIGNALS:
    void statusChanged();

public Q_SLOTS:
    void initialize();
    void viewStatusChanged(KTextEditor::View*, const KTextEditor::Cursor& newPosition);

private:
    void setEditorView(KTextEditor::View* view);
    class TextEditorWidgetPrivate* const d;

    Q_PRIVATE_SLOT(d, void viewEditModeChanged(KTextEditor::View*, KTextEditor::View::EditMode));

};

}


#endif

