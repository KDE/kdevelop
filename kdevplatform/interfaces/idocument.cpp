/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idocument.h"

#include "icore.h"
#include "idocumentcontroller.h"

namespace KDevelop {

class IDocumentPrivate
{
public:
    inline explicit IDocumentPrivate(KDevelop::ICore *core)
        : m_core(core), scriptWrapper(nullptr)
    {}

    KDevelop::ICore* m_core;
    QObject *scriptWrapper;
    QString m_prettyName;

    /* Internal access to the wrapper script object */
    static inline QObject *&getWrapper(IDocument *doc)
    {
        return doc->d_func()->scriptWrapper;
    }
};

/* This allows the scripting backend to register the scripting
   wrapper. Not beautiful, but makes sure it doesn't expand to much code.
*/
QObject *&getWrapper(IDocument *doc)
{
    return IDocumentPrivate::getWrapper(doc);
}

IDocument::IDocument( KDevelop::ICore* core )
    : d_ptr(new IDocumentPrivate(core))
{
}

IDocument::~IDocument()
{
    Q_D(IDocument);

    delete d->scriptWrapper;
}

KDevelop::ICore* IDocument::core()
{
    Q_D(IDocument);

    return d->m_core;
}

void IDocument::notifySaved()
{
    emit core()->documentController()->documentSaved(this);
}

void IDocument::notifyStateChanged()
{
    emit core()->documentController()->documentStateChanged(this);
}

void IDocument::notifyActivated()
{
    // DocumentController::cleanup() closes open documents one by one on shutdown. When the active document is closed,
    // another still-open document is activated and this function is called. Emitting the documentActivated signal then
    // results in useless work, such as UI updates and parsing activated documents with higher priority. It also causes
    // undesirable side effects, for example, a wrong ProjectTreeView row is selected, then saved and restored on next
    // KDevelop start. Don't emit the signal while shutting down to speed up shutdown and prevent bugs. cleanup() closes
    // documents without checking whether they should be saved (Discard mode), so temporary UI inconsistencies caused by
    // skipped signals should be brief and harmless.
    if (!core()->shuttingDown()) {
        emit core()->documentController()->documentActivated(this);
    }
}

void IDocument::notifyContentChanged()
{
    emit core()->documentController()->documentContentChanged(this);
}

bool IDocument::isTextDocument() const
{
    return false;
}

void IDocument::notifyTextDocumentCreated()
{
    emit core()->documentController()->textDocumentCreated(this);
}

KTextEditor::Range IDocument::textSelection() const
{
    return KTextEditor::Range::invalid();
}

QString IDocument::textLine() const
{
    return QString();
}

QString IDocument::textWord() const
{
    return QString();
}

QString IDocument::prettyName() const
{
    Q_D(const IDocument);

    return d->m_prettyName;
}

void IDocument::setPrettyName(const QString& name)
{
    Q_D(IDocument);

    d->m_prettyName = name;
}

void IDocument::notifyUrlChanged()
{
    emit core()->documentController()->documentUrlChanged(this);
}

void IDocument::notifyLoaded()
{
    emit core()->documentController()->documentLoadedPrepare(this);
    emit core()->documentController()->documentLoaded(this);
}

KTextEditor::View* IDocument::activeTextView() const
{
    return nullptr;
}

QString KDevelop::IDocument::text(const KTextEditor::Range& range) const
{
    Q_UNUSED(range);
    return {};
}

}

