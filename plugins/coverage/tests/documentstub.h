/* KDevelop coverage plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef TESTFAKES_DOCUMENTSTUB_H
#define TESTFAKES_DOCUMENTSTUB_H

#include "interfaces/idocument.h"

namespace KDevelop
{
class ICore;
}

namespace TestStubs
{

class KDevDocument : public KDevelop::IDocument
{
public:
    KDevDocument(KDevelop::ICore* c) : IDocument(c) {
        m_isTextDocument = true;
        m_isActive = true;
        m_save = true;
        m_partForView = 0;
        m_state = IDocument::Clean;
        m_textDocument = 0;
    }
    virtual ~KDevDocument() {}

    virtual KUrl url() const {
        return m_url;
    }
    virtual KMimeType::Ptr mimeType() const {
        return KSharedPtr<KMimeType>();
    }
    virtual KParts::Part* partForView(QWidget *view) const {
        return m_partForView;
    }
    virtual bool isTextDocument() const {
        return m_isTextDocument;
    }
    virtual KTextEditor::Document* textDocument() const {
        return m_textDocument;
    }
    virtual bool save(DocumentSaveMode mode = Default) {
        return m_save;
    }
    virtual void reload() {}
    virtual bool close(DocumentSaveMode mode = Default) {
        return true;
    }
    virtual bool isActive() const {
        return m_isActive;
    }
    virtual DocumentState state() const {
        return m_state;
    }
    virtual KTextEditor::Cursor cursorPosition() const {
        return KTextEditor::Cursor();
    }
    virtual void setCursorPosition(const KTextEditor::Cursor &cursor) {
    }
    virtual KTextEditor::Range textSelection() const {
        return KTextEditor::Range();
    }
    virtual void setTextSelection(const KTextEditor::Range &/*range*/) {
    }
    virtual void activate(Sublime::View *activeView, KParts::MainWindow *mainWindow) {
    }

    // inject whatever it is you need
    KUrl m_url;
    bool m_isTextDocument;
    bool m_isActive;
    bool m_save;
    KParts::Part* m_partForView;
    DocumentState m_state;
    KTextEditor::Document* m_textDocument;
    KTextEditor::Cursor m_cursorPosition;
    KTextEditor::Range m_textSelection;
};

}

#include <ktexteditor/document.h>

namespace TestStubs
{

class TextDocument : public KTextEditor::Document
{
//    Q_OBJECT

public:
    TextDocument(QObject *parent = 0) :
        KTextEditor::Document(parent),
        m_createView(0) {}
    virtual ~TextDocument() {}

    QList<KTextEditor::View*> m_views;
    KTextEditor::Editor* m_editor;
    KTextEditor::View* m_createView;
    KTextEditor::View* m_activeView;
    QString m_encoding;
    QString m_documentName;

public:
    virtual KTextEditor::Editor *editor() { return m_editor; }
    virtual KTextEditor::View *createView(QWidget *parent) {
        emit viewCreated(this, m_createView);
        //m_views.clear();
        m_views << m_createView;
        return m_createView;
    }

    virtual KTextEditor::View* activeView() const { return m_activeView; }
    virtual const QList<KTextEditor::View*> &views() const { return m_views; }
    virtual const QString &documentName() const { return m_documentName; }
    virtual QString mimeType() { return ""; }
    virtual bool setEncoding(const QString &encoding) { return true; }
    virtual const QString &encoding() const { return m_encoding; }
    virtual bool documentReload() { return true; }
    virtual bool documentSave() { return true; }
    virtual bool documentSaveAs() { return true; }
    virtual bool startEditing() { return true; }
    virtual bool endEditing() { return true; }
    virtual QString text() const { return ""; }
    virtual QString text(const KTextEditor::Range& /*range*/, bool block = false) const { return ""; }
    virtual QChar character(const KTextEditor::Cursor& position) const { return ' '; }
    virtual QStringList textLines(const KTextEditor::Range& /*range*/, bool block = false) const { return QStringList(); }
    virtual QString line(int /*line*/) const { return QString(); }
    virtual int /*line*/s() const { return 0; }
    virtual KTextEditor::Cursor documentEnd() const { return KTextEditor::Cursor(); }
    virtual int totalCharacters() const { return 0; }
    virtual bool isEmpty() const { return true; }
    virtual int /*line*/Length(int /*line*/) const { return 0; }
    virtual bool setText(const QString &/*text*/) { return true; }
    virtual bool setText(const QStringList &text) { return true; }
    virtual bool clear() { return true; }
    virtual bool insertText(const KTextEditor::Cursor &/*position*/, const QString &/*text*/, bool block = false) { return true; }
    virtual bool insertText(const KTextEditor::Cursor &/*position*/, const QStringList &text, bool block = false) { return true; }
    virtual bool replaceText(const KTextEditor::Range &/*range*/, const QString &/*text*/, bool block = false) { return true; }
    virtual bool replaceText(const KTextEditor::Range &/*range*/, const QStringList &/*text*/, bool block = false) { return true; }
    virtual bool removeText(const KTextEditor::Range &/*range*/, bool block = false) { return true; }
    virtual bool insertLine(int /*line*/, const QString &/*text*/) { return true; }
    virtual bool insertLines(int /*line*/, const QStringList &/*text*/) { return true; }
    virtual bool removeLine(int /*line*/) { return true; }
    virtual QString mode() const { return ""; }
    virtual QString highlightingMode() const { return ""; }
    virtual QStringList modes() const { return QStringList(); }
    virtual QStringList highlightingModes() const { return QStringList(); }
    virtual bool setMode(const QString &/*name*/) { return true; }
    virtual bool setHighlightingMode(const QString &/*name*/) { return true; }
    virtual QString highlightingModeSection(int /*index*/) const { return ""; }
    virtual QString modeSection(int /*index*/) const { return ""; }

    virtual bool openFile() { return true; }
    virtual bool saveFile() { return true; }
    virtual int lines() const { return 0; }
    virtual int lineLength(int) const { return 0; }


#if 0
Q_SIGNALS:
    void viewCreated(KTextEditor::Document *document, KTextEditor::View *view);
    void documentNameChanged(KTextEditor::Document *document);
    void documentUrlChanged(KTextEditor::Document *document);
    void modifiedChanged(KTextEditor::Document *document);
    void textChanged(KTextEditor::Document *document);
    void textInserted(KTextEditor::Document *document, const KTextEditor::Range& /*range*/);
    void textRemoved(KTextEditor::Document *document, const KTextEditor::Range& /*range*/);
    void textChanged(KTextEditor::Document *document, const KTextEditor::Range& oldRange,
                     const KTextEditor::Range& newRange);
    void aboutToClose(KTextEditor::Document *document);
    void aboutToReload(KTextEditor::Document *document);
#endif
};

}


#endif // TESTFAKES_DOCUMENTSTUB_H
