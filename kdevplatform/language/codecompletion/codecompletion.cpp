/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecompletion.h"

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguagesupport.h>

#include "../duchain/duchain.h"
#include "../duchain/topducontext.h"
#include <debug.h>
#include "codecompletionmodel.h"
#include <interfaces/idocumentcontroller.h>

#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KTextEditor/CodeCompletionModel>

using namespace KTextEditor;
using namespace KDevelop;

CodeCompletion::CodeCompletion(QObject* parent, KTextEditor::CodeCompletionModel* aModel, const QString& language)
    : QObject(parent)
    , m_model(aModel)
    , m_language(language)
{
    auto* kdevModel = qobject_cast<KDevelop::CodeCompletionModel*>(aModel);
    if (kdevModel)
        kdevModel->initialize();
    connect(KDevelop::ICore::self()->documentController(), &IDocumentController::textDocumentCreated,
            this, &CodeCompletion::textDocumentCreated);
    connect(ICore::self()->documentController(), &IDocumentController::documentUrlChanged,
            this, &CodeCompletion::documentUrlChanged);
    aModel->setParent(this);

    // prevent deadlock
    QMetaObject::invokeMethod(this, "checkDocuments", Qt::QueuedConnection);
}

CodeCompletion::~CodeCompletion()
{
}

void CodeCompletion::checkDocuments()
{
    const auto documents = KDevelop::ICore::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* doc : documents) {
        if (doc->textDocument()) {
            checkDocument(doc->textDocument());
        }
    }
}

void CodeCompletion::viewCreated(KTextEditor::Document* document, KTextEditor::View* view)
{
    Q_UNUSED(document);
    Q_ASSERT(view);

    view->registerCompletionModel(m_model);
    qCDebug(LANGUAGE) << "Registered completion model";
    emit registeredToView(view);
}

void CodeCompletion::documentUrlChanged(KDevelop::IDocument* document)
{
    // The URL has changed (might have a different language now), so we re-register the document
    Document* textDocument = document->textDocument();

    if (textDocument) {
        checkDocument(textDocument);
    }
}

void CodeCompletion::textDocumentCreated(KDevelop::IDocument* document)
{
    Q_ASSERT(document->textDocument());
    checkDocument(document->textDocument());
}

void CodeCompletion::unregisterDocument(Document* textDocument)
{
    const auto views = textDocument->views();
    for (KTextEditor::View* view : views) {
        view->unregisterCompletionModel(m_model);
        emit unregisteredFromView(view);
    }

    disconnect(textDocument, &Document::viewCreated, this, &CodeCompletion::viewCreated);
}

void CodeCompletion::checkDocument(Document* textDocument)
{
    unregisterDocument(textDocument);

    const auto langs = ICore::self()->languageController()->languagesForUrl(textDocument->url());

    bool found = false;
    for (const auto lang : langs) {
        if (m_language == lang->name()) {
            found = true;
            break;
        }
    }

    if (!found && !m_language.isEmpty())
        return;

    const auto views = textDocument->views();
    for (KTextEditor::View* view : views) {
        viewCreated(textDocument, view);
    }

    connect(textDocument, &Document::viewCreated, this, &CodeCompletion::viewCreated);
}

#include "moc_codecompletion.cpp"
