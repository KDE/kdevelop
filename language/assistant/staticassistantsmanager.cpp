/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not^, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "staticassistantsmanager.h"
#include <interfaces/icodehighlighting.h>
#include "util/debug.h"

#include <QTimer>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/ilanguagecontroller.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>

#include <language/duchain/problem.h>

using namespace KDevelop;
using namespace KTextEditor;

struct StaticAssistantsManager::Private
{
    Private(StaticAssistantsManager* qq)
        : q(qq)
    { }

    void updateReady(const IndexedString& document, const KDevelop::ReferencedTopDUContext& topContext);
    void documentLoaded(KDevelop::IDocument*);
    void textInserted(KTextEditor::Document* document, const Cursor& cursor, const QString& text);
    void textRemoved(KTextEditor::Document* document, const Range& cursor, const QString& removedText);

    StaticAssistantsManager* q;

    QVector<StaticAssistant::Ptr> m_registeredAssistants;
};

StaticAssistantsManager::StaticAssistantsManager(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
    connect(KDevelop::ICore::self()->documentController(),
            &IDocumentController::documentLoaded,
            this, [&] (IDocument* document) { d->documentLoaded(document); });
    foreach (IDocument* document, ICore::self()->documentController()->openDocuments()) {
        d->documentLoaded(document);
    }
}

StaticAssistantsManager::~StaticAssistantsManager()
{
}

void StaticAssistantsManager::registerAssistant(const StaticAssistant::Ptr assistant)
{
    if (d->m_registeredAssistants.contains(assistant))
        return;

    d->m_registeredAssistants << assistant;
}

void StaticAssistantsManager::unregisterAssistant(const StaticAssistant::Ptr assistant)
{
    d->m_registeredAssistants.removeOne(assistant);
}

QVector<StaticAssistant::Ptr> StaticAssistantsManager::registeredAssistants() const
{
    return d->m_registeredAssistants;
}

void StaticAssistantsManager::Private::documentLoaded(IDocument* document)
{
    if (document->textDocument()) {
        auto doc = document->textDocument();
        connect(doc, &KTextEditor::Document::textInserted, q,
                [&] (KTextEditor::Document* doc, const Cursor& cursor, const QString& text) {
                    textInserted(doc, cursor, text);
                });
        connect(doc, &KTextEditor::Document::textRemoved, q,
                [&] (KTextEditor::Document* doc, const Range& range, const QString& removedText) {
                    textRemoved(doc, range, removedText);
                });
    }
}

void StaticAssistantsManager::Private::textInserted(Document* doc, const Cursor& cursor, const QString& text)
{
    Q_FOREACH ( auto assistant, m_registeredAssistants ) {
        auto range = Range(cursor, cursor+Cursor(0, text.size()));
        assistant->textChanged(doc, range, {});
    }
}

void StaticAssistantsManager::Private::textRemoved(Document* doc, const Range& range,
                                      const QString& removedText)
{
    Q_FOREACH ( auto assistant, m_registeredAssistants ) {
        assistant->textChanged(doc, range, removedText);
    }
}

void StaticAssistantsManager::notifyAssistants(const IndexedString& url, const KDevelop::ReferencedTopDUContext& context)
{
    Q_FOREACH ( auto assistant, d->m_registeredAssistants ) {
        assistant->updateReady(url, context);
    }
}

QVector<KDevelop::Problem::Ptr> KDevelop::StaticAssistantsManager::problemsForContext(const KDevelop::ReferencedTopDUContext& top)
{
    View* view = ICore::self()->documentController()->activeTextDocumentView();
    if (!view || !top || IndexedString(view->document()->url()) != top->url()) {
        return {};
    }

    auto doc = top->url();
    auto language = ICore::self()->languageController()->languagesForUrl(doc.toUrl()).value(0);
    if (!language) {
        return {};
    }

    auto ret = QVector<KDevelop::Problem::Ptr>();
    qCDebug(LANGUAGE) << "Trying to find assistants for language" << language->name();
    foreach (const auto& assistant, d->m_registeredAssistants) {
        if (assistant->supportedLanguage() != language)
            continue;

        if (assistant->isUseful()) {
            qDebug() << "assistant is now useful:" << assistant.data();

            auto p = new KDevelop::StaticAssistantProblem();
            auto range = assistant->displayRange();
            qDebug() << "range:" << range;
            p->setFinalLocation(DocumentRange(doc, range));
            p->setSource(KDevelop::IProblem::SemanticAnalysis);
            p->setSeverity(KDevelop::IProblem::Warning);
            p->setDescription(assistant->title());
            p->setSolutionAssistant(IAssistant::Ptr(assistant.data()));

            ret.append(KDevelop::Problem::Ptr(p));
        }
    }
    return ret;
}



#include "moc_staticassistantsmanager.cpp"
